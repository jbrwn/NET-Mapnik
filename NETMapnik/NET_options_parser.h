#pragma once

#include <string>

namespace NETMapnik
{
	ref class NET_options_parser
	{
	public:
		NET_options_parser(System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		generic<typename T> System::Boolean TryGet(System::String^ key, T% value);
		System::Boolean TryGetUInt32(System::String^ key, System::UInt32% value);
		System::Boolean TryGetInt32(System::String^ key, System::Int32% value);
		System::Boolean TryGetBoolean(System::String^ key, System::Boolean% value);
		System::Boolean TryGetDouble(System::String^ key, System::Double% value);
		System::Boolean TryGetString(System::String^ key, std::string& value);
	private:
		System::Collections::Generic::IDictionary<System::String^, System::Object^>^ _options;
	};

}