#pragma once

// mapnik
#include <mapnik\value_types.hpp>
#include <mapnik\value.hpp>
#include <mapnik\util\variant.hpp>

// microsoft
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	struct params_to_object : public mapnik::util::static_visitor<>
	{
		params_to_object(System::Collections::Generic::IDictionary<System::String^, System::Object^>^ ds, System::String^ key) :
			_ds(ds),
			_key(key) {}
		
		void operator() (mapnik::value_integer val) const
		{
			_ds->Add(_key,gcnew int(val));
		}

		void operator() (double val) const
		{
			_ds->Add(_key, gcnew double(val));
		}

		void operator() (bool val) const
		{
			_ds->Add(_key,gcnew bool(val));
		}

		void operator() (std::string const& s) const
		{
			_ds->Add(_key, msclr::interop::marshal_as<System::String^>(s));
		}

		void operator() (mapnik::value_unicode_string const& s) const
		{
			std::string buffer;
			mapnik::to_utf8(s, buffer);
			_ds->Add(_key, msclr::interop::marshal_as<System::String^>(buffer));
		}

		void operator() (mapnik::value_null const& /*s*/) const
		{
			_ds->Add(_key, nullptr);
		}

	private:
		gcroot<System::Collections::Generic::IDictionary<System::String^, System::Object^>^> _ds;
		gcroot<System::String^> _key;
	};
}