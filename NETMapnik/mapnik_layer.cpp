//#define NOMINMAX

#include "stdafx.h"
#include "mapnik_layer.h"

// boost
#include <boost\make_shared.hpp>

// microsoft
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	Layer::Layer(mapnik::layer const& layer) 
	{
		_layer = new layer_ptr(boost::make_shared<mapnik::layer>(layer));
	}

	Layer::Layer(System::String^ name)
	{
		std::string unmanagedName = msclr::interop::marshal_as<std::string>(name);
		_layer = new layer_ptr(boost::make_shared<mapnik::layer>(unmanagedName));
	}

	Layer::Layer(System::String^ name, System::String^ srs)
	{
		std::string unmanagedName = msclr::interop::marshal_as<std::string>(name);
		std::string unmanagedSRS = msclr::interop::marshal_as<std::string>(srs);
		_layer = new layer_ptr(boost::make_shared<mapnik::layer>(unmanagedName, unmanagedSRS));
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

	System::Collections::Generic::IEnumerable<System::String^>^  Layer::Styles::get()
	{
		std::vector<std::string> const& style_names = (*_layer)->styles();
		System::Collections::Generic::List<System::String^>^  s = gcnew System::Collections::Generic::List<System::String^>();
		for (unsigned i = 0; i < style_names.size(); ++i)
		{
			s->Add(msclr::interop::marshal_as<System::String^>((style_names[i].c_str())));
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


	//	Datasource^ Layer::Datasource::get();
	//	void Layer::Datasource::set(Datasource^ value);


	System::Collections::Generic::Dictionary<System::String^, System::Object^>^ Layer::Describe()
	{
		System::Collections::Generic::Dictionary<System::String^, System::Object^>^ description = gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>();
		if ((*_layer)->name() != "")
		{
			description["name"] = msclr::interop::marshal_as<System::String^>((*_layer)->name().c_str());
		}

		if ((*_layer)->srs() != "")
		{
			description["srs"] = msclr::interop::marshal_as<System::String^>((*_layer)->srs().c_str());
		}

		if (!(*_layer)->active())
		{
			description["status"] = (*_layer)->active();
		}

		if ((*_layer)->clear_label_cache())
		{
			description["clear_label_cache"] = (*_layer)->clear_label_cache();
		}

		if ((*_layer)->min_zoom() > 0)
		{
			description["minzoom"] = (*_layer)->min_zoom();
		}

		if ((*_layer)->max_zoom() != (std::numeric_limits<double>::max)())
		{
			description["maxzoom"] = (*_layer)->max_zoom();
		}

		if ((*_layer)->queryable())
		{
			description["queryable"] = (*_layer)->queryable();
		}

		description["styles"] = Styles;

		//mapnik::datasource_ptr datasource = (*_layer)->datasource();
		//Datasource ds;
		//description["datasource"), ds);
		//if (datasource)
		//{
		//	mapnik::parameters::const_iterator it = datasource->params().begin();
		//	mapnik::parameters::const_iterator end = datasource->params().end();
		//	for (; it != end; ++it)
		//	{
		//		node_mapnik::params_to_object serializer(ds, it->first);
		//		MAPNIK_APPLY_VISITOR(serializer, it->second);
		//	}
		//}

		return description;
	}

	
}