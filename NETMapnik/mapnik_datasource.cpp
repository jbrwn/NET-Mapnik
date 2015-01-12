#include "stdafx.h"
#include "mapnik_datasource.h"
#include "mapnik_featureset.h"
#include "mapnik_value_converter.h"
#include "NET_box_utils.h"
#include "ds_emitter.h"

// mapnik
#include <mapnik\attribute_descriptor.hpp>
#include <mapnik\box2d.hpp>
#include <mapnik\datasource.hpp>
#include <mapnik\datasource_cache.hpp>
#include <mapnik\feature_layer_desc.hpp>
#include <mapnik\params.hpp>
#include <mapnik\query.hpp>

// microsoft
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	Datasource::Datasource(System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		mapnik::parameters params;
		for each (System::Collections::Generic::KeyValuePair<System::String^, System::Object^>^ kvp in options)
		{
			std::string key = msclr::interop::marshal_as<std::string>(kvp->Key);
			std::string value = msclr::interop::marshal_as<std::string>(System::Convert::ToString(kvp->Value));
			params[key] = value;
		}

		mapnik::datasource_ptr ds;
		try
		{
			ds = mapnik::datasource_cache::instance().create(params);
		}
		catch (std::exception const& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}

		if (ds)
		{
			if (ds->type() == mapnik::datasource::Raster)
			{
				_type = "raster";
			}
			else
			{
				_type = "vector";
			}
			_ds = new mapnik::datasource_ptr(ds);
		}
	}

	Datasource::Datasource(mapnik::datasource_ptr ds)
	{
		_ds = new mapnik::datasource_ptr(ds);
	}
	
	Datasource::~Datasource()
	{
		if (_ds != NULL)
			delete _ds;
	}

	mapnik::datasource_ptr Datasource::NativeObject()
	{
		return *_ds;
	}

	System::String^ Datasource::Type::get()
	{
		return _type;
	}

	System::Collections::Generic::IDictionary<System::String^, System::Object^>^ Datasource::Paramemters()
	{
		mapnik::parameters const& params = (*_ds)->params();
		mapnik::parameters::const_iterator it = params.begin();
		mapnik::parameters::const_iterator end = params.end();
		System::Collections::Generic::Dictionary<System::String^, System::Object^>^ paramsDictionary = gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>();
		for (; it != end; ++it)
		{
			System::String^ key = msclr::interop::marshal_as<System::String^>(it->first);
			params_to_object serializer(paramsDictionary, key);
			mapnik::util::apply_visitor(serializer, it->second);
		}
		return paramsDictionary;
	}

	array<System::Double>^ Datasource::Extent()
	{
		mapnik::box2d<double> e;
		try
		{
			e = (*_ds)->envelope();
		}
		catch (std::exception const& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}

		return Box2DToArray(e);
	}

	Featureset^ Datasource::Featureset()
	{
		mapnik::featureset_ptr fs;
		try
		{
			mapnik::query q((*_ds)->envelope());
			mapnik::layer_descriptor ld = (*_ds)->get_descriptor();
			std::vector<mapnik::attribute_descriptor> const& desc = ld.get_descriptors();
			std::vector<mapnik::attribute_descriptor>::const_iterator itr = desc.begin();
			std::vector<mapnik::attribute_descriptor>::const_iterator end = desc.end();
			while (itr != end)
			{
				q.add_property_name(itr->get_name());
				++itr;
			}

			fs = (*_ds)->features(q);
		}
		catch (std::exception const& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}

		if (fs)
		{
			return gcnew NETMapnik::Featureset(fs);
		}

		return nullptr;
	}

	System::Collections::Generic::IDictionary<System::String^, System::Object^>^ Datasource::Describe()
	{
		System::Collections::Generic::Dictionary<System::String^, System::Object^>^ description = gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>();
		try {
			describe_datasource(description, *_ds);
		}
		catch (std::exception const& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
		return description;
	}
}