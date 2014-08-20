#pragma once

#include "vector_tile.pb.h"
#include "mapnik_map.h"
#include "mapnik_image.h"

namespace NETMapnik
{
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

}
