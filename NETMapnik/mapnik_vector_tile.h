#pragma once

#include "vector_tile.pb.h"

namespace NETMapnik
{
	//Forward declare
	ref class Image;
	ref class Map;

	public ref class VectorTile
	{
	public:
		VectorTile(System::Int32 z, System::Int32 x, System::Int32 y);
		VectorTile(System::Int32 z, System::Int32 x, System::Int32 y, System::UInt32 width, System::UInt32 height);
		~VectorTile();
		array<System::Byte>^ GetBytes();
		void SetBytes(array<System::Byte>^ data);

		System::Collections::Generic::IEnumerable<System::String^>^ Names();
		System::Boolean Empty();

		void Composite(System::Collections::Generic::IEnumerable<VectorTile^>^ vTiles, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		void Composite(System::Collections::Generic::IEnumerable<VectorTile^>^ vTiles);
		
		void Render(Map^ map, Image^ image);
		void Render(Map^ map, Image^ image, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);

	internal:
		vector_tile::Tile *NativeObject();

	private:
		vector_tile::Tile* _tile;
		int _z;
		int _x;
		int _y;
		unsigned _width;
		unsigned _height;

	};
}
