#include "stdafx.h"
#include "mapnik_image_view.h"
#include "mapnik_image.h"
#include "mapnik_color.h"
#include "mapnik_palette.h"

#include <memory>

#include <mapnik\color.hpp>   
#include <mapnik\image_view.hpp> 
#include <mapnik\image_view_any.hpp>
#include <mapnik\image_util.hpp>

#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	ImageView::ImageView(Image^ image, unsigned x, unsigned y, unsigned w, unsigned h)
	{
		image_ptr p = image->NativeObject();
		_img_view = new image_view_ptr(std::make_shared<mapnik::image_view_any>(mapnik::create_view(*p,x,y,w,h)));
	}

	ImageView::~ImageView()
	{
		if (_img_view != NULL)
		{
			delete _img_view;
		}
	}

	image_view_ptr ImageView::NativeObject()
	{
		return *_img_view;
	}

	System::Boolean ImageView::IsSolid()
	{
		System::Boolean b = false;
		if ((*_img_view)->width() > 0 && (*_img_view)->height() > 0)
		{
			b = mapnik::is_solid(*(*_img_view));
		}
		return b;
	}


	Color^ ImageView::GetPixel(System::Int32 x, System::Int32 y)
	{
		if (x >= 0 && x < static_cast<int>((*_img_view)->width())
			&& y >= 0 && y < static_cast<int>((*_img_view)->height()))
		{
			mapnik::color val = mapnik::get_pixel<mapnik::color>(*(*_img_view), x, y);
			return gcnew Color(val);
		}
		return nullptr;
	}

	System::Int32 ImageView::Width()
	{
		return (*_img_view)->width();
	}
	System::Int32 ImageView::Height()
	{
		return (*_img_view)->height();
	}
	
	array<System::Byte>^ ImageView::Encode(System::String^ format)
	{
		std::string unmanagedFormat = msclr::interop::marshal_as<std::string>(format);

		try
		{
			std::string s = save_to_string(*(*_img_view), unmanagedFormat);
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

	array<System::Byte>^ ImageView::Encode(System::String ^ format, Palette ^ palette)
	{
		std::string unmanagedFormat = msclr::interop::marshal_as<std::string>(format);
		palette_ptr p = palette->NativeObject();
		try
		{
			std::string s = save_to_string(*(*_img_view), unmanagedFormat, *p);
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

	void ImageView::Save(System::String^ path)
	{
		std::string filename = msclr::interop::marshal_as<std::string>(path);
		std::string format("");
		format = mapnik::guess_type(filename);
		if (format == "<unknown>")
		{
			throw gcnew System::Exception("unknown output extension for: " + path);
		}
		Save(path, msclr::interop::marshal_as<System::String^>(format));
	}

	void ImageView::Save(System::String^ path, System::String^ format)
	{
		std::string unmanagedPath = msclr::interop::marshal_as<std::string>(path);
		std::string unmanagedFormat = msclr::interop::marshal_as<std::string>(format);

		try
		{
			mapnik::save_to_file(*(*_img_view), unmanagedPath, unmanagedFormat);
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}
}
