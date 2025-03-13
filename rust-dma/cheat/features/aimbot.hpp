#pragma once
#include "../../include.hpp"

class Aimbot
{
public:
	static void Run();

private:
	static inline std::thread aimbot_thread;
	static inline FrameData frame_buffer;

	static void Step();
};

