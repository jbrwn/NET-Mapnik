#pragma once

#include <mapnik\image_compositing.hpp>
#include <mapnik\pixel_types.hpp>

#include <memory>


namespace mapnik {
	struct image_any;
	//enum image_dtype : std::uint8_t;
}

namespace NETMapnik
{
	public enum class ImageType : int
	{
		Null = mapnik::image_dtype_null,
		Rgba8 = mapnik::image_dtype_rgba8,
		Gray8 = mapnik::image_dtype_gray8,
		Gray8s = mapnik::image_dtype_gray8s,
		Gray16 = mapnik::image_dtype_gray16,
		Gray16s = mapnik::image_dtype_gray16s,
		Gray32 = mapnik::image_dtype_gray32,
		Gray32s = mapnik::image_dtype_gray32s,
		Gray32f = mapnik::image_dtype_gray32f,
		Gray64 = mapnik::image_dtype_gray64,
		Gray64s = mapnik::image_dtype_gray64s,
		Gray64f = mapnik::image_dtype_gray64f
	};

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
	ref class ImageView;

	typedef std::shared_ptr<mapnik::image_any> image_ptr;

	public ref class Image
	{
	public:
		Image(System::Int32 width, System::Int32 height);
		Image(System::Int32 width, System::Int32 height, System::Collections::Generic::IDictionary<System::String^, System::Object^>^ options);
		~Image();

		Color^ GetPixel(System::Int32 x, System::Int32 y);
		void SetPixel(System::Int32 x, System::Int32 y, Color^ value);
		ImageView^ View(System::Int32 x, System::Int32 y, System::Int32 w, System::Int32 h);
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

