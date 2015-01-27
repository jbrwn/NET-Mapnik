#include "stdafx.h"
#include "mapnik_vector_tile.h"
#include "mapnik_map.h"
#include "mapnik_image.h"
#include "mapnik_grid.h"
#include "mapnik_feature.h"
#include "NET_options_parser.h"
#include "proj_transform_adapter.h"

#include <mapnik\map.hpp>
#include <mapnik\layer.hpp>
#include <mapnik\geom_util.hpp>
#include <mapnik\version.hpp>
#include <mapnik\request.hpp>
#include <mapnik\graphics.hpp>
#include <mapnik\feature.hpp>
#include <mapnik\projection.hpp>
#include <mapnik\featureset.hpp>
#include <mapnik\agg_renderer.hpp>
#include <mapnik\grid\grid.hpp>
#include <mapnik\grid\grid_renderer.hpp>
#include <mapnik\box2d.hpp>
#include <mapnik\scale_denominator.hpp>
#include <mapnik\util\geometry_to_geojson.hpp>
#include <mapnik\feature_kv_iterator.hpp>
#include <mapnik\json\geometry_generator_grammar.hpp>
#include <mapnik\json\properties_generator_grammar.hpp>
#include <mapnik\datasource_cache.hpp>

#include <google\protobuf\io\coded_stream.h>
#include "vector_tile_datasource.hpp"
#include "vector_tile_processor.hpp"
#include "vector_tile_backend_pbf.hpp"
#include "vector_tile_projection.hpp"
#include "vector_tile_util.hpp"
#include "vector_tile.pb.h"

#include <memory>

