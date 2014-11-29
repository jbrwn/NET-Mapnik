#pragma once

// mapnik
#include <mapnik\feature.hpp>

namespace NETMapnik
{
	public ref class Feature 
	{
	public:
		~Feature();

		System::Int32 Id();
		array<System::Double>^ Extent();
		System::Collections::Generic::IDictionary<System::String^, System::Object^>^ Attributes();
		//Geometry Geometry();
		System::String^ ToJSON();

	internal:
		Feature(mapnik::feature_ptr feature);

	private:
		mapnik::feature_ptr* _feature;
	};

};