#pragma once

#include <mapnik\datasource.hpp>
#include <mapnik\feature.hpp>
#include <boost\shared_ptr.hpp>

namespace NETMapnik
{
	//Forward Declare
	ref class Feature;

	typedef boost::shared_ptr<mapnik::Featureset> featureset_ptr;
	typedef boost::shared_ptr<mapnik::feature_impl> feature_ptr;

	public ref class Featureset 
	{
	public:
		~Featureset();
		Feature^ Next();

	internal:
		Featureset(featureset_ptr featurest);

	private:
		featureset_ptr* _featureset;
	};
};

