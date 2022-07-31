#pragma once
#include <xmath.h>
#include <cell.hpp>
#include <utils.hpp>

using namespace xmath;

namespace nj
{

struct state
{
	std::vector<std::vector<nj::cell>> cells;
	std::vector<vec<2, unsigned>> active_cells;

	g::game::fps_camera camera;
	g::game::sdf terrain;
	std::default_random_engine rng;

	float t = 0;

	state(const size_t w, const size_t d)
	{
		std::vector<int8_t> v[3];

		{ // generate some entropy for use in the perlin noise functions
			std::default_random_engine generator;
			std::uniform_int_distribution<int> distribution(-127, 128);
			for (unsigned i = 2048; i--;)
			{
				v[0].push_back(distribution(generator));
				v[1].push_back(distribution(generator));
				v[2].push_back(distribution(generator));
			}
		}

		terrain = [v](const vec<3>& p) -> float
		{
			// TODO: make ground dip outside a set radius
			auto d = p[1]; // flat plane for now

			// d = p[0];

			d += g::gfx::noise::perlin(p, v[2]) * 0.5f;
			d += g::gfx::noise::perlin(p * 0.065, v[0]);
			d += std::min<float>(0, g::gfx::noise::perlin(p * 0.0234, v[1]) * 40);
			d += g::gfx::noise::perlin(p * 0.0123, v[2]) * 80;
			return d;
		};


		for (unsigned yi = 0; yi < d; yi++)
		{
			cells.push_back({});
			cells.back().resize(w);
		}

		auto block_area = 400;
		unsigned block_side = sqrt(block_area);

		auto blocks_r = ceil(depth() / block_side);
		auto blocks_c = ceil(width() / block_side);

		for (unsigned br = 0; br < blocks_r; br++)
		{
			for (unsigned bc = 0; bc < blocks_c; bc++)
			{
				auto r = br * block_side, c = bc * block_side;

				for (unsigned ri = r; ri < std::min<unsigned>(depth(), r + block_side); ri++)
				{
					for (unsigned ci = c; ci < std::min<unsigned>(width(), c + block_side); ci++)
					{
						cells[ri][ci].elevation += terrain(vec<3>{(float)ci, cells[ri][ci].elevation, (float)ri});;
						cells[ri][ci].density = 0.5;

						assert(isfinite<float>(cells[ri][ci].elevation));

						if (cells[ri][ci].elevation >= 4)
						{
							active_cells.push_back({ ri, ci });
						}
					}
				}
			}
		}

		//for (unsigned r = 0; r < depth(); r++)
		//{
		//	for (unsigned c = 0; c < width(); c++)
		//	{
		//		cells[r][c].elevation = terrain(vec<3>{(float)c, 0, (float)r});

		//		if (cells[r][c].elevation >= 4)
		//		{
		//			active_cells.push_back({ r, c });
		//		}
		//	}
		//}

		std::cerr << "Cells that can host plants: " << active_cells.size() << std::endl;
	}

	inline size_t width() const { return cells[0].size(); }
	inline size_t depth() const { return cells.size(); }
};

} // namespace game