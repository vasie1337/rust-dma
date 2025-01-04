#pragma once
#include "../../include.hpp"

class Menu
{
public:
	static void Render(Overlay* overlay);
private:
	static inline int current_tab = 0;
	static inline const char* tabs[] = { "Visuals", "Settings", "Debug" };

	static void RenderVisuals(Overlay* overlay);
	static void RenderSettings(Overlay* overlay);
	static void RenderDebug(Overlay* overlay);
};