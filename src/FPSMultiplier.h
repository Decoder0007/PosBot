#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <MinHook.h>
#include <cocos2d.h>
#include <fstream>
#include <iostream>

using namespace cocos2d;

extern float g_target_fps;
extern bool g_enabled;
extern bool g_disable_render;
extern float g_left_over;

void FPSMultiplierSetup();