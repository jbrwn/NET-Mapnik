#include "stdafx.h"
#include "mapnik_grid_view.h"
#include "mapnik_grid.h"
#include "NET_grid_utils.h"
#include "NET_options_parser.h"

#include <mapnik\grid\grid.hpp>
#include <mapnik\grid\grid_view.hpp> 

#include <memory>

#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	GridView::GridView(Grid^ grid, unsigned x, unsigned y, unsigned w, unsigned h)
	{
		grid_ptr p = grid->NativeObject();
		_grid_view = new grid_view_ptr(std::make_shared<mapnik::grid_view>(p->get_view(x, y, w, h)));
	}

	GridView::~GridView()
	{
		if (_grid_view != NULL)
		{
			delete _grid_view;
		}
	}

	grid_view_ptr GridView::NativeObject()
	{
		return *_grid_view;
	}

	System::Boolean GridView::IsSolid()
	{
		if ((*_grid_view)->width() > 0 && (*_grid_view)->height() > 0)
		{
			mapnik::grid_view::value_type first_pixel = (*_grid_view)->getRow(0)[0];
			for (unsigned y = 0; y < (*_grid_view)->height(); ++y)
			{
				mapnik::grid_view::value_type const * row = (*_grid_view)->getRow(y);
				for (unsigned x = 0; x < (*_grid_view)->width(); ++x)
				{
					if (first_pixel != row[x])
					{
						return false;
					}
				}
			}
		}
		return true;
	}


	System::Int64 GridView::GetPixel(System::Int32 x, System::Int32 y)
	{
		if (x < (*_grid_view)->width() && y < (*_grid_view)->height())
		{
			mapnik::grid_view::value_type pixel = (*_grid_view)->getRow(y)[x];
			return pixel;
		}
		throw gcnew System::ArgumentOutOfRangeException("Pixel coordinate out of range");
	}

	System::Int32 GridView::Width()
	{
		return (*_grid_view)->width();
	}
	System::Int32 GridView::Height()
	{
		return (*_grid_view)->height();
	}

	System::Collections::Generic::Dictionary<System::String^, System::Object^>^ GridView::Encode()
	{
		return Encode(gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>());
	}

	System::Collections::Generic::Dictionary<System::String^, System::Object^>^ GridView::Encode(System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		// defaults
		std::string format("utf");
		unsigned int resolution = 4;
		bool add_features = true;

		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		optionsParser->TryGetUInt32("Resolution", resolution);
		optionsParser->TryGetBoolean("AddFeatures", add_features);
		optionsParser->TryGetString("Format", format);

		return grid_encode(*(*_grid_view), format, add_features, resolution);
	}
}
