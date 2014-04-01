#pragma once

namespace NETMapnik
{
	public ref class DatasourceCache abstract sealed
	{
	public:
		static void RegisterDatasources(System::String^ path);
	};

}