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
		Map(System::UInt32 width, System::UInt32 height);
		Map(System::UInt32 width, System::UInt32 height, System::String^ srs);
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

		property System::Int32 BufferSize
		{
			System::Int32 get();
			void set(System::Int32 value);
		}

		property System::String^ SRS
		{
			System::String^ get();
			void set(System::String^ value);
		}

		property System::Int32 AspectFixMode
		{
			System::Int32 get();
			void set(System::Int32 value);
		}

		//property Extent 
		//property BufferedExtent
		//property MaxiumExtent

		property System::Collections::Generic::Dictionary<System::String^, System::Object^>^ Parameters
		{
			System::Collections::Generic::Dictionary<System::String^, System::Object^>^ get();
		}

		void Load(System::String^ path);
		void Load(System::String^ path, System::Boolean strict);
		void Load(System::String^ path, System::Boolean strict, System::String^ basePath);
		void FromString(System::String^ str);
		void FromString(System::String^ str, System::Boolean strict);
		void FromString(System::String^ str, System::Boolean strict, System::String^ basePath);

		void ZoomToBox(System::Double minx, System::Double miny, System::Double maxx, System::Double maxy);
		void ZoomAll();

		void Render(Image^ image);
		void Render(Image^ image, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		void Render(Grid^ grid);
		void Render(Grid^ grid, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		void Render(VectorTile^ tile);
		void Render(VectorTile^ tile, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);

	internal:
		mapnik::Map *NativeObject();

	private:
		//Unmanaged instance of mapnik::map
		mapnik::Map* _map;
	};

}


