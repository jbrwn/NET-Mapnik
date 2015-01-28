#pragma once

// mapnik
#include <mapnik\feature.hpp>

namespace NETMapnik
{
	//Forward Declare
	ref class Geometry;

	public ref class Feature 
	{
	public:
		~Feature();

		System::Int64 Id();
		array<System::Double>^ Extent();
		System::Collections::Generic::IDictionary<System::String^, System::Object^>^ Attributes();
		Geometry^ Geometry();
		System::String^ ToJSON();

		static Feature^ FromJSON(System::String^ json);

	internal:
		Feature(mapnik::feature_ptr feature);

	private:
		mapnik::feature_ptr* _feature;
	};

};