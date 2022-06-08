// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <cocos2d.h>
#include "MinHook.h"
#include "PosBot.h"

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        MH_Initialize();
        PosBot::mem_init();
        ImGuiHook::setupHooks([](void* target, void* hook, void** trampoline) { MH_CreateHook(target, hook, trampoline); });
        ImGuiHook::setRenderFunction(PosBot::RenderGUI);
        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        std::cout << "PosBot Initiated" << std::endl;
        MH_EnableHook(MH_ALL_HOOKS);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

