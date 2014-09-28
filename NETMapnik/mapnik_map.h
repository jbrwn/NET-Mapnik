#pragma once

#include <mapnik\map.hpp>

namespace NETMapnik
{
	//Forward Declare
	ref class Image;
	ref class Grid;
	ref class VectorTile;

	public ref class Map
	{
	public:
		//Constructor
		Map();
		//Destructor
		~Map();

		property System::UInt32 Width
		{
			System::UInt32 get();
			void set(System::UInt32 value);
		}
		property System::UInt32 Height
		{
			System::UInt32 get();
			void set(System::UInt32 value);
		}

		property System::Int32 Buffer
		{
			System::Int32 get();
			void set(System::Int32 value);
		}

		property System::Collections::Generic::Dictionary<System::String^, System::Object^>^ Parameters
		{
			System::Collections::Generic::Dictionary<System::String^, System::Object^>^ get();
		}

		void LoadMap(System::String^ path);
		void FromString(System::String^ str);
		void ZoomToBox(System::Double minx, System::Double miny, System::Double maxx, System::Double maxy);
		void ZoomAll();

		void Render(Image^ image);
		void Render(Grid^ grid, System::UInt32 layerIdx,  System::Collections::Generic::List<System::String^>^ fields);
		void Render(VectorTile^ tile);

	internal:
		mapnik::Map *NativeObject();

	private:
		//Unmanaged instance of mapnik::map
		mapnik::Map* _map;
	};

}


