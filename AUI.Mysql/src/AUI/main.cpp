#ifdef _WIN32
#include <windows.h>

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{
	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
#endif

#include <mysql.h>
#include "AUI/Common/Plugin.h"
#include "AUI/Data/ASqlDatabase.h"
#include "AMysql.h"

AUI_PLUGIN_ENTRY {
    mysql_library_init(0, nullptr, nullptr);
    ASqlDatabase::registerDriver(_new<AMysql>());
}