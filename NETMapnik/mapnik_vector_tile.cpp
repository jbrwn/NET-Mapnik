#include "stdafx.h"
#include "mapnik_vector_tile.h"
#include "mapnik_map.h"
#include "mapnik_image.h"
#include "NET_options_parser.h"

#include <mapnik\map.hpp>
#include <mapnik\graphics.hpp>
#include <mapnik\request.hpp>
#include <mapnik\agg_renderer.hpp>
#include <mapnik\scale_denominator.hpp>

#include "vector_tile_datasource.hpp"
#include "vector_tile_processor.hpp"
#include "vector_tile_backend_pbf.hpp"

#include <msclr\marshal_cppstd.h>

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
	}

	VectorTile::VectorTile(System::Int32 z, System::Int32 x, System::Int32 y, System::UInt32 width, System::UInt32 height)
	{
		_tile = new vector_tile::Tile();
		_z = z;
		_x = x;
		_y = y;
		_width = width;
		_height = height;
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

	array<System::Byte>^ VectorTile::GetBytes()
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

	void VectorTile::SetBytes(array<System::Byte>^ data)
	{
		if (data->Length > 0)
		{
			try
			{
				pin_ptr<unsigned char> pData = &data[0];
				std::string s = std::string(reinterpret_cast<char*>(pData), data->Length);
				_tile->ParseFromString(s);
			}
			catch (const std::exception& ex)
			{
				System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
				throw gcnew System::Exception(managedException);
			}
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
		//See - https://github.com/mapnik/node-mapnik/blob/master/src/mapnik_vector_tile.cpp

		//Unwrap native objects
		mapnik::Map *m = map->NativeObject();
		mapnik::image_32 *i = image->NativeObject();

		// set defaults
		int z = 0;
		int x = 0;
		int y = 0;
		bool zxy_override = false;
		int buffer_size = 0;
		double scale_factor = 1.0;
		double scale_denominator = 0.0;
		
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
}