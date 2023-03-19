#include "PosBot.h"

size_t base = reinterpret_cast<size_t>(GetModuleHandle(0));

// PosBot stuff
const char* mode = "Record";
const char* selectedMode = "Record";
int frame = 0;
bool inLevel = false;
bool mouse1Down = false;
bool mouse2Down = false;
bool player1Dropping = false;
bool player2Dropping = false;

// Dual Click
bool EnableDualClick = false;

// Spam
bool EnableSpam = false;

// Frame Advance
bool EnableFrameAdvance = false;
int FrameAdvanceKey = 0;
bool WaitForSetFrameAdvanceKeybind = false;
bool Update = false;

// Fake Clicks
bool EnableFakeClicks = false;
bool ClicksExist = false;

// Practice Fix
bool EnablePracticeFix = false;

// Lock Delta
bool EnableLockDelta = false;

// Frame Fixes
bool EnableFrameFixes = false;

// RGB Menu
bool EnableRGBMenu = false;
bool EnableRGBMenuPrev = false;
int rDir = -1;
int gDir = 1;
int bDir = -1;

// Menu Keybind
bool waitForFirstClick = false;

// Speedhack
float speedhackInputBox = 1.0f;
float speedhackSpeed = 1.0f;
bool speedhackBtnPressed = false;
void* channel;
float speed;
bool initialized = false;
bool EnableSpeedhackAudio = false;

// FPS Bypass
float fpsbypassCapInput = 60.0f;
float fpsbypassCap = 60.0f;
bool fpsbypassBtnPressed = false;

// Macro Complete
bool showedMacroComplete = false;

// Macro Name Box
static char macroName[30] = "";

// Macro Explorer
bool MXButtonPressed = false;

// ImGui
ImGuiWindowFlags window_flags = 0;
float BGColour[4] = { 31.0f, 31.0f, 31.0f, 1.0f };
float LightColour[4] = { 222.0f, 1.0f, 72.0f, 1.0f };
float DarkColour[4];
float VeryLightColour[4];
bool savedBtnPressed = false;
bool loadBtnPressed = false;
bool KeybindToggle = false;
bool showUI = true;
bool waitForSetKeybind = false;
int keybind = 0;
bool rewinding = false;
int maxFrame = 0;
bool SelectMacro = false;

std::map<int, const char*> Keys = { 
	{KEY_A, "A"},
	{KEY_B, "B"},
	{KEY_C, "C"},
	{KEY_D, "D"},
	{KEY_E, "E"},
	{KEY_F, "F"},
	{KEY_G, "G"},
	{KEY_H, "H"},
	{KEY_I, "I"},
	{KEY_J, "J"},
	{KEY_K, "K"},
	{KEY_L, "L"},
	{KEY_M, "M"},
	{KEY_N, "N"},
	{KEY_O, "O"},
	{KEY_P, "P"},
	{KEY_Q, "Q"},
	{KEY_R, "R"},
	{KEY_S, "S"},
	{KEY_T, "T"},
	{KEY_U, "U"},
	{KEY_V, "V"},
	{KEY_W, "W"},
	{KEY_X, "X"},
	{KEY_Y, "Y"},
	{KEY_Z, "Z"}
};

std::vector<Checkpoint> Checkpoints = {};
std::vector<int> CheckpointFrames = {};
std::vector<Frame> Frames = {};

// Higher FPS = Lower DT
// DT = 1 / fps

void __fastcall Scheduler::updateHook(CCScheduler* self, int, float dt) {
	if (!EnableFrameAdvance || (EnableFrameAdvance && Update)) {
		Update = false;
		float deltaTime = dt;
		if (EnableLockDelta) deltaTime = 1 / fpsbypassCap;
		Scheduler::update(self, dt);
		return;
	}
	else return;
}

namespace SpeedhackAudio {
	void* channel;
	float speed;
	bool initialized = false;
	void* (__stdcall* setVolume)(void* t_channel, float volume);
	void* (__stdcall* setFrequency)(void* t_channel, float frequency);
	void* __stdcall SetVolumeHook(void* t_channel, float volume) {
		channel = t_channel;
		if (speed != 1.f) { setFrequency(channel, speed); }
		return setVolume(channel, volume);
	}

	void InitSpeedhackAudio() {
		if (initialized) return;
		setFrequency = (decltype(setFrequency))GetProcAddress(GetModuleHandleW(L"fmod.dll"), "?setPitch@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z");
		DWORD hkAddr = (DWORD)GetProcAddress(GetModuleHandleW(L"fmod.dll"), "?setVolume@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z");
		MH_CreateHook((PVOID)hkAddr, SetVolumeHook, (PVOID*)&setVolume);
		speed = 1.f;
		initialized = true;
	}

