#include "stdafx.h"
#include <Windows.h>

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOLEAN WINAPI DllMain(IN HINSTANCE hDllHandle,
	IN DWORD     nReason,
	IN LPVOID    Reserved)
{
	switch (nReason)
	{
		case DLL_PROCESS_ATTACH:
			_putenv_s("PROJ_LIB", "share\\proj");
			_putenv_s("ICU_DATA", "share\\icu");
			_putenv_s("GDAL_DATA", "share\\gdal");
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif