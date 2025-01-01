#pragma once
#include "../../include.hpp"

class Esp : public Drawing
{
private:
    static std::string ws2s(const std::wstring& wstr);

    static void RenderEntities();
    static void RenderPlayers();

public:
    static void Render()
    {
		RenderEntities();
        RenderPlayers();
    }
};