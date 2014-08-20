#include "stdafx.h"
#include "mapnik_vector_tile.h"

#include <mapnik\map.hpp>
#include <mapnik\graphics.hpp>
#include <mapnik\request.hpp>
#include <mapnik\agg_renderer.hpp>

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

	void VectorTile::SetBytes(array<System::Byte>^ data)
	{
		pin_ptr<unsigned char> pData = &data[0];
		std::string s = std::string(reinterpret_cast<char*>(pData), data->Length);
		_tile->ParseFromString(s);
	}

	void VectorTile::Render(Map^ map, Image^ image)
	{
		//Unwrap native objects
		mapnik::Map *m = map->NativeObject();
		mapnik::image_32 *i = image->NativeObject();

		mapnik::request m_req(m->width(), m->height(),m->get_current_extent());
		mapnik::agg_renderer<mapnik::image_32> ren(*m, m_req, *i, 1.0, 0U, 0U);
		ren.apply(0);

	}
}