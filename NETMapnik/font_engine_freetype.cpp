#include "StdAfx.h"
#include "font_engine_freetype.h"

// mapnik
#include <mapnik\font_engine_freetype.hpp>

// clr 
#include <msclr\marshal_cppstd.h>


namespace NETMapnik
{	
	bool freetype_engine::RegisterFonts(System::String^ dir, bool recurse)
	{
		std::string unmanagedDir = msclr::interop::marshal_as<std::string>(dir);
		try
		{
			mapnik::freetype_engine::register_fonts(unmanagedDir, recurse);
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
		return true;
	}

	void freetype_engine::RegisterFont(System::String^ file_name)
	{
		std::string unmanagedFileName = msclr::interop::marshal_as<std::string>(file_name);
		try
		{
			mapnik::freetype_engine::register_fonts(unmanagedFileName);
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}
}
