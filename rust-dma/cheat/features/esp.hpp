#pragma once
#include "../../include.hpp"

class Esp : public Drawing
{
private:
    static std::string ws2s(const std::wstring& wstr);

    static void RenderEntities();
    static void RenderPlayers();

    static inline std::vector<Entity> entity_list;
    static inline std::vector<Player> player_list;
    static inline Vector3 camera_position;
    static inline Matrix4x4 view_matrix;
    static inline Player local_player;

public:
    static void Render()
    {
        local_player = Cache::local_player.load();
        player_list = Cache::players.load();
        entity_list = Cache::entities.load();
        camera_position = Cache::camera_pos.load();
        view_matrix = Cache::view_matrix.load();

		RenderEntities();
        RenderPlayers();
    }
};