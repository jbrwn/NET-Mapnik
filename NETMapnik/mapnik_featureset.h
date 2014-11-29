#pragma once

#include <mapnik\datasource.hpp>

namespace NETMapnik
{
	//Forward Declare
	ref class Feature;

	public ref class Featureset 
	{
	public:
		~Featureset();
		Feature^ Next();

	internal:
		Featureset(mapnik::featureset_ptr featurest);

	private:
		mapnik::featureset_ptr* _featureset;
	};
};

