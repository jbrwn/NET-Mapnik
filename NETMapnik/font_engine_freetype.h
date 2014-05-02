#pragma once

namespace NETMapnik
{
	public ref class freetype_engine abstract sealed
	{
	public:
		static bool RegisterFonts(System::String^ dir, bool recurse);
		static void RegisterFont(System::String^ file_name);
	};

}
