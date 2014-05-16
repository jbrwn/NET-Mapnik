#include "stdafx.h"
#include "map.h"

#include <mapnik\map.hpp>
#include <mapnik\load_map.hpp>
#include <mapnik\agg_renderer.hpp>
#include <mapnik\image_util.hpp>
#include <mapnik\grid\grid.hpp>
#include <mapnik\grid\grid_view.hpp>
#include <mapnik\grid\grid_renderer.hpp>
#include <mapnik\grid\grid_util.hpp>

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

	//width property
	System::UInt32 Map::width::get()
	{
		return _map->width();
	}

	void Map::width::set(System::UInt32 value)
	{
		_map->set_width(value);
	}

	//height property
	System::UInt32 Map::height::get()
	{
		return _map->height();
	}

	void Map::height::set(System::UInt32 value)
	{
		_map->set_height(value);
	}

	//load map
	void Map::load_map(System::String^ path)
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

	//zoom_to_box
	void Map::zoom_to_box(System::Double minx, System::Double miny, System::Double maxx, System::Double maxy)
	{
		_map->zoom_to_box(mapnik::box2d<double>(minx,miny,maxx,maxy));
	}

	//save to byte array
	array<System::Byte>^ Map::save_to_bytes(System::String^ format)
	{
		std::string unmanagedFormat = msclr::interop::marshal_as<std::string>(format);

		mapnik::image_32 buf(_map->width(),_map->height());
		mapnik::agg_renderer<mapnik::image_32> ren(*_map,buf);
		try
		{
			ren.apply();
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
		std::string s = save_to_string(buf,unmanagedFormat);
		array<System::Byte>^ data = gcnew array<System::Byte>(s.size());
		System::Runtime::InteropServices::Marshal::Copy(System::IntPtr(&s[0]), data, 0, s.size());
		return data;
	}

	//save to file
	void Map::save_to_file(System::String^ path, System::String^ format)
	{
		std::string unmanagedPath = msclr::interop::marshal_as<std::string>(path);
		std::string unmanagedFormat = msclr::interop::marshal_as<std::string>(format);

		mapnik::image_32 buf(_map->width(),_map->height());
		mapnik::agg_renderer<mapnik::image_32> ren(*_map,buf);
		try
		{
			ren.apply();
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
		mapnik::save_to_file(buf,unmanagedPath,unmanagedFormat);
	}

	void Map::RenderLayer(Grid^ grid, System::UInt32 layerIdx,  System::Collections::Generic::List<System::String^>^ fields)
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
		ren.apply(layer,attributes);

	}
	//System::Collections::Generic::Dictionary<System::String^, System::Object^>^ Map::SaveToGrid(System::String^ layer, System::Collections::Generic::List<System::String^> fields, double resolution)
	//{
		////find layer 
		//std::string uLayer = msclr::interop::marshal_as<std::string>(layer);
		//std::vector<mapnik::layer> layers  = _map->layers();
		//
		////create new grid
		//mapnik::grid grid(_map->width(), _map->height(), "__id__", 1);

		//
		////args
		////layer_idx - int (or layer name if possible - managed String)
		////Managed List<String^>^ of fields
		////double resolution

		////returns Dictionary<string, object>

		////1. create grid(mapwidth, mapheight)
		////2. grid_utils::render_layer_for_grid()
		//	//void render_layer_for_grid(const mapnik::Map& map,
		//	//                                mapnik::grid& grid,
		//	//                                unsigned layer_idx, // TODO - layer by name or index
		//	//                                boost::python::list const& fields);
		//
		////3. grid_utils::grid_encode()
		//	//	dictionar<string, object> grid_encode(const& grid, std::string const& format, bool add_features, unsigned int resolution);
		//

		////other grid_util members:
		////dictionary<string, object> grid_encode_utf
		////list<> = grid2utf(grid,add_features)
		////list<> = grid2utf(grid,add_features,resolution) -> for resampling




		//
		////choose layer
		//unsigned layer_idx = 0;
		////create hit grid
		//
		////add fields
		//grid.add_property_name("ScaleRank");

		//

		//mapnik::grid_renderer<mapnik::grid> ren(*_map,grid,1.0,0);
		//mapnik::layer const& layer = _map->getLayer(layer_idx);
		//std::set<std::string> attributes = grid.property_names();
		//ren.apply(layer,attributes);
	
		//mapnik::grid_view gv = grid.get_view(0,0,_map->width(),_map->height());
		//
		////resample if needed

		////begin encode to utf
		////grid_type = gv; //arg
		////boost::python::list& l //arg
		//System::Collections::Generic::List<array<System::Char^>^>^ l = gcnew System::Collections::Generic::List<array<System::Char^>^>(); ///arg;
		//std::vector<mapnik::grid::lookup_type> key_order; //arg

		////do work
		//typedef std::map<mapnik::grid::lookup_type,mapnik::value_integer> keys_type;
		//typedef keys_type::const_iterator keys_iterator;

		//mapnik::grid::data_type const& data = grid.data();
		//mapnik::grid::feature_key_type const& feature_keys = grid.get_feature_keys();
		//mapnik::grid::feature_key_type::const_iterator feature_pos;

		//keys_type keys;
		//// start counting at utf8 codepoint 32, aka space character
		//boost::uint16_t codepoint = 32;

		//unsigned array_size = data.width();
		//for (unsigned y = 0; y < data.height(); ++y)
		//{
		//	boost::uint16_t idx = 0;
		//	//boost::scoped_array<Py_UNICODE> line(new Py_UNICODE[array_size]);
		//	array<System::Char^>^ line = gcnew array<System::Char^>(array_size);
		//	mapnik::value_integer const* row = data.getRow(y);
		//	for (unsigned x = 0; x < data.width(); ++x)
		//	{
		//		mapnik::value_integer feature_id = row[x];
		//		feature_pos = feature_keys.find(feature_id);
		//		
		//		if (feature_pos != feature_keys.end())
		//		{
		//			mapnik::grid::lookup_type val = feature_pos->second;
		//			keys_iterator key_pos = keys.find(val);
		//			if (key_pos == keys.end())
		//			{
		//				// Create a new entry for this key. Skip the codepoints that
		//				// can't be encoded directly in JSON.
		//				if (codepoint == 34) ++codepoint;      // Skip "
		//				else if (codepoint == 92) ++codepoint; // Skip backslash
		//				if (feature_id == mapnik::grid::base_mask)
		//				{
		//					keys[""] = codepoint;
		//					key_order.push_back("");
		//				}
		//				else
		//				{
		//					keys[val] = codepoint;
		//					key_order.push_back(val);
		//				}
		//				line[idx++] = (System::Char)codepoint;
		//				++codepoint;

		//			}
		//			else
		//			{
		//				line[idx++] = (System::Char)key_pos->second;
		//			}
		//		}
		//	}
		//	//l.append(boost::python::object(
  // //             boost::python::handle<>(
  // //                 PyUnicode_FromUnicode(line.get(), array_size))));
		//	l->Add(line);
		//}
		//
		//// ok we got the grid
		////get keys now
		//System::Collections::Generic::List<System::String^>^ keys_a = gcnew System::Collections::Generic::List<System::String^>();
		//BOOST_FOREACH ( mapnik::grid::lookup_type const& key_id, key_order )
		//{
		//	System::String^ k = msclr::interop::marshal_as<System::String^>(key_id);
		//	keys_a->Add(k);
		//}

		////get data now (i.e., write_features...)
		////System::Collections::Generic::Dictionary<>^ = gcnew System::Collections::Generic::Dictionary<>()

		//mapnik::grid::feature_type const& g_features = grid.get_grid_features();
		//if (g_features.size() <= 0)
		//{
		//	return;
		//}

		////std::set<std::string> const& attributes = grid.property_names();
		//mapnik::grid::feature_type::const_iterator feat_end = g_features.end();
		//
		////for ( std::string const& key_item :key_order )
		//for ( std::vector<mapnik::grid::lookup_type>::const_iterator iter = key_order.begin(), stop = key_order.end(); iter != stop; ++iter) 
		//{
		//	mapnik::grid::lookup_type key_item = *iter;
		//	if (key_item.empty())
		//	{
		//		continue;
		//	}

		//	mapnik::grid::feature_type::const_iterator feat_itr = g_features.find(key_item);
		//	if (feat_itr == feat_end)
		//	{
		//		continue;
		//	}

		//	bool found = false;
		//	/*boost::python::dict feat;*/
		//	mapnik::feature_ptr feature = feat_itr->second;
		//	//for ( std::string const& attr : attributes )
		//	for ( std::set<std::string>::const_iterator iter = attributes.begin(), stop = attributes.end(); iter != stop; ++iter)
		//	{
		//		std::string attr = *iter;
		//		if (attr == "__id__")
		//		{
		//			/*feat[attr.c_str()] = feature->id();*/
		//		}
		//		else if (feature->has_key(attr))
		//		{
		//			found = true;
		//			/*feat[attr.c_str()] = feature->get(attr);*/
		//		}
		//	}

		//	if (found)
		//	{
		//		/*feature_data[feat_itr->first] = feat;*/
		//	}
		//}

	//}
}
