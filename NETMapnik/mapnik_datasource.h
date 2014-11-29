#pragma once

// mapnik
#include <mapnik\datasource.hpp>

namespace NETMapnik
{
	//Forward Declare
	ref class Featureset;

	public ref class Datasource
	{
	public:
		Datasource(System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		~Datasource();

		property System::String^ Type
		{
			System::String^ get();
		}

		System::Collections::Generic::IDictionary<System::String^, System::Object^>^ Paramemters();
		array<System::Double>^ Extent();
		System::Collections::Generic::IDictionary<System::String^, System::Object^>^ Describe();
		Featureset^ Featureset();
	internal:
		mapnik::datasource_ptr NativeObject();
		Datasource(mapnik::datasource_ptr ds);

	private:

		System::String^ _type;
		mapnik::datasource_ptr *_ds;
	};

}