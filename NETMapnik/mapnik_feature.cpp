#include "stdafx.h"
#include "mapnik_feature.h"
#include "utils.h"
#include "mapnik_geometry.h"

#include <memory>

// mapnik
#include <mapnik\feature_factory.hpp>
#include <mapnik\json\feature_parser.hpp>
#include <mapnik\value_types.hpp>
#include <mapnik\util\feature_to_geojson.hpp>

// microsoft
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	Feature::Feature(mapnik::feature_ptr feature)
	{
		_feature = new mapnik::feature_ptr(feature);

	}

	Feature::~Feature()
	{
		if (_feature != NULL)
			delete _feature;
	}

	System::Int64 Feature::Id()
	{
		return (*_feature)->id();
	}

	array<System::Double>^ Feature::Extent()
	{
		return Box2DToArray((*_feature)->envelope());
	}

	System::Collections::Generic::IDictionary<System::String^, System::Object^>^ Feature::Attributes()
	{
		System::Collections::Generic::IDictionary<System::String^, System::Object^>^ feat = gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>();

		mapnik::feature_ptr feature = *_feature;
		mapnik::feature_impl::iterator itr = feature->begin();
		mapnik::feature_impl::iterator end = feature->end();
		for (; itr != end; ++itr)
		{
			System::String^ key = msclr::interop::marshal_as<System::String^>(std::get<0>(*itr));
			params_to_object serializer(feat, key);
			mapnik::util::apply_visitor(serializer, std::get<1>(*itr));
		}
		return feat;

	}

	Geometry^ Feature::Geometry()
	{
		return gcnew NETMapnik::Geometry(*_feature);
	}

	System::String^ Feature::ToJSON()
	{
		std::string json;
		if (!mapnik::util::to_geojson(json,*(*_feature)))
		{
			throw gcnew System::Exception("Failed to generate GeoJSON");
		}
		return msclr::interop::marshal_as<System::String^>(json);
	}

	Feature^ Feature::FromJSON(System::String^ json)
	{
		std::string unmanagedJson = msclr::interop::marshal_as<std::string>(json);
		mapnik::feature_ptr f(mapnik::feature_factory::create(std::make_shared<mapnik::context_type>(), 1));
		if (!mapnik::json::from_geojson(unmanagedJson, *f))
		{
			throw gcnew System::Exception("Failed to parse geojson feature");
		}
		return gcnew Feature(f);
	}

}