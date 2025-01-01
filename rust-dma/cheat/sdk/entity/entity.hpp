#pragma once
#include "../../../include.hpp"

class Entity
{
public:
	Entity() {}
	~Entity() {}

	int idx = 0;

	char name_buffer[128] = { 0 };
	std::string obj_name = "";
	std::string formatted_name = "";
	Vector3 position = Vector3();

	std::uint16_t tag = 0;

	std::uintptr_t object_ptr = 0;
	std::uintptr_t base_object = 0;
	std::uintptr_t object = 0;
	std::uintptr_t object_class = 0;

	std::uintptr_t nameptr = 0;
	std::uintptr_t transform = 0;
	std::uintptr_t visual_state = 0;

	std::uintptr_t model = 0;
};

class EntityListData 
{
public:
	uintptr_t content;
	uint32_t size;

	bool operator!(void) const
	{
		return content == 0 || size == 0;
	}
};