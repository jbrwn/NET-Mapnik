#pragma once

#include "vector_tile.pb.h"

namespace NETMapnik
{
	//Forward declare
	ref class Image;
	ref class Grid;
	ref class Map;
	ref class Feature;

	public ref struct VectorQueryResult : System::IComparable<VectorQueryResult^>
	{
	public:
		VectorQueryResult(System::String^ layer, System::Double distance, Feature^ feature) :
			_layer(layer),
			_distance(distance),
			_feature(feature) {}

		property System::String^ Layer
		{
			System::String^ get() { return _layer; };
		}

		property System::Double Distance
		{
			System::Double get() { return _distance; };
		}

		property NETMapnik::Feature^ Feature
		{
			NETMapnik::Feature^ get() { return _feature; };
		}
		virtual System::Int32 CompareTo(VectorQueryResult^ other)
		{
			if (_distance < other->Distance)
			{
				return 1;
			}
			else if (_distance > other->Distance)
			{
				return -1;
			}
			else return 0;
		}

	private:
		System::String^ _layer;
		System::Double _distance;
		NETMapnik::Feature^ _feature;
	};

	public ref class VectorTileFeature
	{
	public:
		VectorTileFeature(System::UInt64 id, array<System::Byte>^ raster, System::Int32 type, System::Collections::Generic::IList<System::Int32>^ geometry, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ properties) :
			_id(id),
			_raster(raster),
			_type(type),
			_geometry(geometry),
			_properties(properties) {}

		property System::UInt64 Id
		{
			System::UInt64 get() { return _id; };
		}
		property array<System::Byte>^ Raster
		{
			array<System::Byte>^ get() { return _raster; }
		}
		property System::Int32 Type
		{
			System::Int32 get() { return _type; };
		}
		property System::Collections::Generic::IReadOnlyList<System::Int32>^ Geometry
		{
			System::Collections::Generic::IReadOnlyList<System::Int32>^ get() { return gcnew System::Collections::ObjectModel::ReadOnlyCollection<System::Int32>(_geometry); };
		}
		property System::Collections::Generic::IReadOnlyDictionary<System::String^, System::Object^>^ Properties
		{
			System::Collections::Generic::IReadOnlyDictionary<System::String^, System::Object^>^ get() { return gcnew System::Collections::ObjectModel::ReadOnlyDictionary<System::String^, System::Object^>(_properties); };
		}
	private:
		System::UInt64 _id;
		array<System::Byte>^ _raster;
		System::Int32 _type;
		System::Collections::Generic::IList<System::Int32>^ _geometry;
		System::Collections::Generic::IDictionary<System::String^, System::Object^>^ _properties;
	};
	
	public ref class VectorTileLayer
	{
	public:
		VectorTileLayer(System::String^ name, System::Int32 extent, System::Int32 version, System::Collections::Generic::IList<VectorTileFeature^>^ features) :
			_name(name),
			_extent(extent),
			_version(version),
			_features(features){}

		property System::String^ Name
		{
			System::String^ get() { return _name; };
		}
		property System::Int32 Extent
		{
			System::Int32 get() { return _extent; };
		}
		property System::Int32 Version
		{
			System::Int32 get() { return _version; };
		}
		property System::Collections::Generic::IReadOnlyList<VectorTileFeature^>^ Features
		{
			System::Collections::Generic::IReadOnlyList<VectorTileFeature^>^ get() { return gcnew System::Collections::ObjectModel::ReadOnlyCollection<VectorTileFeature^>(_features);  };
		}
	private:
		System::String^ _name;
		System::Int32 _extent;
		System::Int32 _version;
		System::Collections::Generic::IList<VectorTileFeature^>^ _features;
	};

	public ref class VectorTileJSON : public System::Collections::Generic::IReadOnlyList<VectorTileLayer^>
	{
	public:
		VectorTileJSON(System::Collections::Generic::IReadOnlyList<VectorTileLayer^>^ layers) :
			_layers(layers) {}

		virtual property System::Int32 Count
		{
			System::Int32 get() { return _layers->Count; };
		}

		virtual property VectorTileLayer^ default[System::Int32]
		{
			VectorTileLayer^ get(System::Int32 i) { return _layers[i]; };
		}

			virtual System::Collections::IEnumerator^ GetEnumeratorBase() = System::Collections::IEnumerable::GetEnumerator
		{
			return GetEnumerator();
		}
			virtual System::Collections::Generic::IEnumerator<VectorTileLayer^>^ GetEnumerator()
		{
			return _layers->GetEnumerator();
		}
	private:
		System::Collections::Generic::IReadOnlyList<VectorTileLayer^>^ _layers;
	};

	public ref class VectorTile
	{
	public:
		VectorTile(System::Int32 z, System::Int32 x, System::Int32 y);
		VectorTile(System::Int32 z, System::Int32 x, System::Int32 y, System::Int32 width, System::Int32 height);
		~VectorTile();
		void Composite(System::Collections::Generic::IEnumerable<VectorTile^>^ vTiles, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		void Composite(System::Collections::Generic::IEnumerable<VectorTile^>^ vTiles);
		System::Collections::Generic::IEnumerable<System::String^>^ Names();
		System::Boolean Empty();
		System::Int32 Width();
		System::Int32 Height();
		System::Boolean Painted();
		System::Collections::Generic::IEnumerable<VectorQueryResult^>^ Query(double lon, double lat);
		System::Collections::Generic::IEnumerable<VectorQueryResult^>^ Query(double lon, double lat, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		//QueryMany(double x, double y, System::String^ layer);
		//QueryMany(double x, double y, System::String^ layer, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		VectorTileJSON^ ToJSON();
		System::String^ ToGeoJSON(System::Int32 layer);
		System::String^ ToGeoJSON(System::String^ layer);
		void AddGeoJSON(System::String^ json, System::String^ layer);
		void AddGeoJSON(System::String^ json, System::String^ layer, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		void AddImage(array<System::Byte>^ bytes, System::String^ layer);
		void AddData(array<System::Byte>^ data);
		array<System::Byte>^ GetData();
		void SetData(array<System::Byte>^ data);
		void Render(Map^ map, Image^ image);
		void Render(Map^ map, Image^ image, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		void Render(Map^ map, Grid^ grid);
		void Render(Map^ map, Grid^ grid, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		void Clear();
		System::String^ IsSolid();

	internal:
		vector_tile::Tile *NativeObject();
		void Painted(bool painted);

	private:
		vector_tile::Tile* _tile;
		int _z;
		int _x;
		int _y;
		unsigned _width;
		unsigned _height;
		bool _painted;
		void write_geojson_to_string(std::string & result,bool array,bool all,int layer_idx);
		bool layer_to_geojson(vector_tile::Tile_Layer const& layer, std::string & result, unsigned x, unsigned y, unsigned z, unsigned width);

	};
}
