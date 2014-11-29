#include "StdAfx.h"
#include "mapnik_grid.h"
#include "NET_grid_utils.h"

// mapnik
#include <mapnik\grid\grid.hpp>

// microsoft
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	Grid::Grid(System::UInt32 width, System::UInt32 height)
	{
		_grid = new mapnik::grid(width,height,"__id__", 1);
	}

	Grid::~Grid()
	{
		if (_grid != NULL)
		{
			delete _grid;
		}
	}

	mapnik::grid *Grid::NativeObject()
	{
		return _grid;
	}

	System::Collections::Generic::Dictionary<System::String^, System::Object^>^ Grid::Encode(System::String^ format, System::Boolean addFeatures, System::UInt32 resolution)
	{
		std::string unmanagedFormat = msclr::interop::marshal_as<std::string>(format);
		return grid_encode(*_grid,unmanagedFormat,addFeatures,resolution);
	}
}