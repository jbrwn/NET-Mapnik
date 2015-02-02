#include "StdAfx.h"
#include "mapnik_grid.h"
#include "mapnik_grid_view.h"
#include "NET_grid_utils.h"
#include "NET_options_parser.h"

#include <memory>

// mapnik
#include <mapnik\grid\grid.hpp>

// microsoft
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	Grid::Grid(System::Int32 width, System::Int32 height)
	{
		_grid = new grid_ptr(std::make_shared<mapnik::grid>(width, height, "__id__", 1));
	}

	Grid::Grid(System::Int32 width, System::Int32 height, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		// defaults
		std::string key = "__id__";
		int resolution = 1;

		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		optionsParser->TryGetString("Key", key);
		optionsParser->TryGetInt32("Resolution", resolution);
		_grid = new grid_ptr(std::make_shared<mapnik::grid>(width, height, key, resolution));
	}

	Grid::~Grid()
	{
		if (_grid != NULL)
		{
			delete _grid;
		}
	}

	grid_ptr Grid::NativeObject()
	{
		return *_grid;
	}

	System::String^ Grid::Key::get()
	{
		return msclr::interop::marshal_as<System::String^>((*_grid)->get_key());
	}

	void Grid::Key::set(System::String^ value)
	{
		(*_grid)->set_key(msclr::interop::marshal_as<std::string>(value));
	}

	System::Int64 Grid::BaseMask::get()
	{
		return mapnik::grid::base_mask;
	}

	void Grid::Clear()
	{
		(*_grid)->clear();
	}

	System::Boolean Grid::Painted()
	{
		return (*_grid)->painted();
	}

	System::Int32 Grid::Width()
	{
		return (*_grid)->width();
	}

	System::Int32 Grid::Height()
	{
		return (*_grid)->height();
	}

	System::Collections::Generic::IEnumerable<System::String^>^ Grid::Fields()
	{
		std::set<std::string> const& a = (*_grid)->property_names();
		std::set<std::string>::const_iterator itr = a.begin();
		std::set<std::string>::const_iterator end = a.end();
		System::Collections::Generic::List<System::String^>^ l = gcnew System::Collections::Generic::List<System::String^>();
		int idx = 0;
		for (; itr != end; ++itr)
		{
			std::string name = *itr;
			l->Add(msclr::interop::marshal_as<System::String^>(name));
			++idx;
		}
		return l;
	}

	GridView^ Grid::View(System::Int32 x, System::Int32 y, System::Int32 w, System::Int32 h)
	{
		return gcnew GridView(this, x, y, w, h);
	}

	System::Collections::Generic::Dictionary<System::String^, System::Object^>^ Grid::Encode()
	{
		return Encode(gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>());
	}

	System::Collections::Generic::Dictionary<System::String^, System::Object^>^ Grid::Encode(System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		// defaults
		std::string format("utf");
		unsigned int resolution = 4;
		bool add_features = true;

		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		optionsParser->TryGetUInt32("Resolution", resolution);
		optionsParser->TryGetBoolean("AddFeatures", add_features);
		optionsParser->TryGetString("Format", format);

		return grid_encode(*(*_grid), format, add_features, resolution);
	}
}