#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	template <typename PathType>
	double path_to_point_distance(PathType & path, double x, double y)
	{
		double x0 = 0;
		double y0 = 0;
		double distance = -1;
		path.rewind(0);
		MAPNIK_GEOM_TYPE geom_type = static_cast<MAPNIK_GEOM_TYPE>(path.type());
		switch (geom_type)
		{
		case MAPNIK_POINT:
		{
			unsigned command;
			bool first = true;
			while (mapnik::SEG_END != (command = path.vertex(&x0, &y0)))
			{
				if (command == mapnik::SEG_CLOSE) continue;
				if (first)
				{
					distance = mapnik::distance(x, y, x0, y0);
					first = false;
					continue;
				}
				double d = mapnik::distance(x, y, x0, y0);
				if (d < distance) distance = d;
			}
			return distance;
			break;
		}
		case MAPNIK_POLYGON:
		{
			double x1 = 0;
			double y1 = 0;
			bool inside = false;
			unsigned command = path.vertex(&x0, &y0);
			if (command == mapnik::SEG_END) return distance;
			while (mapnik::SEG_END != (command = path.vertex(&x1, &y1)))
			{
				if (command == mapnik::SEG_CLOSE) continue;
				if (command == mapnik::SEG_MOVETO)
				{
					x0 = x1;
					y0 = y1;
					continue;
				}
				if ((((y1 <= y) && (y < y0)) ||
					((y0 <= y) && (y < y1))) &&
					(x < (x0 - x1) * (y - y1) / (y0 - y1) + x1))
				{
					inside = !inside;
				}
				x0 = x1;
				y0 = y1;
			}
			return inside ? 0 : -1;
			break;
		}
		case MAPNIK_LINESTRING:
		{
			double x1 = 0;
			double y1 = 0;
			bool first = true;
			unsigned command = path.vertex(&x0, &y0);
			if (command == mapnik::SEG_END) return distance;
			while (mapnik::SEG_END != (command = path.vertex(&x1, &y1)))
			{
				if (command == mapnik::SEG_CLOSE) continue;
				if (command == mapnik::SEG_MOVETO)
				{
					x0 = x1;
					y0 = y1;
					continue;
				}
				if (first)
				{
					distance = mapnik::point_to_segment_distance(x, y, x0, y0, x1, y1);
					first = false;
				}
				else
				{
					double d = mapnik::point_to_segment_distance(x, y, x0, y0, x1, y1);
					if (d >= 0 && d < distance) distance = d;
				}
				x0 = x1;
				y0 = y1;
			}
			return distance;
			break;
		}
		default:
			return distance;
			break;
		}
		return distance;
	}


	VectorTile::VectorTile(System::Int32 z, System::Int32 x, System::Int32 y)
	{
		_tile = new vector_tile::Tile();
		_z = z;
		_x = x;
		_y = y;
		_width = 256;
		_height = 256;
		_painted = false;
	}

	VectorTile::VectorTile(System::Int32 z, System::Int32 x, System::Int32 y, System::Int32 width, System::Int32 height)
	{
		_tile = new vector_tile::Tile();
		_z = z;
		_x = x;
		_y = y;
		_width = width;
		_height = height;
		_painted = false;
	}

	VectorTile::~VectorTile()
	{
		if (_tile != NULL)
		{
			delete _tile;
		}
	}

	vector_tile::Tile *VectorTile::NativeObject()
	{
		return _tile;
	}

	void VectorTile::Painted(bool painted)
	{
		_painted = painted;
	}

	void VectorTile::Composite(System::Collections::Generic::IEnumerable<VectorTile^>^ vTiles)
	{
		Composite(vTiles, gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>());
	}

	void VectorTile::Composite(System::Collections::Generic::IEnumerable<VectorTile^>^ vTiles, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		// set defaults
		unsigned path_multiplier = 16;
		int buffer_size = 0;
		double scale_factor = 1.0;
		unsigned offset_x = 0;
		unsigned offset_y = 0;
		unsigned tolerance = 1;
		double scale_denominator = 0.0;

		// get options
		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		optionsParser->TryGet<int>("BufferSize", buffer_size);
		optionsParser->TryGet<double>("Scale", scale_factor);
		optionsParser->TryGet<double>("ScaleDenominator", scale_denominator);
		optionsParser->TryGet<unsigned>("OffsetX", offset_x);
		optionsParser->TryGet<unsigned>("OffsetY", offset_y);
		optionsParser->TryGet<unsigned>("Tolernace", tolerance);
		optionsParser->TryGet<unsigned>("PathMultiplier", path_multiplier);

		try
		{
			mapnik::box2d<double> max_extent(-20037508.34, -20037508.34, 20037508.34, 20037508.34);
			std::string merc_srs("+init=epsg:3857");

			for each (VectorTile^ vTile in vTiles)
			{
				//get native tile object
				vector_tile::Tile const *vt = vTile->NativeObject();

				if (_z == vTile->_z &&
					_x == vTile->_x &&
					_y == vTile->_y)
				{
					// merge 
					_tile->MergeFrom(*vt);
				}
				else
				{
					// set up to render to new vtile
					typedef mapnik::vector_tile_impl::backend_pbf backend_type;
					typedef mapnik::vector_tile_impl::processor<backend_type> renderer_type;
					vector_tile::Tile new_tiledata;
					backend_type backend(new_tiledata,
						path_multiplier);

					// get mercator extent of target tile
					mapnik::vector_tile_impl::spherical_mercator merc(_width);
					double minx, miny, maxx, maxy;
					merc.xyz(_x, _y, _z, minx, miny, maxx, maxy);
					mapnik::box2d<double> map_extent(minx, miny, maxx, maxy);
					// create request
					mapnik::request m_req(_width, _height, map_extent);
					m_req.set_buffer_size(buffer_size);
					// create map
					mapnik::Map map(_width, _height, merc_srs);
					map.set_maximum_extent(max_extent);

					// source tile properties
					unsigned x = vTile->_x;
					unsigned y = vTile->_y;
					unsigned z = vTile->_y;
					unsigned width = vTile->_width;

					unsigned num_layers = vt->layers_size();
					if (num_layers > 0)
					{
						for (int i = 0; i < vt->layers_size(); ++i)
						{
							vector_tile::Tile_Layer const& layer = vt->layers(i);
							mapnik::layer lyr(layer.name(), merc_srs);
							MAPNIK_SHARED_PTR<mapnik::vector_tile_impl::tile_datasource> ds = MAPNIK_MAKE_SHARED<
								mapnik::vector_tile_impl::tile_datasource>(
								layer,
								x,//vTile->_x,
								y,//vTile->_y,
								z,//vTile->_z,
								width//vTile->_width
								);
							ds->set_envelope(m_req.get_buffered_extent());
							lyr.set_datasource(ds);
							map.MAPNIK_ADD_LAYER(lyr);
						}
						renderer_type ren(backend,
							map,
							m_req,
							scale_factor,
							offset_x,
							offset_y,
							tolerance);
						ren.apply(scale_denominator);

						//merge
						_tile->MergeFrom(new_tiledata);
					}
				}
			}
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}

	System::Collections::Generic::IEnumerable<System::String^>^ VectorTile::Names()
	{
		System::Collections::Generic::List<System::String^>^ names = gcnew System::Collections::Generic::List<System::String^>();
		for (int i = 0; i < _tile->layers_size(); ++i)
		{
			vector_tile::Tile_Layer const& layer = _tile->layers(i);
			names->Add(msclr::interop::marshal_as<System::String^>(layer.name().c_str()));
		}
		return names;
	}

	System::Boolean VectorTile::Empty()
	{
		if (_tile->layers_size() == 0)
		{
			return true;
		}
		else
		{
			for (int i = 0; i < _tile->layers_size(); ++i)
			{
				vector_tile::Tile_Layer const& layer = _tile->layers(i);
				if (layer.features_size())
				{
					return false;
				}
			}
		}
		return true;
	}

	System::Int32 VectorTile::Width()
	{
		return _width;
	}

	System::Int32 VectorTile::Height()
	{
		return _height;
	}

	System::Boolean VectorTile::Painted()
	{
		return _painted;
	}

	System::Collections::Generic::IEnumerable<VectorQueryResult^>^ VectorTile::Query(double lon, double lat)
	{
		return Query(lon, lat, gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>());
	}

	System::Collections::Generic::IEnumerable<VectorQueryResult^>^ VectorTile::Query(double lon, double lat, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		double tolerance = 0.0; // meters
		std::string layer_name("");
		double x = lon;
		double y = lat;
		double z = 0;
		unsigned tile_x = _x;
		unsigned tile_y = _y;
		unsigned tile_z = _z;
		unsigned tile_w = _width;

		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		optionsParser->TryGet<double>("Tolerance", tolerance);
		System::String^ managedLayer;
		if (optionsParser->TryGet<System::String^>("Layer", managedLayer))
		{
			layer_name = msclr::interop::marshal_as<std::string>(managedLayer);
		}

		System::Collections::Generic::List<VectorQueryResult^>^ arr = gcnew System::Collections::Generic::List<VectorQueryResult^>();
		mapnik::projection wgs84("+init=epsg:4326", true);
		mapnik::projection merc("+init=epsg:3857", true);
		mapnik::proj_transform tr(wgs84, merc);

		if (!tr.forward(x, y, z))
		{
			throw gcnew System::Exception("could not reproject lon/lat to mercator");
		}
		mapnik::coord2d pt(x, y);
		if (!layer_name.empty())
		{
			int layer_idx = -1;
			for (int j = 0; j < _tile->layers_size(); ++j)
			{
				vector_tile::Tile_Layer const& layer = _tile->layers(j);
				if (layer_name == layer.name())
				{
					layer_idx = j;
					break;
				}
			}
			if (layer_idx > -1)
			{
				vector_tile::Tile_Layer const& layer = _tile->layers(layer_idx);
				MAPNIK_SHARED_PTR<mapnik::vector_tile_impl::tile_datasource> ds = MAPNIK_MAKE_SHARED<
					mapnik::vector_tile_impl::tile_datasource>(
					layer,
					tile_x,
					tile_y,
					tile_z,
					tile_w
					);
				mapnik::featureset_ptr fs = ds->features_at_point(pt, tolerance);
				if (fs)
				{
					mapnik::feature_ptr feature;
					while ((feature = fs->next()))
					{
						double distance = -1;
						for (mapnik::geometry_type const& geom : feature->paths())
						{
							double d = path_to_point_distance(geom, x, y);
							if (d >= 0)
							{
								if (distance >= 0)
								{
									if (d < distance) distance = d;
								}
								else
								{
									distance = d;
								}
							}
						}
						if (distance >= 0)
						{
							VectorQueryResult^ res = gcnew VectorQueryResult(
								msclr::interop::marshal_as<System::String^>(layer.name()),
								distance,
								gcnew Feature(feature)
							);
							arr->Add(res);
						}
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < _tile->layers_size(); ++i)
			{

				vector_tile::Tile_Layer const& layer = _tile->layers(i);
				MAPNIK_SHARED_PTR<mapnik::vector_tile_impl::tile_datasource> ds = MAPNIK_MAKE_SHARED<
					mapnik::vector_tile_impl::tile_datasource>(
					layer,
					tile_x,
					tile_y,
					tile_z,
					tile_w
					);
				mapnik::featureset_ptr fs = ds->features_at_point(pt, tolerance);
				if (fs)
				{
					mapnik::feature_ptr feature;
					while ((feature = fs->next()))
					{
						double distance = -1;
						for (mapnik::geometry_type const& geom : feature->paths())
						{
							double d = path_to_point_distance(geom, x, y);
							if (d >= 0)
							{
								if (distance >= 0)
								{
									if (d < distance) distance = d;
								}
								else
								{
									distance = d;
								}
							}
						}
						if (distance >= 0)
						{
							VectorQueryResult^ res = gcnew VectorQueryResult(
								msclr::interop::marshal_as<System::String^>(layer.name()),
								distance,
								gcnew Feature(feature)
							);
							arr->Add(res);
						}
					}
				}
			}
		}
		arr->Sort();
		return arr;
	}

	VectorTileJSON^ VectorTile::ToJSON()
	{
		System::Collections::Generic::List<VectorTileLayer^>^ arr = gcnew System::Collections::Generic::List<VectorTileLayer^>();
		for (int i = 0; i < _tile->layers_size(); ++i)
		{
			vector_tile::Tile_Layer const& layer = _tile->layers(i);
			System::Collections::Generic::List<VectorTileFeature^>^ f_arr = gcnew System::Collections::Generic::List<VectorTileFeature^>();
			for (int j = 0; j < layer.features_size(); ++j)
			{
				vector_tile::Tile_Feature const& f = layer.features(j);
				System::UInt64 id = 0;
				if (f.has_id())
				{
					id = f.id();
				}
				array<System::Byte>^ raster_bytes;
				if (f.has_raster())
				{
					std::string const& raster = f.raster();
					raster_bytes = gcnew array<System::Byte>(raster.size());
					System::Runtime::InteropServices::Marshal::Copy(System::IntPtr(raster[0]), raster_bytes, 0, raster.size());
				}
				int type = f.type();
				System::Collections::Generic::List<System::Int32>^ g_arr = gcnew System::Collections::Generic::List<System::Int32>();
				for (int k = 0; k < f.geometry_size(); ++k)
				{
					g_arr->Add(f.geometry(k));
				}
				System::Collections::Generic::Dictionary<System::String^, System::Object^>^ att_obj = gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>();
				for (int m = 0; m < f.tags_size(); m += 2)
				{
					std::size_t key_name = f.tags(m);
					std::size_t key_value = f.tags(m + 1);
					if (key_name < static_cast<std::size_t>(layer.keys_size())
						&& key_value < static_cast<std::size_t>(layer.values_size()))
					{
						System::String^ name = msclr::interop::marshal_as<System::String^>(layer.keys(key_name));
						vector_tile::Tile_Value const& value = layer.values(key_value);
						if (value.has_string_value())
						{
							att_obj->Add(name, msclr::interop::marshal_as<System::String^>(value.string_value()));
						}
						else if (value.has_int_value())
						{
							att_obj->Add(name, value.int_value());
						}
						else if (value.has_double_value())
						{
							att_obj->Add(name, value.double_value());
						}
						else if (value.has_float_value())
						{
							att_obj->Add(name, value.float_value());
						}
						else if (value.has_bool_value())
						{
							att_obj->Add(name, value.bool_value());
						}
						else if (value.has_sint_value())
						{
							att_obj->Add(name, value.sint_value());
						}
						else if (value.has_uint_value())
						{
							att_obj->Add(name, value.uint_value());
						}
						else
						{
							att_obj->Add(name, nullptr);
						}
					}
				}
				VectorTileFeature^ feature_obj = gcnew VectorTileFeature(
					id,
					raster_bytes,
					type,
					g_arr,
					att_obj
				);
				f_arr->Add(feature_obj);
			}
			VectorTileLayer^ layer_obj = gcnew VectorTileLayer(
				msclr::interop::marshal_as<System::String^>(layer.name()),
				layer.extent(),
				layer.version(),
				f_arr
			);
			arr->Add(layer_obj);
		}
		return gcnew VectorTileJSON(arr);
	}

	System::String^ VectorTile::ToGeoJSON(System::Int32 layer)
	{

		int layer_idx = layer;
		bool all_array = false;
		bool all_flattened = false;
		std::string error_msg;
		std::string result;
		try
		{
			unsigned layer_num = _tile->layers_size();
			if (layer_idx < 0) {
				std::ostringstream s;
				s << "Zero-based layer index '" << layer_idx << "' not valid"
					<< " must be a positive integer";
				if (layer_num > 0)
				{
					s << "only '" << layer_num << "' layers exist in map";
				}
				else
				{
					s << "no layers found in map";
				}
				error_msg = s.str();
			}
			else if (layer_idx >= static_cast<int>(layer_num)) {
				std::ostringstream s;
				s << "Zero-based layer index '" << layer_idx << "' not valid, ";
				if (layer_num > 0)
				{
					s << "only '" << layer_num << "' layers exist in map";
				}
				else
				{
					s << "no layers found in map";
				}
				error_msg = s.str();
			}
			if (!error_msg.empty())
			{
				throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(error_msg));
			}
			write_geojson_to_string(result, all_array, all_flattened, layer_idx);
		}
		catch (std::exception const& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
		return msclr::interop::marshal_as<System::String^>(result);
	}

	System::String^ VectorTile::ToGeoJSON(System::String^ layer)
	{
		int layer_idx = -1;
		bool all_array = false;
		bool all_flattened = false;
		std::string error_msg;
		std::string result;
		try
		{
			unsigned layer_num = _tile->layers_size();

			std::string layer_name = msclr::interop::marshal_as<std::string>(layer);
			if (layer_name == "__array__")
			{
				all_array = true;
			}
			else if (layer_name == "__all__")
			{
				all_flattened = true;
			}
			else
			{
				bool found = false;
				unsigned int idx(0);
				for (unsigned i = 0; i < layer_num; ++i)
				{
					vector_tile::Tile_Layer const& layer = _tile->layers(i);
					if (layer.name() == layer_name)
					{
						found = true;
						layer_idx = idx;
						break;
					}
					++idx;
				}
				if (!found)
				{
					std::ostringstream s;
					s << "Layer name '" << layer_name << "' not found";
					error_msg = s.str();
				}
			}
			if (!error_msg.empty())
			{
				throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(error_msg));
			}
			write_geojson_to_string(result, all_array, all_flattened, layer_idx);
		}
		catch (std::exception const& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
		return msclr::interop::marshal_as<System::String^>(result);
	}

	void VectorTile::write_geojson_to_string(std::string & result, bool array, bool all, int layer_idx)
	{
		if (array)
		{
			unsigned layer_num = _tile->layers_size();
			result += "[";
			bool first = true;
			for (unsigned i = 0; i<layer_num; ++i)
			{
				vector_tile::Tile_Layer const& layer = _tile->layers(i);
				if (first) first = false;
				else result += ",";
				result += "{\"type\":\"FeatureCollection\",";
				result += "\"name\":\"" + layer.name() + "\",\"features\":[";
				std::string features;
				bool hit = layer_to_geojson(layer, features, _x,_y,_z,_width);
				if (hit)
				{
					result += features;
				}
				result += "]}";
			}
			result += "]";
		}
		else
		{
			if (all)
			{
				result += "{\"type\":\"FeatureCollection\",\"features\":[";
				bool first = true;
				unsigned layer_num = _tile->layers_size();
				for (unsigned i = 0; i < layer_num; ++i)
				{
					vector_tile::Tile_Layer const& layer = _tile->layers(i);
					std::string features;
					bool hit = layer_to_geojson(layer, features, _x, _y, _z, _width);
					if (hit)
					{
						if (first) first = false;
						else result += ",";
						result += features;
					}
				}
				result += "]}";
			}
			else
			{
				vector_tile::Tile_Layer const& layer = _tile->layers(layer_idx);
				result += "{\"type\":\"FeatureCollection\",";
				result += "\"name\":\"" + layer.name() + "\",\"features\":[";
				layer_to_geojson(layer, result, _x, _y, _z, _width);
				result += "]}";
			}
		}
	}

	bool VectorTile::layer_to_geojson(vector_tile::Tile_Layer const& layer, std::string & result, unsigned x, unsigned y, unsigned z, unsigned width)
	{
		mapnik::vector_tile_impl::tile_datasource ds(layer,
			x,
			y,
			z,
			width,
			true);
		mapnik::projection wgs84("+init=epsg:4326", true);
		mapnik::projection merc("+init=epsg:3857", true);
		mapnik::proj_transform prj_trans(merc, wgs84);
		mapnik::query q(ds.envelope());
		mapnik::layer_descriptor ld = ds.get_descriptor();
		for (auto const& item : ld.get_descriptors())
		{
			q.add_property_name(item.get_name());
		}
		mapnik::featureset_ptr fs = ds.features(q);
		if (!fs)
		{
			return false;
		}
		else
		{
			using sink_type = std::back_insert_iterator<std::string>;
			static const mapnik::json::properties_generator_grammar<sink_type, mapnik::feature_impl> prop_grammar;
			static const mapnik::json::multi_geometry_generator_grammar<sink_type, proj_transform_container> proj_grammar;
			mapnik::feature_ptr feature;
			bool first = true;
			while ((feature = fs->next()))
			{
				if (first)
				{
					first = false;
				}
				else
				{
					result += "\n,";
				}
				result += "{\"type\":\"Feature\",\"geometry\":";
				if (feature->paths().empty())
				{
					result += "null";
				}
				else
				{
					std::string geometry;
					sink_type sink(geometry);
					proj_transform_container projected_paths;
					for (auto & geom : feature->paths())
					{
						projected_paths.push_back(new proj_transform_path_type(geom, prj_trans));
					}
					if (boost::spirit::karma::generate(sink, proj_grammar, projected_paths))
					{
						result += geometry;
					}
					else
					{
						throw std::runtime_error("Failed to generate GeoJSON geometry");
					}
				}
				result += ",\"properties\":";
				std::string properties;
				sink_type sink(properties);
				if (boost::spirit::karma::generate(sink, prop_grammar, *feature))
				{
					result += properties;
				}
				else
				{
					throw std::runtime_error("Failed to generate GeoJSON properties");
				}
				result += "}";
			}
			return !first;
		}
	}


	void VectorTile::AddGeoJSON(System::String^ json, System::String^ layer)
	{
		return AddGeoJSON(json, layer, gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>());
	}

	void VectorTile::AddGeoJSON(System::String^ json, System::String^ layer, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		std::string geojson_string = msclr::interop::marshal_as<std::string>(json);
		std::string geojson_name = msclr::interop::marshal_as<std::string>(layer);
		unsigned tolerance = 1;
		unsigned path_multiplier = 16;

		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		optionsParser->TryGet<unsigned>("Tolerance", tolerance);
		optionsParser->TryGet<unsigned>("PathMultiplier", path_multiplier);

		try
		{
			typedef mapnik::vector_tile_impl::backend_pbf backend_type;
			typedef mapnik::vector_tile_impl::processor<backend_type> renderer_type;
			backend_type backend(*_tile, path_multiplier);
			mapnik::Map map(_width, _height, "+init=epsg:3857");
			mapnik::vector_tile_impl::spherical_mercator merc(_width);
			double minx, miny, maxx, maxy;
			merc.xyz(_x, _y, _z, minx, miny, maxx, maxy);
			map.zoom_to_box(mapnik::box2d<double>(minx, miny, maxx, maxy));
			mapnik::request m_req(map.width(), map.height(), map.get_current_extent());
			m_req.set_buffer_size(8);
			mapnik::parameters p;
			// TODO - use mapnik core GeoJSON parser
			p["type"] = "ogr";
			p["file"] = geojson_string;
			p["layer_by_index"] = "0";
			mapnik::layer lyr(geojson_name, "+init=epsg:4326");
			lyr.set_datasource(mapnik::datasource_cache::instance().create(p));
			map.MAPNIK_ADD_LAYER(lyr);
			renderer_type ren(backend,
				map,
				m_req,
				1,
				0,
				0,
				tolerance);
			ren.apply();
			_painted = ren.painted();
		}
		catch (std::exception const& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}

	}

	void VectorTile::AddImage(array<System::Byte>^ bytes, System::String^ layer)
	{
		std::string layer_name = msclr::interop::marshal_as<std::string>(layer);

		// how to ensure buffer width/height?
		vector_tile::Tile_Layer * new_layer = _tile->add_layers();
		new_layer->set_name(layer_name);
		new_layer->set_version(1);
		new_layer->set_extent(256 * 16);
		// no need
		// current_feature_->set_id(feature.id());
		vector_tile::Tile_Feature * new_feature = new_layer->add_features();
		pin_ptr<unsigned char> pData = &bytes[0];
		std::string s = std::string(reinterpret_cast<char*>(pData), bytes->Length);
		new_feature->set_raster(s);
		// report that we have data
		_painted = true;
	}

	void VectorTile::AddData(array<System::Byte>^ data)
	{
		pin_ptr<unsigned char> pData = &data[0];
		google::protobuf::io::CodedInputStream input(
			reinterpret_cast<const google::protobuf::uint8*>(pData),
			data->Length
		);
		if (_tile->MergeFromCodedStream(&input))
		{
			_painted = true;
		}
		else
		{
			throw gcnew System::Exception("could not merge buffer as protobuf");
		}
	}

	array<System::Byte>^ VectorTile::GetData()
	{
		try
		{
			std::string s;
			_tile->SerializeToString(&s);
			array<System::Byte>^ data = gcnew array<System::Byte>(s.size());
			System::Runtime::InteropServices::Marshal::Copy(System::IntPtr(&s[0]), data, 0, s.size());
			return data;
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}

	void VectorTile::SetData(array<System::Byte>^ data)
	{
		if (data->Length > 0)
		{
			pin_ptr<unsigned char> pData = &data[0];
			std::string s = std::string(reinterpret_cast<char*>(pData), data->Length);
			if (_tile->ParseFromString(s))
			{
				_painted = true;
			}
			else
			{
				throw gcnew System::Exception("could not parse buffer as protobuf");
			}
		}
	}




	template <typename Renderer>
	void process_layers(Renderer & ren,
		mapnik::request const& m_req,
		mapnik::projection const& map_proj,
		std::vector<mapnik::layer> const& layers,
		double scale_denom,
		vector_tile::Tile const& tiledata,
		int z,
		int x,
		int y,
		unsigned tileSize,
		mapnik::box2d<double> const& map_extent)
	{

		// loop over layers in map and match by name
		// with layers in the vector tile
		unsigned layers_size = layers.size();
		for (unsigned i = 0; i < layers_size; ++i)
		{
			mapnik::layer const& lyr = layers[i];
			if (lyr.visible(scale_denom))
			{
				for (int j = 0; j < tiledata.layers_size(); ++j)
				{
					vector_tile::Tile_Layer  const& layer = tiledata.layers(j);
					if (lyr.name() == layer.name())
					{
						mapnik::layer lyr_copy(lyr);
						MAPNIK_SHARED_PTR<mapnik::vector_tile_impl::tile_datasource> ds = MAPNIK_MAKE_SHARED<
							mapnik::vector_tile_impl::tile_datasource>(
							layer,
							x,
							y,
							z,
							tileSize
							);
						ds->set_envelope(m_req.get_buffered_extent());
						lyr_copy.set_datasource(ds);
						std::set<std::string> names;
						ren.apply_to_layer(lyr_copy,
							ren,
							map_proj,
							m_req.scale(),
							scale_denom,
							m_req.width(),
							m_req.height(),
							m_req.extent(),
							m_req.buffer_size(),
							names);
					}
				}
			}
		}
	}

	void VectorTile::Render(Map^ map, Image^ image)
	{
		Render(map, image, gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>());
	}

	void VectorTile::Render(Map^ map, Image^ image, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		//Unwrap native objects
		map_ptr m = map->NativeObject();
		image_ptr i = image->NativeObject();

		// set defaults
		int z = 0;
		int x = 0;
		int y = 0;
		bool zxy_override = false;
		int buffer_size = 0;
		double scale_factor = 1.0;
		double scale_denominator = 0.0;
		auto variables = mapnik::attributes();

		// get options
		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		optionsParser->TryGet<int>("BufferSize", buffer_size);
		optionsParser->TryGet<double>("Scale", scale_factor);
		optionsParser->TryGet<double>("ScaleDenominator", scale_denominator);
		if (optionsParser->TryGet<int>("Z", z))
			zxy_override = true;
		if (optionsParser->TryGet<int>("X", x))
			zxy_override = true;
		if (optionsParser->TryGet<int>("Y", y))
			zxy_override = true;

		try
		{
			//get vtile extent
			mapnik::vector_tile_impl::spherical_mercator merc(_width);
			double minx, miny, maxx, maxy;
			if (zxy_override)
			{
				merc.xyz(x, y, z, minx, miny, maxx, maxy);
			}
			else
			{
				merc.xyz(_x, _y, _z, minx, miny, maxx, maxy);
			}
			mapnik::box2d<double> map_extent(minx, miny, maxx, maxy);

			//create request
			mapnik::request m_req(i->width(),i->height(), map_extent);
			m_req.set_buffer_size(buffer_size);

			//get map projection from map object
			mapnik::projection map_proj(m->srs(), true);

			double scale_denom = scale_denominator;
			if (scale_denom <= 0.0)
			{
				scale_denom = mapnik::scale_denominator(m_req.scale(), map_proj.is_geographic());
			}
			scale_denom *= scale_factor;

			//get map layers
			std::vector<mapnik::layer> const& layers = m->layers();

			//render
			mapnik::agg_renderer<mapnik::image_32> ren(
				*m,
				m_req,
				variables,
				*i,
				scale_factor
			);
			ren.start_map_processing(*m);
			process_layers(ren, m_req, map_proj, layers, scale_denom,*_tile,_z,_x,_y,_width, map_extent);
			ren.end_map_processing(*m);
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}

	void VectorTile::Render(Map^ map, Grid^ grid)
	{
		return Render(map, grid, gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>());
	}

	void VectorTile::Render(Map^ map, Grid^ grid, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		//Unwrap native objects
		map_ptr m = map->NativeObject();
		grid_ptr g = grid->NativeObject();

		// set defaults
		int z = 0;
		int x = 0;
		int y = 0;
		bool zxy_override = false;
		int buffer_size = 0;
		double scale_factor = 1.0;
		double scale_denominator = 0.0;
		auto variables = mapnik::attributes();
		std::size_t layer_idx = 0;

		// get options
		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		optionsParser->TryGet<int>("BufferSize", buffer_size);
		optionsParser->TryGet<double>("Scale", scale_factor);
		optionsParser->TryGet<double>("ScaleDenominator", scale_denominator);
		if (optionsParser->TryGet<int>("Z", z))
			zxy_override = true;
		if (optionsParser->TryGet<int>("X", x))
			zxy_override = true;
		if (optionsParser->TryGet<int>("Y", y))
			zxy_override = true;
		std::vector<mapnik::layer> const& layers = m->layers();
		System::String^ layer;
		if (optionsParser->TryGet<System::String^>("Layer", layer))
		{
			bool found = false;
			unsigned int idx(0);
			std::string const& layer_name = msclr::interop::marshal_as<std::string>(layer);
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
				throw gcnew System::ArgumentException("Layer name " + layer + " not found", "Layer");
			}
		}
		else
		{
			throw gcnew System::ArgumentNullException("Layer option is required");
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
			//get vtile extent
			mapnik::vector_tile_impl::spherical_mercator merc(_width);
			double minx, miny, maxx, maxy;
			if (zxy_override)
			{
				merc.xyz(x, y, z, minx, miny, maxx, maxy);
			}
			else
			{
				merc.xyz(_x, _y, _z, minx, miny, maxx, maxy);
			}
			mapnik::box2d<double> map_extent(minx, miny, maxx, maxy);

			//create request
			mapnik::request m_req(g->width(), g->height(), map_extent);
			m_req.set_buffer_size(buffer_size);

			//get map projection from map object
			mapnik::projection map_proj(m->srs(), true);

			double scale_denom = scale_denominator;
			if (scale_denom <= 0.0)
			{
				scale_denom = mapnik::scale_denominator(m_req.scale(), map_proj.is_geographic());
			}
			scale_denom *= scale_factor;

			//get map layers
			std::vector<mapnik::layer> const& layers = m->layers();

			//render
			mapnik::grid_renderer<mapnik::grid> ren(
				*m,
				m_req,
				variables,
				*g,
				scale_factor);
			ren.start_map_processing(*m);

			mapnik::layer const& lyr = layers[layer_idx];
			if (lyr.visible(scale_denom))
			{
				int layer_idx = -1;
				for (int j = 0; j < _tile->layers_size(); ++j)
				{
					vector_tile::Tile_Layer const& layer = _tile->layers(j);
					if (lyr.name() == layer.name())
					{
						layer_idx = j;
						break;
					}
				}
				if (layer_idx > -1)
				{
					vector_tile::Tile_Layer const& layer = _tile->layers(layer_idx);
					if (layer.features_size() <= 0)
					{
						return;
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

					unsigned tile_x = _x;
					unsigned tile_y = _y;
					unsigned tile_z = _z;
					unsigned tile_w = _width;
					mapnik::layer lyr_copy(lyr);
					MAPNIK_SHARED_PTR<mapnik::vector_tile_impl::tile_datasource> ds = MAPNIK_MAKE_SHARED<
						mapnik::vector_tile_impl::tile_datasource>(
						layer,
						tile_x,
						tile_y,
						tile_z,
						tile_w
					);
					ds->set_envelope(m_req.get_buffered_extent());
					lyr_copy.set_datasource(ds);
					ren.apply_to_layer(lyr_copy,
						ren,
						map_proj,
						m_req.scale(),
						scale_denom,
						m_req.width(),
						m_req.height(),
						m_req.extent(),
						m_req.buffer_size(),
						attributes);
				}
				ren.end_map_processing(*m);
			}
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}


	}

	void VectorTile::Clear()
	{
		_tile->Clear();
		_painted = false;
	}

	System::String^ VectorTile::IsSolid()
	{
		try
		{
			std::string key;
			bool is_solid = mapnik::vector_tile_impl::is_solid_extent(*_tile, key);
			if (is_solid)
			{
				return msclr::interop::marshal_as<System::String^>(key);
			}
			else
			{
				return nullptr;
			}
		}
		catch (std::exception const& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
	}


}