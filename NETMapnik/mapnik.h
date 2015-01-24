#pragma once

namespace NETMapnik
{
	public ref class Mapnik abstract sealed
	{
	public:
		static Mapnik();
		//static bool RegisterFonts(System::String^ dir, bool recurse);
		//static bool RegisterFonts(System::String^ dir);
		//static System::Collections::Generic::IEnumerable<System::String^>^ Fonts();
		//static System::Collections::Generic::IEnumerable<System::String^>^ FontFiles();
		//static System::Collections::Generic::IEnumerable<System::String^>^ MemoryFonts();

		//static bool RegisterDatasource(System::String^ path);
		//static bool RegisterDatasources(System::String^ path);
		//static System::Collections::Generic::IEnumerable<System::String^>^ Datasources();

		static property System::Collections::Generic::IReadOnlyDictionary<System::String^, System::Boolean>^ Supports
		{
			System::Collections::Generic::IReadOnlyDictionary<System::String^, System::Boolean>^ get()
			{
				return gcnew System::Collections::ObjectModel::ReadOnlyDictionary<System::String^, System::Boolean>(_supports);
			}
		}

		static property System::Collections::Generic::IReadOnlyDictionary<System::String^, System::String^>^ Versions
		{
			System::Collections::Generic::IReadOnlyDictionary<System::String^, System::String^>^ get()
			{
				return gcnew System::Collections::ObjectModel::ReadOnlyDictionary<System::String^, System::String^>(_versions);
			};
		}
	private:
		static initonly System::Collections::Generic::Dictionary<System::String^, System::String^>^ _versions;
		static initonly System::Collections::Generic::Dictionary<System::String^, System::Boolean>^ _supports;
	};

}