#include "stdafx.h"
#include "map.h"

#include <mapnik\map.hpp>
#include <mapnik\load_map.hpp>
#include <mapnik\agg_renderer.hpp>
#include <mapnik\image_util.hpp>

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
}
