#include "stdafx.h"
#include "mapnik_projection.h"

#include <memory>
#include <mapnik\projection.hpp>

#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	Projection::Projection(System::String^ name)
	{
		try
		{	
			std::string unmanagedName = msclr::interop::marshal_as<std::string>(name);
			_proj = new proj_ptr(std::make_shared<mapnik::projection>(unmanagedName));
		}
		catch (const std::exception& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
	}

	Projection::~Projection()
	{
		if (_proj != NULL)
		{
			delete _proj;
		}
	}

	array<System::Double>^ Projection::Forward(System::Double x, System::Double y)
	{
		try
		{
			(*_proj)->forward(x, y);
			return gcnew array<System::Double>(2) { x, y };
		}
		catch (const std::exception& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
	}

	array<System::Double>^ Projection::Forward(System::Double minx, System::Double miny, System::Double maxx, System::Double maxy)
	{
		try
		{
			(*_proj)->forward(minx, miny);
			(*_proj)->forward(maxx, maxy);
			return gcnew array<System::Double>(4) { minx, miny, maxx, maxy };
		}
		catch (const std::exception& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
	}

	array<System::Double>^ Projection::Inverse(System::Double x, System::Double y)
	{
		try
		{
			(*_proj)->inverse(x, y);
			return gcnew array<System::Double>(2) { x, y };
		}
		catch (const std::exception& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
	}

	array<System::Double>^ Projection::Inverse(System::Double minx, System::Double miny, System::Double maxx, System::Double maxy)
	{
		try
		{
			(*_proj)->inverse(minx, miny);
			(*_proj)->inverse(maxx, maxy);
			return gcnew array<System::Double>(4) { minx, miny, maxx, maxy };
		}
		catch (const std::exception& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
	}

	proj_ptr Projection::NativeObject()
	{
		return *_proj;
	}
}
