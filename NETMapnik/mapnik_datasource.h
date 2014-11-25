#pragma once

#include <mapnik\datasource.hpp>
#include <boost\shared_ptr.hpp>

namespace NETMapnik
{
	//Forward Declare
	ref class Featureset;

	typedef boost::shared_ptr<mapnik::datasource> datasource_ptr;

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
		datasource_ptr NativeObject();
		Datasource(datasource_ptr ds);

	private:

		System::String^ _type;
		datasource_ptr *_ds;
	};

}