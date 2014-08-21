#include "stdafx.h"
#include "mapnik_vector_tile.h"
#include "mapnik_map.h"
#include "mapnik_image.h"

#include <mapnik\map.hpp>
#include <mapnik\graphics.hpp>
#include <mapnik\request.hpp>
#include <mapnik\agg_renderer.hpp>

#include "vector_tile_datasource.hpp"

namespace NETMapnik
{
	VectorTile::VectorTile()
	{
		_tile = new mapnik::vector::tile();

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

		int z = 0;
		int y = 0;
		int x = 0;
		int buffer_size = 0;
		double scale = 1.0;
		double scale_denom = 0;

		mapnik::projection map_proj(m->srs(), true);
		std::vector<mapnik::layer> layers = m->layers();

		mapnik::request m_req(m->width(), m->height(),m->get_current_extent());
		mapnik::agg_renderer<mapnik::image_32> ren(*m, m_req, *i, 1.0, 0U, 0U);
		ren.start_map_processing(*m);
		process_layers(ren, m_req, map_proj, layers, scale_denom, *_tile, m->get_current_extent());
		ren.end_map_processing(*m);

	}

	template <typename Renderer>
	void process_layers(Renderer & ren,
		mapnik::request const& m_req,
		mapnik::projection const& map_proj,
		std::vector<mapnik::layer> const& layers,
		double scale_denom,
		mapnik::vector::tile const& tiledata,
//		vector_tile_render_baton_t *closure,
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
							0,//closure->d->x_,
							0,//closure->d->y_,
							0,//closure->d->z_,
							256//closure->d->width()
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