#include "stdafx.h"
#include "mapnik_featureset.h"
#include "mapnik_feature.h"

// boost
#include <boost\make_shared.hpp>

// microsoft
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	Featureset::Featureset(featureset_ptr featureset)
	{
		_featureset = new featureset_ptr(featureset);
	}

	Featureset::~Featureset()
	{
		if (_featureset != NULL)
			delete _featureset;
	}

	Feature^ Featureset::Next()
	{
		if (_featureset) {
			feature_ptr fp;
			try
			{
				fp = (*_featureset)->next();
			}
			catch (std::exception const& ex)
			{
				System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
				throw gcnew System::Exception(managedException);
			}

			if (fp) {
				return gcnew Feature(fp);
			}
		}
		return nullptr;
	}
}