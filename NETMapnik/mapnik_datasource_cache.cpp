#include "StdAfx.h"
#include "mapnik_datasource_cache.h"

// mapnik
#include <mapnik\datasource_cache.hpp>

// clr 
#include <msclr\marshal_cppstd.h>


namespace NETMapnik
{	
	void DatasourceCache::RegisterDatasources(System::String^ path)
	{
		std::string unmanagedPath = msclr::interop::marshal_as<std::string>(path);
		try
		{
			mapnik::datasource_cache::instance().register_datasources(unmanagedPath);
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}
}