#include "FPSMultiplier.h"

float g_target_fps = 240;
bool g_enabled = true;
bool g_disable_render = false;
float g_left_over = 0.f;

void(__thiscall* CCScheduler_update)(CCScheduler*, float);
void __fastcall CCScheduler_update_H(CCScheduler* self, int, float dt) {
    if (!g_enabled)
        return CCScheduler_update(self, dt);
    auto speedhack = self->getTimeScale();
    const float newdt = 1.f / g_target_fps / speedhack;
    g_disable_render = true;

    const int times = min(static_cast<int>((dt + g_left_over) / newdt), 100); // limit it to 100x just in case
    for (int i = 0; i < times; ++i) {
        if (i == times - 1)
            g_disable_render = false;
        CCScheduler_update(self, newdt);
    }
    g_left_over += dt - newdt * times;
}

void(__thiscall* PlayLayer_updateVisibility)(void*);
void __fastcall PlayLayer_updateVisibility_H(void* self) {
    if (!g_disable_render)
        PlayLayer_updateVisibility(self);
}

void FPSMultiplierSetup() {

    auto base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));
    auto libcocos = GetModuleHandleA("libcocos2d.dll");

    MH_CreateHook((void*)(base + 0x205460), PlayLayer_updateVisibility_H, (void**)&PlayLayer_updateVisibility);
    MH_CreateHook(GetProcAddress(libcocos, "?update@CCScheduler@cocos2d@@UAEXM@Z"), CCScheduler_update_H, (void**)&CCScheduler_update);

    MH_EnableHook(MH_ALL_HOOKS);
}