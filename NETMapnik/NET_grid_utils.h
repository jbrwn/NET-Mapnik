#include <mapnik\grid\grid.hpp>

#pragma once

namespace NETMapnik
{
	template <typename T> 
	void grid2utf(T const& grid_type,
					System::Collections::Generic::List<System::String^>^ l,
					std::vector<typename T::lookup_type>& key_order);

	template <typename T> 
	void grid2utf(T const& grid_type,
					System::Collections::Generic::List<System::String^>^ l,
					std::vector<typename T::lookup_type>& key_order,
					unsigned int resolution);

	template <typename T> 
	void write_features(T const& grid_type,
						System::Collections::Generic::Dictionary<System::String^, System::Object^>^ feature_data,
						std::vector<typename T::lookup_type> const& key_order);

	template <typename T> 
	void grid_encode_utf(T const& grid_type,
						System::Collections::Generic::Dictionary<System::String^, System::Object^>^ json,
						bool add_features,
						unsigned int resolution);
	
	template <typename T>
	System::Collections::Generic::Dictionary<System::String^, System::Object^>^ grid_encode( T const& grid, 
						std::string const& format, 
						bool add_features, 
						unsigned int resolution);

}

