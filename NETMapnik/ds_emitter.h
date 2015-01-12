#pragma once

// mapnik
#include <mapnik\attribute_descriptor.hpp> 
#include <mapnik\datasource.hpp>
#include <mapnik\feature_layer_desc.hpp>

// microsoft
#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{

	static void describe_datasource(System::Collections::Generic::Dictionary<System::String^, System::Object^>^ description, mapnik::datasource_ptr ds)
	{
		try
		{
			// type
			if (ds->type() == mapnik::datasource::Raster)
			{
				description->Add("type", "raster");
			}
			else
			{
				description->Add("type", "vector");
			}

			mapnik::layer_descriptor ld = ds->get_descriptor();

			// encoding
			description->Add("encoding", msclr::interop::marshal_as<System::String^>(ld.get_encoding().c_str()));

			// field names and types
			System::Collections::Generic::Dictionary<System::String^, System::String^>^ fields = gcnew System::Collections::Generic::Dictionary<System::String^, System::String^>();

			std::vector<mapnik::attribute_descriptor> const& desc = ld.get_descriptors();
			std::vector<mapnik::attribute_descriptor>::const_iterator itr = desc.begin();
			std::vector<mapnik::attribute_descriptor>::const_iterator end = desc.end();
			while (itr != end)
			{
				unsigned field_type = itr->get_type();
				System::String^ type("");
				if (field_type == mapnik::Integer) type = "Number";
				else if (field_type == mapnik::Float) type = "Number";
				else if (field_type == mapnik::Double) type = "Number";
				else if (field_type == mapnik::String) type = "String";
				else if (field_type == mapnik::Boolean) type = "Boolean";
				else if (field_type == mapnik::Geometry) type = "Geometry";
				else if (field_type == mapnik::Object) type = "Object";
				else type = "Unknown";
				fields->Add(msclr::interop::marshal_as<System::String^>(itr->get_name().c_str()), type);
				++itr;
			}
			description->Add("fields", fields);

			System::String^ js_type = "unknown";
			if (ds->type() == mapnik::datasource::Raster)
			{
				js_type = "raster";
			}
			else
			{
				boost::optional<mapnik::datasource::geometry_t> geom_type = ds->get_geometry_type();
				if (geom_type)
				{
					mapnik::datasource::geometry_t g_type = *geom_type;
					switch (g_type)
					{
					case mapnik::datasource::Point:
					{
						js_type = "point";
						break;
					}
					case mapnik::datasource::LineString:
					{
						js_type = "linestring";
						break;
					}
					case mapnik::datasource::Polygon:
					{
						js_type = "polygon";
						break;
					}
					case mapnik::datasource::Collection:
					{
						js_type = "collection";
						break;
					}
					default:
					{
						break;
					}
					}
				}
			}
			description->Add("geometry_type", js_type);
		}
		catch (std::exception const& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
		catch (...)
		{
			throw gcnew System::Exception("unknown exception happened when calling describe_datasource, please file bug");
		}
	}
}