#include "stdafx.h"
#include "mapnik_map.h"
#include "mapnik_grid.h"
#include "mapnik_image.h"
#include "mapnik_vector_tile.h"
#include "utils.h"
#include "NET_options_parser.h"
#include "mapnik_color.h"
#include "mapnik_layer.h"
#include "mapnik_featureset.h"

#include <memory>

// mapnik
#include <mapnik\agg_renderer.hpp>
#include <mapnik\grid\grid.hpp>
#include <mapnik\box2d.hpp>       
#include <mapnik\color.hpp>         
#include <mapnik\attribute.hpp>       
#include <mapnik\datasource.hpp>        
#include <mapnik\graphics.hpp>  
#include <mapnik\grid\grid_view.hpp>
#include <mapnik\grid\grid_renderer.hpp>
#include <mapnik\grid\grid_util.hpp>
#include <mapnik\image_data.hpp>        
#include <mapnik\image_util.hpp>        
#include <mapnik\layer.hpp>            
#include <mapnik\load_map.hpp>   
#include <mapnik\map.hpp>       
#include <mapnik\params.hpp>       
#include <mapnik\save_map.hpp>     
#include <mapnik\image_scaling.hpp>
#include <mapnik\request.hpp>

// vector tile api
#include "vector_tile_processor.hpp"
#include "vector_tile_backend_pbf.hpp"

// boost
#include <boost\foreach.hpp>
#include <boost\optional\optional.hpp> 

