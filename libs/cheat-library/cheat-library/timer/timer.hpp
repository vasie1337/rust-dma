#pragma once
#include "../include.hpp"

class TimerScoped
{
public:
	TimerScoped(std::string name, int maxtime = 0) : _name(name)
	{
		_start = std::chrono::high_resolution_clock::now();
		_maxtime = maxtime;
	}

	~TimerScoped()
	{
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - _start).count();

		std::cout << _name << " took " << duration << "us" << std::endl;

		if (_maxtime != 0 && duration > _maxtime)
		{
			std::cout << "		[!!!]" << _name << " took too long! (" << duration << "us)" << std::endl;
		}
	}

private:
	std::string _name;
	std::chrono::time_point<std::chrono::high_resolution_clock> _start;
	int _maxtime;
};

#define TIMER_SCOPED(name, maxtime) TimerScoped timer_scoped_instance(name, maxtime)
#define FUNCTION_TIMER(maxtime) TIMER_SCOPED(__FUNCTION__, maxtime)