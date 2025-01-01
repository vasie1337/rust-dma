#pragma once
#include "../include.hpp"

class Cheat : public Cache, public Overlay, public Esp
{
public:
	Cheat();
	~Cheat();

private:
	static void StyleCallback(Overlay* overlay);
	static void WindowCallback(Overlay* overlay);
	static void RenderCallback(Overlay* overlay);
};