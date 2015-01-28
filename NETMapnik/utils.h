#pragma once

#include <mapnik\box2d.hpp>
#include <mapnik\value_types.hpp>
#include <mapnik\value.hpp>
#include <mapnik\util\variant.hpp>

#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	inline mapnik::box2d<double> ArrayToBox2D(array<System::Double>^ boxArray)
	{
		if (boxArray->Length != 4)
			throw gcnew System::Exception("Bbox array must contain 4 values: minx,miny,maxx,maxy");

		return mapnik::box2d<double>(boxArray[0], boxArray[1], boxArray[2], boxArray[3]);
	}

	inline array<System::Double>^ Box2DToArray(mapnik::box2d<double> bbox)
	{
		array<System::Double>^ boxArray = gcnew array<System::Double>(4);
		boxArray[0] = bbox.minx();
		boxArray[1] = bbox.miny();
		boxArray[2] = bbox.maxx();
		boxArray[3] = bbox.maxy();

		return boxArray;
	}

	struct params_to_object : public mapnik::util::static_visitor<>
	{
		params_to_object(System::Collections::Generic::IDictionary<System::String^, System::Object^>^ ds, System::String^ key) :
			_ds(ds),
			_key(key) {}

		void operator() (mapnik::value_integer val) const
		{
			_ds->Add(_key, gcnew mapnik::value_integer(val));
		}

		void operator() (double val) const
		{
			_ds->Add(_key, gcnew double(val));
		}

		void operator() (bool val) const
		{
			_ds->Add(_key, gcnew bool(val));
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