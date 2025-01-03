#pragma once

namespace Offsets
{
	// Global
	inline std::uintptr_t main_camera = 233036928;
	inline std::uintptr_t base_net_workable = 233477144;
	inline std::uintptr_t view_matrix = 0x30C;
	inline std::uintptr_t camera_pos = 0x454;

	// BaseEntity
	inline std::uintptr_t model = 0xC8;
	inline std::uintptr_t object_class = 0x30;
	inline std::uintptr_t tag = 0x54;
	inline std::uintptr_t prefab_name = 0x60;
	inline std::uintptr_t transform = 0x8;
	inline std::uintptr_t visual_state = 0x38;
	inline std::uintptr_t vec3_position = 0x90;

	// BaseCombatEntity
	inline std::uintptr_t player_model = 0x258;
	inline std::uintptr_t player_name = 0x270;
	inline std::uintptr_t is_npc = 0x2E2;
	inline std::uintptr_t bone_transforms = 0x50;
}