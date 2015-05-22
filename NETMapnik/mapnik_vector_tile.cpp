#include "stdafx.h"
#include "mapnik_vector_tile.h"
#include "mapnik_map.h"
#include "mapnik_image.h"
#include "mapnik_grid.h"
#include "mapnik_feature.h"
#include "NET_options_parser.h"

#include <mapnik\map.hpp>
#include <mapnik\layer.hpp>
#include <mapnik\geom_util.hpp>
#include <mapnik\version.hpp>
#include <mapnik\request.hpp>
#include <mapnik\image_any.hpp>
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
#include <mapnik\datasource_cache.hpp>
#include <mapnik\util\feature_to_geojson.hpp>
#include <mapnik\geometry_reprojection.hpp>
#include <mapnik\hit_test_filter.hpp>

#include <google\protobuf\io\coded_stream.h>
#include "vector_tile_datasource.hpp"
#include "vector_tile_processor.hpp"
#include "vector_tile_backend_pbf.hpp"
#include "vector_tile_projection.hpp"
#include "vector_tile_util.hpp"
#include "vector_tile.pb.h"

#include <memory>

#include <msclr\marshal_cppstd.h>

namespace detail {

	struct p2p_distance
	{
		p2p_distance(double x, double y)
			: x_(x),
			y_(y) {}

		double operator() (mapnik::geometry::geometry_empty const&) const
		{
			return -1;
		}

		double operator() (mapnik::geometry::point<double> const& geom) const
		{
			return mapnik::distance(geom.x, geom.y, x_, y_);
		}
		double operator() (mapnik::geometry::multi_point<double> const& geom) const
		{
			double distance = -1;
			for (auto const& pt : geom)
			{
				double dist = operator()(pt);
				if (dist >= 0 && (distance < 0 || dist < distance)) distance = dist;
			}
			return distance;
		}
		double operator() (mapnik::geometry::line_string<double> const& geom) const
		{
			double distance = -1;
			std::size_t num_points = geom.num_points();
			if (num_points > 1)
			{
				for (std::size_t i = 1; i < num_points; ++i)
				{
					auto const& pt0 = geom[i - 1];
					auto const& pt1 = geom[i];
					double dist = mapnik::point_to_segment_distance(x_, y_, pt0.x, pt0.y, pt1.x, pt1.y);
					if (dist >= 0 && (distance < 0 || dist < distance)) distance = dist;
				}
			}
			return distance;
		}
		double operator() (mapnik::geometry::multi_line_string<double> const& geom) const
		{
			double distance = -1;
			for (auto const& line : geom)
			{
				double dist = operator()(line);
				if (dist >= 0 && (distance < 0 || dist < distance)) distance = dist;
			}
			return distance;
		}
		double operator() (mapnik::geometry::polygon<double> const& geom) const
		{
			auto const& exterior = geom.exterior_ring;
			std::size_t num_points = exterior.num_points();
			if (num_points < 4) return -1;
			bool inside = false;
			for (std::size_t i = 1; i < num_points; ++i)
			{
				auto const& pt0 = exterior[i - 1];
				auto const& pt1 = exterior[i];
				// todo - account for tolerance
				if (mapnik::detail::pip(pt0.x, pt0.y, pt1.x, pt1.y, x_, y_))
				{
					inside = !inside;
				}
			}
			if (!inside) return -1;
			for (auto const& ring : geom.interior_rings)
			{
				std::size_t num_interior_points = ring.size();
				if (num_interior_points < 4)
				{
					continue;
				}
				for (std::size_t j = 1; j < num_interior_points; ++j)
				{
					auto const& pt0 = ring[j - 1];
					auto const& pt1 = ring[j];
					if (mapnik::detail::pip(pt0.x, pt0.y, pt1.x, pt1.y, x_, y_))
					{
						inside = !inside;
					}
				}
			}
			return inside ? 0 : -1;
		}
		double operator() (mapnik::geometry::multi_polygon<double> const& geom) const
		{
			double distance = -1;
			for (auto const& poly : geom)
			{
				double dist = operator()(poly);
				if (dist >= 0 && (distance < 0 || dist < distance)) distance = dist;
			}
			return distance;
		}
		double operator() (mapnik::geometry::geometry_collection<double> const& collection) const
		{
			double distance = -1;
			for (auto const& geom : collection)
			{
				double dist = mapnik::util::apply_visitor((*this), geom);
				if (dist >= 0 && (distance < 0 || dist < distance)) distance = dist;
			}
			return distance;
		}

