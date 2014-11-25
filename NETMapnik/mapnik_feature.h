#pragma once

#include <mapnik\feature.hpp>
#include <boost\shared_ptr.hpp>

namespace NETMapnik
{
	typedef boost::shared_ptr<mapnik::feature_impl> feature_ptr;
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
		Feature(feature_ptr feature);

	private:
		feature_ptr* _feature;
	};

};