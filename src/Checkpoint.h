#pragma once

#include "gd.h"

struct CheckpointStorage {
	double x_accel;
	double y_accel;
	double jump_accel;
	bool is_holding;
	bool has_just_held;
	bool is_holding2;
	bool has_just_held2;
	bool can_robot_jump;
	bool is_upside_down;
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
	float rotation_x;
	float rotation_y;
	float x_pos;
	float y_pos;

	static CheckpointStorage from(gd::PlayerObject* player) {
		return (CheckpointStorage({
			player->m_xAccel,
			player->m_yAccel,
			player->m_jumpAccel,
			player->m_isHolding,
			player->m_hasJustHeld,
			player->m_isHolding2,
			player->m_hasJustHeld2,
			player->m_canRobotJump,
			player->m_isUpsideDown,
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
			player->getRotationX(),
			player->getRotationY(),
			player->m_position.x,
			player->m_position.y
		}));
	}

	void restore(gd::PlayerObject* player) {
		player->m_xAccel = x_accel;
		player->m_yAccel = y_accel;
		player->m_jumpAccel = jump_accel;
		player->m_isHolding = is_holding;
		player->m_hasJustHeld = has_just_held;
		player->m_isHolding2 = is_holding2;
		player->m_hasJustHeld2 = has_just_held2;
		player->m_canRobotJump = can_robot_jump;
		player->m_isUpsideDown = is_upside_down;
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
		player->setRotationX(rotation_x);
		player->setRotationY(rotation_y);
		player->m_position.x = x_pos;
		player->m_position.y = y_pos;
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
		p1.restore(playLayer->m_pPlayer1);
		p2.restore(playLayer->m_pPlayer2);
	}
};