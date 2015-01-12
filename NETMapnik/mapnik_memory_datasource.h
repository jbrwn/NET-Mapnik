#pragma once

// mapnik
#include <mapnik\datasource.hpp>
#include <mapnik\unicode.hpp>

namespace NETMapnik
{
	//Forward Declare
	ref class Featureset;

	public ref class MemoryDatasource
	{
	public:
		MemoryDatasource(System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		~MemoryDatasource();
		System::Collections::Generic::IDictionary<System::String^, System::Object^>^ Paramemters();
		System::Collections::Generic::IDictionary<System::String^, System::Object^>^ Describe();
		Featureset^ Featureset();
		void Add(double x, double y);
		void Add(double x, double y, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ properties);
	internal:
		mapnik::datasource_ptr NativeObject();
		MemoryDatasource(mapnik::datasource_ptr ds);

	private:
		mapnik::datasource_ptr *_ds;
		unsigned int _feature_id;
		mapnik::transcoder *_tr;
	};
}