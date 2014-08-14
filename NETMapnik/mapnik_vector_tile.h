#include "vector_tile.pb.h"

#pragma once
namespace NETMapnik
{
	public ref class VectorTile
	{
	public:
		VectorTile();
		~VectorTile();
		array<System::Byte>^ GetBytes();
	internal:
		mapnik::vector::tile *NativeObject();

	private:
		mapnik::vector::tile* _tile;

	};

}
