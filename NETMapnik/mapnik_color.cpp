#include "stdafx.h"
#include "mapnik_color.h"

// microsoft
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{

	Color::Color(System::String^ color)
	{
		std::string unmanagedColor = msclr::interop::marshal_as<std::string>(color);
		_color = new mapnik::color(unmanagedColor);
	}

	Color::Color(System::UInt32 r, System::UInt32 g, System::UInt32 b)
	{
		_color = new mapnik::color(r, g, b);
	}
	
	Color::Color(System::UInt32 r, System::UInt32 g, System::UInt32 b, System::UInt32 a)
	{
		_color = new mapnik::color(r, g, b, a);
	}
	
	Color::~Color()
	{
		if (_color != NULL)
			delete _color;
	}

	mapnik::color *Color::NativeObject()
	{
		return _color;
	}

	System::UInt32 Color::R::get()
	{
		return _color->red();
	}

	void Color::R::set(System::UInt32 value)
	{
		_color->set_red(value);
	}

	System::UInt32 Color::G::get()
	{
		return _color->green();
	}

	void Color::G::set(System::UInt32 value)
	{
		_color->set_green(value);
	}

	System::UInt32 Color::B::get()
	{
		return _color->blue();
	}

	void Color::B::set(System::UInt32 value)
	{
		_color->set_blue(value);
	}

	System::UInt32 Color::A::get()
	{
		return _color->alpha();
	}

	void Color::A::set(System::UInt32 value)
	{
		_color->set_alpha(value);
	}

	System::String^ Color::ToString()
	{
		return msclr::interop::marshal_as<System::String^>(_color->to_string());
	}

	System::String^ Color::Hex()
	{
		return msclr::interop::marshal_as<System::String^>(_color->to_hex_string());
	}
}