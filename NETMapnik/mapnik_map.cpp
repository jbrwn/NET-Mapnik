#include "stdafx.h"
#include "mapnik_map.h"
#include "mapnik_grid.h"
#include "mapnik_image.h"
#include "mapnik_vector_tile.h"
#include "mapnik_value_converter.h"

#include <mapnik\params.hpp>
#include <mapnik\map.hpp>
#include <mapnik\load_map.hpp>
#include <mapnik\agg_renderer.hpp>
#include <mapnik\grid\grid.hpp>
#include <mapnik\grid\grid_view.hpp>
#include <mapnik\grid\grid_renderer.hpp>
#include <mapnik\grid\grid_util.hpp>

// vector output api
#include "vector_tile_processor.hpp"
#include "vector_tile_backend_pbf.hpp"

#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	//Constructor
	Map::Map()
	{
		_map = new mapnik::Map();
	}

	//Destructor
	Map::~Map()
	{
		if (_map != NULL)
			delete _map;
	}

	mapnik::Map *Map::NativeObject()
	{
		return _map;
	}

	//width property
	System::UInt32 Map::Width::get()
	{
		return _map->width();
	}

	void Map::Width::set(System::UInt32 value)
	{
		_map->set_width(value);
	}

	//height property
	System::UInt32 Map::Height::get()
	{
		return _map->height();
	}

	void Map::Height::set(System::UInt32 value)
	{
		_map->set_height(value);
	}

	//buffer
	System::Int32 Map::Buffer::get()
	{
		return _map->buffer_size();
	}

	void Map::Buffer::set(System::Int32 value)
	{
		_map->set_buffer_size(value);
	}

	//parameters
	// TO DO: implement paramaters class with get and set
	System::Collections::Generic::Dictionary<System::String^, System::Object^>^ Map::Parameters::get()
	{
		
		mapnik::parameters const& params = _map->get_extra_parameters();
		mapnik::parameters::const_iterator it = params.begin();
		mapnik::parameters::const_iterator end = params.end();
		System::Collections::Generic::Dictionary<System::String^, System::Object^>^ paramsDictionary = gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>();
		for (; it != end; ++it)
		{
			System::String^ key = msclr::interop::marshal_as<System::String^>(it->first);
			mapnik::value_holder valueHolder = it->second;
			mapnik_value_holder_to_managed *c = new mapnik_value_holder_to_managed();
			paramsDictionary[key] = c->convert(valueHolder);
		}
		return paramsDictionary;
	}

	//load map
	void Map::LoadMap(System::String^ path)
	{
		std::string unmanagedPath = msclr::interop::marshal_as<std::string>(path);
		try
		{
			mapnik::load_map(*_map,unmanagedPath);
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}

	void Map::FromString(System::String^ str)
	{
		std::string unmanagedStr = msclr::interop::marshal_as<std::string>(str);
		try
		{
			mapnik::load_map_string(*_map, unmanagedStr);
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}

	//zoom_to_box
	void Map::ZoomToBox(System::Double minx, System::Double miny, System::Double maxx, System::Double maxy)
	{
		_map->zoom_to_box(mapnik::box2d<double>(minx,miny,maxx,maxy));
	}

	//zoom all
	void Map::ZoomAll()
	{
		_map->zoom_all();
	}

	void Map::Render(Image^ image)
	{
		try
		{
			mapnik::image_32* i = image->NativeObject();
			mapnik::request m_req(_map->width(), _map->height(), _map->get_current_extent());
			m_req.set_buffer_size(_map->buffer_size());
			mapnik::agg_renderer<mapnik::image_32> ren(
				*_map, 
				m_req, 
				*i,
				1.0,
				0U,
				0U
			);
			ren.apply(0);
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}

	void Map::Render(Grid^ grid, System::UInt32 layerIdx,  System::Collections::Generic::List<System::String^>^ fields)
	{
		mapnik::grid* g = grid->NativeObject();
		
		std::vector<mapnik::layer> const& layers = _map->layers();
		std::size_t layer_num = layers.size();
		if (layerIdx >= layer_num) {
			throw gcnew System::Exception(System::String::Format("Zero-based layer index {0} not valid only {1} layers in map",layerIdx,layer_num));
		}
		
		// convert .NET list to std::set
		for each(System::String^ name in fields)
		{
			g->add_property_name(msclr::interop::marshal_as<std::string>(name));
		}

		// copy property names
		std::set<std::string> attributes = g->property_names();
		// todo - make this a static constant
		std::string known_id_key = "__id__";
		if (attributes.find(known_id_key) != attributes.end())
		{
			attributes.erase(known_id_key);
		}

		std::string join_field = g->get_key();
		if (known_id_key != join_field &&
			attributes.find(join_field) == attributes.end())
		{
			attributes.insert(join_field);
		}

		mapnik::grid_renderer<mapnik::grid> ren(*_map,*g,1.0,0,0);
		mapnik::layer const& layer = layers[layerIdx];

		try
		{
			ren.apply(layer, attributes);
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}

	void Map::Render(VectorTile^ tile)
	{
		typedef mapnik::vector::backend_pbf backend_type;
		typedef mapnik::vector::processor<backend_type> renderer_type;

		unsigned tolerance = 1;
		unsigned path_multiplier = 16;
		int buffer_size = 0;
		double scale_factor = 1.0;
		double scale_denominator = 0.0;
		unsigned offset_x = 0;
		unsigned offset_y = 0;
		std::string image_format = "jpeg";
		mapnik::scaling_method_e scaling_method = mapnik::SCALING_NEAR;

		try
		{
			mapnik::vector::tile* vTile = tile->NativeObject();
			backend_type backend(*vTile, path_multiplier);
			mapnik::request m_req(_map->width(), _map->height(), _map->get_current_extent());
			m_req.set_buffer_size(_map->buffer_size());
			renderer_type ren(
				backend,
				*_map,
				m_req,
				scale_factor,
				offset_x,
				offset_y,
				tolerance,
				image_format,
				scaling_method
			);

			ren.apply(scale_denominator);
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}


	}
}
