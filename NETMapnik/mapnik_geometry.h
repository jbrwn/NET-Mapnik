#pragma once

#include <mapnik\feature.hpp>

namespace NETMapnik
{
	public ref class Geometry
	{
	public:
		~Geometry();
		array<System::Double>^ Extent();
		System::String^ ToWKT();
		array<System::Byte>^ ToWKB();
		System::String^ ToJSON();

	internal:
		Geometry(mapnik::feature_ptr f);

	private:
		mapnik::feature_ptr* _geom;
	};
}

