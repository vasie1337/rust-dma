#pragma once
#include "../../../include.hpp"

struct FrameData 
{
	Matrix4x4 view_matrix;
	Vector3 camera_pos;
	std::vector<Entity> entities;
	std::vector<Player> players;
	std::chrono::steady_clock::time_point last_cache_time;
};

class CacheData 
{
public:
    inline static uintptr_t base_address;
    inline static uintptr_t camera_address;
    inline static uintptr_t entity_list_address;

	inline static std::mutex frame_mtx;
	inline static FrameData frame_data;
};

