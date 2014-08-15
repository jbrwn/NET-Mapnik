#include "mapnik_grid.h"
#include "mapnik_vector_tile.h"

#include <mapnik\map.hpp>

#pragma once

namespace NETMapnik
{

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

		void LoadMap(System::String^ path);
		void ZoomToBox(System::Double minx, System::Double miny, System::Double maxx, System::Double maxy);
		void ZoomAll();
		array<System::Byte>^ SaveToBytes(System::String^ format);
		void SaveToFile(System::String^ path, System::String^ format);
		void Render(Grid^ grid, System::UInt32 layerIdx,  System::Collections::Generic::List<System::String^>^ fields);
		void Render(VectorTile^ tile);

	private:
		//Unmanaged instance of mapnik::map
		mapnik::Map* _map;
	};

}


