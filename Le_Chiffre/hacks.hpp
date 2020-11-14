#ifndef HACKS_HPP
#define HACKS_HPP
#pragma once

#include "client.hpp"
#include "memory.hpp"
#include "player_entity.hpp"

using namespace hazedumper;

struct GlowStruct {
	BYTE base[4];
	float red;
	float green;
	float blue;
	float alpha;
	BYTE buffer[16];
	bool redner_occluded;
	bool render_unoccluded;
	bool fullBloom;
	BYTE buffer1[5];
	int glow_style;
};

class Hacks {
private:
	Memory* memory;
	Client* client;

	// true if aiming on player, false otherwise
	bool _aim_on_enemy(PlayerEntity* player) {
		if (!player->valid_player()) return false;

		DWORD crosshair = player->get_crosshair_id();
		if (crosshair <= 0 && crosshair > 65) return false;

		PlayerEntity enemy(memory, memory->read_mem<DWORD>(memory->clientBaseAddr + signatures::dwEntityList + 0x10 * (crosshair - 1)));
		if (!enemy.valid_player()) return false;

		// check if crosshair if pointed on enemy player
		return player->get_team() != enemy.get_team();
	}

	void _team_glow(PlayerEntity* target, DWORD glow_obj) {
		GlowStruct gt = memory->read_mem<GlowStruct>(glow_obj + (target->get_glow_index() * 0x38));
		gt.red = 1.0f;
		gt.green = 1.0f;
		gt.blue = 1.0f;
		gt.alpha = 0.8f;
		gt.redner_occluded = true;
		gt.render_unoccluded = false;
		memory->write_mem<GlowStruct>(glow_obj + (target->get_glow_index() * 0x38), gt);
	}

	void _enemy_glow(PlayerEntity* target, DWORD glow_obj) {
		GlowStruct gt = memory->read_mem<GlowStruct>(glow_obj + (target->get_glow_index() * 0x38));
		gt.red = 1.0f;
		gt.green = 0;
		gt.blue = 0;
		gt.alpha = 0.9f;
		gt.redner_occluded = true;
		gt.render_unoccluded = false;
		memory->write_mem<GlowStruct>(glow_obj + (target->get_glow_index() * 0x38), gt);
	}
public:
	void trigger_bot() {
		PlayerEntity player = client->get_local_player();

		if (_aim_on_enemy(&player)) {
			player.set_attack_state(5);
			// 25-30ms
			Sleep((rand() % 6) + 25); // https://www.cplusplus.com/forum/beginner/183358/
			player.set_attack_state(4);
		} else player.set_attack_state(4);
	}

	void no_flash() {
		PlayerEntity player = client->get_local_player();
		if (player.valid_player() && player.get_flash_duration() > 0.f) player.set_flash_duration(0.f);
	}

	void glow_esp_radar(bool glow_on_teammate, bool glow_on_enemy, bool radar_hack) {
		PlayerEntity player = client->get_local_player();
		DWORD glow_obj;
		int player_team = player.get_team();

		if (glow_on_teammate || glow_on_enemy) glow_obj = memory->read_mem<DWORD>(memory->clientBaseAddr + signatures::dwGlowObjectManager);

		for (short i = 0; i < 64; ++i) {
			PlayerEntity target(memory, memory->read_mem<DWORD>(memory->clientBaseAddr + signatures::dwEntityList + 0x10 * i));

			if (target.valid_player()) {
				int target_team = target.get_team();

				if (player_team == target_team && glow_on_teammate) _team_glow(&target, glow_obj);
				else if (player_team != target_team && glow_on_enemy) _enemy_glow(&target, glow_obj);
				
				if (player_team != target_team && radar_hack) target.set_spotted(true);
			}
		}

		Sleep(5);
	}

	Hacks(Memory* memory, Client* client) {
		this->memory = memory;
		this->client = client;
	}
};
#endif