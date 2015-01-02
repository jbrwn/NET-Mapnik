#include "stdafx.h"
#include "mapnik_geometry.h"
#include "mapnik_projection.h"
#include "proj_transform_adapter.h"
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

	System::String ^ Geometry::ToJSON(ProjTransform ^ transform)
	{
		std::string json;
		mapnik::proj_transform const& prj_trans = *transform->NativeObject();
		NETMapnik::proj_transform_container projected_paths;
		for (auto & geom : (*_geom)->paths())
		{
			projected_paths.push_back(new NETMapnik::proj_transform_path_type(geom, prj_trans));
		}
		using sink_type = std::back_insert_iterator<std::string>;
		static const mapnik::json::multi_geometry_generator_grammar<sink_type, NETMapnik::proj_transform_container> proj_grammar;
		sink_type sink(json);
		if (!boost::spirit::karma::generate(sink, proj_grammar, projected_paths))
		{
			throw gcnew System::Exception("Failed to generate GeoJSON");
		}
		return msclr::interop::marshal_as<System::String^>(json);
	}
}
