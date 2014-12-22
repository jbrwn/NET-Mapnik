#pragma once

#include <memory>

#include <mapnik\map.hpp>

namespace NETMapnik
{	
	//Forward Declare
	ref class Image;
	ref class Grid;
	ref class VectorTile;
	ref class Color;
	ref class Layer;
	ref class Featureset;

	typedef std::shared_ptr<mapnik::Map> map_ptr;

	public ref struct MapQueryResult
	{
	public:
		MapQueryResult(System::String^ layer, Featureset^ featureset) :
			_layer(layer),
			_featureset(featureset) {}

		property System::String^ Layer
		{
			System::String^ get() { return _layer; };
		}

		property NETMapnik::Featureset^ Featureset
		{
			NETMapnik::Featureset^ get() { return _featureset; };
		}

	private:
		System::String^ _layer;
		NETMapnik::Featureset^ _featureset;
	};

	public enum class AspectFixMode : int
	{

		ASPECT_GROW_BBOX = mapnik::Map::GROW_BBOX,
		ASPECT_GROW_CANVAS = mapnik::Map::GROW_CANVAS,
		ASPECT_SHRINK_BBOX = mapnik::Map::SHRINK_BBOX,
		ASPECT_SHRINK_CANVAS = mapnik::Map::SHRINK_CANVAS,
		ASPECT_ADJUST_BBOX_WIDTH = mapnik::Map::ADJUST_BBOX_WIDTH,
		ASPECT_ADJUST_BBOX_HEIGHT = mapnik::Map::ADJUST_BBOX_HEIGHT,
		ASPECT_ADJUST_CANVAS_WIDTH = mapnik::Map::ADJUST_CANVAS_WIDTH,
		ASPECT_ADJUST_CANVAS_HEIGHT = mapnik::Map::ADJUST_CANVAS_HEIGHT,
		ASPECT_RESPECT = mapnik::Map::RESPECT,
	};

	public ref class Map
	{
	public:
		//Constructor
		Map(System::Int32 width, System::Int32 height);
		Map(System::Int32 width, System::Int32 height, System::String^ srs);
		//Destructor
		~Map();

		property array<System::Double>^ Extent
		{
			array<System::Double>^ get();
			void set(array<System::Double>^ value);
		}

		property array<System::Double>^ BufferedExtent
		{
			array<System::Double>^ get();
		}

		property array<System::Double>^ MaximumExtent
		{
			array<System::Double>^ get();
			void set(array<System::Double>^ value);
		}

		property System::Int32 Width
		{
			System::Int32 get();
			void set(System::Int32 value);
		}
		property System::Int32 Height
		{
			System::Int32 get();
			void set(System::Int32 value);
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

		property NETMapnik::AspectFixMode AspectFixMode
		{
			NETMapnik::AspectFixMode get();
			void set(NETMapnik::AspectFixMode value);
		}

		property Color^ Background
		{
			Color^ get();
			void set(Color^ value);
		}

		property System::Collections::Generic::Dictionary<System::String^, System::Object^>^ Parameters
		{
			System::Collections::Generic::Dictionary<System::String^, System::Object^>^ get();
			void set(System::Collections::Generic::Dictionary<System::String^, System::Object^>^);
		}

		System::Double Scale();
		System::Double ScaleDenominator();
		void Clear();
		void Resize(System::Int32 width, System::Int32 heigt);

		System::Collections::Generic::IEnumerable<System::String^>^ Fonts();
		System::Collections::Generic::IDictionary<System::String^, System::String^>^ FontFiles();
		System::String^ FontDirectory();
		void LoadFonts();
		System::Collections::Generic::IEnumerable<System::String^>^ MemoryFonts();
		void RegisterFonts(System::String^ path);
		void RegisterFonts(System::String^ path, System::Boolean recurse);
		Map^ Clone();
		void Save(System::String^ path);
		System::String^ ToXML();
		
		System::Collections::Generic::IEnumerable<MapQueryResult^>^ QueryPoint(System::Double x, System::Double y);
		System::Collections::Generic::IEnumerable<MapQueryResult^>^ QueryPoint(System::Double x, System::Double y, System::Int32 layerIndex);
		System::Collections::Generic::IEnumerable<MapQueryResult^>^ QueryPoint(System::Double x, System::Double y, System::String^ layerName);


		Layer^ GetLayer(System::Int32 index);
		Layer^ GetLayer(System::String^ name);
		void AddLayer(Layer^ layer);
		System::Collections::Generic::IEnumerable<Layer^>^ Layers();

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
		//void RenderFile(System::String^ path, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);

	internal:
		map_ptr NativeObject();
		Map(mapnik::Map const& map);

	private:
		map_ptr* _map;
		System::Collections::Generic::IEnumerable<MapQueryResult^>^ _queryPoint(double x, double y, int layer_idx, bool geo_coords);
	};

}


