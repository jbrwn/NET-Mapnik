#include "stdafx.h"
#include "mapnik_vector_tile.h"


namespace NETMapnik
{
	VectorTile::VectorTile()
	{
		_tile = new mapnik::vector::tile();

	}

	VectorTile::~VectorTile()
	{
		if (_tile != NULL)
		{
			delete _tile;
		}
	}

	mapnik::vector::tile *VectorTile::NativeObject()
	{
		return _tile;
	}

	array<System::Byte>^ VectorTile::GetBytes()
	{
		
		std::string s;
		_tile->SerializeToString(&s);
		array<System::Byte>^ data = gcnew array<System::Byte>(s.size());
		System::Runtime::InteropServices::Marshal::Copy(System::IntPtr(&s[0]), data, 0, s.size());
		return data;
	}

}