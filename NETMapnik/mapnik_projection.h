#pragma once

#include <memory>
#include <mapnik\projection.hpp>

namespace NETMapnik
{
	typedef std::shared_ptr<mapnik::projection> proj_ptr;

	public ref class Projection
	{
	public:
		Projection(System::String^ name);
		~Projection();
		array<System::Double>^ Forward(System::Double x, System::Double y);
		array<System::Double>^ Forward(System::Double minx, System::Double miny, System::Double maxx, System::Double maxy);
		array<System::Double>^ Inverse(System::Double x, System::Double y);
		array<System::Double>^ Inverse(System::Double minx, System::Double miny, System::Double maxx, System::Double maxy);
	internal:
		proj_ptr NativeObject();
	private:
		proj_ptr* _proj;
	};

}