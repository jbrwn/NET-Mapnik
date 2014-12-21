#include "stdafx.h"
#include "mapnik_palette.h"

#include <memory>
#include <vector>
#include <iomanip>
#include <sstream>

#include <mapnik\palette.hpp>

#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{

	Palette::Palette(System::String ^ palette)
	{
		try
		{
			std::string unmanagedPalette = msclr::interop::marshal_as<std::string>(palette);
			_palette = new palette_ptr(std::make_shared<mapnik::rgba_palette>(unmanagedPalette));
		}
		catch (const std::exception& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
	}

	Palette::Palette(System::String ^ palette, PaletteType type)
	{
		try
		{
			std::string unmanagedPalette = msclr::interop::marshal_as<std::string>(palette);
			mapnik::rgba_palette::palette_type paletteType = static_cast<mapnik::rgba_palette::palette_type>(type);
			_palette = new palette_ptr(std::make_shared<mapnik::rgba_palette>(unmanagedPalette, paletteType));
		}
		catch (const std::exception& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
	}

	Palette::Palette(array<System::Byte>^ palette)
	{
		try
		{
			pin_ptr<unsigned char> pBytes = &palette[0];
			std::string paletteString(reinterpret_cast<char*>(pBytes), palette->Length);
			_palette = new palette_ptr(std::make_shared<mapnik::rgba_palette>(paletteString));
		}
		catch (const std::exception& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
	}

	Palette::Palette(array<System::Byte>^ palette, PaletteType type)
	{
		try
		{
			pin_ptr<unsigned char> pBytes = &palette[0];
			std::string paletteString(reinterpret_cast<char*>(pBytes), palette->Length);
			mapnik::rgba_palette::palette_type paletteType = static_cast<mapnik::rgba_palette::palette_type>(type);
			_palette = new palette_ptr(std::make_shared<mapnik::rgba_palette>(paletteString, paletteType));
		}
		catch (const std::exception& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
	}

	System::String^ Palette::ToString()
	{
		const std::vector<mapnik::rgb>& colors = (*_palette)->palette();
		unsigned length = colors.size();
		const std::vector<unsigned>& alpha = (*_palette)->alphaTable();
		unsigned alphaLength = alpha.size();

		std::ostringstream str("");
		str << "[Palette " << length;
		if (length == 1) str << " color";
		else str << " colors";

		str << std::hex << std::setfill('0');

		for (unsigned i = 0; i < length; i++) {
			str << " #";
			str << std::setw(2) << (unsigned)colors[i].r;
			str << std::setw(2) << (unsigned)colors[i].g;
			str << std::setw(2) << (unsigned)colors[i].b;
			if (i < alphaLength) str << std::setw(2) << alpha[i];
		}

		str << "]";
		return msclr::interop::marshal_as<System::String^>(str.str());
	}

	array<System::Byte>^ Palette::ToBytes()
	{
		const std::vector<mapnik::rgb>& colors = (*_palette)->palette();
		unsigned length = colors.size();
		const std::vector<unsigned>& alpha = (*_palette)->alphaTable();
		unsigned alphaLength = alpha.size();

		char palette[256 * 4];
		for (unsigned i = 0, pos = 0; i < length; i++) {
			palette[pos++] = colors[i].r;
			palette[pos++] = colors[i].g;
			palette[pos++] = colors[i].b;
			palette[pos++] = (i < alphaLength) ? alpha[i] : 0xFF;
		}

		array<System::Byte>^ data = gcnew array<System::Byte>(length * 4);
		System::Runtime::InteropServices::Marshal::Copy(System::IntPtr(&palette[0]), data, 0, length * 4);
		return data;
	}

	Palette::~Palette()
	{
		if (_palette != NULL)
			delete _palette;
	}

	palette_ptr Palette::NativeObject()
	{
		return (*_palette);
	}

}