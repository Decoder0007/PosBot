#include "FPSBypass.h"

fSharedApplication sharedApplication;
fSetAnimationInterval setAnimInterval;
float interval = 0;

void thread(void* app, float frames) {
	while (interval == frames) {
		setAnimInterval(app, frames);
	}
}

void FPSBypass::SetFPS(int FPS) {
	interval = 1.0f / FPS;

	HMODULE hMod = LoadLibrary(L"libcocos2d.dll");
	sharedApplication = (fSharedApplication)GetProcAddress(hMod, "?sharedApplication@CCApplication@cocos2d@@SAPAV12@XZ");
	setAnimInterval = (fSetAnimationInterval)GetProcAddress(hMod, "?setAnimationInterval@CCApplication@cocos2d@@UAEXN@Z");

	void* application = sharedApplication();

	std::thread t(thread, application, interval);
	t.detach();
}