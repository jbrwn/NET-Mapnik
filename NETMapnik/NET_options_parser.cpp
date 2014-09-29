#include "stdafx.h"
#include "NET_options_parser.h"

namespace NETMapnik
{
	NET_options_parser::NET_options_parser(System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options)
	{
		_options = options;
	}

	generic<typename T> System::Boolean NET_options_parser::TryGet(System::String^ key, T% value)
	{
		System::Object^ obj;
		if (_options->TryGetValue(key, obj))
		{
			value = static_cast<T>(obj);
			return true;
		}
		return false;
	}
}