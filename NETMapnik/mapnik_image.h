#pragma once

#include <mapnik\graphics.hpp>

namespace NETMapnik
{
	public ref class Image
	{
	public:
		Image(System::Int32 width, System::Int32 height);
		~Image();

		void Save(System::String^ path, System::String^ format);
		array<System::Byte>^ Encode(System::String^ format);

	internal:
		mapnik::image_32 *NativeObject();

	private:
		mapnik::image_32 *_image_32;
	};
}

