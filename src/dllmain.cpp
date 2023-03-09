// dllmain.cpp : Defines the entry point for the DLL application.
#include "PosBot.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Speedhack::InintDLL, (LPVOID)hModule, 0, NULL);
            DisableThreadLibraryCalls(hModule);
            MH_Initialize();
            ImGuiHook::setupHooks([](void* target, void* hook, void** trampoline) { MH_CreateHook(target, hook, trampoline); });
            ImGuiHook::setRenderFunction(PosBot::RenderGUI);
            PosBot::mem_init();
            MH_EnableHook(MH_ALL_HOOKS);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}