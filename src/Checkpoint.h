#pragma once

#include "gd.h"
#include "PosBot.h"

struct CheckpointStorage {
	float x_pos;
	float y_pos;
	float rotation;
	bool is_upside_down;
	bool is_holding;
	bool is_holding2;
	double x_accel;
	double y_accel;
	double jump_accel;
	bool has_just_held;
	bool has_just_held2;
	bool can_robot_jump;
	bool is_on_ground;
	bool is_dashing;
	bool is_sliding;
	bool is_rising;
	bool is_dropping;
	bool unk662;
	bool unk630;
	bool unk631;
	float vehicle_size;
	float player_speed;

	static CheckpointStorage from(gd::PlayerObject* player) {
		return (CheckpointStorage({
			player->m_position.x,
			player->m_position.y,
			player->getRotation(),
			player->m_isUpsideDown,
			player->m_isHolding,
			player->m_isHolding2,
			player->m_xAccel,
			player->m_yAccel,
			player->m_jumpAccel,
			player->m_hasJustHeld,
			player->m_hasJustHeld2,
			player->m_canRobotJump,
			player->m_isOnGround,
			player->m_isDashing,
			player->m_isSliding,
			player->m_isRising,
			player->m_bDropping,
			player->unk662,
			player->unk630,
			player->unk631,
			player->m_vehicleSize,
			player->m_playerSpeed,
		}));
	}

	int restore(gd::PlayerObject* player) {
		int click = 0;
		player->m_position.x = x_pos;
		player->m_position.y = y_pos;
		player->setRotation(rotation);
		player->m_isUpsideDown = is_upside_down;
		if (is_holding != player->m_isHolding) {
			if (player->m_isHolding) { click = 2; }
			else { click = 1; };
		}
		player->m_isHolding = is_holding;
		player->m_isHolding2 = is_holding2;
		player->m_xAccel = x_accel;
		player->m_yAccel = y_accel;
		player->m_jumpAccel = jump_accel;
		player->m_hasJustHeld = has_just_held;
		player->m_hasJustHeld2 = has_just_held2;
		player->m_canRobotJump = can_robot_jump;
		player->m_isOnGround = is_on_ground;
		player->m_isDashing = is_dashing;
		player->m_isSliding = is_sliding;
		player->m_isRising = is_rising;
		player->m_bDropping = is_dropping;
		player->unk662 = unk662;
		player->unk630 = unk630;
		player->unk631 = unk631;
		player->m_vehicleSize = vehicle_size;
		player->m_playerSpeed = player_speed;
		return click;
	}
};

struct Checkpoint {
	CheckpointStorage p1;
	CheckpointStorage p2;

	static Checkpoint from(gd::PlayLayer* playLayer) {
		gd::PlayerObject* player1 = playLayer->m_pPlayer1;
		gd::PlayerObject* player2 = playLayer->m_pPlayer2;

		return (
			Checkpoint({
				CheckpointStorage::from(player1),
				CheckpointStorage::from(player2)
			})
		);
	}

	void restore(gd::PlayLayer* playLayer) {
		int click = p1.restore(playLayer->m_pPlayer1);
		if (click != 0) {
			if (click == 2) playLayer->pushButton(0, true);
			else playLayer->releaseButton(0, true);
		}
		click = p2.restore(playLayer->m_pPlayer2);
		if (click != 0) {
			if (click == 2) playLayer->pushButton(0, false);
			else playLayer->releaseButton(0, false);
		}
	}
};