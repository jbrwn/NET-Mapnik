#pragma once

namespace NETMapnik
{
	ref class NET_options_parser
	{
	public:
		NET_options_parser(System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		generic<typename T> System::Boolean TryGet(System::String^ key, T% value);
	private:
		System::Collections::Generic::IDictionary<System::String^, System::Object^>^ _options;
	};

}