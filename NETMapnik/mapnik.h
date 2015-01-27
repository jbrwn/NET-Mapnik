#pragma once

namespace NETMapnik
{
	public ref class Mapnik abstract sealed
	{
	public:
		static Mapnik();
		static System::Boolean RegisterFonts(System::String^ dir, bool recurse);
		static System::Boolean RegisterFonts(System::String^ dir);
		static System::Boolean RegisterDefaultFonts();
		static System::Boolean RegisterSystemFonts();
		static System::Collections::Generic::IEnumerable<System::String^>^ Fonts();
		static System::Collections::Generic::IDictionary<System::String^,System::String^>^ FontFiles();
		static System::Collections::Generic::IEnumerable<System::String^>^ MemoryFonts();

		static System::Boolean RegisterDatasource(System::String^ path);
		static System::Boolean RegisterDatasources(System::String^ path);
		static void RegisterDefaultInputPlugins();
		static System::Collections::Generic::IEnumerable<System::String^>^ Datasources();


		static property System::Collections::Generic::IDictionary<System::String^, System::Boolean>^ Supports
		{
			System::Collections::Generic::IDictionary<System::String^, System::Boolean>^ get()
			{
				return gcnew System::Collections::ObjectModel::ReadOnlyDictionary<System::String^, System::Boolean>(_supports);
			}
		}

		static property System::Collections::Generic::IDictionary<System::String^, System::String^>^ Versions
		{
			System::Collections::Generic::IDictionary<System::String^, System::String^>^ get()
			{
				return gcnew System::Collections::ObjectModel::ReadOnlyDictionary<System::String^, System::String^>(_versions);
			};
		}

		static property System::Collections::Generic::IDictionary<System::String^, System::String^>^ Paths
		{
			System::Collections::Generic::IDictionary<System::String^, System::String^>^ get()
			{
				return gcnew System::Collections::ObjectModel::ReadOnlyDictionary<System::String^, System::String^>(_paths);
			};
		}
	private:
		static initonly System::Collections::Generic::Dictionary<System::String^, System::String^>^ _versions;
		static initonly System::Collections::Generic::Dictionary<System::String^, System::Boolean>^ _supports;
		static initonly System::Collections::Generic::Dictionary<System::String^, System::String^>^ _paths;
	};

}