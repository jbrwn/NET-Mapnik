#include <mapnik\grid\grid.hpp>

#pragma once

namespace NETMapnik
{
	public ref class Grid
	{
	public:
		Grid(System::UInt32 width, System::UInt32 height);
		~Grid();

		System::Collections::Generic::Dictionary<System::String^, System::Object^>^ Encode(System::String^ format, System::Boolean addFeatures, System::UInt32 resolution);
	internal:
		mapnik::grid *NativeObject();

	private:
		mapnik::grid* _grid;
	};
}