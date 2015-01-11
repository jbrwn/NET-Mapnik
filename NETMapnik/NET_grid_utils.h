#pragma once

#include "StdAfx.h"
#include "NET_grid_utils.h"

// mapnik
#include <mapnik\grid\grid.hpp>

// microsoft
#include <cstdint>
#include <msclr\marshal_cppstd.h>


namespace NETMapnik
{
	template <typename T>
	void grid2utf(T const& grid_type,
		System::Collections::Generic::List<System::String^>^ l,
		std::vector<typename T::lookup_type>& key_order)
	{
		typedef std::map< typename T::lookup_type, typename T::value_type> keys_type;
		typedef typename keys_type::const_iterator keys_iterator;

		typename T::data_type const& data = grid_type.data();
		typename T::feature_key_type const& feature_keys = grid_type.get_feature_keys();
		typename T::feature_key_type::const_iterator feature_pos;

		keys_type keys;
		// start counting at utf8 codepoint 32, aka space character
		std::uint16_t codepoint = 32;

		unsigned array_size = data.width();
		for (unsigned y = 0; y < data.height(); ++y)
		{
			std::uint16_t idx = 0;
			array<wchar_t>^ line = gcnew array<wchar_t>(array_size);
			typename T::value_type const* row = data.getRow(y);
			for (unsigned x = 0; x < data.width(); ++x)
			{
				typename T::value_type feature_id = row[x];
				feature_pos = feature_keys.find(feature_id);
				if (feature_pos != feature_keys.end())
				{
					mapnik::grid::lookup_type val = feature_pos->second;
					keys_iterator key_pos = keys.find(val);
					if (key_pos == keys.end())
					{
						// Create a new entry for this key. Skip the codepoints that
						// can't be encoded directly in JSON.
						if (codepoint == 34) ++codepoint;      // Skip "
						else if (codepoint == 92) ++codepoint; // Skip backslash
						if (feature_id == mapnik::grid::base_mask)
						{
							keys[""] = codepoint;
							key_order.push_back("");
						}
						else
						{
							keys[val] = codepoint;
							key_order.push_back(val);
						}
						line[idx++] = (wchar_t)codepoint;
						++codepoint;
					}
					else
					{
						line[idx++] = (wchar_t)key_pos->second;
					}
				}
				// else, shouldn't get here...
			}
			l->Add(gcnew System::String(line));
		}
	}

	template <typename T>
	void grid2utf(T const& grid_type,
		System::Collections::Generic::List<System::String^>^ l,
		std::vector<typename T::lookup_type>& key_order,
		unsigned int resolution)
	{
		typedef std::map< typename T::lookup_type, typename T::value_type> keys_type;
		typedef typename keys_type::const_iterator keys_iterator;

		typename T::feature_key_type const& feature_keys = grid_type.get_feature_keys();
		typename T::feature_key_type::const_iterator feature_pos;

		keys_type keys;
		// start counting at utf8 codepoint 32, aka space character
		std::uint16_t codepoint = 32;

		unsigned array_size = std::ceil(grid_type.width() / static_cast<float>(resolution));
		for (unsigned y = 0; y < grid_type.height(); y = y + resolution)
		{
			std::uint16_t idx = 0;
			array<wchar_t>^ line = gcnew array<wchar_t>(array_size);
			mapnik::grid::value_type const* row = grid_type.getRow(y);
			for (unsigned x = 0; x < grid_type.width(); x = x + resolution)
			{
				typename T::value_type feature_id = row[x];
				feature_pos = feature_keys.find(feature_id);
				if (feature_pos != feature_keys.end())
				{
					mapnik::grid::lookup_type val = feature_pos->second;
					keys_iterator key_pos = keys.find(val);
					if (key_pos == keys.end())
					{
						// Create a new entry for this key. Skip the codepoints that
						// can't be encoded directly in JSON.
						if (codepoint == 34) ++codepoint;      // Skip "
						else if (codepoint == 92) ++codepoint; // Skip backslash
						if (feature_id == mapnik::grid::base_mask)
						{
							keys[""] = codepoint;
							key_order.push_back("");
						}
						else
						{
							keys[val] = codepoint;
							key_order.push_back(val);
						}
						line[idx++] = (wchar_t)codepoint;
						++codepoint;
					}
					else
					{
						line[idx++] = (wchar_t)key_pos->second;
					}
				}
				// else, shouldn't get here...
			}
			l->Add(gcnew System::String(line));
		}
	}

