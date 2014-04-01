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

		property System::UInt32 width
		{
			System::UInt32 get();
			void set(System::UInt32 value);
		}
		property System::UInt32 height
		{
			System::UInt32 get();
			void set(System::UInt32 value);
		}

		void load_map(System::String^ path);
		void zoom_to_box(System::Double minx, System::Double miny, System::Double maxx, System::Double maxy);
		array<System::Byte>^ save_to_bytes(System::String^ format);
		void save_to_file(System::String^ path, System::String^ format);

	private:
		//Unmanaged instance of mapnik::map
		mapnik::Map* _map;
	};
}


