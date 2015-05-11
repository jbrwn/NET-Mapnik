#include "stdafx.h"

// mapnik
#include <mapnik\color.hpp>
#include <mapnik\image.hpp>
#include <mapnik\image_any.hpp>
#include <mapnik\image_reader.hpp>
#include <mapnik\image_util.hpp>
#include <mapnik\image_copy.hpp>

#include <mapnik\image_compositing.hpp>
#include <mapnik\image_filter_types.hpp>
#include <mapnik\image_filter.hpp> 

#include "mapnik_image.h"
#include "mapnik_color.h"
#include "mapnik_palette.h"
#include "mapnik_image_view.h"
#include "NET_options_parser.h"

// boost
#include <boost\optional\optional.hpp>

#include <memory>

#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	Image::Image(System::Int32 width, System::Int32 height)
	{
		_img = new image_ptr(std::make_shared<mapnik::image_any>(width, height));
	} 
	Image::Image(System::Int32 width, System::Int32 height, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		mapnik::image_dtype dtype = mapnik::image_dtype_rgba8;
		bool initialize = true;
		bool premultiplied = false;
		bool painted = false;
		
		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		int type;
		if (optionsParser->TryGetInt32("Type", type))
		{
			dtype = static_cast<mapnik::image_dtype>(type);
			if (dtype >= mapnik::image_dtype::IMAGE_DTYPE_MAX)
			{
				throw gcnew System::Exception("Image 'type' must be a valid image type");
			}
		}
		optionsParser->TryGetBoolean("Initialize", initialize);
		optionsParser->TryGetBoolean("Premultiplied", premultiplied);
		optionsParser->TryGetBoolean("Painted", painted);
		_img = new image_ptr(std::make_shared<mapnik::image_any>(width, height, dtype, initialize, premultiplied, painted));
	}

	Image::Image(image_ptr img)
	{
		_img = new image_ptr(img);
	}

	Image::~Image()
	{
		if (_img != NULL)
		{
			delete _img;
		}
	}

	image_ptr Image::NativeObject()
	{
		return *_img;
	}

	void Image::Save(System::String^ path)
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

	void Image::Save(System::String^ path, System::String^ format)
	{
		std::string unmanagedPath = msclr::interop::marshal_as<std::string>(path);
		std::string unmanagedFormat = msclr::interop::marshal_as<std::string>(format);

		try
		{
			mapnik::save_to_file(*(*_img), unmanagedPath, unmanagedFormat);
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}

	array<System::Byte>^ Image::Encode(System::String^ format)
	{
		std::string unmanagedFormat = msclr::interop::marshal_as<std::string>(format);

		try
		{
			std::string s = save_to_string(*(*_img), unmanagedFormat);
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

	array<System::Byte>^ Image::Encode(System::String ^ format, Palette ^ palette)
	{
		std::string unmanagedFormat = msclr::interop::marshal_as<std::string>(format);
		palette_ptr p = palette->NativeObject();
		try
		{
			std::string s = save_to_string(*(*_img), unmanagedFormat,*p);
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

	Color^ Image::GetPixel(System::Int32 x, System::Int32 y)
	{
		if (x >= 0 && x < static_cast<int>((*_img)->width())
			&& y >= 0 && y < static_cast<int>((*_img)->height()))
		{
			mapnik::color val = mapnik::get_pixel<mapnik::color>(*(*_img), x, y);
			return gcnew Color(val);
		}
		return nullptr;
	}

	void Image::SetPixel(System::Int32 x, System::Int32 y, Color^ value)
	{
		if(x < 0 || x >= static_cast<int>((*_img)->width()) || y < 0 || y >= static_cast<int>((*_img)->height()))
		{
			throw gcnew System::Exception("Invalid pixel requested");
		}
		mapnik::set_pixel(*(*_img), x, y, *value->NativeObject());
		return;

	}
	void Image::SetGrayScaleToAlpha()
	{
		mapnik::set_grayscale_to_alpha(*(*_img));
	}

	void Image::SetGrayScaleToAlpha(Color^ color)
	{
		mapnik::set_grayscale_to_alpha(*(*_img), *color->NativeObject());
	}

	System::Int32 Image::Width()
	{
		return (*_img)->width();
	}
	System::Int32 Image::Height()
	{
		return (*_img)->height();
	}

	System::Boolean Image::Painted()
	{
		return (*_img)->painted();
	}

	Image^ Image::Composite(Image ^ image)
	{
		return Composite(image, gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>());
	}

	Image^ Image::Composite(Image ^ image, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		try
		{
			// set defaults 
			mapnik::composite_mode_e mode = mapnik::src_over;
			float opacity = 1.0;
			std::vector<mapnik::filter::filter_type> filters;
			int dx = 0;
			int dy = 0;

			// get options
			NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
			int comp_op;
			if (optionsParser->TryGetInt32("CompOp", comp_op))
			{
				mode = static_cast<mapnik::composite_mode_e>(comp_op);
			}
			optionsParser->TryGet<float>("Opacity", opacity);
			optionsParser->TryGetInt32("Dx", dx);
			optionsParser->TryGetInt32("Dy", dy);
			System::String^ image_filters;
			if (optionsParser->TryGet<System::String^>("ImageFilters", image_filters))
			{
				std::string filter_str = msclr::interop::marshal_as<std::string>(image_filters);
				bool result = mapnik::filter::parse_image_filters(filter_str, filters);
				if (!result)
				{
					throw gcnew System::Exception("could not parse image_filters");
				}
			}

			// do work
			if (filters.size() > 0)
			{
				mapnik::filter::filter_visitor<mapnik::image_any> visitor(*(image->NativeObject()));
				for (mapnik::filter::filter_type const& filter_tag : filters)
				{
					mapnik::util::apply_visitor(visitor, filter_tag);
				}
			}
			bool demultiply_im1 = mapnik::premultiply_alpha(*(*_img));
			bool demultiply_im2 = mapnik::premultiply_alpha(*image->NativeObject());

			mapnik::composite(
				*(*_img), 
				*image->NativeObject(), 
				mode, 
				opacity, 
				dx,
				dy
			);
			if (demultiply_im1)
			{
				mapnik::demultiply_alpha(*(*_img));
			}
			if (demultiply_im2)
			{
				mapnik::demultiply_alpha(*image->NativeObject());
			}
		}
		catch (std::exception const& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
		return gcnew Image(*_img);
	}

	void Image::Premultiply()
	{
		mapnik::premultiply_alpha(*(*_img));
	}

	void Image::Demultiply()
	{
		mapnik::demultiply_alpha(*(*_img));
	}

	void Image::Clear()
	{
		try
		{
			mapnik::fill(*(*_img), 0);
		}
		catch (std::exception const& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
	}

	System::Int32 Image::Compare(Image ^ image)
	{
		return Compare(image, gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>());
	}

	System::Int32 Image::Compare(Image ^ image, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		// defaults
		int threshold = 16;
		bool alpha = true;

		// get options
		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		optionsParser->TryGetInt32("Threshold", threshold);
		optionsParser->TryGetBoolean("Alpha", alpha);

		// do work
		image_ptr img2 = image->NativeObject();
		if ((*_img)->width() != img2->width() ||
			(*_img)->height() != img2->height()) 
		{
			throw gcnew System::Exception("image dimensions do not match");
		}

		unsigned diff =  mapnik::compare(*(*_img), *img2, threshold, alpha);
		if (diff > INT_MAX)
		{
			throw gcnew System::OverflowException("Difference is greater than int32 max");
		}
		return static_cast<System::Int32>(diff);
	}

	Image^ Image::Open(System::String ^ path)
	{
		try
		{
			std::string filename = msclr::interop::marshal_as<std::string>(path);
			boost::optional<std::string> type = mapnik::type_from_filename(filename);
			if (type)
			{
				std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename, *type));
				if (reader.get())
				{
					std::shared_ptr<mapnik::image_any> image_ptr = std::make_shared<mapnik::image_any>(reader->read(0, 0, reader->width(), reader->height()));
					return gcnew Image(image_ptr);
				}
				throw gcnew System::Exception("Failed to load: " + path);
			}
			throw gcnew System::Exception("Unsupported image format:" + path);
		}
		catch (std::exception const& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
	}

	Image^ Image::FromBytes(array<System::Byte>^ bytes)
	{
		try
		{
			pin_ptr<unsigned char> pBytes = &bytes[0];
			std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(reinterpret_cast<char*>(pBytes), bytes->Length));
			if (reader.get())
			{
				std::shared_ptr<mapnik::image_any> image_ptr = std::make_shared<mapnik::image_any>(reader->read(0, 0, reader->width(), reader->height()));
				return gcnew Image(image_ptr);
			}
			throw gcnew System::Exception("Failed to load from buffer");
		}
		catch (std::exception const& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
	}

	ImageView^ Image::View(System::Int32 x, System::Int32 y, System::Int32 w, System::Int32 h)
	{
		return gcnew ImageView(this, x, y, w, h);
	}

}