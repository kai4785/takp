#include "log.h"
#include "globals.h"

#include <windows.h>
#include <winuser.h>

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved )  // reserved
{
    // Perform actions based on the reason for calling.
    LPCSTR libname;
    switch( fdwReason )
    {
        case DLL_PROCESS_ATTACH:
        {
            // Initialize once for each new process.
            // Return FALSE to fail DLL load.
            libname = "d3d8-dxvk.dll";
            g_d3d8 = LoadLibrary(libname);
            if (!g_d3d8)
            {
                libname = "C:\\Windows\\System32\\d3d8.dll";
                g_d3d8 = LoadLibrary(libname);
            }
            log("Loaded %s\n", libname);

            break;

        }
        case DLL_THREAD_ATTACH:
            // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
            // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:

            if (lpvReserved != NULL)
            {
                break; // do not do cleanup if process termination scenario
            }

            FreeLibrary(g_d3d8);

            // Perform any necessary cleanup.
            break;
    }

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}