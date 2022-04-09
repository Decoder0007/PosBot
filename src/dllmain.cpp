// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <cocos2d.h>
#include "MinHook.h"
#include "PosBot.h"
#include <imgui-hook.hpp>
#include "imgui.h"

DWORD WINAPI my_thread(void* hModule) {
    //Your code goes here
    //====================

    ImGuiHook::setRenderFunction(PosBot::RenderGUI);
    MH_Initialize();
    PosBot::mem_init();
    ImGuiHook::setupHooks([](void* target, void* hook, void** trampoline) { MH_CreateHook(target, hook, trampoline); });
    MH_EnableHook(MH_ALL_HOOKS);
    
    //This line will dettach your DLL when executed. Remove if needed
    //FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(hModule), 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0x1000, my_thread, hModule, 0, 0);
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

