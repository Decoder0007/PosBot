﻿#include "PosBot.h"

size_t base = reinterpret_cast<size_t>(GetModuleHandle(0));

// PosBot stuff
const char* mode = "Record";
const char* selectedMode = "Record";
int frame = 0;
bool inLevel = false;
bool mouse1Down = false;
bool mouse2Down = false;
bool waitForFirstClick = false;
bool showedMacroComplete = false;
static char macroName[30] = "";

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

std::map<std::string, std::variant<std::deque<float>, std::deque<bool>, std::deque<int>>> Player1Data = {
	{"Xpos", std::deque<float>{}},
	{"Ypos", std::deque<float>{}},
	{"Rotation", std::deque<float>{}},
	{"Pushed", std::deque<bool>{}},
	{"Checkpoints", std::deque<int>{}},
	{"Yvelo", std::deque<float>{}}
};

std::map<std::string, std::variant<std::deque<float>, std::deque<bool>, std::deque<int>>> Player2Data = {
	{"Xpos", std::deque<float>{}},
	{"Ypos", std::deque<float>{}},
	{"Rotation", std::deque<float>{}},
	{"Pushed", std::deque<bool>{}},
	{"Yvelo", std::deque<float>{}}
};

bool __fastcall PlayLayer::initHook(gd::PlayLayer* self, int edx, gd::GJGameLevel* level) {
	bool ret = PlayLayer::init(self, level);
	if (!ret) { return ret; }
	frame = 0;
	maxFrame = 0;
	std::get<std::deque<float>>(Player1Data["Xpos"]).clear();
	std::get<std::deque<float>>(Player1Data["Ypos"]).clear();
	std::get<std::deque<float>>(Player1Data["Rotation"]).clear();
	std::get<std::deque<bool>>(Player1Data["Pushed"]).clear();
	std::get<std::deque<int>>(Player1Data["Checkpoints"]).clear();
	std::get<std::deque<float>>(Player1Data["Yvelo"]).clear();
	
	std::get<std::deque<float>>(Player2Data["Xpos"]).clear();
	std::get<std::deque<float>>(Player2Data["Ypos"]).clear();
	std::get<std::deque<float>>(Player2Data["Rotation"]).clear();
	std::get<std::deque<bool>>(Player2Data["Pushed"]).clear();
	std::get<std::deque<float>>(Player2Data["Yvelo"]).clear();

	inLevel = true;

	return ret;
}

