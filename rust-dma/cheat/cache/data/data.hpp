#pragma once
#include "../../../include.hpp"

template <typename T>
class BufferedData 
{
public:
    BufferedData() = default;

    void store(T new_data) noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        back_buffer_ = std::move(new_data);
        buffer_updated_ = true;
    }

    T load() noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        if (buffer_updated_) {
            std::swap(front_buffer_, back_buffer_);
            buffer_updated_ = false;
        }
        return front_buffer_;
    }

    T swap(T new_data) noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        std::swap(front_buffer_, new_data);
        return new_data;
    }

    bool has_update() const noexcept {
        return buffer_updated_;
    }

private:
    mutable std::mutex mutex_;
    T front_buffer_ = T();
    T back_buffer_ = T();
    bool buffer_updated_ = false;
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
};