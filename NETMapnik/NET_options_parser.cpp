#include "stdafx.h"
#include "NET_options_parser.h"

#include <msclr\marshal_cppstd.h>

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
			try
			{
				value = safe_cast<T>(obj);
				return true;
			}
			catch (System::Exception^)
			{
				//re throw better error
				throw gcnew System::Exception(System::String::Format("Invalid option value for '{0}'. Type '{1}' not expected ", key, T::typeid->Name));
			}
		}
		return false;
	}

	System::Boolean NET_options_parser::TryGetUInt32(System::String^ key, System::UInt32% value)
	{
		System::Object^ obj;
		if (_options->TryGetValue(key, obj))
		{
			System::Type^ t = obj->GetType();
			if (t == System::UInt32::typeid)
			{
				value = safe_cast<System::UInt32>(obj);
				return true;
			}
			if (t == System::Int32::typeid)
			{
				System::Int32 i = safe_cast<System::Int32>(obj);
				value = safe_cast<System::UInt32>(i);
				return true;
			}
			throw gcnew System::Exception(System::String::Format("Invalid option value for '{0}'. Type '{1}' not expected ", key, t->Name));
		}
		return false;
	}

	System::Boolean NET_options_parser::TryGetString(System::String^ key, std::string& value)
	{
		System::Object^ obj;
		if (_options->TryGetValue(key, obj))
		{
			System::Type^ t = obj->GetType();
			if (t == System::String::typeid)
			{
				System::String^ s = safe_cast<System::String^>(obj);
				value = msclr::interop::marshal_as<std::string>(s);
				return true;
			}
			throw gcnew System::Exception(System::String::Format("Invalid option value for '{0}'. Type '{1}' not expected ", key, t->Name));
		}
		return false;
	}

	System::Boolean NET_options_parser::TryGetInt32(System::String^ key, System::Int32% value)
	{
		return TryGet<System::Int32>(key, value);
	}

	System::Boolean NET_options_parser::TryGetBoolean(System::String^ key, System::Boolean% value)
	{
		return TryGet<System::Boolean>(key, value);
	}

	System::Boolean NET_options_parser::TryGetDouble(System::String^ key, System::Double% value)
	{
		return TryGet<System::Double>(key, value);
	}
}