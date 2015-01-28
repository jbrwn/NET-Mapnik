#include "stdafx.h"
#include "mapnik_memory_datasource.h"
#include "mapnik_featureset.h"
#include "utils.h"
#include "ds_emitter.h"

#include <memory>

// mapnik
#include <mapnik\version.hpp>
#include <mapnik\unicode.hpp>
#include <mapnik\feature_factory.hpp>
#include <mapnik\memory_datasource.hpp>
#include <mapnik\value_types.hpp>

// microsoft
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	MemoryDatasource::MemoryDatasource(System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		mapnik::parameters params;
		for each (System::Collections::Generic::KeyValuePair<System::String^, System::Object^>^ kvp in options)
		{
			std::string key = msclr::interop::marshal_as<std::string>(kvp->Key);
			std::string value = msclr::interop::marshal_as<std::string>(System::Convert::ToString(kvp->Value));
			params[key] = value;
		}
		params["type"] = "memory";

		_ds = new mapnik::datasource_ptr(std::make_shared<mapnik::memory_datasource>(params));
		_feature_id = 1;
		_tr = new mapnik::transcoder("utf8");
	}

	MemoryDatasource::MemoryDatasource(mapnik::datasource_ptr ds)
	{
		_ds = new mapnik::datasource_ptr(ds);
		_feature_id = 1;
		_tr = new mapnik::transcoder("utf8");
	}

	MemoryDatasource::~MemoryDatasource()
	{
		if (_ds != NULL)
			delete _ds;
		if (_tr != NULL)
			delete _tr;
	}

	mapnik::datasource_ptr MemoryDatasource::NativeObject()
	{
		return *_ds;
	}

	System::Collections::Generic::IDictionary<System::String^, System::Object^>^ MemoryDatasource::Paramemters()
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

	System::Collections::Generic::IDictionary<System::String^, System::Object^>^ MemoryDatasource::Describe()
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

	Featureset^ MemoryDatasource::Featureset()
	{
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
			mapnik::featureset_ptr fs = (*_ds)->features(q);
			if (fs)
			{
				return gcnew NETMapnik::Featureset(fs);
			}
		}
		catch (std::exception const& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
		return nullptr;
	}

	void MemoryDatasource::Add(double x, double y)
	{
		Add(x, y, nullptr);
	}

	void MemoryDatasource::Add(double x, double y, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ properties)
	{
		mapnik::geometry_type * pt = new mapnik::geometry_type(mapnik::geometry_type::types::Point);
		pt->move_to(x, y);
		mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
		mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx, _feature_id));
		++_feature_id;
		feature->add_geometry(pt);
		if (properties != nullptr)
		{
			for each (System::Collections::Generic::KeyValuePair<System::String^, System::Object^>^ kvp in properties)
			{
				std::string key = msclr::interop::marshal_as<std::string>(kvp->Key);
				System::Object^ managedValue = kvp->Value;
				if (managedValue->GetType() == System::String::typeid)
				{
					std::string s = msclr::interop::marshal_as<std::string>(safe_cast<System::String^>(managedValue));
					mapnik::value_unicode_string ustr = _tr->transcode(s.c_str());
					feature->put_new(key, ustr);
				}
				else if (managedValue->GetType() == System::Int32::typeid)
				{
					int i = safe_cast<int>(managedValue);
					feature->put_new(key,static_cast<mapnik::value_integer>(i));
				}
				else if (managedValue->GetType() == System::Double::typeid)
				{
					double d = safe_cast<double>(managedValue);
					feature->put_new(key,d);
				}
				else
				{
					feature->put_new(key, mapnik::value_null());
				}
			}
		}
		mapnik::memory_datasource *cache = dynamic_cast<mapnik::memory_datasource *>((*_ds).get());
		cache->push(feature);
	}


}