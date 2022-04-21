#pragma once
#include <windows.h>
#include <iostream>
#include <thread>

typedef void* (__cdecl* fSharedApplication)();
typedef void(__thiscall* fSetAnimationInterval)(void* instance, double delay);
extern fSharedApplication sharedApplication;
extern fSetAnimationInterval setAnimInterval;
extern float interval;

namespace FPSBypass {
	void SetFPS(int FPS);
}