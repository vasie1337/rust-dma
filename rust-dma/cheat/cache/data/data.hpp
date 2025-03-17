#pragma once
#include "../../../include.hpp"

struct FrameData 
{
	Matrix4x4 view_matrix;
	Vector3 camera_pos;
	std::vector<Entity> entities;
	std::vector<Player> players;
};

class CacheData 
{
public:
    inline static uintptr_t base_address;
    inline static uintptr_t camera_address;
    inline static uintptr_t entity_list_address;

	inline static FrameData frame_data;
};

