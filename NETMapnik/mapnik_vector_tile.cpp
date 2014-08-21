#include "stdafx.h"
#include "mapnik_vector_tile.h"
#include "mapnik_map.h"
#include "mapnik_image.h"

#include <mapnik\map.hpp>
#include <mapnik\graphics.hpp>
#include <mapnik\request.hpp>
#include <mapnik\agg_renderer.hpp>
#include <mapnik\scale_denominator.hpp>

#include "vector_tile_datasource.hpp"

namespace NETMapnik
{
	VectorTile::VectorTile(System::Int32 Z, System::Int32 X, System::Int32 Y, System::UInt32 Width, System::UInt32 Height)
	{
		_tile = new mapnik::vector::tile();
		_z = Z;
		_x = X;
		_y = Y;
		_width = Width;
		_height = Height;

	}

	VectorTile::~VectorTile()
	{
		if (_tile != NULL)
		{
			delete _tile;
		}
	}

	mapnik::vector::tile *VectorTile::NativeObject()
	{
		return _tile;
	}

	array<System::Byte>^ VectorTile::GetBytes()
	{
		std::string s;
		_tile->SerializeToString(&s);
		array<System::Byte>^ data = gcnew array<System::Byte>(s.size());
		System::Runtime::InteropServices::Marshal::Copy(System::IntPtr(&s[0]), data, 0, s.size());
		return data;
	}

	void VectorTile::SetBytes(array<System::Byte>^ data)
	{
		pin_ptr<unsigned char> pData = &data[0];
		std::string s = std::string(reinterpret_cast<char*>(pData), data->Length);
		_tile->ParseFromString(s);
	}

	void VectorTile::Render(Map^ map, Image^ image)
	{
		//Unwrap native objects
		mapnik::Map *m = map->NativeObject();
		mapnik::image_32 *i = image->NativeObject();

		/*
		z
		x
		y
		
		layer_idx = 0?
		i->width (closure->width)
		i->height (closure->height)
		*/
		unsigned offset_x = 0;
		unsigned offset_y = 0;
		int buffer_size = 0;
		double scale_factor = 1.0; //closure->scale_factor
		double scale_denom = 0.0; //closure->scale_denominator

		//get vtile extent
		mapnik::vector::spherical_mercator merc(_width);
		double minx, miny, maxx, maxy;
		merc.xyz(_x, _y, _z, minx, miny, maxx, maxy);
		mapnik::box2d<double> map_extent(minx, miny, maxx, maxy);

		//create request
		// use width and height from image and extent from vector tile
		mapnik::request m_req(i->width(),i->height(), map_extent);
		m_req.set_buffer_size(buffer_size);

		//get map projection from map object
		mapnik::projection map_proj(m->srs(), true);

		//set scale denominator?
		if (scale_denom <= 0.0)
		{
			scale_denom = mapnik::scale_denominator(m_req.scale(), map_proj.is_geographic());
		}
		scale_denom *= scale_factor;

		//get map layers
		std::vector<mapnik::layer> const& layers = m->layers();


		//start rendering
		mapnik::agg_renderer<mapnik::image_32> ren(*m,m_req,*i,scale_factor,offset_x,offset_y);
		ren.start_map_processing(*m);
		process_layers(ren, m_req, map_proj, layers, scale_denom,*_tile,_z,_x,_y,_width, map_extent);
		ren.end_map_processing(*m);

	}

	template <typename Renderer>
	void process_layers(Renderer & ren,
		mapnik::request const& m_req,
		mapnik::projection const& map_proj,
		std::vector<mapnik::layer> const& layers,
		double scale_denom,
		mapnik::vector::tile const& tiledata,
		//vector_tile_render_baton_t *closure,
		//VectorTile^ vTile,
		int z,
		int x,
		int y,
		unsigned width,
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
					mapnik::vector::tile_layer const& layer = tiledata.layers(j);
					if (lyr.name() == layer.name())
					{
						mapnik::layer lyr_copy(lyr);
						MAPNIK_SHARED_PTR<mapnik::vector::tile_datasource> ds = MAPNIK_MAKE_SHARED<
							mapnik::vector::tile_datasource>(
							layer,
							x,//closure->d->x_,
							y,//closure->d->y_,
							z,//closure->d->z_,
							width//closure->d->width()
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



}