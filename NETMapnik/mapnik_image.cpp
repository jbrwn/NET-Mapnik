#include "stdafx.h"
#include "mapnik_image.h"

#include <mapnik\graphics.hpp>
#include <mapnik\image_util.hpp>

#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	Image::Image(System::Int32 width, System::Int32 height)
	{
		_image_32 = new mapnik::image_32(width, height);
	}

	Image::~Image()
	{
		if (_image_32 != NULL)
		{
			delete _image_32;
		}
	}

	mapnik::image_32 *Image::NativeObject()
	{
		return _image_32;
	}

	void Image::Save(System::String^ path, System::String^ format)
	{
		std::string unmanagedPath = msclr::interop::marshal_as<std::string>(path);
		std::string unmanagedFormat = msclr::interop::marshal_as<std::string>(format);

		try
		{
			mapnik::save_to_file(*_image_32, unmanagedPath, unmanagedFormat);
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
			std::string s = save_to_string(*_image_32, unmanagedFormat);
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
}