#pragma once

#include <memory>

#include <mapnik\graphics.hpp>
#include <mapnik\image_compositing.hpp>

namespace NETMapnik
{
	public enum class CompositeOp : int
	{
		Clear = mapnik::clear,
		Src = mapnik::src,
		Dst = mapnik::dst,
		SrcOver = mapnik::src_over,
		DstOver = mapnik::dst_over,
		SrcIn = mapnik::src_in,
		DstIn = mapnik::dst_in,
		SrcOut = mapnik::src_out,
		DstOut = mapnik::dst_out,
		SrcAtop = mapnik::src_atop,
		DstAtop = mapnik::dst_atop,
		Xor = mapnik::_xor,
		Plus = mapnik::plus,
		Minus = mapnik::minus,
		Multiply = mapnik::multiply,
		Screen = mapnik::screen,
		Overlay = mapnik::overlay,
		Darken = mapnik::darken,
		Lighten = mapnik::lighten,
		ColorDodge = mapnik::color_dodge,
		ColorBurn = mapnik::color_burn,
		HardLight = mapnik::hard_light,
		SoftLight = mapnik::soft_light,
		Difference = mapnik::difference,
		Exclusion = mapnik::exclusion,
		Contrast = mapnik::contrast,
		Invert = mapnik::invert,
		InvertRGB = mapnik::invert_rgb,
		GrainMerge = mapnik::grain_merge,
		GrainExtract = mapnik::grain_extract,
		Hue = mapnik::hue,
		Saturation = mapnik::saturation,
		Color = mapnik::_color,
		LinearDodge = mapnik::linear_dodge,
		LinearBurn = mapnik::linear_burn,
		Divide = mapnik::divide
	};


	//Forward Declare
	ref class Color;
	ref class Palette;

	typedef std::shared_ptr<mapnik::image_32> image_ptr;

	public ref class Image
	{
	public:
		Image(System::Int32 width, System::Int32 height);
		~Image();

		property Color^ Background
		{
			Color^ get();
			void set(Color^ value);
		}

		Color^ GetPixel(System::Int32 x, System::Int32 y);
		void SetPixel(System::Int32 x, System::Int32 y, Color^ value);
		
		//ImageView View(Image image, System::Int32 x, System::Int32 y, System::Int32 w, System::Int32 h);
		
		void SetGrayScaleToAlpha();
		void SetGrayScaleToAlpha(Color^ color);
		System::Int32 Width();
		System::Int32 Height();
		System::Boolean Painted();
		Image^ Composite(Image^ image);
		Image^ Composite(Image^ image, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		void Premultiply();
		void Demultiply();
		void Clear();
		System::Int32 Compare(Image^ image);
		System::Int32 Compare(Image^ image, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		static Image^ Open(System::String^ path);
		static Image^ FromBytes(array<System::Byte>^ bytes);
		void Save(System::String^ path);
		void Save(System::String^ path, System::String^ format);
		array<System::Byte>^ Encode(System::String^ format);
		array<System::Byte>^ Encode(System::String^ format, Palette^ palette);

	internal:
		Image(image_ptr img);
		image_ptr NativeObject();

	private:
		image_ptr *_img;
	};
}

