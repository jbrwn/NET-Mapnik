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
		VectorTile(System::Int32 Z, System::Int32 X, System::Int32 Y, System::UInt32 Width, System::UInt32 Height);
		~VectorTile();
		array<System::Byte>^ GetBytes();
		void SetBytes(array<System::Byte>^ data);
		void Composite(System::Collections::Generic::IEnumerable<VectorTile^>^ vTiles);
		void Render(Map^ map, Image^ image);

	internal:
		mapnik::vector::tile *NativeObject();

	private:
		mapnik::vector::tile* _tile;
		int _z;
		int _x;
		int _y;
		unsigned _width;
		unsigned _height;

	};

	template <typename Renderer>
	void process_layers(Renderer & ren,
		mapnik::request const& m_req,
		mapnik::projection const& map_proj,
		std::vector<mapnik::layer> const& layers,
		double scale_denom,
		mapnik::vector::tile const& tiledata,
		int z,
		int x,
		int y,
		unsigned tileSize,
		mapnik::box2d<double> const& map_extent);

}
