#pragma once
#include "../../include.hpp"

class Esp : public Drawing
{
private:
    static std::string ws2s(const std::wstring& wstr);

    static void RenderEntities();
    static void RenderPlayers();

	static inline FrameData frame_buffer;

public:
    static void Render()
    {
        frame_buffer = CacheData::frame_data;

		RenderEntities();
        RenderPlayers();
    }
};