	void SetSpeedhackAudio(float frequency) {
		if (!initialized) InitSpeedhackAudio();
		if (channel == nullptr) return;
		speed = frequency;
		setFrequency(channel, frequency);
	}
}

bool __fastcall PlayLayer::initHook(gd::PlayLayer* self, int edx, gd::GJGameLevel* level) {
	bool ret = PlayLayer::init(self, level);
	if (!ret) { return ret; }

	frame = 0;
	maxFrame = 0;
	Frames.clear();
	Checkpoints.clear();
	CheckpointFrames.clear();
	
	inLevel = true;

	return ret;
}

void __fastcall PlayLayer::updateHook(gd::PlayLayer* self, int edx, float dt) {
	if (EnableSpeedhackAudio) { SpeedhackAudio::SetSpeedhackAudio(speedhackSpeed); }
	else { SpeedhackAudio::SetSpeedhackAudio(1.0f); }

	float deltaTime = dt;
	if (EnableLockDelta) deltaTime = 1 / fpsbypassCap;
	PlayLayer::update(self, deltaTime);

	auto director = CCDirector::sharedDirector();
	auto winSize = director->getWinSize();

	if (!rewinding) {

		if (self->m_pPlayer1->getPositionX() == 0) { frame = 0; }
		else { frame++; }

		if (frame > maxFrame) { maxFrame = frame; }
		
		if (mode == "Record") {
			if (frame > 0) {
				if (EnableSpam) {
					if (mouse1Down) PlayLayer::releaseButton(self, 0, true);
					else PlayLayer::pushButton(self, 0, true);
					if (mouse2Down) PlayLayer::releaseButton(self, 0, false);
					else PlayLayer::pushButton(self, 0, false);
				}

				if (self->m_hasCompletedLevel) return;

				if (Frames.size() < frame) Frames.push_back(Frame::from(self, mouse1Down, mouse2Down));
				else Frames.at(frame) = Frame::from(self, mouse1Down, mouse2Down);
			}
		}
		else if (mode == "Playback") {
			if (Frames.size() < frame) {
				frame--;
				if (!showedMacroComplete) {
					CCMenu* macroCompleteMenu = CCMenu::create();
					CCLabelBMFont* macroComplete = CCLabelBMFont::create("Macro Complete", "bigFont.fnt");
					macroCompleteMenu->addChild(macroComplete);
					macroCompleteMenu->setPosition({ 0, 0 });
					macroComplete->setPosition({ CCDirector::sharedDirector()->getWinSize().width / 2, CCDirector::sharedDirector()->getWinSize().height - 50 });
					self->addChild(macroCompleteMenu);
					CCFadeOut* fadeOut = CCFadeOut::create(2.0f);
					macroComplete->runAction(fadeOut);
					showedMacroComplete = true;
				}
			}
			if (self->m_hasCompletedLevel) return;
			if (frame != 0) {
				if (Frames[frame].p1.mouseDown && !mouse1Down) {
					PlayLayer::pushButton(self, 0, true);
					mouse1Down = true;
					if (EnableFakeClicks && ClicksExist) {
						gd::GameSoundManager::sharedState()->playSound("PosBot/Clicks/Click.wav");
					}
				}

				if (!Frames[frame].p1.mouseDown && mouse1Down) {
					PlayLayer::releaseButton(self, 0, true);
					mouse1Down = false;
					if (EnableFakeClicks && ClicksExist) {
						gd::GameSoundManager::sharedState()->playSound("PosBot/Clicks/Release.wav");
					}
				}
				
				if (Frames[frame].p2.mouseDown && !mouse2Down) {
					mouse2Down = true;
					PlayLayer::pushButton(self, 0, false);
					if (EnableFakeClicks && ClicksExist) {
						gd::GameSoundManager::sharedState()->playSound("PosBot/Clicks/Click.wav");
					}
				}
				
				if (!Frames[frame].p2.mouseDown && mouse2Down) {
					mouse2Down = false;
					PlayLayer::releaseButton(self, 0, false);
					if (EnableFakeClicks && ClicksExist) {
						gd::GameSoundManager::sharedState()->playSound("PosBot/Clicks/Release.wav");
					}
				}
				Frames[frame].restore(self, EnableFrameFixes);
			}
		}
	}
	else {
		Frames[frame].restore(self, EnableFrameFixes);
		if (Frames[frame].p1.mouseDown && !mouse1Down) { PlayLayer::pushButton(self, 0, true); mouse1Down = true; }
		if (!Frames[frame].p1.mouseDown && mouse1Down) { PlayLayer::releaseButton(self, 0, true); mouse1Down = false; }
		if (Frames[frame].p2.mouseDown && !mouse2Down) { PlayLayer::pushButton(self, 0, false); mouse2Down = true; }
		if (!Frames[frame].p2.mouseDown && mouse2Down) { PlayLayer::releaseButton(self, 0, false); mouse2Down = false; }
	}
}

