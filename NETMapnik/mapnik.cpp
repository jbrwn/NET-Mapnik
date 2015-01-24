#include "stdafx.h"
#include "mapnik.h"

// mapnik
#include <mapnik\version.hpp>
//#include <mapnik\font_engine_freetype.hpp>

// boost
#include <boost\version.hpp>

// cairo
#if defined(HAVE_CAIRO)
#include <cairo.h>
#endif

#include <Windows.h>
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	//bool Mapnik::RegisterFonts(System::String^ dir)
	//{
	//	return RegisterFonts(dir, false);
	//}

	//bool Mapnik::RegisterFonts(System::String^ dir, bool recurse)
	//{
	//	try
	//	{
	//		std::string path = msclr::interop::marshal_as<std::string>(dir);
	//		return mapnik::freetype_engine::register_fonts(path, recurse);
	//	}
	//	catch (const std::exception& ex)
	//	{
	//		System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
	//		throw gcnew System::Exception(managedException);
	//	}
	//}

	//System::Collections::Generic::IEnumerable<System::String^>^ Mapnik::Fonts()
	//{
	//	System::Collections::Generic::List<System::String^>^ a = gcnew System::Collections::Generic::List<System::String^>();
	//	std::vector<std::string> const& names = mapnik::freetype_engine::face_names();
	//	for (unsigned i = 0; i < names.size(); ++i)
	//	{
	//		a->Add(msclr::interop::marshal_as<System::String^>(names[i]));
	//	}
	//	return a;
	//}

	static System::String^ format_version(int version)
	{
		return System::String::Format("{0}.{1}.{2}", version / 100000, 100 % 1000, version % 100);
	}

	Mapnik::Mapnik()
	{
		_versions = gcnew System::Collections::Generic::Dictionary<System::String^, System::String^>(System::StringComparer::InvariantCultureIgnoreCase);
		_versions->Add("CLR", System::Environment::Version->ToString());
		_versions->Add("Boost",format_version(BOOST_VERSION));
		_versions->Add("Mapnik",format_version(MAPNIK_VERSION));
		// To do: implement MAPNIK_GIT_REVISION
		//versions->Add("mapnik_git_describe",MAPNIK_GIT_REVISION);
#if defined(HAVE_CAIRO)
		_versions->Add("cairo",CAIRO_VERSION_STRING);
#endif

		_supports = gcnew System::Collections::Generic::Dictionary<System::String^, System::Boolean>(System::StringComparer::InvariantCultureIgnoreCase);
#ifdef GRID_RENDERER
		_supports->Add("grid", true);
#else
		_supports->Add("grid", false);
#endif

#ifdef SVG_RENDERER
		_supports->Add("svg", true);
#else
		_supports->Add("svg", false);
#endif

#if defined(HAVE_CAIRO)
		_supports->Add("cairo", true);
#ifdef CAIRO_HAS_PDF_SURFACE
		_supports->Add("cairo_pdf", true);
#else
		_supports->Add("cairo_pdf", false);
#endif
#ifdef CAIRO_HAS_SVG_SURFACE
		_supports->Add("cairo_svg", true);
#else
		_supports->Add("cairo_svg", false);
#endif
#else
		_supports->Add("cairo", false);
#endif

#if defined(HAVE_PNG)
		_supports->Add("png", true);
#else
		_supports->Add("png", false);
#endif

#if defined(HAVE_JPEG)
		_supports->Add("jpeg", true);
#else
		_supports->Add("jpeg", false);
#endif

#if defined(HAVE_TIFF)
		_supports->Add("tiff", true);
#else
		_supports->Add("tiff", false);
#endif

#if defined(HAVE_WEBP)
		_supports->Add("webp", true);
#else
		_supports->Add("webp", false);
#endif

#if defined(MAPNIK_USE_PROJ4)
		_supports->Add("proj4", true);
#else
		_supports->Add("proj4", false);
#endif

#if defined(MAPNIK_THREADSAFE)
		_supports->Add("threadsafe", true);
#else
		_supports->Add("threadsafe", false);
#endif		
	}
}


/*
Native entry point for NETMapnik assembly
Set environmental variables for required libs
*/

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOLEAN WINAPI DllMain(IN HINSTANCE hDllHandle,
	IN DWORD     nReason,
	IN LPVOID    Reserved)
{
	switch (nReason)
	{
		case DLL_PROCESS_ATTACH:
			_putenv_s("PROJ_LIB", "share\\proj");
			_putenv_s("ICU_DATA", "share\\icu");
			_putenv_s("GDAL_DATA", "share\\gdal");
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif