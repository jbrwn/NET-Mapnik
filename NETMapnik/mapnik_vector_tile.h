#pragma once

#include "vector_tile.pb.h"
#include <mapnik\request.hpp>
#include <mapnik\projection.hpp>
#include <mapnik\layer.hpp>

namespace NETMapnik
{
	//Forward declare
	ref class Image;
	ref class Map;

	public ref class VectorTile
	{
	public:
		VectorTile();
		~VectorTile();
		array<System::Byte>^ GetBytes();
		void SetBytes(array<System::Byte>^ data);
		void Render(Map^ map, Image^ image);
	internal:
		mapnik::vector::tile *NativeObject();

	private:
		mapnik::vector::tile* _tile;

	};

	template <typename Renderer>
	void process_layers(Renderer & ren,
		mapnik::request const& m_req,
		mapnik::projection const& map_proj,
		std::vector<mapnik::layer> const& layers,
		double scale_denom,
		mapnik::vector::tile const& tiledata,

		mapnik::box2d<double> const& map_extent);

}
