#include "stdafx.h"
#include "mapnik_color.h"

#include <memory>

// mapnik
#include <mapnik\color.hpp>

// microsoft
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{

	Color::Color(System::String^ color)
	{
		std::string unmanagedColor = msclr::interop::marshal_as<std::string>(color);
		try
		{
			_color = new color_ptr(std::make_shared<mapnik::color>(unmanagedColor));
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}

	Color::Color(System::Int32 r, System::Int32 g, System::Int32 b)
	{
		try
		{
			_color = new color_ptr(std::make_shared<mapnik::color>(r, g, b));
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}
	
	Color::Color(System::Int32 r, System::Int32 g, System::Int32 b, System::Int32 a)
	{
		try
		{
			_color = new color_ptr(std::make_shared<mapnik::color>(r, g, b, a));
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}
	
	Color::Color(mapnik::color const& color)
	{
		try 
		{
			_color = new color_ptr(std::make_shared<mapnik::color>(color));
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}

	Color::~Color()
	{
		if (_color != NULL)
			delete _color;
	}

	color_ptr Color::NativeObject()
	{
		return *_color;
	}

	System::Int32 Color::R::get()
	{
		return (*_color)->red();
	}

	void Color::R::set(System::Int32 value)
	{
		(*_color)->set_red(value);
	}

	System::Int32 Color::G::get()
	{
		return (*_color)->green();
	}

	void Color::G::set(System::Int32 value)
	{
		(*_color)->set_green(value);
	}

	System::Int32 Color::B::get()
	{
		return (*_color)->blue();
	}

	void Color::B::set(System::Int32 value)
	{
		(*_color)->set_blue(value);
	}

	System::Int32 Color::A::get()
	{
		return (*_color)->alpha();
	}

	void Color::A::set(System::Int32 value)
	{
		(*_color)->set_alpha(value);
	}

	System::String^ Color::ToString()
	{
		return msclr::interop::marshal_as<System::String^>((*_color)->to_string());
	}

	System::String^ Color::Hex()
	{
		return msclr::interop::marshal_as<System::String^>((*_color)->to_hex_string());
	}
}