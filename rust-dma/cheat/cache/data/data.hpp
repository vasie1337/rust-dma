#pragma once
#include "../../../include.hpp"

template <typename T>
class DataEntry
{
public:
	DataEntry() = default;
	DataEntry(T data) : data(data) {}

	void store(T data)
	{
		std::lock_guard<std::mutex> lock(mutex);
		this->data = data;
	}

	T load() const
	{
		std::lock_guard<std::mutex> lock(mutex);
		return data;
	}

private:
	mutable std::mutex mutex;
	T data = T();
};

class CacheData
{
public:
	inline static DataEntry<uintptr_t> base_address;
	inline static DataEntry<uintptr_t> camera_object;
	inline static DataEntry<uintptr_t> entity_list;

	inline static DataEntry<Player> local_player;

	inline static DataEntry<std::vector<Entity>> entities = std::vector<Entity>();
	inline static DataEntry<std::vector<Player>> players = std::vector<Player>();

	inline static DataEntry<Matrix4x4> view_matrix = Matrix4x4();
	inline static DataEntry<Vector3> camera_pos = Vector3();
};