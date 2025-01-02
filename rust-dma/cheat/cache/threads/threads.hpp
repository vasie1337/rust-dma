#pragma once
#include "../../../include.hpp"

class CacheStatistics 
{
public:
    float average_time;
    float min_time;
    float max_time;
    float current_time;
};

class CacheThread
{
public:
    CacheThread(std::function<void(HANDLE)> func, int delay, std::string name = "Unknown")
        : func(std::move(func)), delay(delay), name(std::move(name)) {
    }

    ~CacheThread() {
        if (thread.joinable()) {
            Stop();
        }
    }

    void Run();
    void Stop();

	void Pause() { paused.store(true); }
	void Resume() { paused.store(false); }

    CacheStatistics GetStatistics() const;
    std::vector<float> GetIterationTimes() const;
    const std::string& GetName() const { return name; }
	void SetDelay(int delay) { this->delay = delay; }
	int GetDelay() const { return delay; }

private:
    void UpdateStatistics(const std::chrono::high_resolution_clock::time_point& start_time,
        const std::chrono::high_resolution_clock::time_point& end_time);

    float CalculateAverage() const;

    std::thread thread;
    std::function<void(HANDLE)> func;
    int delay = 0;
    std::atomic<bool> running{ false };
	std::atomic<bool> paused{ false };
    HANDLE scatter_handle = 0;
    std::string name;

    mutable std::mutex stats_mutex;
    float iteration_time = 0.0f;
    float min_iteration_time = 0.0f;
    float max_iteration_time = 0.0f;
    std::chrono::high_resolution_clock::time_point last_iteration_time;
    std::vector<float> iteration_times = std::vector<float>(128, 0.0f);
    size_t iteration_times_index = 0;
};