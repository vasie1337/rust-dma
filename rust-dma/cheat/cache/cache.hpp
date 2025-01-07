#pragma once
#include "../../include.hpp"

class Cache : public CacheData
{
public:
	Cache() {}
	~Cache() {}

	void Run();
	void Stop();

private:
	void FetchGlobals(HANDLE scatter_handle);
	void FetchEntities(HANDLE scatter_handle);

	void FetchEntityData(HANDLE scatter_handle, std::vector<Entity*>& entities_to_update);
	void FetchPlayerData(HANDLE scatter_handle, std::vector<Player*>& players_to_update);
	void FetchPlayerBones(HANDLE scatter_handle, std::vector<Player*>& players_to_update);

	void UpdateFrame(HANDLE scatter_handle);

	std::string FormatObjectName(const std::string& object_name);

public:
	CacheThread globals_thread = CacheThread(
		std::function<void(HANDLE)>(std::bind(&Cache::FetchGlobals, this, std::placeholders::_1)), 
		5000,
		"Globals Fetch"
	);
	CacheThread entities_thread = CacheThread(
		std::function<void(HANDLE)>(std::bind(&Cache::FetchEntities, this, std::placeholders::_1)),
		1000,
		"Entities Fetch"
	);
	CacheThread frame_thread = CacheThread(
		std::function<void(HANDLE)>(std::bind(&Cache::UpdateFrame, this, std::placeholders::_1)),
		1,
		"Frame Update"
	);

	static inline std::vector<std::reference_wrapper<CacheThread>> threads = {};
};

inline std::string Cache::FormatObjectName(const std::string& object_name)
{
    size_t start = object_name.find_last_of('/');
    std::string name = (start == std::string::npos) ? object_name : object_name.substr(start + 1);
    const std::array<std::string_view, 4> suffixes = { ".prefab", ".entity", "_spawned", "deployed" };
    bool found_suffix;
    do {
        found_suffix = false;
        for (const auto& suffix : suffixes) {
            if (name.size() >= suffix.size() &&
                name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0) {
                name.resize(name.size() - suffix.size());
                found_suffix = true;
                break;
            }
        }
    } while (found_suffix);
    std::string result;
    result.reserve(name.size());
    bool capitalize_next = true;
    for (char c : name) {
        if (c == '.') continue;
        if (c == '-' || c == '_') {
            result.push_back(' ');
            capitalize_next = true;
            continue;
        }
        if (std::isdigit(c)) continue;
        if (std::isalpha(c)) {
            if (capitalize_next) {
                result.push_back(std::toupper(c));
                capitalize_next = false;
            }
            else {
                result.push_back(std::tolower(c));
            }
        }
        else {
            result.push_back(c);
        }
    }

    size_t world_pos = result.find("(world)");
    if (world_pos != std::string::npos) {
        result.replace(world_pos, 7, "(dropped)");
    }

    return result;
}