	template <typename T>
	void write_features(T const& grid_type,
		System::Collections::Generic::Dictionary<System::String^, System::Object^>^ feature_data,
		std::vector<typename T::lookup_type> const& key_order)
	{
		typename T::feature_type const& g_features = grid_type.get_grid_features();
		if (g_features.size() <= 0)
		{
			return;
		}

		std::set<std::string> const& attributes = grid_type.property_names();
		typename T::feature_type::const_iterator feat_end = g_features.end();
		for (std::vector<mapnik::grid::lookup_type>::const_iterator iter = key_order.begin(), stop = key_order.end(); iter != stop; ++iter)
		{
			mapnik::grid::lookup_type key_item = *iter;
			if (key_item.empty())
			{
				continue;
			}

			typename T::feature_type::const_iterator feat_itr = g_features.find(key_item);
			if (feat_itr == feat_end)
			{
				continue;
			}

			bool found = false;
			System::Collections::Generic::Dictionary<System::String^, System::String^>^ feat = gcnew System::Collections::Generic::Dictionary<System::String^, System::String^>();
			mapnik::feature_ptr feature = feat_itr->second;
			for (std::set<std::string>::const_iterator iter = attributes.begin(), stop = attributes.end(); iter != stop; ++iter)
			{
				std::string attr = *iter;
				if (attr == "__id__")
				{
					System::String^ key = msclr::interop::marshal_as<System::String^>(attr);
					System::String^ value = System::Convert::ToString(feature->id());
					feat[key] = value;
				}
				else if (feature->has_key(attr))
				{
					found = true;
					System::String^ key = msclr::interop::marshal_as<System::String^>(attr);
					System::String^ value = msclr::interop::marshal_as<System::String^>(feature->get(attr).to_string());
					feat[key] = value;
				}
			}

			if (found)
			{
				System::String^ key = msclr::interop::marshal_as<System::String^>(feat_itr->first);
				feature_data[key] = feat;
			}
		}
	}

	template <typename T>
	void grid_encode_utf(T const& grid_type,
		System::Collections::Generic::Dictionary<System::String^, System::Object^>^ json,
		bool add_features,
		unsigned int resolution)
	{
		// convert buffer to utf and gather key order
		System::Collections::Generic::List<System::String^>^ l = gcnew System::Collections::Generic::List<System::String^>();
		std::vector<typename T::lookup_type> key_order;

		if (resolution != 1) {
			// resample on the fly - faster, less accurate
			grid2utf<T>(grid_type, l, key_order, resolution);

			// resample first - slower, more accurate
			//mapnik::grid2utf2<T>(grid_type,l,key_order,resolution);
		}
		else
		{
			grid2utf<T>(grid_type, l, key_order);
		}

		// convert key order to proper .NET list
		System::Collections::Generic::List<System::String^>^ keys_a = gcnew System::Collections::Generic::List<System::String^>();
		for (std::vector<mapnik::grid::lookup_type>::const_iterator iter = key_order.begin(), stop = key_order.end(); iter != stop; ++iter)
		{
			mapnik::grid::lookup_type key_id = *iter;
			System::String^ k = msclr::interop::marshal_as<System::String^>(key_id);
			keys_a->Add(k);
		}

		// gather feature data
		System::Collections::Generic::Dictionary<System::String^, System::Object^>^ feature_data = gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>();
		if (add_features) {
			write_features<T>(grid_type, feature_data, key_order);
		}

		json["grid"] = l;
		json["keys"] = keys_a;
		json["data"] = feature_data;

	}

	template <typename T>
	System::Collections::Generic::Dictionary<System::String^, System::Object^>^ grid_encode(T const& grid,
		std::string const& format,
		bool add_features,
		unsigned int resolution)
	{

		if (format == "utf")
		{
			System::Collections::Generic::Dictionary<System::String^, System::Object^>^ json = gcnew System::Collections::Generic::Dictionary<System::String^, System::Object^>();
			grid_encode_utf(grid, json, add_features, resolution);
			return json;
		}
		else
		{
			throw gcnew System::Exception("'utf' is currently the only supported encoding format.");
		}

	}
}