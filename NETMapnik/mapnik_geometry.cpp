#include "stdafx.h"
#include "mapnik_geometry.h"
#include "mapnik_projection.h"
#include "utils.h"

// mapnik
#include <mapnik\datasource.hpp>
#include <mapnik\geometry_reprojection.hpp>
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
		if (!mapnik::util::to_wkt(wkt, (*_geom)->get_geometry()))
		{
			throw gcnew System::Exception("Failed to generate WKT");
		}
		return msclr::interop::marshal_as<System::String^>(wkt);
	}

	array<System::Byte>^ Geometry::ToWKB()
	{
		mapnik::util::wkb_buffer_ptr wkb = mapnik::util::to_wkb((*_geom)->get_geometry(), mapnik::wkbNDR);
		if (!wkb)
		{
			throw gcnew System::Exception("Failed to generate WKB - geometry likely null");
		}
		array<System::Byte>^ data = gcnew array<System::Byte>(wkb->size());
		System::Runtime::InteropServices::Marshal::Copy(System::IntPtr(&wkb->buffer()[0]), data, 0, wkb->size());
		return data;
	}

	bool to_geojson_projected(std::string & json,
		mapnik::geometry::geometry<double> const& geom,
		mapnik::proj_transform const& prj_trans)
	{
		unsigned int n_err = 0;
		mapnik::geometry::geometry<double> projected_geom = mapnik::geometry::reproject_copy(geom, prj_trans, n_err);
		if (n_err > 0) return false;
		return mapnik::util::to_geojson(json, projected_geom);
	}

	System::String^ Geometry::ToJSON()
	{
		std::string json;
		if (!mapnik::util::to_geojson(json, (*_geom)->get_geometry()))
		{
			throw gcnew System::Exception("Failed to generate GeoJSON");
		}
		return msclr::interop::marshal_as<System::String^>(json);
	}

	System::String ^ Geometry::ToJSON(ProjTransform ^ transform)
	{
		std::string json;
		mapnik::proj_transform const& prj_trans = *transform->NativeObject();
		mapnik::geometry::geometry<double> const& geom = (*_geom)->get_geometry();
		if (!to_geojson_projected(json, geom, prj_trans))
		{
			throw gcnew System::Exception("Failed to generate GeoJSON");
		}
		return msclr::interop::marshal_as<System::String^>(json);
	}
}