void __fastcall PlayLayer::resetLevelHook(gd::PlayLayer* self) {
	PlayLayer::resetLevel(self);
	mode = selectedMode;
	showedMacroComplete = false;

	if (!gd::GameManager::sharedState()->getPlayLayer()->m_isPracticeMode) {
		frame = 0;
		maxFrame = 0;
		if (mode == "Record") waitForFirstClick = true;
	}
	else {
		if (mode == "Record") {
			if (CheckpointFrames.size() == 0) CheckpointFrames.push_back(0);

			Frames.resize(CheckpointFrames.back());
		}
		if ((int)CheckpointFrames.back() != 0) { frame = (int)CheckpointFrames.back(); }
		else {
			frame = 0;
			maxFrame = 0;
		}
		if (mode == "Record") {
			// Practice Fix
			if (EnablePracticeFix) {
				if (Checkpoints.size() == 0) return;
				Checkpoints.back().restore(self);
			}
		}
	}

	if (Frames.size() == 0 && mode == "Record") {
		Frame f = Frame::from(self, mouse1Down, mouse2Down);
		Frames.push_back(f);
	}
}

bool __fastcall PlayLayer::pushButtonHook(gd::PlayLayer* self, uintptr_t, int state, bool player) {
	mouse1Down = player;
	mouse2Down = !player;

	if (EnableDualClick) mouse1Down = true; mouse2Down = true;

	if (mode == "Record" || mode == "Disabled") { 
		if (EnableDualClick) {
			PlayLayer::pushButton(self, state, false);
			PlayLayer::pushButton(self, state, true);
		}
		else { PlayLayer::pushButton(self, state, player); }
	}
	if (mode == "Record") {
		if (waitForFirstClick && !self->m_isPracticeMode) {
			waitForFirstClick = false;
			if (!rewinding) {
				Frames.resize(frame);
			}
		}
	}

	return true;
}

bool __fastcall PlayLayer::releaseButtonHook(gd::PlayLayer* self, uintptr_t, int state, bool player) {
	mouse1Down = !player;
	mouse2Down = player;
	if (EnableDualClick) mouse1Down = false; mouse2Down = false;
	if (mode == "Record" || mode == "Disabled") {
		if (EnableDualClick) {
			PlayLayer::releaseButton(self, state, false);
			PlayLayer::releaseButton(self, state, true);
		}
		else {
			PlayLayer::releaseButton(self, state, player);
		}
	}
	return true;
}

void __fastcall PlayLayer::onQuitHook(gd::PlayLayer* self) {
	PlayLayer::onQuit(self);
	inLevel = false;
}

int __fastcall PlayLayer::createCheckpointHook(gd::PlayLayer* self) {
	auto ret = PlayLayer::createCheckpoint(self);
	Checkpoints.push_back(Checkpoint::from(self));
	CheckpointFrames.push_back(frame);
	return ret;
}

int __fastcall PlayLayer::removeCheckpointHook(gd::PlayLayer* self) {
	auto ret = PlayLayer::removeCheckpoint(self);
	if (Checkpoints.size() > 0) Checkpoints.pop_back();
	if (CheckpointFrames.size() > 0) CheckpointFrames.pop_back();
	return ret;
}

void __fastcall PlayLayer::togglePracticeModeHook(gd::PlayLayer* self, int edx, bool toggle) {
	return PlayLayer::togglePracticeMode(self, toggle);
}

bool __fastcall LevelEditorLayer::initHook(gd::LevelEditorLayer* self, int edx, gd::GJGameLevel* level) {
	if (!LevelEditorLayer::init(self, level)) return false;
	inLevel = false;
	return true;
}

std::ostream& operator<<(std::ostream& out, const Frame& frame) {
	return out.write(reinterpret_cast<const char*>(&frame), sizeof(frame));
}

std::istream& operator>>(std::istream& in, Frame& frame) {
	return in.read(reinterpret_cast<char*>(&frame), sizeof(frame));
}

