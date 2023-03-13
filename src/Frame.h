#pragma once

#include "gd.h"

struct FrameData {
	float x_pos;
	float y_pos;
	float rotation_x;
	float rotation_y;
	bool is_upside_down;
	bool mouseDown;

	static FrameData from(gd::PlayerObject* player, bool mouseDown) {
		return (FrameData({
			player->m_position.x,
			player->m_position.y,
			player->getRotationX(),
			player->getRotationY(),
			player->m_isUpsideDown,
			mouseDown
		}));
	}

	void restore(gd::PlayerObject* player, bool frameFix) {
		if (frameFix) {
			player->m_position.x = x_pos;
			player->m_position.y = y_pos;
			player->setRotationX(rotation_x);
			player->setRotationY(rotation_y);
			player->m_isUpsideDown = is_upside_down;
		}
	}
};

struct Frame {
	FrameData p1;
	FrameData p2;

	static Frame from(gd::PlayLayer* playLayer, bool p1down, bool p2down) {
		gd::PlayerObject* player1 = playLayer->m_pPlayer1;
		gd::PlayerObject* player2 = playLayer->m_pPlayer2;

		return (
			Frame({
				FrameData::from(player1, p1down),
				FrameData::from(player2, p2down)
			})
		);
	}

	void restore(gd::PlayLayer* playLayer, bool frameFix) {
		gd::PlayerObject* player1 = playLayer->m_pPlayer1;
		gd::PlayerObject* player2 = playLayer->m_pPlayer2;
		p1.restore(player1, frameFix);
		p2.restore(player2, frameFix);
	}
};