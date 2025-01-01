#include "../../../include.hpp"

void CacheThread::Run()
{
    scatter_handle = dma.CreateScatterHandle();
    running = true;
    thread = std::thread([this]()
    {
        while (running.load(std::memory_order_relaxed))
        {
            auto start_time = std::chrono::high_resolution_clock::now();

            func(scatter_handle);

            auto end_time = std::chrono::high_resolution_clock::now();
            UpdateStatistics(start_time, end_time);

            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
    });
}

void CacheThread::Stop()
{
    running.store(false, std::memory_order_relaxed);
    if (thread.joinable()) {
        thread.join();
    }
    dma.CloseScatterHandle(scatter_handle);
}

CacheStatistics CacheThread::GetStatistics() const
{
    std::lock_guard<std::mutex> lock(stats_mutex);
    return {
        CalculateAverage(),
        min_iteration_time,
        max_iteration_time,
        iteration_time
    };
}

std::vector<float> CacheThread::GetIterationTimes() const
{
    std::lock_guard<std::mutex> lock(stats_mutex);

    std::vector<float> ordered_times;
    ordered_times.reserve(iteration_times.size());

    for (size_t i = 0; i < iteration_times.size(); ++i) {
        size_t idx = (iteration_times_index + i) % iteration_times.size();
        ordered_times.push_back(iteration_times[idx]);
    }

    return ordered_times;
}

void CacheThread::UpdateStatistics(const std::chrono::high_resolution_clock::time_point& start_time,
    const std::chrono::high_resolution_clock::time_point& end_time) {
    std::lock_guard<std::mutex> lock(stats_mutex);

    float current_time = std::chrono::duration<float, std::milli>(end_time - start_time).count();

    iteration_time = current_time;

    iteration_times[iteration_times_index] = current_time;
    iteration_times_index = (iteration_times_index + 1) % iteration_times.size();

    if (current_time < min_iteration_time || min_iteration_time == 0.0f) {
        min_iteration_time = current_time;
    }
    if (current_time > max_iteration_time) {
        max_iteration_time = current_time;
    }

    last_iteration_time = end_time;
}

float CacheThread::CalculateAverage() const {
    float sum = 0.0f;
    size_t count = 0;

    for (float time : iteration_times) {
        if (time > 0.0f) {
            sum += time;
            count++;
        }
    }

    return count > 0 ? sum / static_cast<float>(count) : 0.0f;
}