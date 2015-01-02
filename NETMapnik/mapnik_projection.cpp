#include "stdafx.h"
#include "mapnik_projection.h"
#include "NET_box_utils.h"

#include <memory>
#include <mapnik\box2d.hpp>
#include <mapnik\proj_transform.hpp>
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

	ProjTransform::ProjTransform(Projection^ src, Projection^ dest)
	{
		try
		{
			proj_ptr src_prj = src->NativeObject();
			proj_ptr dest_prj = dest->NativeObject();
			_projTran = new projTran_ptr(std::make_shared<mapnik::proj_transform>(*src_prj, *dest_prj));
		}
		catch (const std::exception& ex)
		{
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
		}
	}

	ProjTransform::~ProjTransform()
	{
		if (_projTran != NULL)
		{
			delete _projTran;
		}
	}

	projTran_ptr ProjTransform::NativeObject()
	{
		return *_projTran;
	}

	array<System::Double>^ ProjTransform::Backward(System::Double x, System::Double y)
	{
		double z = 0;
		if (!(*_projTran)->backward(x, y, z))
		{
			throw gcnew System::Exception(
				System::String::Format("Failed to forward project {0}, {1} from {2} to {3}",
					x,
					y,
					msclr::interop::marshal_as<System::String^>((*_projTran)->dest().params()),
					msclr::interop::marshal_as<System::String^>((*_projTran)->source().params())
				)
			);
		}
		return gcnew array<System::Double>(2) { x, y };
	}

	array<System::Double>^ ProjTransform::Backward(System::Double minx, System::Double miny, System::Double maxx, System::Double maxy)
	{
		mapnik::box2d<double> box(minx, miny, maxx, maxy);
		if (!(*_projTran)->backward(box))
		{
			throw gcnew System::Exception(
				System::String::Format("Failed to forward project {0} from {1} to {2}",
					msclr::interop::marshal_as<System::String^>(box.to_string()),
					msclr::interop::marshal_as<System::String^>((*_projTran)->dest().params()),
					msclr::interop::marshal_as<System::String^>((*_projTran)->source().params())
				)
			);
		}
		return Box2DToArray(box);
	}

	array<System::Double>^ ProjTransform::Forward(System::Double x, System::Double y)
	{
		double z = 0;
		if (!(*_projTran)->forward(x, y, z))
		{
			throw gcnew System::Exception(
				System::String::Format("Failed to forward project {0}, {1} from {2} to {3}",
					x,
					y,
					msclr::interop::marshal_as<System::String^>((*_projTran)->source().params()),
					msclr::interop::marshal_as<System::String^>((*_projTran)->dest().params())
				)
			);
		}
		return gcnew array<System::Double>(2) { x, y };
	}

	array<System::Double>^ ProjTransform::Forward(System::Double minx, System::Double miny, System::Double maxx, System::Double maxy)
	{
		mapnik::box2d<double> box(minx, miny, maxx, maxy);
		if (!(*_projTran)->forward(box))
		{
			throw gcnew System::Exception(
				System::String::Format("Failed to forward project {0} from {1} to {2}",
					msclr::interop::marshal_as<System::String^>(box.to_string()),
					msclr::interop::marshal_as<System::String^>((*_projTran)->source().params()),
					msclr::interop::marshal_as<System::String^>((*_projTran)->dest().params())
				)
			);
		}
		return Box2DToArray(box);
	}


}
