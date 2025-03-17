#pragma once
#include "../../include.hpp"

class RateLimiter {
private:
    std::chrono::high_resolution_clock::time_point last_tick;
    std::chrono::milliseconds interval;
    std::mutex mtx;

public:
    explicit RateLimiter(int interval_ms)
        : last_tick(std::chrono::high_resolution_clock::now())
        , interval(interval_ms) {
    }

    bool should_run() {
        std::lock_guard<std::mutex> lock(mtx);
        auto now = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_tick);

        if (delta < interval)
            return false;

        last_tick = now;
        return true;
    }

    static RateLimiter& get_globals() {
        static RateLimiter instance(5000);
        return instance;
    }

    static RateLimiter& get_entities() {
        static RateLimiter instance(1000);
        return instance;
    }

    static RateLimiter& get_frame() {
        static RateLimiter instance(10);
        return instance;
    }
};

class Cache : public CacheData
{
public:
	Cache() {}
	~Cache() {}

	void Run();
	void Stop();

	static void TickCache();

private:
    static void FetchGlobals(HANDLE scatter_handle);
    static void FetchEntities(HANDLE scatter_handle);

    static void FetchEntityData(HANDLE scatter_handle, std::vector<Entity*>& entities_to_update);
    static void FetchPlayerData(HANDLE scatter_handle, std::vector<Player*>& players_to_update);
    static void FetchPlayerBones(HANDLE scatter_handle, std::vector<Player*>& players_to_update);

    static void UpdateFrame(HANDLE scatter_handle);
    static void UpdateView(HANDLE scatter_handle);

    static std::string FormatObjectName(const std::string& object_name);

public:

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
		if (c >= -1 && c <= 255) {
			result.push_back(c);
			continue;
		}
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