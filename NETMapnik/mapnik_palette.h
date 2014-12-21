#pragma once

#include <memory>

#include <mapnik\palette.hpp>

namespace NETMapnik
{
	public enum class PaletteType : int
	{
		PALETTE_RGBA = mapnik::rgba_palette::PALETTE_RGBA,
		PALETTE_RGB = mapnik::rgba_palette::PALETTE_RGB,
		PALETEE_ACT = mapnik::rgba_palette::PALETTE_ACT
	};

	typedef std::shared_ptr<mapnik::rgba_palette> palette_ptr;

	public ref class Palette
	{
	public:
		Palette(System::String^ palette);
		Palette(System::String^ palette, PaletteType type);
		Palette(array<System::Byte>^ palette);
		Palette(array<System::Byte>^ palette, PaletteType type);

		System::String^ ToString() override;
		array<System::Byte>^ ToBytes();
		~Palette();
	internal:
		palette_ptr NativeObject();
	private:
		palette_ptr* _palette;
	};

}