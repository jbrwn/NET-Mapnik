#pragma once

#include <memory>
#include <mapnik\grid\grid_view.hpp>

namespace NETMapnik
{
	//Forward Declare
	ref class Grid;

	typedef std::shared_ptr<mapnik::grid_view> grid_view_ptr;

	public ref class GridView
	{
	public:
		~GridView();
		System::Boolean IsSolid();
		System::Int32 Width();
		System::Int32 Height();
		System::Int64 GetPixel(System::Int32 x, System::Int32 y);
		System::Collections::Generic::Dictionary<System::String^, System::Object^>^ Encode();
		System::Collections::Generic::Dictionary<System::String^, System::Object^>^ Encode(System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
	internal:
		GridView(Grid^ grid, unsigned x, unsigned y, unsigned w, unsigned h);
		grid_view_ptr NativeObject();
	private:
		grid_view_ptr* _grid_view;
	};
}