void PosBot::SaveMacro(std::string macroName) {
	int val = _mkdir("PosBot");

	std::string a = "PosBot/" + macroName + ".pbor";
	std::ofstream outfile(a.c_str(), std::ios_base::binary);
	std::copy(Frames.begin(), Frames.end(), std::ostream_iterator<Frame>(outfile));
	outfile.close();

	// Json output

	a = "PosBot/" + macroName + ".json";
	std::fstream jsonOutFile;
	jsonOutFile.open(a.c_str(), std::ios::out);
	jsonOutFile << "{\n";
	int fileFrame = 0;
	for (Frame const &frame : Frames) {
		jsonOutFile << "    \"" << fileFrame << "\": {\n";

		jsonOutFile << "        \"p1\": {\n";
		jsonOutFile << "            \"x_pos\": " << frame.p1.x_pos << ",\n";
		jsonOutFile << "            \"y_pos\": " << frame.p1.y_pos << ",\n";
		jsonOutFile << "            \"rotation_x\": " << frame.p1.rotation_x << ",\n";
		jsonOutFile << "            \"rotation_y\": " << frame.p1.rotation_y << ",\n";
		jsonOutFile << "            \"is_upside_down\": " << frame.p1.is_upside_down << ",\n";
		jsonOutFile << "            \"mouseDown\": " << frame.p1.mouseDown << ",\n";
		jsonOutFile << "        },\n";

		jsonOutFile << "        \"p2\": {\n";
		jsonOutFile << "            \"x_pos\": " << frame.p2.x_pos << ",\n";
		jsonOutFile << "            \"y_pos\": " << frame.p2.y_pos << ",\n";
		jsonOutFile << "            \"rotation_x\": " << frame.p2.rotation_x << ",\n";
		jsonOutFile << "            \"rotation_y\": " << frame.p2.rotation_y << ",\n";
		jsonOutFile << "            \"is_upside_down\": " << frame.p2.is_upside_down << ",\n";
		jsonOutFile << "            \"mouseDown\": " << frame.p2.mouseDown<< ",\n";
		jsonOutFile << "        }\n";
		jsonOutFile << "    },\n";

		fileFrame++;
	}
	jsonOutFile << "}";
}

void PosBot::LoadMacro(std::string macroName) {
	Frames.clear();
	Checkpoints.clear();
	CheckpointFrames.clear();
	std::string a = "PosBot/" + macroName + ".pbor";
	std::ifstream infile(a.c_str(), std::ios_base::binary);
	std::istream_iterator<Frame> iter(infile);
	std::copy(iter, std::istream_iterator<Frame>(), std::back_inserter(Frames));
	infile.close();
}

void __fastcall Extra::dispatchKeyboardMSGHook(void* self, void*, int key, bool down) {
	dispatchKeyboardMSG(self, key, down);
	// Menu keybind
	if (!waitForSetKeybind) {
		KeybindToggle = !KeybindToggle;
		if (key == keybind && KeybindToggle) { showUI = !showUI; }
	}
	else {
		if (Keys.count(key) > 0) {
			gd::GameManager::sharedState()->setIntGameVariable("posbotkeybind", key);
			waitForSetKeybind = false;
			KeybindToggle = true;
		}
	}

	// Frame advance keybind
	if (WaitForSetFrameAdvanceKeybind) {
		if (Keys.count(key) > 0) {
			gd::GameManager::sharedState()->setIntGameVariable("posbotframeadvancekeybind", key);
			WaitForSetFrameAdvanceKeybind = false;
		}
	}

	if (key == FrameAdvanceKey && down) Update = true;
}

bool fontadded = false;

ImVec4 PosBot::RGBAtoIV4(float rgb[4]) {
	float newr = rgb[0] / 255.0f;
	float newg = rgb[1] / 255.0f;
	float newb = rgb[2] / 255.0f;
	float newa = rgb[3];
	return ImVec4(newr, newg, newb, newa);
}

ImVec4 PosBot::HEXAtoIV4(const char* hex, float a) {
	int r, g, b;
	auto ret = sscanf_s(hex, "%02x%02x%02x", &r, &g, &b);
	return ImVec4(r/255.0f, g/255.0f, b/255.0f, a);
}