		double x_;
		double y_;
	};

}

double path_to_point_distance(mapnik::geometry::geometry<double> const& geom, double x, double y)
{
	return mapnik::util::apply_visitor(detail::p2p_distance(x, y), geom);
}

namespace NETMapnik
{
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
		int buffer_size = 1;
		double scale_factor = 1.0;
		unsigned offset_x = 0;
		unsigned offset_y = 0;
		double area_threshold = 0.1;
		double scale_denominator = 0.0;

		// get options
		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		optionsParser->TryGetInt32("BufferSize", buffer_size);
		optionsParser->TryGetDouble("Scale", scale_factor);
		optionsParser->TryGetDouble("ScaleDenominator", scale_denominator);
		optionsParser->TryGetUInt32("OffsetX", offset_x);
		optionsParser->TryGetUInt32("OffsetY", offset_y);
		optionsParser->TryGetDouble("AreaThreshold", area_threshold);
		optionsParser->TryGetUInt32("PathMultiplier", path_multiplier);

		try
		{
			vector_tile::Tile new_tiledata;
			std::string merc_srs("+init=epsg:3857");
			mapnik::box2d<double> max_extent(-20037508.34, -20037508.34, 20037508.34, 20037508.34);
			if (_width <= 0 || _height <= 0)
			{
				throw gcnew System::ArgumentOutOfRangeException("Vector tile width and height must be great than zero");
			}
			for each (VectorTile^ vt in vTiles)
			{
				if (vt->Width() <= 0 || vt->Height() <= 0)
				{
					throw gcnew System::ArgumentOutOfRangeException("Vector tile width and height must be great than zero");
				}

				if (_z == vt->_z &&
					_x == vt->_x &&
					_y == vt->_y)
				{
					// merge 
					_tile->MergeFrom(*vt->NativeObject());
				}
				else
				{
					new_tiledata.Clear();
					// set up to render to new vtile
					typedef mapnik::vector_tile_impl::backend_pbf backend_type;
					typedef mapnik::vector_tile_impl::processor<backend_type> renderer_type;
					backend_type backend(new_tiledata, path_multiplier);

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
					unsigned x = vt->_x;
					unsigned y = vt->_y;
					unsigned z = vt->_y;
					unsigned width = vt->_width;

					vector_tile::Tile const& tiledata = *vt->NativeObject();
					unsigned num_layers = tiledata.layers_size();
					if (num_layers > 0)
					{
						for (int i = 0; i < tiledata.layers_size(); ++i)
						{
							vector_tile::Tile_Layer const& layer = tiledata.layers(i);
							mapnik::layer lyr(layer.name(), merc_srs);
							std::shared_ptr<mapnik::vector_tile_impl::tile_datasource> ds = std::make_shared<
								mapnik::vector_tile_impl::tile_datasource>(
								layer,
								x,//vt->_x,
								y,//vt->_y,
								z,//vt->_z,
								width//vt->_width
								);
							ds->set_envelope(m_req.get_buffered_extent());
							lyr.set_datasource(ds);
							map.add_layer(lyr);
						}
						renderer_type ren(backend,
							map,
							m_req,
							scale_factor,
							offset_x,
							offset_y,
							area_threshold);
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
			names->Add(msclr::interop::marshal_as<System::String^>(layer.name()));
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
		if (_width <= 0 || _height <= 0)
		{
			throw gcnew System::ArgumentOutOfRangeException("Can not query a vector tile with width or height of zero");
		}

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
		optionsParser->TryGetDouble("Tolerance", tolerance);
		optionsParser->TryGetString("Layer", layer_name);

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
				std::shared_ptr<mapnik::vector_tile_impl::tile_datasource> ds = std::make_shared<
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
						auto const& geom = feature->get_geometry();
						double distance = path_to_point_distance(geom, x, y);
						if (distance >= 0 && distance <= tolerance)
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
				std::shared_ptr<mapnik::vector_tile_impl::tile_datasource> ds = std::make_shared<
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
						auto const& geom = feature->get_geometry();
						double distance = path_to_point_distance(geom, x, y);
						if (distance >= 0 && distance <= tolerance)
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

	System::Collections::Generic::IEnumerable<VectorTileLayer^>^ VectorTile::ToJSON()
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
		return arr->AsReadOnly();
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
			width);
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
		bool first = true;
		if (fs)
		{
			mapnik::feature_ptr feature;
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
				std::string feature_str;
				mapnik::feature_impl feature_new(feature->context(), feature->id());
				feature_new.set_data(feature->get_data());
				unsigned int n_err = 0;
				feature_new.set_geometry(mapnik::geometry::reproject_copy(feature->get_geometry(), prj_trans, n_err));
				if (!mapnik::util::to_geojson(feature_str, feature_new))
				{
					throw std::runtime_error("Failed to generate GeoJSON geometry");
				}
				result += feature_str;
			}
		}
		return !first;
	}


	void VectorTile::AddGeoJSON(System::String^ json, System::String^ layer)
	{
		return AddGeoJSON(json, layer, gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>());
	}

	void VectorTile::AddGeoJSON(System::String^ json, System::String^ layer, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		std::string geojson_string = msclr::interop::marshal_as<std::string>(json);
		std::string geojson_name = msclr::interop::marshal_as<std::string>(layer);
		double area_threshold = 0.1;
		double simplify_distance = 0.0;
		unsigned path_multiplier = 16;

		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		optionsParser->TryGetDouble("AreaThreshold", area_threshold);
		optionsParser->TryGetDouble("SimplifyDistance", simplify_distance);
		optionsParser->TryGetUInt32("PathMultiplier", path_multiplier);

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
			p["type"] = "geojson";
			p["inline"] = geojson_string;
			mapnik::layer lyr(geojson_name, "+init=epsg:4326");
			lyr.set_datasource(mapnik::datasource_cache::instance().create(p));
			map.add_layer(lyr);
			renderer_type ren(backend,
				map,
				m_req,
				1,
				0,
				0,
				area_threshold);
			ren.set_simplify_distance(simplify_distance);
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
						std::shared_ptr<mapnik::vector_tile_impl::tile_datasource> ds = std::make_shared<
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
		optionsParser->TryGetInt32("BufferSize", buffer_size);
		optionsParser->TryGetDouble("Scale", scale_factor);
		optionsParser->TryGetDouble("ScaleDenominator", scale_denominator);
		if (optionsParser->TryGetInt32("Z", z))
			zxy_override = true;
		if (optionsParser->TryGetInt32("X", x))
			zxy_override = true;
		if (optionsParser->TryGetInt32("Y", y))
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
			if (i->is<mapnik::image_rgba8>())
			{
				mapnik::image_rgba8 & im_data = mapnik::util::get<mapnik::image_rgba8>(*i);
				mapnik::agg_renderer<mapnik::image_rgba8> ren(
					*m,
					m_req,
					variables,
					im_data,
					scale_factor
				);
				ren.start_map_processing(*m);
				process_layers(ren, m_req, map_proj, layers, scale_denom, *_tile, _z, _x, _y, _width, map_extent);
				ren.end_map_processing(*m);
			}
			else
			{
				throw gcnew System::Exception("This image type is not currently supported for rendering.");
			}
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
		optionsParser->TryGetInt32("BufferSize", buffer_size);
		optionsParser->TryGetDouble("Scale", scale_factor);
		optionsParser->TryGetDouble("ScaleDenominator", scale_denominator);
		if (optionsParser->TryGetInt32("Z", z))
			zxy_override = true;
		if (optionsParser->TryGetInt32("X", x))
			zxy_override = true;
		if (optionsParser->TryGetInt32("Y", y))
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
				g->add_field(msclr::interop::marshal_as<std::string>(name));
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
					std::set<std::string> attributes = g->get_fields();
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
					std::shared_ptr<mapnik::vector_tile_impl::tile_datasource> ds = std::make_shared<
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