void __fastcall PlayLayer::updateHook(gd::PlayLayer* self, int edx, float deltaTime) {
	PlayLayer::update(self, deltaTime);
	if (!rewinding) {

		if (self->m_pPlayer1->getPositionX() == 0) { frame = 0; }
		else { frame++; }

		if (frame > maxFrame) {
			maxFrame = frame;
		}

		if (mode == "Record") {
			if (frame > 0) {
				std::get<std::deque<float>>(Player1Data["Xpos"]).insert(std::get<std::deque<float>>(Player1Data["Xpos"]).end(), self->m_pPlayer1->m_position.x);
				std::get<std::deque<float>>(Player1Data["Ypos"]).insert(std::get<std::deque<float>>(Player1Data["Ypos"]).end(), self->m_pPlayer1->m_position.y);
				std::get<std::deque<float>>(Player1Data["Rotation"]).insert(std::get<std::deque<float>>(Player1Data["Rotation"]).end(), self->m_pPlayer1->getRotation());
				std::get<std::deque<bool>>(Player1Data["Pushed"]).insert(std::get<std::deque<bool>>(Player1Data["Pushed"]).end(), mouse1Down);
				std::get<std::deque<float>>(Player1Data["Yvelo"]).insert(std::get<std::deque<float>>(Player1Data["Yvelo"]).end(), self->m_pPlayer1->m_yAccel);
				
				std::get<std::deque<float>>(Player2Data["Xpos"]).insert(std::get<std::deque<float>>(Player2Data["Xpos"]).end(), self->m_pPlayer2->m_position.x);
				std::get<std::deque<float>>(Player2Data["Ypos"]).insert(std::get<std::deque<float>>(Player2Data["Ypos"]).end(), self->m_pPlayer2->m_position.y);
				std::get<std::deque<float>>(Player2Data["Rotation"]).insert(std::get<std::deque<float>>(Player2Data["Rotation"]).end(), self->m_pPlayer2->getRotation());
				std::get<std::deque<bool>>(Player2Data["Pushed"]).insert(std::get<std::deque<bool>>(Player2Data["Pushed"]).end(), mouse2Down);
				std::get<std::deque<float>>(Player2Data["Yvelo"]).insert(std::get<std::deque<float>>(Player2Data["Yvelo"]).end(), self->m_pPlayer2->m_yAccel);
			}
		}
		else if (mode == "Playback") {
			if ((int)std::get<std::deque<float>>(Player1Data["Xpos"]).size() < frame) {
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
			if (frame != 0) {
				self->m_pPlayer1->m_position.x = std::get<std::deque<float>>(Player1Data["Xpos"])[frame - 1];
				self->m_pPlayer1->m_position.y = std::get<std::deque<float>>(Player1Data["Ypos"])[frame - 1];
				self->m_pPlayer1->setRotation(std::get<std::deque<float>>(Player1Data["Rotation"])[frame - 1]);
				self->m_pPlayer1->m_yAccel = std::get<std::deque<float>>(Player1Data["Yvelo"])[frame - 1];
				if (std::get<std::deque<bool>>(Player1Data["Pushed"])[frame] && !mouse1Down) { PlayLayer::pushButton(self, 0, true); mouse1Down = true; }
				if (!std::get<std::deque<bool>>(Player1Data["Pushed"])[frame] && mouse1Down) { PlayLayer::releaseButton(self, 0, true); mouse1Down = false; }

				self->m_pPlayer2->m_position.x = std::get<std::deque<float>>(Player2Data["Xpos"])[frame - 1];
				self->m_pPlayer2->m_position.y = std::get<std::deque<float>>(Player2Data["Ypos"])[frame - 1];
				self->m_pPlayer2->setRotation(std::get<std::deque<float>>(Player2Data["Rotation"])[frame - 1]);
				self->m_pPlayer2->m_yAccel = std::get<std::deque<float>>(Player2Data["Yvelo"])[frame - 1];
				if (std::get<std::deque<bool>>(Player2Data["Pushed"])[frame] && !mouse2Down) { PlayLayer::pushButton(self, 0, false); mouse2Down = true; }
				if (!std::get<std::deque<bool>>(Player2Data["Pushed"])[frame] && mouse2Down) { PlayLayer::releaseButton(self, 0, false); mouse2Down = false; }
			}
		}
	}
	else {
		self->m_pPlayer1->m_position.x = std::get<std::deque<float>>(Player1Data["Xpos"])[frame];
		self->m_pPlayer1->m_position.y = std::get<std::deque<float>>(Player1Data["Ypos"])[frame];
		self->m_pPlayer1->setRotation(std::get<std::deque<float>>(Player1Data["Rotation"])[frame]);
		self->m_pPlayer1->m_yAccel = std::get<std::deque<float>>(Player1Data["Yvelo"])[frame];
		if (std::get<std::deque<bool>>(Player1Data["Pushed"])[frame] && !mouse1Down) { PlayLayer::pushButton(self, 0, true); mouse1Down = true; }
		if (std::get<std::deque<bool>>(Player1Data["Pushed"])[frame] && mouse1Down) { PlayLayer::releaseButton(self, 0, true); mouse1Down = false; }

		self->m_pPlayer2->m_position.x = std::get<std::deque<float>>(Player2Data["Xpos"])[frame];
		self->m_pPlayer2->m_position.y = std::get<std::deque<float>>(Player2Data["Ypos"])[frame];
		self->m_pPlayer2->setRotation(std::get<std::deque<float>>(Player2Data["Rotation"])[frame]);
		self->m_pPlayer2->m_yAccel = std::get<std::deque<float>>(Player2Data["Yvelo"])[frame];
		if (std::get<std::deque<bool>>(Player2Data["Pushed"])[frame] && !mouse2Down) { PlayLayer::pushButton(self, 0, false); mouse2Down = true; }
		if (std::get<std::deque<bool>>(Player2Data["Pushed"])[frame] && mouse2Down) { PlayLayer::releaseButton(self, 0, false); mouse2Down = false; }
	}
}

void __fastcall PlayLayer::resetLevelHook(gd::PlayLayer* self) {
	PlayLayer::resetLevel(self);
	mode = selectedMode;
	showedMacroComplete = false;
	
	if (!gd::GameManager::sharedState()->getPlayLayer()->m_isPracticeMode) {
		frame = 0;
		maxFrame = 0;
		if(mode == "Record") waitForFirstClick = true;
	}
	else {
		if (std::get<std::deque<int>>(Player1Data["Checkpoints"]).size() == 0) { std::get<std::deque<int>>(Player1Data["Checkpoints"]).insert(std::get<std::deque<int>>(Player1Data["Checkpoints"]).begin(), 0); }

		/*
		Notation for this because if i get it wrong it fucks me up
		On death, we go back to the last frame in the checkpoints list
		Assume that the last item in checkpoints is 1000,
		We keep calling pop_back on the list until it is less than 1000 because we write the fram on respawn
		Now the macro should work almost perfectly
		*/

		while ((int)std::get<std::deque<int>>(Player1Data["Checkpoints"]).back() < (int)(std::get<std::deque<float>>(Player1Data["Xpos"]).size())) { if (std::get<std::deque<float>>(Player1Data["Xpos"]).size() != 0) { std::get<std::deque<float>>(Player1Data["Xpos"]).pop_back(); } else { break; } };
		while ((int)std::get<std::deque<int>>(Player1Data["Checkpoints"]).back() < (int)(std::get<std::deque<float>>(Player1Data["Ypos"]).size())) { if (std::get<std::deque<float>>(Player1Data["Ypos"]).size() != 0) { std::get<std::deque<float>>(Player1Data["Ypos"]).pop_back(); } else { break; } };
		while ((int)std::get<std::deque<int>>(Player1Data["Checkpoints"]).back() < (int)(std::get<std::deque<float>>(Player1Data["Rotation"]).size())) { if (std::get<std::deque<float>>(Player1Data["Rotation"]).size() != 0) { std::get<std::deque<float>>(Player1Data["Rotation"]).pop_back(); } else { break; } };
		while ((int)std::get<std::deque<int>>(Player1Data["Checkpoints"]).back() < (int)(std::get<std::deque<bool>>(Player1Data["Pushed"]).size())) { if (std::get<std::deque<bool>>(Player1Data["Pushed"]).size() != 0) { std::get<std::deque<bool>>(Player1Data["Pushed"]).pop_back(); } else { break; } };
		while ((int)std::get<std::deque<int>>(Player1Data["Checkpoints"]).back() < (int)(std::get<std::deque<float>>(Player1Data["Yvelo"]).size())) { if (std::get<std::deque<float>>(Player1Data["Yvelo"]).size() != 0) { std::get<std::deque<float>>(Player1Data["Yvelo"]).pop_back(); } else { break; } };

		while ((int)std::get<std::deque<int>>(Player1Data["Checkpoints"]).back() < (int)(std::get<std::deque<float>>(Player2Data["Xpos"]).size())) { if (std::get<std::deque<float>>(Player2Data["Xpos"]).size() != 0) { std::get<std::deque<float>>(Player2Data["Xpos"]).pop_back(); } else { break; } };
		while ((int)std::get<std::deque<int>>(Player1Data["Checkpoints"]).back() < (int)(std::get<std::deque<float>>(Player2Data["Ypos"]).size())) { if (std::get<std::deque<float>>(Player2Data["Ypos"]).size() != 0) { std::get<std::deque<float>>(Player2Data["Ypos"]).pop_back(); } else { break; } };
		while ((int)std::get<std::deque<int>>(Player1Data["Checkpoints"]).back() < (int)(std::get<std::deque<float>>(Player2Data["Rotation"]).size())) { if (std::get<std::deque<float>>(Player2Data["Rotation"]).size() != 0) { std::get<std::deque<float>>(Player2Data["Rotation"]).pop_back(); } else { break; } };
		while ((int)std::get<std::deque<int>>(Player1Data["Checkpoints"]).back() < (int)(std::get<std::deque<bool>>(Player2Data["Pushed"]).size())) { if (std::get<std::deque<bool>>(Player2Data["Pushed"]).size() != 0) { std::get<std::deque<bool>>(Player2Data["Pushed"]).pop_back(); } else { break; } };
		while ((int)std::get<std::deque<int>>(Player1Data["Checkpoints"]).back() < (int)(std::get<std::deque<float>>(Player2Data["Yvelo"]).size())) { if (std::get<std::deque<float>>(Player2Data["Yvelo"]).size() != 0) { std::get<std::deque<float>>(Player2Data["Yvelo"]).pop_back(); } else { break; } };
		
		if ((int)std::get<std::deque<int>>(Player1Data["Checkpoints"]).back() != 0) { frame = (int)std::get<std::deque<int>>(Player1Data["Checkpoints"]).back(); }
		else { 
			frame = 0;
			maxFrame = 0;
		}
	}
}

bool __fastcall PlayLayer::pushButtonHook(gd::PlayLayer* self, uintptr_t, int state, bool player) {
	if (player) mouse1Down = true;
	if (!player) mouse2Down = true;
	if (mode == "Record" || mode == "Disabled") { PlayLayer::pushButton(self, state, player); }
	if (mode == "Record") {
		if (waitForFirstClick && !self->m_isPracticeMode) {
			waitForFirstClick = false;
			if (!rewinding) {
				while ((int)frame < (int)(std::get<std::deque<float>>(Player1Data["Xpos"]).size())) { if (std::get<std::deque<float>>(Player1Data["Xpos"]).size() != 0) { std::get<std::deque<float>>(Player1Data["Xpos"]).pop_back(); } else { break; } };
				while ((int)frame < (int)(std::get<std::deque<float>>(Player1Data["Ypos"]).size())) { if (std::get<std::deque<float>>(Player1Data["Ypos"]).size() != 0) { std::get<std::deque<float>>(Player1Data["Ypos"]).pop_back(); } else { break; } };
				while ((int)frame < (int)(std::get<std::deque<float>>(Player1Data["Rotation"]).size())) { if (std::get<std::deque<float>>(Player1Data["Rotation"]).size() != 0) { std::get<std::deque<float>>(Player1Data["Rotation"]).pop_back(); } else { break; } };
				while ((int)frame < (int)(std::get<std::deque<bool>>(Player1Data["Pushed"]).size())) { if (std::get<std::deque<bool>>(Player1Data["Pushed"]).size() != 0) { std::get<std::deque<bool>>(Player1Data["Pushed"]).pop_back(); } else { break; } };
				while ((int)frame < (int)(std::get<std::deque<float>>(Player1Data["Yvelo"]).size())) { if (std::get<std::deque<float>>(Player1Data["Yvelo"]).size() != 0) { std::get<std::deque<float>>(Player1Data["Yvelo"]).pop_back(); } else { break; } };

				while ((int)frame < (int)(std::get<std::deque<float>>(Player2Data["Xpos"]).size())) { if (std::get<std::deque<float>>(Player2Data["Xpos"]).size() != 0) { std::get<std::deque<float>>(Player2Data["Xpos"]).pop_back(); } else { break; } };
				while ((int)frame < (int)(std::get<std::deque<float>>(Player2Data["Ypos"]).size())) { if (std::get<std::deque<float>>(Player2Data["Ypos"]).size() != 0) { std::get<std::deque<float>>(Player2Data["Ypos"]).pop_back(); } else { break; } };
				while ((int)frame < (int)(std::get<std::deque<float>>(Player2Data["Rotation"]).size())) { if (std::get<std::deque<float>>(Player2Data["Rotation"]).size() != 0) { std::get<std::deque<float>>(Player2Data["Rotation"]).pop_back(); } else { break; } };
				while ((int)frame < (int)(std::get<std::deque<bool>>(Player2Data["Pushed"]).size())) { if (std::get<std::deque<bool>>(Player2Data["Pushed"]).size() != 0) { std::get<std::deque<bool>>(Player2Data["Pushed"]).pop_back(); } else { break; } };
				while ((int)frame < (int)(std::get<std::deque<float>>(Player2Data["Yvelo"]).size())) { if (std::get<std::deque<float>>(Player2Data["Yvelo"]).size() != 0) { std::get<std::deque<float>>(Player2Data["Yvelo"]).pop_back(); } else { break; } };
			}
		}
	}
	return true;
}

bool __fastcall PlayLayer::releaseButtonHook(gd::PlayLayer* self, uintptr_t, int state, bool player) {
	if (player) mouse1Down = false;
	if (!player) mouse2Down = false;
	if (mode == "Record" || mode == "Disabled") { PlayLayer::releaseButton(self, state, player); }
	return true;
}

void __fastcall PlayLayer::onQuitHook(gd::PlayLayer* self) {
	PlayLayer::onQuit(self);
	inLevel = false;
}

int __fastcall PlayLayer::createCheckpointHook(gd::PlayLayer* self) {
	auto ret = PlayLayer::createCheckpoint(self);
	std::get<std::deque<int>>(Player1Data["Checkpoints"]).insert(std::get<std::deque<int>>(Player1Data["Checkpoints"]).end(), frame);
	return ret;
}

int __fastcall PlayLayer::removeCheckpointHook(gd::PlayLayer* self) {
	auto ret = PlayLayer::removeCheckpoint(self);
	if (std::get<std::deque<int>>(Player1Data["Checkpoints"]).size() > 0) std::get<std::deque<int>>(Player1Data["Checkpoints"]).pop_back();
	return ret;
}

bool __fastcall LevelEditorLayer::initHook(gd::LevelEditorLayer* self, int edx, gd::GJGameLevel* level) {
	if (!LevelEditorLayer::init(self, level)) return false;
	inLevel = false;
	return true;
}

void PosBot::SaveMacro(std::string macroName) {
	int val = _mkdir("PosBot");
	std::fstream myfile;
	myfile.open("PosBot/" + macroName + ".pbor", std::ios::out);
	if (myfile.is_open()) {
		myfile << std::get<std::deque<float>>(Player1Data["Xpos"]).size() << "\n";
		myfile << std::get<std::deque<float>>(Player1Data["Ypos"]).size() << "\n";
		myfile << std::get<std::deque<float>>(Player1Data["Rotation"]).size() << "\n";
		myfile << std::get<std::deque<bool>>(Player1Data["Pushed"]).size() << "\n";
		myfile << std::get<std::deque<float>>(Player1Data["Yvelo"]).size() << "\n";
		for (float xpos : std::get<std::deque<float>>(Player1Data["Xpos"])) myfile << std::setprecision(10) << std::fixed << xpos << "\n";
		for (float ypos : std::get<std::deque<float>>(Player1Data["Ypos"])) myfile << std::setprecision(10) << std::fixed << ypos << "\n";
		for (float rotation : std::get<std::deque<float>>(Player1Data["Rotation"])) myfile << std::setprecision(10) << std::fixed << rotation << "\n";
		for (bool pushed : std::get<std::deque<bool>>(Player1Data["Pushed"])) myfile << pushed << "\n";
		for (float yvelo : std::get<std::deque<float>>(Player1Data["Yvelo"])) myfile << std::setprecision(10) << std::fixed << yvelo << "\n";
		
		myfile << std::get<std::deque<float>>(Player2Data["Xpos"]).size() << "\n";
		myfile << std::get<std::deque<float>>(Player2Data["Ypos"]).size() << "\n";
		myfile << std::get<std::deque<float>>(Player2Data["Rotation"]).size() << "\n";
		myfile << std::get<std::deque<bool>>(Player2Data["Pushed"]).size() << "\n";
		myfile << std::get<std::deque<float>>(Player2Data["Yvelo"]).size() << "\n";
		for (float xpos : std::get<std::deque<float>>(Player2Data["Xpos"])) myfile << std::setprecision(10) << std::fixed << xpos << "\n";
		for (float ypos : std::get<std::deque<float>>(Player2Data["Ypos"])) myfile << std::setprecision(10) << std::fixed << ypos << "\n";
		for (float rotation : std::get<std::deque<float>>(Player2Data["Rotation"])) myfile << std::setprecision(10) << std::fixed << rotation << "\n";
		for (bool pushed : std::get<std::deque<bool>>(Player2Data["Pushed"])) myfile << pushed << "\n";
		for (float yvelo : std::get<std::deque<float>>(Player2Data["Yvelo"])) myfile << std::setprecision(10) << std::fixed << yvelo << "\n";
		myfile.close();
	}
}

void PosBot::LoadMacro(std::string macroName) {
	std::fstream myfile;
	std::string line;
	myfile.open(("PosBot/" + macroName + ".pbor"), std::ios::in);
	std::get<std::deque<float>>(Player1Data["Xpos"]).clear();
	std::get<std::deque<float>>(Player1Data["Ypos"]).clear();
	std::get<std::deque<float>>(Player1Data["Rotation"]).clear();
	std::get<std::deque<bool>>(Player1Data["Pushed"]).clear();
	std::get<std::deque<int>>(Player1Data["Checkpoints"]).clear();
	std::get<std::deque<float>>(Player1Data["Yvelo"]).clear();
	
	std::get<std::deque<float>>(Player2Data["Xpos"]).clear();
	std::get<std::deque<float>>(Player2Data["Ypos"]).clear();
	std::get<std::deque<float>>(Player2Data["Rotation"]).clear();
	std::get<std::deque<bool>>(Player2Data["Pushed"]).clear();
	std::get<std::deque<float>>(Player2Data["Yvelo"]).clear();
	
	if (myfile.is_open()) {
		getline(myfile, line);
		int XposLines = stoi(line);
		getline(myfile, line);
		int YposLines = stoi(line);
		getline(myfile, line);
		int RotationLines = stoi(line);
		getline(myfile, line);
		int PushedLines = stoi(line);
		getline(myfile, line);
		int YVeloLines = stoi(line);
		for (int lineno = 1; lineno <= XposLines; lineno++) { getline(myfile, line); std::get<std::deque<float>>(Player1Data["Xpos"]).insert(std::get<std::deque<float>>(Player1Data["Xpos"]).end(), stof(line)); }
		for (int lineno = 1; lineno <= YposLines; lineno++) { getline(myfile, line); std::get<std::deque<float>>(Player1Data["Ypos"]).insert(std::get<std::deque<float>>(Player1Data["Ypos"]).end(), stof(line)); }
		for (int lineno = 1; lineno <= RotationLines; lineno++) { getline(myfile, line); std::get<std::deque<float>>(Player1Data["Rotation"]).insert(std::get<std::deque<float>>(Player1Data["Rotation"]).end(), stof(line)); }
		bool b;
		for (int lineno = 1; lineno <= PushedLines; lineno++) { getline(myfile, line); std::stringstream(line) >> b; std::get<std::deque<bool>>(Player1Data["Pushed"]).insert(std::get<std::deque<bool>>(Player1Data["Pushed"]).end(), b); }
		for (int lineno = 1; lineno <= YVeloLines; lineno++) { getline(myfile, line); std::get<std::deque<float>>(Player1Data["Yvelo"]).insert(std::get<std::deque<float>>(Player1Data["Yvelo"]).end(), stof(line)); }
		
		getline(myfile, line);
		XposLines = stoi(line);
		getline(myfile, line);
		YposLines = stoi(line);
		getline(myfile, line);
		RotationLines = stoi(line);
		getline(myfile, line);
		PushedLines = stoi(line);
		getline(myfile, line);
		YVeloLines = stoi(line);
		for (int lineno = 1; lineno <= XposLines; lineno++) { getline(myfile, line); std::get<std::deque<float>>(Player2Data["Xpos"]).insert(std::get<std::deque<float>>(Player2Data["Xpos"]).end(), stof(line)); }
		for (int lineno = 1; lineno <= YposLines; lineno++) { getline(myfile, line); std::get<std::deque<float>>(Player2Data["Ypos"]).insert(std::get<std::deque<float>>(Player2Data["Ypos"]).end(), stof(line)); }
		for (int lineno = 1; lineno <= RotationLines; lineno++) { getline(myfile, line); std::get<std::deque<float>>(Player2Data["Rotation"]).insert(std::get<std::deque<float>>(Player2Data["Rotation"]).end(), stof(line)); }
		for (int lineno = 1; lineno <= PushedLines; lineno++) { getline(myfile, line); std::stringstream(line) >> b; std::get<std::deque<bool>>(Player2Data["Pushed"]).insert(std::get<std::deque<bool>>(Player2Data["Pushed"]).end(), b); }
		for (int lineno = 1; lineno <= YVeloLines; lineno++) { getline(myfile, line); std::get<std::deque<float>>(Player2Data["Yvelo"]).insert(std::get<std::deque<float>>(Player2Data["Yvelo"]).end(), stof(line)); }
		myfile.close();
	}
}

void __fastcall Extra::dispatchKeyboardMSGHook(void* self, void*, int key, bool down) {
	dispatchKeyboardMSG(self, key, down);
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
		if (gd::GameManager::sharedState()->getIntGameVariable("posbotcolour0") != 0 || gd::GameManager::sharedState()->getIntGameVariable("posbotcolour1") != 0 || gd::GameManager::sharedState()->getIntGameVariable("posbotcolour2") != 0) {
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
	}

	// Fix for UI Dissapearing after setting keybind
	if (keybind != gd::GameManager::sharedState()->getIntGameVariable("posbotkeybind")) {
		showUI = true;
		keybind = gd::GameManager::sharedState()->getIntGameVariable("posbotkeybind");
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
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoScrollbar;

	// Render the UI
	if (showUI) {
		if (ImGui::Begin("PosBot", nullptr, window_flags)) {
			ImGui::SetWindowSize("PosBot", ImVec2(421, 443));
			ImGui::Text("PosBot v1.3");

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
				if (gd::GameManager::sharedState()->getPlayLayer()->m_isPracticeMode) { ImGui::Text("Practice Mode (can be buggy sometimes)"); ImGui::NewLine(); }
				
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
					PosBot::LoadMacro(macroName);
				}
				else { loadBtnPressed = false; }
				ImGui::NewLine();

				// Menu keybind button
				ImGui::Text("Menu Keybind:  ");
				ImGui::SameLine();
				if (waitForSetKeybind) {
					ImGui::Button("Waiting for keypress...");
				}
				else {
					if (ImGui::Button(Keys[keybind], ImVec2(80, 20))) { waitForSetKeybind = true; }
				}
				ImGui::NewLine();

				// Colour Themes O_O
				ImGui::Text("Colour Theme");
				ImGuiSliderFlags slider_flags = 0;
				slider_flags |= ImGuiSliderFlags_AlwaysClamp;
				slider_flags |= ImGuiSliderFlags_NoInput;
				ImGui::SliderFloat("R", &LightColour[0], 0.0f, 255.0f, "%.0f", slider_flags);
				ImGui::SliderFloat("G", &LightColour[1], 0.0f, 255.0f, "%.0f", slider_flags);
				ImGui::SliderFloat("B", &LightColour[2], 0.0f, 255.0f, "%.0f", slider_flags);

				// Rewinding
				ImGui::NewLine();
				if (ImGui::Button("Rewind", ImVec2(80, 20)) || rewinding) {
					rewinding = true;
					ImGui::SliderInt("Frame", &frame, 0, maxFrame);
					if (ImGui::Button("Cancel")) {
						frame = maxFrame;
						rewinding = false;
					}
					ImGui::SameLine();
					if (ImGui::Button("Confirm")) {
						while ((int)frame < (int)(std::get<std::deque<float>>(Player1Data["Xpos"]).size())) { if (std::get<std::deque<float>>(Player1Data["Xpos"]).size() != 0) { std::get<std::deque<float>>(Player1Data["Xpos"]).pop_back(); } else { break; } };
						while ((int)frame < (int)(std::get<std::deque<float>>(Player1Data["Ypos"]).size())) { if (std::get<std::deque<float>>(Player1Data["Ypos"]).size() != 0) { std::get<std::deque<float>>(Player1Data["Ypos"]).pop_back(); } else { break; } };
						while ((int)frame < (int)(std::get<std::deque<float>>(Player1Data["Rotation"]).size())) { if (std::get<std::deque<float>>(Player1Data["Rotation"]).size() != 0) { std::get<std::deque<float>>(Player1Data["Rotation"]).pop_back(); } else { break; } };
						while ((int)frame < (int)(std::get<std::deque<bool>>(Player1Data["Pushed"]).size())) { if (std::get<std::deque<bool>>(Player1Data["Pushed"]).size() != 0) { std::get<std::deque<bool>>(Player1Data["Pushed"]).pop_back(); } else { break; } };
						while ((int)frame < (int)(std::get<std::deque<float>>(Player1Data["Yvelo"]).size())) { if (std::get<std::deque<float>>(Player1Data["Yvelo"]).size() != 0) { std::get<std::deque<float>>(Player1Data["Yvelo"]).pop_back(); } else { break; } };

						while ((int)frame < (int)(std::get<std::deque<float>>(Player2Data["Xpos"]).size())) { if (std::get<std::deque<float>>(Player2Data["Xpos"]).size() != 0) { std::get<std::deque<float>>(Player2Data["Xpos"]).pop_back(); } else { break; } };
						while ((int)frame < (int)(std::get<std::deque<float>>(Player2Data["Ypos"]).size())) { if (std::get<std::deque<float>>(Player2Data["Ypos"]).size() != 0) { std::get<std::deque<float>>(Player2Data["Ypos"]).pop_back(); } else { break; } };
						while ((int)frame < (int)(std::get<std::deque<float>>(Player2Data["Rotation"]).size())) { if (std::get<std::deque<float>>(Player2Data["Rotation"]).size() != 0) { std::get<std::deque<float>>(Player2Data["Rotation"]).pop_back(); } else { break; } };
						while ((int)frame < (int)(std::get<std::deque<bool>>(Player2Data["Pushed"]).size())) { if (std::get<std::deque<bool>>(Player2Data["Pushed"]).size() != 0) { std::get<std::deque<bool>>(Player2Data["Pushed"]).pop_back(); } else { break; } };
						while ((int)frame < (int)(std::get<std::deque<float>>(Player2Data["Yvelo"]).size())) { if (std::get<std::deque<float>>(Player2Data["Yvelo"]).size() != 0) { std::get<std::deque<float>>(Player2Data["Yvelo"]).pop_back(); } else { break; } };
						maxFrame = frame;
						rewinding = false;
					}
				}
			}
		}
	}
}

void PosBot::mem_init() {

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
		(LPVOID*)&PlayLayer::createCheckpoint);

	MH_CreateHook(
		(PVOID)(base + 0x20B830),
		PlayLayer::removeCheckpointHook,
		(LPVOID*)&PlayLayer::removeCheckpoint);

	MH_CreateHook(
		(PVOID)(base + 0x15EE00),
		LevelEditorLayer::initHook,
		(LPVOID*)&LevelEditorLayer::init);

	auto cocos = GetModuleHandleA("libcocos2d.dll");
	auto addr = GetProcAddress(cocos, "?dispatchKeyboardMSG@CCKeyboardDispatcher@cocos2d@@QAE_NW4enumKeyCodes@2@_N@Z");
	MH_CreateHook(
		addr,
		Extra::dispatchKeyboardMSGHook,
		reinterpret_cast<void**>(&Extra::dispatchKeyboardMSG)
	);
}