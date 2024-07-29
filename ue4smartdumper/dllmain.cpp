#include "global_settings.h"
#include <Windows.h>
#include <iostream>

#include "dumper.h"

#pragma comment(lib, "ue4lib")
void run(HMODULE hModule)
{
    if (app::show_console)
    {
        AllocConsole();
        FILE* Dummy;
        freopen_s(&Dummy, "CONOUT$", "w", stdout);
        freopen_s(&Dummy, "CONIN$", "r", stdin);
    }

    dumper::init();
    dumper::dump();
}

// Exporting function usable with SetWindowsHookEx
extern "C" __declspec(dllexport) int NextHook(int code, WPARAM wParam, LPARAM lParam) {
    return CallNextHookEx(NULL, code, wParam, lParam);
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)run, hModule, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

