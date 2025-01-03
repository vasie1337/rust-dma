#pragma once
#include "../../../include.hpp"

template <typename T>
class BufferedData 
{
public:
    BufferedData() = default;

    void store(T new_data) {
        std::lock_guard<std::mutex> lock(mutex);
        back_buffer = new_data;

        if (!middle_in_use.exchange(true)) {
            std::swap(middle_buffer, back_buffer);
            middle_updated = true;
        }
    }

    T load() {
        std::lock_guard<std::mutex> lock(mutex);
        if (middle_updated) {
            std::swap(front_buffer, middle_buffer);
            middle_updated = false;
            middle_in_use = false;
        }
        return front_buffer;
    }

	T swap(T new_data) {
		std::lock_guard<std::mutex> lock(mutex);
		std::swap(front_buffer, new_data);
		return front_buffer;
	}

private:
    mutable std::mutex mutex;
    T front_buffer = T();
    T middle_buffer = T();
    T back_buffer = T();

    std::atomic<bool> middle_in_use{ false };
    bool middle_updated = false;
};

class CacheData {
public:
    inline static BufferedData<uintptr_t> base_address;
    inline static BufferedData<uintptr_t> camera_address;
    inline static BufferedData<uintptr_t> entity_list_address;

	inline static BufferedData<std::vector<Entity>> entities;
	inline static BufferedData<std::vector<Player>> players;

    inline static BufferedData<Matrix4x4> view_matrix;
    inline static BufferedData<Vector3> camera_pos;

	inline static BufferedData<EntityListData> entity_list_data;

};