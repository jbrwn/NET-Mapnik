#include "stdafx.h"
#include "mapnik_layer.h"
#include "mapnik_datasource.h"
#include "mapnik_memory_datasource.h"
#include "utils.h"

#include <memory>

// mapnik
#include <mapnik\datasource.hpp>
#include <mapnik\layer.hpp>
#include <mapnik\params.hpp>
#include <mapnik\memory_datasource.hpp>

// microsoft
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	Layer::Layer(mapnik::layer const& layer) 
	{
		_layer = new layer_ptr(std::make_shared<mapnik::layer>(layer));
	}

	Layer::Layer(System::String^ name)
	{
		std::string unmanagedName = msclr::interop::marshal_as<std::string>(name);
		_layer = new layer_ptr(std::make_shared<mapnik::layer>(unmanagedName));
	}

	Layer::Layer(System::String^ name, System::String^ srs)
	{
		std::string unmanagedName = msclr::interop::marshal_as<std::string>(name);
		std::string unmanagedSRS = msclr::interop::marshal_as<std::string>(srs);
		_layer = new layer_ptr(std::make_shared<mapnik::layer>(unmanagedName, unmanagedSRS));
	}

	Layer::~Layer()
	{
		if (_layer != NULL)
			delete _layer;
	}

	layer_ptr Layer::NativeObject()
	{
		return  *_layer;
	}

	System::String^ Layer::Name::get()
	{
		return msclr::interop::marshal_as<System::String^>((*_layer)->name());
	}
	void Layer::Name::set(System::String^ value)
	{
		(*_layer)->set_name(msclr::interop::marshal_as<std::string>(value));
	}

	System::String^ Layer::SRS::get()
	{
		return msclr::interop::marshal_as<System::String^>((*_layer)->srs());
	}
	void Layer::SRS::set(System::String^ value)
	{
		(*_layer)->set_srs(msclr::interop::marshal_as<std::string>(value));
	}

	System::Boolean Layer::CacheFeatures::get()
	{
		return (*_layer)->cache_features();
	}
	void Layer::CacheFeatures::set(System::Boolean value)
	{
		(*_layer)->set_cache_features(value);
	}

	System::Collections::Generic::IEnumerable<System::String^>^  Layer::Styles::get()
	{
		std::vector<std::string> const& style_names = (*_layer)->styles();
		System::Collections::Generic::List<System::String^>^  s = gcnew System::Collections::Generic::List<System::String^>();
		for (unsigned i = 0; i < style_names.size(); ++i)
		{
			s->Add(msclr::interop::marshal_as<System::String^>((style_names[i])));
		}
		return s;
	}

	void Layer::Styles::set(System::Collections::Generic::IEnumerable<System::String^>^ value)
	{
		for each (System::String^ style in value)
		{
			(*_layer)->add_style(msclr::interop::marshal_as<std::string>(style));
		}
	}


	Datasource^ Layer::Datasource::get()
	{
		mapnik::datasource_ptr ds = (*_layer)->datasource();
		if (ds)
		{
			return gcnew NETMapnik::Datasource(ds);
		}
		return nullptr;
	}

	void Layer::Datasource::set(NETMapnik::Datasource^ value)
	{
		(*_layer)->set_datasource(value->NativeObject());
	}


	System::Collections::Generic::IDictionary<System::String^, System::Object^>^ Layer::Describe()
	{
		System::Collections::Generic::Dictionary<System::String^, System::Object^>^ description = gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>();

		description["name"] = msclr::interop::marshal_as<System::String^>((*_layer)->name());
		description["srs"] = msclr::interop::marshal_as<System::String^>((*_layer)->srs());
		description["status"] = (*_layer)->active();
		description["clear_label_cache"] = (*_layer)->clear_label_cache();
		description["minzoom"] = (*_layer)->min_zoom();
		description["maxzoom"] = (*_layer)->max_zoom();
		description["queryable"] = (*_layer)->queryable();
		description["styles"] = Styles;

		mapnik::datasource_ptr datasource = (*_layer)->datasource();
		if (datasource)
		{
			System::Collections::Generic::Dictionary<System::String^, System::Object^>^ ds = gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>();
			mapnik::parameters::const_iterator it = datasource->params().begin();
			mapnik::parameters::const_iterator end = datasource->params().end();
			for (; it != end; ++it)
			{
				System::String^ key = msclr::interop::marshal_as<System::String^>(it->first);
				params_to_object serializer(ds, key);
				mapnik::util::apply_visitor(serializer, it->second);
				
			}
			description["datasource"] = ds;
		}
		return description;
	}

	
}