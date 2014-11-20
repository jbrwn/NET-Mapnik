#pragma once

#include <mapnik\color.hpp>

namespace NETMapnik
{
	public ref class Color
	{
	public:
		Color(System::String^ color);
		Color(System::UInt32 r, System::UInt32 g, System::UInt32 b);
		Color(System::UInt32 r, System::UInt32 g, System::UInt32 b, System::UInt32 a);
		~Color();

		property System::UInt32 R
		{
			System::UInt32 get();
			void set(System::UInt32);
		}

		property System::UInt32 G
		{
			System::UInt32 get();
			void set(System::UInt32);
		}

		property System::UInt32 B
		{
			System::UInt32 get();
			void set(System::UInt32);
		}

		property System::UInt32 A
		{
			System::UInt32 get();
			void set(System::UInt32);
		}

		 System::String^ ToString() override;
		 System::String^ Hex();

	internal:
		mapnik::color *NativeObject();

	private:
		mapnik::color* _color;
	};
}
