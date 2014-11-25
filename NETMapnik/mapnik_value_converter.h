#pragma once

#include <mapnik\value_types.hpp>
#include <mapnik\value.hpp>

// boost
#include <boost\variant\static_visitor.hpp>

#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	struct value_converter : public boost::static_visitor<System::Object^>
	{
		System::Object^ operator() (mapnik::value_integer val) const
		{
			return gcnew int(val);
		}

		System::Object^ operator() (double val) const
		{
			return gcnew double(val);
		}

		System::Object^ operator() (bool val) const
		{
			return gcnew bool(val);
		}

		System::Object^ operator() (std::string const& s) const
		{
			return msclr::interop::marshal_as<System::String^>(s);
		}

		System::Object^ operator() (mapnik::value_unicode_string const& s) const
		{
			std::string buffer;
			mapnik::to_utf8(s, buffer);
			return  msclr::interop::marshal_as<System::String^>(buffer);
		}

		System::Object^ operator() (mapnik::value_null const& /*s*/) const
		{
			return nullptr;
		}
	};
}