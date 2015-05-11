#pragma once

#include <memory>
#include <mapnik\image.hpp>
#include <mapnik\image_view_any.hpp>

namespace NETMapnik
{
	//Forward Declare
	ref class Image;
	ref class Color;
	ref class Palette;

	typedef std::shared_ptr<mapnik::image_view_any> image_view_ptr;

	public ref class ImageView
	{
	public:
		~ImageView();
		System::Boolean IsSolid();
		System::Int32 Width();
		System::Int32 Height();
		array<System::Byte>^ Encode(System::String^ format);
		array<System::Byte>^ Encode(System::String^ format, Palette^ palette);
		void Save(System::String^ path);
		void Save(System::String^ path, System::String^ format);
		Color^ GetPixel(System::Int32 x, System::Int32 y);

	internal:
		ImageView(Image^ image, unsigned x, unsigned y, unsigned w, unsigned h);
		image_view_ptr NativeObject();
	private:
		image_view_ptr* _img_view;
	};
}


