#pragma once
#include "../../../include.hpp"

class EntityCategorie
{
public:
	EntityCategorie() = default;
	EntityCategorie(ImColor color, std::string name = "Unknown", bool is_obj_static = true)
		: color(color), name(name), is_obj_static(is_obj_static)
	{
	}
	~EntityCategorie() = default;

	EntityCategorie& operator=(const EntityCategorie& other)
	{
		paths = other.paths;
		enabled = other.enabled;
		color = other.color;
		is_obj_static = other.is_obj_static;
		name = other.name;
		return *this;
	}

	bool IsEntityInCategory(std::string obj_name) const
	{
		for (const std::string& path : paths)
		{
			if (obj_name.length() >= path.length() && memcmp(obj_name.data(), path.data(), path.length()) == 0)
			{
				return true;
			}
		}
		return false;
	}

	bool Enabled() const
	{
		return enabled && !paths.empty();
	}

	bool IsStatic() const
	{
		return is_obj_static;
	}

	ImColor GetColor() const
	{
		return color;
	}

	void AddPath(const std::string& path)
	{
		paths.push_back(path);
	}

	std::string Name() const
	{
		return name;
	}

private:
	std::vector<std::string> paths;

public:
	bool enabled = false;
	bool is_obj_static = true;
	ImColor color;
	std::string name;
};