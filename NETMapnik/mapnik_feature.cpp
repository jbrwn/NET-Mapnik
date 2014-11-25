#include "stdafx.h"
#include "mapnik_feature.h"
#include "NET_box_utils.h"
#include "mapnik_value_converter.h"

#include <mapnik/feature_factory.hpp>
#include <mapnik/json/feature_parser.hpp>
#include <mapnik/json/feature_generator_grammar.hpp>

// microsoft
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	Feature::Feature(feature_ptr feature)
	{
		_feature = new feature_ptr(feature);

	}

	Feature::~Feature()
	{
		if (_feature != NULL)
			delete _feature;
	}

	System::Int32 Feature::Id()
	{
		return (*_feature)->id();
	}

	array<System::Double>^ Feature::Extent()
	{
		return Box2DToArray((*_feature)->envelope());
	}

	System::Collections::Generic::IDictionary<System::String^, System::Object^>^ Feature::Attributes()
	{
		System::Collections::Generic::IDictionary<System::String^, System::Object^>^ feat = gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>();

		mapnik::feature_impl::iterator itr = (*_feature)->begin();
		mapnik::feature_impl::iterator end = (*_feature)->end();
		for (; itr != end; ++itr)
		{
			System::String^ key = msclr::interop::marshal_as<System::String^>(boost::get<0>(*itr));
			feat[key] = boost::apply_visitor(value_converter(), boost::get<1>(*itr).base());
		}
		return feat;

	}

	//Geometry Geometry();

	System::String^ Feature::ToJSON()
	{
		typedef std::back_insert_iterator<std::string> sink_type;
		static const mapnik::json::feature_generator_grammar<sink_type> grammar;
		std::string json;
		sink_type sink(json);
		if (!boost::spirit::karma::generate(sink, grammar,*(*_feature)))
		{
			throw gcnew System::Exception("Failed to generate GeoJSON");
		}
		return msclr::interop::marshal_as<System::String^>(json);
	}

}