#pragma once
#include "../../../include.hpp"

class CacheThread
{
public:
	CacheThread(std::function<void(HANDLE)> func, int delay)
		: func(std::move(func)), delay(delay) {
	}

	~CacheThread() {
		if (thread.joinable()) {
			Stop();
		}
	}

	void Run() {
		scatter_handle = dma.CreateScatterHandle();
		running = true;
		thread = std::thread([this]()
			{
				while (running.load(std::memory_order_relaxed))
				{
					func(scatter_handle);
					std::this_thread::sleep_for(std::chrono::milliseconds(delay));
				}
			});
	}

	void Stop() {
		running.store(false, std::memory_order_relaxed);
		if (thread.joinable()) {
			thread.join();
		}

		dma.CloseScatterHandle(scatter_handle);
	}

private:
	std::thread thread;
	std::function<void(HANDLE)> func;
	int delay = 0;
	std::atomic<bool> running{ false };
	HANDLE scatter_handle = 0;
};