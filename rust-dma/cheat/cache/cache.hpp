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
	Cache() {
        cached_entities.reserve(1000);
        cached_players.reserve(128);
    }
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

    static inline std::vector<Entity> cached_entities;
    static inline std::vector<Player> cached_players;
    static inline std::unordered_map<uintptr_t, Entity*> entity_ptr_map;
    static inline std::unordered_map<uintptr_t, Player*> player_ptr_map;
};

inline std::string Cache::FormatObjectName(const std::string& object_name)
{
    // Reserve memory for result string based on input size to avoid reallocations
    std::string result;
    result.reserve(object_name.size());
    
    // Find last slash once
    size_t start = object_name.find_last_of('/');
    std::string_view name_view = (start == std::string::npos) ? 
                                std::string_view(object_name) : 
                                std::string_view(object_name).substr(start + 1);
    
    // Check and remove suffixes
    static const std::array<std::string_view, 4> suffixes = { ".prefab", ".entity", "_spawned", "deployed" };
    size_t end_pos = name_view.size();
    
    for (const auto& suffix : suffixes) {
        while (end_pos >= suffix.size() && 
              std::string_view(name_view.data() + end_pos - suffix.size(), suffix.size()) == suffix) {
            end_pos -= suffix.size();
        }
    }
    
    name_view = name_view.substr(0, end_pos);
    
    // Process the name and build the result string
    bool capitalize_next = true;
    
    for (char c : name_view) {
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

    // Use find and replace instead of creating substrings
    size_t world_pos = result.find("(world)");
    if (world_pos != std::string::npos) {
        result.replace(world_pos, 7, "(dropped)");
    }

    return result;
}