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
	void UpdatePositions(HANDLE scatter_handle);
	void UpdateViewMatrix(HANDLE scatter_handle);

	std::string FormatObjectName(const std::string& object_name);

public:
	CacheThread globals_thread = CacheThread(
		std::function<void(HANDLE)>(std::bind(&Cache::FetchGlobals, this, std::placeholders::_1)), 
		5000,
		"Globals Fetch"
	);
	CacheThread entities_thread = CacheThread(
		std::function<void(HANDLE)>(std::bind(&Cache::FetchEntities, this, std::placeholders::_1)),
		3000,
		"Entities Fetch"
	);
	CacheThread pos_thread = CacheThread(
		std::function<void(HANDLE)>(std::bind(&Cache::UpdatePositions, this, std::placeholders::_1)),
		50,
		"Positions Update"
	);
	CacheThread view_thread = CacheThread(
		std::function<void(HANDLE)>(std::bind(&Cache::UpdateViewMatrix, this, std::placeholders::_1)),
		1,
		"View Update"
	);

	static inline std::vector<std::reference_wrapper<CacheThread>> threads = {};
};

inline std::string Cache::FormatObjectName(const std::string& object_name)
{
	size_t start = object_name.find_last_of('/');
	std::string name = (start == std::string::npos) ? object_name : object_name.substr(start + 1);

	const std::array<std::string_view, 4> suffixes = { ".prefab", ".entity", "_spawned", "deployed" };

	for (const auto& suffix : suffixes) {
		if (name.size() >= suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0) {
			name.resize(name.size() - suffix.size());
			break;
		}
	}

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
	return result;
}