// microsoft
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	//Constructor
	Map::Map(System::Int32 width, System::Int32 height)
	{
		_map = new map_ptr(std::make_shared<mapnik::Map>(width, height));
	}
	Map::Map(System::Int32 width, System::Int32 height, System::String^ srs)
	{
		_map = new map_ptr(std::make_shared<mapnik::Map>(width, height, msclr::interop::marshal_as<std::string>(srs)));
	}
	Map::Map(mapnik::Map const& map)
	{
		_map = new map_ptr(std::make_shared<mapnik::Map>(map));
	}

	//Destructor
	Map::~Map()
	{
		if (_map != NULL)
			delete _map;
	}

	map_ptr Map::NativeObject()
	{
		return *_map;
	}

	//width property
	System::Int32 Map::Width::get()
	{
		return (*_map)->width();
	}

	void Map::Width::set(System::Int32 value)
	{
		(*_map)->set_width(value);
	}

	//height property
	System::Int32 Map::Height::get()
	{
		return (*_map)->height();
	}

	void Map::Height::set(System::Int32 value)
	{
		(*_map)->set_height(value);
	}

	//buffer
	System::Int32 Map::BufferSize::get()
	{
		return (*_map)->buffer_size();
	}

	void Map::BufferSize::set(System::Int32 value)
	{
		(*_map)->set_buffer_size(value);
	}

	//SRS
	System::String^ Map::SRS::get()
	{
		std::string const srs = (*_map)->srs();
		return msclr::interop::marshal_as<System::String^>(srs);
	}

	void Map::SRS::set(System::String^ value)
	{
		std::string srs = msclr::interop::marshal_as<std::string>(value);
		(*_map)->set_srs(srs);
	}

	//Aspect Fix Mode
	AspectFixMode Map::AspectFixMode::get()
	{
		return static_cast<NETMapnik::AspectFixMode>((*_map)->get_aspect_fix_mode());
	}

	void Map::AspectFixMode::set(NETMapnik::AspectFixMode value)
	{
		int enumValue = static_cast<int>(value);
		if (enumValue < (*_map)->aspect_fix_mode_MAX)
			(*_map)->set_aspect_fix_mode(static_cast<mapnik::Map::aspect_fix_mode>(enumValue));
		else
			throw gcnew System::Exception("AspectFixMode is invalid");
	}

	//Extent
	array<System::Double>^ Map::Extent::get()
	{
		return Box2DToArray((*_map)->get_current_extent());
	}

	void Map::Extent::set(array<System::Double>^ value)
	{
		(*_map)->zoom_to_box(ArrayToBox2D(value));
	}

	//BufferedExtent
	array<System::Double>^ Map::BufferedExtent::get()
	{
		return Box2DToArray((*_map)->get_buffered_extent());
	}

	//MaximumExtent
	array<System::Double>^ Map::MaximumExtent::get()
	{
		boost::optional<mapnik::box2d<double>> const& extent = (*_map)->maximum_extent();
		if (!extent)
			return nullptr;
		return Box2DToArray(*extent);
	}

	void Map::MaximumExtent::set(array<System::Double>^ value)
	{
		(*_map)->set_maximum_extent(ArrayToBox2D(value));
	}

	Color^ Map::Background::get()
	{
		boost::optional<mapnik::color> const& bg = (*_map)->background();
		if (!bg)
			return nullptr;
		return gcnew Color(*bg);
	}

	void Map::Background::set(Color^ value)
	{
		color_ptr c = value->NativeObject();
		(*_map)->set_background(*c);
	}

	//Scale
	System::Double Map::Scale()
	{
		return (*_map)->scale();
	}

	//ScaleDenominator
	System::Double Map::ScaleDenominator()
	{
		return (*_map)->scale_denominator();
	}

	//clear
	void Map::Clear()
	{
		(*_map)->remove_all();
	}

	//Resize
	void Map::Resize(System::Int32 width, System::Int32 heigt)
	{
		(*_map)->resize(width, heigt);
	}

	System::Collections::Generic::IEnumerable<System::String^>^ Map::Fonts()
	{
		auto const& mapping = (*_map)->get_font_file_mapping();
		System::Collections::Generic::List<System::String^>^ a = gcnew System::Collections::Generic::List<System::String^>();
		unsigned i = 0;
		for (auto const& kv : mapping)
		{
			a->Add(msclr::interop::marshal_as<System::String^>(kv.first.c_str()));
		}
		return a;
	}

	System::Collections::Generic::IDictionary<System::String^, System::String^>^ Map::FontFiles()
	{
		auto const& mapping = (*_map)->get_font_file_mapping();
		System::Collections::Generic::IDictionary<System::String^, System::String^>^ d = gcnew System::Collections::Generic::Dictionary<System::String^, System::String^>();
		unsigned i = 0;
		for (auto const& kv : mapping)
		{
			System::String^ key  = msclr::interop::marshal_as<System::String^>(kv.first.c_str());
			System::String^ value = msclr::interop::marshal_as<System::String^>(kv.second.second.c_str());
			d->Add(key, value);
		}
		return d;
	}

	System::String ^ Map::FontDirectory()
	{
		boost::optional<std::string> const& fdir = (*_map)->font_directory();
		if (fdir)
		{
			return msclr::interop::marshal_as<System::String^>(*fdir);
		}
		return nullptr;
	}

	System::Boolean Map::LoadFonts()
	{
		return (*_map)->load_fonts();
	}

	System::Collections::Generic::IEnumerable<System::String^>^ Map::MemoryFonts()
	{
		auto const& font_cache = (*_map)->get_font_memory_cache();
		System::Collections::Generic::List<System::String^>^ a = gcnew System::Collections::Generic::List<System::String^>();
		unsigned i = 0;
		for (auto const& kv : font_cache)
		{
			a->Add(msclr::interop::marshal_as<System::String^>(kv.first.c_str()));
		}
		return a;
	}

	System::Boolean Map::RegisterFonts(System::String ^ path)
	{
		return (*_map)->register_fonts(msclr::interop::marshal_as<std::string>(path));
	}

	System::Boolean Map::RegisterFonts(System::String ^ path, System::Boolean recurse)
	{
		return (*_map)->register_fonts(msclr::interop::marshal_as<std::string>(path), recurse);
	}

	Map ^ Map::Clone()
	{
		return gcnew Map(*(*_map));
	}

	void Map::Save(System::String ^ path)
	{
		std::string filename = msclr::interop::marshal_as<std::string>(path);
		bool explicit_defaults = false;
		mapnik::save_map(*(*_map), filename, explicit_defaults);
	}

	System::String ^ Map::ToXML()
	{
		bool explicit_defaults = false;
		std::string map_string = mapnik::save_map_to_string(*(*_map), explicit_defaults);
		return msclr::interop::marshal_as<System::String^>(map_string.c_str());
	}

	System::Collections::Generic::IEnumerable<MapQueryResult^>^ Map::QueryMapPoint(System::Double x, System::Double y)
	{
		return _queryPoint(x, y, -1, false);
	}

	System::Collections::Generic::IEnumerable<MapQueryResult^>^ Map::QueryMapPoint(System::Double x, System::Double y, System::Int32 layerIndex)
	{
		return _queryPoint(x, y, layerIndex, false);
	}

	System::Collections::Generic::IEnumerable<MapQueryResult^>^ Map::QueryMapPoint(System::Double x, System::Double y, System::String ^ layerName)
	{
		std::vector<mapnik::layer> const& layers = (*_map)->layers();
		int layer_idx = -1;
		bool found = false;
		unsigned int idx(0);
		std::string layer_name = msclr::interop::marshal_as<std::string>(layerName);
		for (mapnik::layer const& lyr : layers)
		{
			if (lyr.name() == layer_name)
			{
				found = true;
				layer_idx = idx;
				break;
			}
			++idx;
		}
		if (!found)
		{
			throw gcnew System::Exception(System::String::Format("Layer name {0} not found", layerName));
		}
		return _queryPoint(x, y, layer_idx, false);
	}


	System::Collections::Generic::IEnumerable<MapQueryResult^>^ Map::QueryPoint(System::Double x, System::Double y)
	{
		return _queryPoint(x, y, -1, true);
	}

	System::Collections::Generic::IEnumerable<MapQueryResult^>^ Map::QueryPoint(System::Double x, System::Double y, System::Int32 layerIndex)
	{
		return _queryPoint(x, y, layerIndex, true);
	}

	System::Collections::Generic::IEnumerable<MapQueryResult^>^ Map::QueryPoint(System::Double x, System::Double y, System::String ^ layerName)
	{
		std::vector<mapnik::layer> const& layers = (*_map)->layers();
		int layer_idx = -1;
		bool found = false;
		unsigned int idx(0);
		std::string layer_name = msclr::interop::marshal_as<std::string>(layerName);
		for (mapnik::layer const& lyr : layers)
		{
			if (lyr.name() == layer_name)
			{
				found = true;
				layer_idx = idx;
				break;
			}
			++idx;
		}
		if (!found)
		{
			throw gcnew System::Exception(System::String::Format("Layer name {0} not found", layerName));
		}
		return _queryPoint(x, y, layer_idx, true);
	}
		
	System::Collections::Generic::IEnumerable<MapQueryResult^>^ Map::_queryPoint(double x, double y, int layer_idx, bool geo_coords)
	{
		System::Collections::Generic::List<MapQueryResult^>^ featuresets = gcnew System::Collections::Generic::List<MapQueryResult^>();
		try
		{
			std::vector<mapnik::layer> const& layers = (*_map)->layers();
			if (layer_idx >= 0)
			{
				mapnik::featureset_ptr fs;
				if (geo_coords)
				{
					fs = (*_map)->query_point(layer_idx, x, y);
				}
				else
				{
					fs = (*_map)->query_map_point(layer_idx, x, y);
				}
				mapnik::layer const& lyr = layers[layer_idx];
				MapQueryResult^ mqr = gcnew MapQueryResult(
					msclr::interop::marshal_as<System::String^>(lyr.name()),
					gcnew Featureset(fs)
				);
				featuresets->Add(mqr);
			}
			else
			{
				// query all layers
				unsigned idx = 0;
				for (mapnik::layer const& lyr : layers)
				{
					mapnik::featureset_ptr fs;
					if (geo_coords)
					{
						fs = (*_map)->query_point(idx, x, y);
					}
					else
					{
						fs = (*_map)->query_map_point(idx, x, y);
					}
					MapQueryResult^ mqr = gcnew MapQueryResult(
						msclr::interop::marshal_as<System::String^>(lyr.name()),
						gcnew Featureset(fs)
						);
					featuresets->Add(mqr);
					++idx;
				}
			}
		}
		catch (std::exception const& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
		return featuresets;
	}


	//parameters
	// TO DO: implement paramaters class with get and set
	System::Collections::Generic::Dictionary<System::String^, System::Object^>^ Map::Parameters::get()
	{
		
		mapnik::parameters const& params = (*_map)->get_extra_parameters();
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

	void Map::Parameters::set(System::Collections::Generic::Dictionary<System::String^, System::Object^>^ paramsDictionary)
	{
		mapnik::parameters params;
		for each (System::Collections::Generic::KeyValuePair<System::String^, System::Object^>^ kvp in paramsDictionary)
		{
			std::string key = msclr::interop::marshal_as<std::string>(kvp->Key);
			System::Object^ managedValue = kvp->Value;
			if (managedValue->GetType() == System::String::typeid)
			{
			    params[key] = msclr::interop::marshal_as<std::string>(safe_cast<System::String^>(managedValue));
			}
			else if (managedValue->GetType() == System::Int32::typeid)
			{
				int i = safe_cast<int>(managedValue);
				params[key] = static_cast<mapnik::value_integer>(i);
			}
			else if (managedValue->GetType() == System::Double::typeid)
			{
				double d = safe_cast<double>(managedValue);
				params[key] = d;
			}
		}
		(*_map)->set_extra_parameters(params);
	}

	//GetLayer
	Layer^ Map::GetLayer(System::Int32 index)
	{
		std::vector<mapnik::layer> const& layers = (*_map)->layers();
		if (index < layers.size())
		{
			return gcnew Layer(layers[index]);
		}
		else
		{
			throw gcnew System::ArgumentOutOfRangeException("invalid layer index");
		}
		
	}

	Layer^ Map::GetLayer(System::String^ name)
	{
		bool found = false;
		unsigned int idx(0);
		std::string layer_name = msclr::interop::marshal_as<std::string>(name);
		std::vector<mapnik::layer> const& layers = (*_map)->layers();
		for (mapnik::layer const& lyr : layers)
		{
			if (lyr.name() == layer_name)
			{
				found = true;
				return gcnew Layer(layers[idx]);
			}
			++idx;
		}
		if (!found)
		{
			throw gcnew System::Exception(System::String::Format("Layer name {0} not found",name));
		}
	}

	//AddLayer
	void Map::AddLayer(Layer^ layer)
	{
		(*_map)->MAPNIK_ADD_LAYER(*layer->NativeObject());
	}

	//Layers
	System::Collections::Generic::IEnumerable<Layer^>^ Map::Layers()
	{
		std::vector<mapnik::layer> const& layers = (*_map)->layers();
		System::Collections::Generic::List<Layer^>^ layerCollection = gcnew System::Collections::Generic::List<Layer^>();
		for (unsigned i = 0; i < layers.size(); ++i)
		{
			layerCollection->Add(gcnew Layer(layers[i]));
		}
		return layerCollection;
	}

	//load map
	void Map::Load(System::String^ path)
	{
		return Load(path, false, "");
	}

	void Map::Load(System::String^ path, System::Boolean strict)
	{
		return Load(path, strict, "");
	}

	void Map::Load(System::String^ path, System::Boolean strict, System::String^ basePath)
	{
		std::string unmanagedPath = msclr::interop::marshal_as<std::string>(path);
		std::string unmanagedBasePath = msclr::interop::marshal_as<std::string>(basePath);
		try
		{
			mapnik::load_map(*(*_map),unmanagedPath,strict,unmanagedBasePath);
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}

	//FromString
	void Map::FromString(System::String^ str)
	{
		return FromString(str, false, "");
	}

	void Map::FromString(System::String^ str, System::Boolean strict)
	{
		return FromString(str, strict, "");
	}

	void Map::FromString(System::String^ str, System::Boolean strict, System::String^ basePath)
	{
		std::string unmanagedStr = msclr::interop::marshal_as<std::string>(str);
		std::string unmanagedBasePath = msclr::interop::marshal_as<std::string>(basePath);
		try
		{
			mapnik::load_map_string(*(*_map), unmanagedStr,strict,unmanagedBasePath);
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
		(*_map)->zoom_to_box(mapnik::box2d<double>(minx,miny,maxx,maxy));
	}

	//zoom all
	void Map::ZoomAll()
	{
		try
		{
			(*_map)->zoom_all();
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}

	void Map::Render(Image^ image)
	{
		Render(image, gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>());
	}
	void Map::Render(Image^ image, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		// set defaults 
		int buffer_size = 0;
		double scale_factor = 1.0;
		double scale_denominator = 0.0;
		unsigned offset_x = 0;
		unsigned offset_y = 0;
		auto variables = mapnik::attributes();
		
		// get options
		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		optionsParser->TryGet<int>("BufferSize", buffer_size);
		optionsParser->TryGet<double>("Scale", scale_factor);
		optionsParser->TryGet<double>("ScaleDenominator", scale_denominator);
		optionsParser->TryGet<unsigned>("OffsetX", offset_x);
		optionsParser->TryGet<unsigned>("OffsetY", offset_y);

		try
		{
			image_ptr i = image->NativeObject();
			mapnik::request m_req((*_map)->width(), (*_map)->height(), (*_map)->get_current_extent());
			m_req.set_buffer_size(buffer_size);
			mapnik::agg_renderer<mapnik::image_32> ren(
				*(*_map),
				m_req,
				variables,
				*i,
				scale_factor,
				offset_x,
				offset_y
			);
			ren.apply(scale_denominator);
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}

	void Map::Render(Grid^ grid)
	{
		Render(grid, gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>());
	}

	void Map::Render(Grid^ grid, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		// unwrap grid
		grid_ptr g = grid->NativeObject();

		// set defaults 
		int buffer_size = 0;
		double scale_factor = 1.0;
		double scale_denominator = 0.0;
		unsigned offset_x = 0;
		unsigned offset_y = 0;
		std::size_t layer_idx = 0;

		// get options
		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		optionsParser->TryGet<int>("BufferSize", buffer_size);
		optionsParser->TryGet<double>("Scale", scale_factor);
		optionsParser->TryGet<double>("ScaleDenominator", scale_denominator);
		optionsParser->TryGet<unsigned>("OffsetX", offset_x);
		optionsParser->TryGet<unsigned>("OffsetY", offset_y);

		std::vector<mapnik::layer> const& layers = (*_map)->layers();

		System::String^ layer;
		if (optionsParser->TryGet<System::String^>("Layer", layer))
		{
			bool found = false;
			unsigned int idx(0);
			std::string const& layer_name = msclr::interop::marshal_as<std::string>(layer);
			BOOST_FOREACH(mapnik::layer const& lyr, layers)
			{
				if (lyr.name() == layer_name)
				{
					found = true;
					layer_idx = idx;
					break;
				}
				++idx;
			}
			if (!found)
			{
				throw gcnew System::ArgumentException("Layer name " + layer + " not found", "Layer");
			}
		}

		System::Collections::Generic::IEnumerable<System::String^>^ fields;
		if (optionsParser->TryGet<System::Collections::Generic::IEnumerable<System::String^>^>("Fields", fields))
		{
			for each(System::String^ name in fields)
			{
				g->add_property_name(msclr::interop::marshal_as<std::string>(name));
			}
		}

		try
		{
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

			mapnik::grid_renderer<mapnik::grid> ren(
				*(*_map),
				*g,
				scale_factor,
				offset_x,
				offset_y
			);
			mapnik::layer const& layer = layers[layer_idx];
			ren.apply(layer, attributes, scale_denominator);
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}
	void Map::Render(VectorTile^ tile)
	{
		Render(tile, gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>());
	}

	void Map::Render(VectorTile^ tile, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		// set defaults 
		int buffer_size = 0;
		double scale_factor = 1.0;
		double scale_denominator = 0.0;
		unsigned offset_x = 0;
		unsigned offset_y = 0;
		unsigned tolerance = 1;
		unsigned path_multiplier = 16;
		std::string image_format = "jpeg";
		mapnik::scaling_method_e scaling_method = mapnik::SCALING_NEAR;

		// get options
		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		optionsParser->TryGet<int>("BufferSize", buffer_size);
		optionsParser->TryGet<double>("Scale", scale_factor);
		optionsParser->TryGet<double>("ScaleDenominator", scale_denominator);
		optionsParser->TryGet<unsigned>("OffsetX", offset_x);
		optionsParser->TryGet<unsigned>("OffsetY", offset_y);
		optionsParser->TryGet<unsigned>("Tolernace", tolerance);
		optionsParser->TryGet<unsigned>("PathMultiplier", path_multiplier);

		System::String^ format;
		if (optionsParser->TryGet<System::String^>("ImageFormat", format))
			image_format = msclr::interop::marshal_as<std::string>(format);

		System::String^ scaling;
		if (optionsParser->TryGet<System::String^>("ImageScaling", scaling))
		{
			std::string scaling_str = msclr::interop::marshal_as<std::string>(scaling);
			boost::optional<mapnik::scaling_method_e> method = mapnik::scaling_method_from_string(scaling_str);
			if (!method)
				throw gcnew System::ArgumentException("ImageScaling must be a valid scaling (e.g 'bilinear')", "ImageScaling");
			scaling_method = *method;
		}

		typedef mapnik::vector_tile_impl::backend_pbf backend_type;
		typedef mapnik::vector_tile_impl::processor<backend_type> renderer_type;

		try
		{
			vector_tile::Tile* vTile = tile->NativeObject();
			backend_type backend(*vTile, path_multiplier);
			mapnik::request m_req((*_map)->width(), (*_map)->height(), (*_map)->get_current_extent());
			m_req.set_buffer_size(buffer_size);
			renderer_type ren(
				backend,
				*(*_map),
				m_req,
				scale_factor,
				offset_x,
				offset_y,
				tolerance,
				image_format,
				scaling_method
			);

			ren.apply(scale_denominator);
			tile->Painted(ren.painted());
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}
	
	//void NETMapnik::Map::RenderFile(System::String ^ path, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	//{
	//	std::string unamangedPath = msclr::interop::marshal_as<std::string>(path);
	//	
	//	 defaults
	//	double scale_factor = 1.0;
	//	double scale_denominator = 0.0;
	//	int buffer_size = 0;
	//	std::string format = "png";
	//	palette_ptr palette;
	//}
}