void PosBot::RenderGUI() {
	// ImGui init stuff
	auto& style = ImGui::GetStyle();
	style.ColorButtonPosition = ImGuiDir_Left;
	ImVec4* colours = ImGui::GetStyle().Colors;
	ImGuiIO& io = ImGui::GetIO();

	// Loading stuff
	if (!fontadded) {

		// Load the font
		fontadded = true;
		io.Fonts->AddFontDefault();
		
		// Load the colours if they exist otherwise 
		// use the default ones which are defined
		// at the top of of the file
		if (	gd::GameManager::sharedState()->getIntGameVariable("posbotcolour0") != 0
				|| gd::GameManager::sharedState()->getIntGameVariable("posbotcolour1") != 0
				|| gd::GameManager::sharedState()->getIntGameVariable("posbotcolour2") != 0) {

			LightColour[0] = gd::GameManager::sharedState()->getIntGameVariable("posbotcolour0");
			LightColour[1] = gd::GameManager::sharedState()->getIntGameVariable("posbotcolour1");
			LightColour[2] = gd::GameManager::sharedState()->getIntGameVariable("posbotcolour2");
		}
		else {
			gd::GameManager::sharedState()->setIntGameVariable("posbotcolour0", LightColour[0]);
			gd::GameManager::sharedState()->setIntGameVariable("posbotcolour1", LightColour[1]);
			gd::GameManager::sharedState()->setIntGameVariable("posbotcolour2", LightColour[2]);
		}

		// Sort out the keybind
		keybind = gd::GameManager::sharedState()->getIntGameVariable("posbotkeybind");
		if (keybind == 0) {
			gd::GameManager::sharedState()->setIntGameVariable("posbotkeybind", KEY_P);
			keybind = gd::GameManager::sharedState()->getIntGameVariable("posbotkeybind");
		}

		// Sort out frame advance key
		FrameAdvanceKey = gd::GameManager::sharedState()->getIntGameVariable("posbotframeadvancekeybind");
		if (FrameAdvanceKey == 0) {
			gd::GameManager::sharedState()->setIntGameVariable("posbotframeadvancekeybind", KEY_A);
			FrameAdvanceKey = gd::GameManager::sharedState()->getIntGameVariable("posbotframeadvancekeybind");
		}
	}

	// Fix for UI Dissapearing after setting keybind
	if (keybind != gd::GameManager::sharedState()->getIntGameVariable("posbotkeybind")) {
		showUI = true;
		keybind = gd::GameManager::sharedState()->getIntGameVariable("posbotkeybind");
	}

	FrameAdvanceKey = gd::GameManager::sharedState()->getIntGameVariable("posbotframeadvancekeybind");

	if (EnableRGBMenu != EnableRGBMenuPrev) {
		EnableRGBMenuPrev = EnableRGBMenu;
		if (EnableRGBMenu) {
			LightColour[0] = 254;
			LightColour[1] = 1;
			LightColour[2] = 127;
			rDir = -1;
			gDir = 1;
			bDir = -1;
		}
		else {
			LightColour[0] = gd::GameManager::sharedState()->getIntGameVariable("posbotcolour0");
			LightColour[1] = gd::GameManager::sharedState()->getIntGameVariable("posbotcolour1");
			LightColour[2] = gd::GameManager::sharedState()->getIntGameVariable("posbotcolour2");
		}
	}
	if (EnableRGBMenu) {
		LightColour[0] = LightColour[0] + rDir;
		LightColour[1] = LightColour[1] + gDir;
		LightColour[2] = LightColour[2] + bDir;

		if (LightColour[0] >= 255 || LightColour[0] <= 0) { rDir = rDir * -1; }
		if (LightColour[1] >= 255 || LightColour[1] <= 0) { gDir = gDir * -1; }
		if (LightColour[2] >= 255 || LightColour[2] <= 0) { bDir = bDir * -1; }
	}

	// Darken and Lighten Colours for the Theme
	DarkColour[0] = (LightColour[0] * 0.5f);
	DarkColour[1] = (LightColour[1] * 0.5f);
	DarkColour[2] = (LightColour[2] * 0.5f);
	DarkColour[3] = LightColour[3];
	VeryLightColour[0] = (LightColour[0] * 1.5f);
	VeryLightColour[1] = (LightColour[1] * 1.5f);
	VeryLightColour[2] = (LightColour[2] * 1.5f);
	VeryLightColour[3] = LightColour[3];

	// Store the colors in a game variable
	gd::GameManager::sharedState()->setIntGameVariable("posbotcolour0", (int)LightColour[0]);
	gd::GameManager::sharedState()->setIntGameVariable("posbotcolour1", (int)LightColour[1]);
	gd::GameManager::sharedState()->setIntGameVariable("posbotcolour2", (int)LightColour[2]);

	// Setup the colours and make it look nice
	style.FrameRounding = 4.0f;
	style.GrabRounding = 4.0f;
	style.Alpha = 1.f;
	style.WindowRounding = 12.f;
	style.FrameRounding = 4.f;
	style.ScrollbarSize = 2.f;
	style.ScrollbarRounding = 12.f;
	style.PopupRounding = 4.f;
	style.WindowBorderSize = 3.f;
	colours[ImGuiCol_TitleBg] = PosBot::RGBAtoIV4(BGColour);
	colours[ImGuiCol_TitleBgActive] = PosBot::RGBAtoIV4(BGColour);
	colours[ImGuiCol_WindowBg] = PosBot::RGBAtoIV4(BGColour);
	colours[ImGuiCol_Border] = PosBot::RGBAtoIV4(LightColour);
	colours[ImGuiCol_FrameBg] = PosBot::RGBAtoIV4(DarkColour);
	colours[ImGuiCol_FrameBgHovered] = PosBot::RGBAtoIV4(DarkColour);
	colours[ImGuiCol_FrameBgActive] = PosBot::RGBAtoIV4(LightColour);
	colours[ImGuiCol_PlotHistogram] = PosBot::RGBAtoIV4(LightColour);
	colours[ImGuiCol_Button] = PosBot::RGBAtoIV4(LightColour);
	colours[ImGuiCol_ButtonHovered] = PosBot::RGBAtoIV4(VeryLightColour);
	colours[ImGuiCol_Header] = PosBot::RGBAtoIV4(DarkColour);
	colours[ImGuiCol_HeaderHovered] = PosBot::RGBAtoIV4(LightColour);
	colours[ImGuiCol_HeaderActive] = PosBot::RGBAtoIV4(VeryLightColour);
	colours[ImGuiCol_SliderGrab] = PosBot::RGBAtoIV4(LightColour);
	colours[ImGuiCol_SliderGrabActive] = PosBot::RGBAtoIV4(VeryLightColour);
	colours[ImGuiCol_CheckMark] = PosBot::RGBAtoIV4(VeryLightColour);
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoScrollbar;

	// Render the UI
	if (showUI) {
		if (ImGui::Begin("PosBot", nullptr, window_flags)) {
			ImGui::SetWindowSize("PosBot", ImVec2(420, 450));
			auto pos = ImGui::GetWindowPos();
			ImGui::Text("PosBot v1.7");

			if (!inLevel) {
				// Do this if you aren't in a level
				ImGui::Text("Head into a level to start!");
			}
			else {
				// Do this if you are
				// Level name
				auto levelText = "Level Name:  " + gd::GameManager::sharedState()->getPlayLayer()->m_level->m_sLevelName;
				ImGui::Text(levelText.c_str());
				ImGui::NewLine();

				// Current frame
				auto text = "Frame:  " + std::to_string(frame);
				ImGui::Text(text.c_str());

				// Progress bar
				float levelLen = (float)gd::GameManager::sharedState()->getPlayLayer()->m_levelLength;
				float fraction = 0;
				if (frame == 0) { fraction = 0; }
				else { fraction = (gd::GameManager::sharedState()->getPlayLayer()->m_pPlayer1->getPositionX() - 50) / levelLen; }
				ImGui::ProgressBar(fraction);
				ImGui::NewLine();

				// Mode select
				ImGui::Text("Mode:  ");
				ImGui::SameLine();
				const char* items[] = { "Record", "Playback", "Disabled" };
				if (ImGui::BeginCombo("##combo", selectedMode))
				{
					for (int n = 0; n < IM_ARRAYSIZE(items); n++)
					{
						bool is_selected = (selectedMode == items[n]);
						if (ImGui::Selectable(items[n], is_selected))
							selectedMode = items[n];
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				ImGui::NewLine();

				// Restart to apply text
				if (selectedMode != mode) { ImGui::Text("Restart to apply"); ImGui::NewLine(); }

				// Practice mode text
				if (gd::GameManager::sharedState()->getPlayLayer()->m_isPracticeMode) { ImGui::Text("Practice Mode"); ImGui::NewLine(); }
				
				// Macro name box
				ImGui::InputText("Level Name", macroName, IM_ARRAYSIZE(macroName));

				// Save button
				if (ImGui::Button("Save Macro") && !savedBtnPressed) {
					savedBtnPressed = true;
					PosBot::SaveMacro(macroName);
				}
				else { savedBtnPressed = false; }
				ImGui::SameLine();

				// Load button
				if (ImGui::Button("Load Macro") && !loadBtnPressed) {
					loadBtnPressed = true;
					SelectMacro = true;
				}
				else { loadBtnPressed = false; }
				ImGui::NewLine();

				if (SelectMacro) {
					if (ImGui::Begin("Select Macro", nullptr, window_flags)) {
						ImGui::Text("Select a Macro");
						ImGui::SetWindowSize(ImVec2(421, 443));
						ImGui::SetWindowPos(ImVec2(pos.x - 425, pos.y));
						auto path = "PosBot/";
						std::vector<std::string>Macros;
						for (const auto& file : std::filesystem::directory_iterator(path)) {
							if (file.is_regular_file()) {
								Macros.insert(Macros.end(), file.path().filename().string());
							}
						}
						for (int i = 0; i <= Macros.size() -1; i++) {
							if (ImGui::Button(Macros.at(i).c_str(), ImVec2(405, 20))) {
								SelectMacro = false;
								PosBot::LoadMacro(Macros.at(i).substr(0, Macros.at(i).size()-5));
							}
						}
					}
					ImGui::End();
				}

				// Features Text
				ImGui::Text("Features");

				// Dual Click
				ImGui::Checkbox("Dual Click", &EnableDualClick);
				ImGui::SameLine();

				// Spam
				ImGui::Checkbox("Spam", &EnableSpam);
				ImGui::SameLine();

				// Frame Advance
				ImGui::Checkbox("Frame Advance", &EnableFrameAdvance);

				// Fake Clicks
				ImGui::Checkbox("Fake Clicks", &EnableFakeClicks);
				ImGui::SameLine();

				// RGB Menu
				ImGui::Checkbox("RGB Menu", &EnableRGBMenu);
				ImGui::SameLine();

				// Speedhack Audio
				ImGui::Checkbox("Speedhack Audio", &EnableSpeedhackAudio);

				// Practice Fix
				ImGui::Checkbox("Practice Fix", &EnablePracticeFix);
				ImGui::SameLine();

				// Lock Delta
				ImGui::Checkbox("Lock Delta", &EnableLockDelta);
				ImGui::SameLine();

				// Frame Fixes
				ImGui::Checkbox("Frame Fixes", &EnableFrameFixes);

				// Fake Clicks Exist
				std::fstream ClickFile;
				std::fstream ReleaseFile;
				ClickFile.open("PosBot/Clicks/Click.wav", std::ios::in);
				ReleaseFile.open("PosBot/Clicks/Release.wav", std::ios::in);
				ClicksExist = ClickFile.is_open() && ReleaseFile.is_open();
				if (!ClicksExist && EnableFakeClicks) {
					ImGui::NewLine();
					ImGui::Text("Clicks do not exist.\nPut Click.wav and Release.wav in PosBot/Clicks/");
				}

				// Menu keybind button
				ImGui::NewLine();
				ImGui::Text("Menu Keybind:  ");
				ImGui::SameLine();
				if (waitForSetKeybind) { ImGui::Button("Waiting for keypress..."); }
				else { if (ImGui::Button(Keys[keybind], ImVec2(80, 20))) { waitForSetKeybind = true; } }

				// Frame advance keybind button
				ImGui::Text("Frame Advance Keybind:  ");
				ImGui::SameLine();
				if (WaitForSetFrameAdvanceKeybind) { ImGui::Button("Waiting for keypress..."); }
				else { if (ImGui::Button(Keys[FrameAdvanceKey], ImVec2(80, 20))) { WaitForSetFrameAdvanceKeybind = true; } }
				ImGui::NewLine();

				// Speedhack
				ImGui::Text("Speedhack: ");
				ImGui::SameLine();
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("", &speedhackInputBox);
				ImGui::PopItemWidth();
				ImGui::SameLine();
				if (ImGui::Button("Apply Speed") && !speedhackBtnPressed) {
					speedhackSpeed = speedhackInputBox;
					speedhackBtnPressed = true;
					SpeedhackAudio::SetSpeedhackAudio(speedhackSpeed);
					Speedhack::InitializeSpeedHack(speedhackSpeed);
				}
				else { speedhackBtnPressed = false; }

				// Fps Bypass
				ImGui::Text("FPS Bypass: ");
				ImGui::SameLine();
				ImGui::PushItemWidth(100);
				ImGui::InputFloat(" ", &fpsbypassCapInput);
				ImGui::PopItemWidth();
				ImGui::SameLine();
				if (ImGui::Button("Apply FPS") && !fpsbypassBtnPressed) {
					fpsbypassBtnPressed = true;
					fpsbypassCap = fpsbypassCapInput;
					FPSBypass::SetFPS(fpsbypassCap);
				}
				else { fpsbypassBtnPressed = false; }
				ImGui::NewLine();

				// Colour Themes O_O
				ImGui::Text("Colour Theme");
				ImGuiSliderFlags slider_flags = 0;
				slider_flags |= ImGuiSliderFlags_AlwaysClamp;
				slider_flags |= ImGuiSliderFlags_NoInput;
				ImGui::SliderFloat("R", &LightColour[0], 0.0f, 255.0f, "%.0f", slider_flags, EnableRGBMenu);
				ImGui::SliderFloat("G", &LightColour[1], 0.0f, 255.0f, "%.0f", slider_flags, EnableRGBMenu);
				ImGui::SliderFloat("B", &LightColour[2], 0.0f, 255.0f, "%.0f", slider_flags, EnableRGBMenu);
				
				// Rewinding
				ImGui::NewLine();
				if (ImGui::Button("Rewind", ImVec2(120, 20)) || rewinding) {
					rewinding = true;
					ImGui::SliderInt("Frame", &frame, 0, maxFrame);
					if (ImGui::Button("Cancel")) {
						frame = maxFrame;
						rewinding = false;
					}
					ImGui::SameLine();
					if (ImGui::Button("Confirm")) {
						Frames.resize(frame);
						maxFrame = frame;
						rewinding = false;
					}
				}

				if (ImGui::Button("Clamp Frames", ImVec2(120, 20))) {
					Frames.resize(frame);
				}

				/*
				// Macro Explorer
				if (ImGui::Button("Macro Explorer", ImVec2(120, 20)) && !MXButtonPressed) {
					MXButtonPressed = true;
				}
				else {
					MXButtonPressed = false;
				}*/
			}
		}
	}
}

void PosBot::mem_init() {

	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	std::cout << "PosBot Initiated" << std::endl;

	SpeedhackAudio::InitSpeedhackAudio();

	MH_CreateHook(
		(PVOID)(base + 0x01FB780),
		PlayLayer::initHook,
		(LPVOID*)&PlayLayer::init
	);

	MH_CreateHook(
		(PVOID)(base + 0x2029C0),
		PlayLayer::updateHook,
		(LPVOID*)&PlayLayer::update
	);

	MH_CreateHook(
		(PVOID)(base + 0x20BF00),
		PlayLayer::resetLevelHook,
		(LPVOID*)&PlayLayer::resetLevel
	);

	MH_CreateHook(
		(PVOID)(base + 0x111500),
		PlayLayer::pushButtonHook,
		(LPVOID*)&PlayLayer::pushButton
	);

	MH_CreateHook(
		(PVOID)(base + 0x111660),
		PlayLayer::releaseButtonHook,
		(LPVOID*)&PlayLayer::releaseButton
	);

	MH_CreateHook(
		(PVOID)(base + 0x20D810),
		PlayLayer::onQuitHook,
		(LPVOID*)&PlayLayer::onQuit
	);

	MH_CreateHook(
		(PVOID)(base + 0x20B050),
		PlayLayer::createCheckpointHook,
		(LPVOID*)&PlayLayer::createCheckpoint
	);

	MH_CreateHook(
		(PVOID)(base + 0x20B830),
		PlayLayer::removeCheckpointHook,
		(LPVOID*)&PlayLayer::removeCheckpoint
	);

	MH_CreateHook(
		(PVOID)(base + 0x15EE00),
		LevelEditorLayer::initHook,
		(LPVOID*)&LevelEditorLayer::init
	);

	MH_CreateHook(
		(PVOID)(base + 0x20D0D1),
		PlayLayer::togglePracticeModeHook,
		(LPVOID*)&PlayLayer::togglePracticeMode
	);

	auto libcocos = GetModuleHandleA("libcocos2d.dll");

	MH_CreateHook(
		(PVOID)(GetProcAddress(libcocos, "?dispatchKeyboardMSG@CCKeyboardDispatcher@cocos2d@@QAE_NW4enumKeyCodes@2@_N@Z")),
		Extra::dispatchKeyboardMSGHook,
		(LPVOID*)&Extra::dispatchKeyboardMSG
	);
	
	MH_CreateHook(
		GetProcAddress(libcocos, "?update@CCScheduler@cocos2d@@UAEXM@Z"),
		Scheduler::updateHook,
		(LPVOID*)&Scheduler::update
	);
}