#pragma once

#include <memory>
#include <mapnik\grid\grid.hpp>

namespace NETMapnik
{
	//Forward Declare
	ref class GridView;

	typedef std::shared_ptr<mapnik::grid> grid_ptr;

	public ref class Grid
	{
	public:
		Grid(System::Int32 width, System::Int32 height);
		Grid(System::Int32 width, System::Int32 height, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		~Grid();

		property System::String^ Key
		{
			System::String^ get();
			void set(System::String^ value);
		}

		static property System::Int64 BaseMask
		{
			System::Int64 get();
		}

		void Clear();
		System::Boolean Painted();
		System::Int32 Width();
		System::Int32 Height();
		System::Collections::Generic::IEnumerable<System::String^>^ Fields();
		GridView^ View(System::Int32 x, System::Int32 y, System::Int32 w, System::Int32 h);
		System::Collections::Generic::Dictionary<System::String^, System::Object^>^ Encode();
		System::Collections::Generic::Dictionary<System::String^, System::Object^>^ Encode(System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
	internal:
		grid_ptr NativeObject();
	private:
		grid_ptr* _grid;
	};
}