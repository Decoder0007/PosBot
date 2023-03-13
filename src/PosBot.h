#pragma once
#include <cocos2d.h>
#include "MinHook.h"
#include "gd.h"
#include "imgui-hook.hpp"
#include "imgui.h"
#include <direct.h>
#include "FPSBypass.h"
#include "Speedhack.h"
#include "Checkpoint.h"
#include "Frame.h"
#include <string>
#include <fstream>

using namespace cocos2d;

namespace Scheduler {
	inline void(__thiscall* update)(CCScheduler*, float);
	void __fastcall updateHook(CCScheduler* self, int, float dt);
}

namespace PlayLayer {
	inline bool(__thiscall* init)(gd::PlayLayer* self, gd::GJGameLevel* GJGameLevel);
	bool __fastcall initHook(gd::PlayLayer* self, int edx, gd::GJGameLevel* GJGameLevel);

	inline void(__thiscall* update)(gd::PlayLayer* self, float deltatime);
	void __fastcall updateHook(gd::PlayLayer* self, int edx, float deltatime);

	inline void(__thiscall* resetLevel)(gd::PlayLayer* self);
	void __fastcall resetLevelHook(gd::PlayLayer* self);

	inline bool(__thiscall* pushButton)(gd::PlayLayer* self, int state, bool player);
	bool __fastcall pushButtonHook(gd::PlayLayer* self, uintptr_t, int state, bool player);

	inline bool(__thiscall* releaseButton)(gd::PlayLayer* self, int state, bool player);
	bool __fastcall releaseButtonHook(gd::PlayLayer* self, uintptr_t, int state, bool player);

	inline void(__thiscall* onQuit)(gd::PlayLayer* self);
	void __fastcall onQuitHook(gd::PlayLayer* self);

	inline int(__thiscall* createCheckpoint)(gd::PlayLayer* self);
	int __fastcall createCheckpointHook(gd::PlayLayer* self);

	inline int(__thiscall* removeCheckpoint)(gd::PlayLayer* self);
	int __fastcall removeCheckpointHook(gd::PlayLayer* self);

	inline void(__thiscall* togglePracticeMode)(gd::PlayLayer* self, bool toggle);
	void __fastcall togglePracticeModeHook(gd::PlayLayer* self, int edx, bool toggle);
}

namespace LevelEditorLayer {
	inline bool(__thiscall* init)(gd::LevelEditorLayer* self, gd::GJGameLevel* GJGameLevel);
	bool __fastcall initHook(gd::LevelEditorLayer* self, int edx, gd::GJGameLevel* GJGameLevel);
}

namespace Extra {
	inline void(__thiscall* dispatchKeyboardMSG)(void* self, int key, bool down);
	void __fastcall dispatchKeyboardMSGHook(void* self, void*, int key, bool down);
}

namespace PosBot {
	ImVec4 RGBAtoIV4(float rgb[4]);
	ImVec4 HEXAtoIV4(const char* hex, float a);
	void RenderGUI();
	void mem_init();
	void SaveMacro(std::string macroName);
	void LoadMacro(std::string macroName);
	template<class T>
	bool Write(uint32_t vaddress, const T& value) {
		return WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(vaddress), &value, sizeof(T), NULL);
	}
}