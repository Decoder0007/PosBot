#pragma once

#include "gd.h"

struct FrameData {
	float x_pos;
	float y_pos;
	float rotation_x;
	float rotation_y;
	bool is_upside_down;
	bool is_holding;
	bool is_holding2;

	static FrameData from(gd::PlayerObject* player) {
		return (FrameData({
			player->m_position.x,
			player->m_position.y,
			player->getRotationX(),
			player->getRotationY(),
			player->m_isUpsideDown,
			player->m_isHolding,
			player->m_isHolding2
		}));
	}

	void restore(gd::PlayerObject* player) {
		player->m_position.x = x_pos;
		player->m_position.y = y_pos;
		player->setRotationX(rotation_x);
		player->setRotationY(rotation_y);
		player->m_isUpsideDown = is_upside_down;
		player->m_isHolding = is_holding;
		player->m_isHolding2 = is_holding2;
	}
};

struct Frame {
	FrameData p1;
	FrameData p2;

	static Frame from(gd::PlayLayer* playLayer) {
		gd::PlayerObject* player1 = playLayer->m_pPlayer1;
		gd::PlayerObject* player2 = playLayer->m_pPlayer2;

		return (
			Frame({
				FrameData::from(player1),
				FrameData::from(player2)
			})
		);
	}

	void restore(gd::PlayLayer* playLayer) {
		gd::PlayerObject* player1 = playLayer->m_pPlayer1;
		gd::PlayerObject* player2 = playLayer->m_pPlayer2;
		p1.restore(player1);
		p2.restore(player2);
	}
};