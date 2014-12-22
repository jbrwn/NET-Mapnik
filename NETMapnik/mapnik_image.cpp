#include "stdafx.h"
#include "mapnik_image.h"
#include "mapnik_color.h"
#include "mapnik_palette.h"
#include "mapnik_image_view.h"
#include "NET_options_parser.h"

#include <memory>

// mapnik
#include <mapnik\color.hpp>
#include <mapnik\graphics.hpp>
#include <mapnik\image_data.hpp>
#include <mapnik\image_reader.hpp>
#include <mapnik\image_util.hpp>
#include <mapnik\image_compositing.hpp>
#include <mapnik\image_filter_types.hpp>
#include <mapnik\image_filter.hpp> 

// boost
#include <boost\optional\optional.hpp>

#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	Image::Image(System::Int32 width, System::Int32 height)
	{
		_img = new image_ptr(std::make_shared<mapnik::image_32>(width, height));
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

	Color^ Image::Background::get()
	{
		boost::optional<mapnik::color> const& bg = (*_img)->get_background();
		if (!bg)
			return nullptr;
		return gcnew Color(*bg);
	}

	void Image::Background::set(Color^ value)
	{
		color_ptr c = value->NativeObject();
		(*_img)->set_background(*c);
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
		mapnik::image_data_32 const& data = (*_img)->data();
		if (x >= 0 && x < static_cast<int>(data.width())
			&& y >= 0 && y < static_cast<int>(data.height()))
		{
			unsigned pixel = data(x, y);
			unsigned r = pixel & 0xff;
			unsigned g = (pixel >> 8) & 0xff;
			unsigned b = (pixel >> 16) & 0xff;
			unsigned a = (pixel >> 24) & 0xff;
			return gcnew Color(mapnik::color(r, g, b, a));
		}
		return nullptr;
	}
	void Image::SetPixel(System::Int32 x, System::Int32 y, Color^ value)
	{
		mapnik::image_data_32 & data = (*_img)->data();
		if (x < static_cast<int>(data.width()) && y < static_cast<int>(data.height()))
		{
			data(x, y) = value->NativeObject()->rgba();
			return;
		}
		throw gcnew System::Exception("invalid pixel requested");
	}
	void Image::SetGrayScaleToAlpha()
	{
		(*_img)->set_grayscale_to_alpha();
	}

	void Image::SetGrayScaleToAlpha(Color^ color)
	{
		mapnik::image_data_32 & data = (*_img)->data();
		for (unsigned int y = 0; y < data.height(); ++y)
		{
			unsigned int* row_from = data.getRow(y);
			for (unsigned int x = 0; x < data.width(); ++x)
			{
				unsigned rgba = row_from[x];
				// TODO - big endian support
				unsigned r = rgba & 0xff;
				unsigned g = (rgba >> 8) & 0xff;
				unsigned b = (rgba >> 16) & 0xff;

				// magic numbers for grayscale
				unsigned a = (int)((r * .3) + (g * .59) + (b * .11));

				row_from[x] = (a << 24) |
					(color->NativeObject()->blue() << 16) |
					(color->NativeObject()->green() << 8) |
					(color->NativeObject()->red());
			}
		}
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
			if (optionsParser->TryGet<int>("CompOp", comp_op))
			{
				mode = static_cast<mapnik::composite_mode_e>(comp_op);
			}
			optionsParser->TryGet<float>("Opacity", opacity);
			optionsParser->TryGet<int>("Dx", dx);
			optionsParser->TryGet<int>("Dy", dy);
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
				mapnik::filter::filter_visitor<mapnik::image_32> visitor(*(image->NativeObject()));
				for (mapnik::filter::filter_type const& filter_tag : filters)
				{
					mapnik::util::apply_visitor(visitor, filter_tag);
				}
			}
			mapnik::composite(
				(*_img)->data(), 
				image->NativeObject()->data(), 
				mode, 
				opacity, 
				dx,
				dy
			);
			return gcnew Image(*_img);
		}
		catch (std::exception const& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
	}

	void Image::Premultiply()
	{
		(*_img)->premultiply();
	}

	void Image::Demultiply()
	{
		(*_img)->demultiply();
	}

	void Image::Clear()
	{
		(*_img)->clear();
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
		unsigned difference = 0;

		// get options
		NET_options_parser^ optionsParser = gcnew NET_options_parser(options);
		optionsParser->TryGet<int>("Threshold", threshold);
		optionsParser->TryGet<bool>("Alpha", alpha);

		// do work
		image_ptr img2 = image->NativeObject();
		if ((*_img)->width() != img2->width() ||
			(*_img)->height() != img2->height()) {
			throw gcnew System::Exception("image dimensions do not match");
		}
		mapnik::image_data_32 const& data = (*_img)->data();
		mapnik::image_data_32 const& data2 = img2->data();
		for (unsigned int y = 0; y < data.height(); ++y)
		{
			const unsigned int* row_from = data.getRow(y);
			const unsigned int* row_from2 = data2.getRow(y);
			for (unsigned int x = 0; x < data.width(); ++x)
			{
				unsigned rgba = row_from[x];
				unsigned rgba2 = row_from2[x];
				unsigned r = rgba & 0xff;
				unsigned g = (rgba >> 8) & 0xff;
				unsigned b = (rgba >> 16) & 0xff;
				unsigned r2 = rgba2 & 0xff;
				unsigned g2 = (rgba2 >> 8) & 0xff;
				unsigned b2 = (rgba2 >> 16) & 0xff;
				if (std::abs(static_cast<int>(r - r2)) > threshold ||
					std::abs(static_cast<int>(g - g2)) > threshold ||
					std::abs(static_cast<int>(b - b2)) > threshold) {
					++difference;
					continue;
				}
				if (alpha) {
					unsigned a = (rgba >> 24) & 0xff;
					unsigned a2 = (rgba2 >> 24) & 0xff;
					if (std::abs(static_cast<int>(a - a2)) > threshold) {
						++difference;
						continue;
					}
				}
			}
		}
		return difference;
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
					std::shared_ptr<mapnik::image_32> image_ptr(new mapnik::image_32(reader->width(), reader->height()));
					reader->read(0, 0, image_ptr->data());
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
				std::shared_ptr<mapnik::image_32> image_ptr(new mapnik::image_32(reader->width(), reader->height()));
				reader->read(0, 0, image_ptr->data());
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