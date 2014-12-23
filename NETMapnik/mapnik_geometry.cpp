#include "stdafx.h"
#include "mapnik_geometry.h"
#include "NET_box_utils.h"

// mapnik
#include <mapnik\feature.hpp>
#include <mapnik\util\geometry_to_geojson.hpp>
#include <mapnik\util\geometry_to_wkt.hpp>
#include <mapnik\util\geometry_to_wkb.hpp>

// microsoft
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	Geometry::Geometry(mapnik::feature_ptr f)
	{
		_geom = new mapnik::feature_ptr(f);
	}

	Geometry::~Geometry()
	{
		if (_geom != NULL)
			delete _geom;
	}

	array<System::Double>^ Geometry::Extent()
	{
		return Box2DToArray((*_geom)->envelope());
	}

	System::String^ Geometry::ToWKT()
	{
		std::string wkt;
		if (!mapnik::util::to_wkt(wkt, (*_geom)->paths()))
		{
			throw gcnew System::Exception("Failed to generate WKT");
		}
		return msclr::interop::marshal_as<System::String^>(wkt);
	}

	array<System::Byte>^ Geometry::ToWKB()
	{
		mapnik::util::wkb_buffer_ptr wkb = mapnik::util::to_wkb((*_geom)->paths(), mapnik::util::wkbNDR);
		array<System::Byte>^ data = gcnew array<System::Byte>(wkb->size());
		System::Runtime::InteropServices::Marshal::Copy(System::IntPtr(&wkb->buffer()[0]), data, 0, wkb->size());
		return data;
	}

	System::String^ Geometry::ToJSON()
	{
		std::string json;
		if (!mapnik::util::to_geojson(json, (*_geom)->paths()))
		{
			throw gcnew System::Exception("Failed to generate GeoJSON");
		}
		return msclr::interop::marshal_as<System::String^>(json);
	}
}
