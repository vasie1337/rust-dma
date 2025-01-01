#pragma once
#include "../../include.hpp"

class Esp : public Drawing
{
private:
    static void RenderEntities()
    {
        auto entity_list = Cache::entities.load();
        auto camera_position = Cache::camera_pos.load();
        auto view_matrix = Cache::view_matrix.load();

        for (Entity& entity : entity_list)
        {
            const auto& category = Filter::GetCategory(entity.obj_name);
            if (!category.Enabled())
                continue;

            float distance = camera_position.distance(entity.position);
            if (distance > max_entity_distance)
                continue;

            Vector2 screen_position;
            if (Math::WorldToScreen(entity.position, screen_position, view_matrix))
            {
                std::string text = entity.formatted_name + " [" + std::to_string(static_cast<int>(distance)) + "m]";

                DrawString(screen_position, category.GetColor(), text);
            }
        }
    }
    static void RenderPlayers()
    {
        auto local_player = Cache::local_player.load();
        auto player_list = Cache::players.load();
        auto camera_position = Cache::camera_pos.load();
        auto view_matrix = Cache::view_matrix.load();

		const ImColor player_color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);

        for (Player& player : player_list)
        {
            Vector3 head_bone = player.GetBonePosition(BoneList::head);
            if (head_bone.invalid())
                continue;
            
            float distance = camera_position.distance(head_bone);
            if (distance > max_entity_distance)
                continue;
            
            Vector2 head_screen;
            if (Math::WorldToScreen(head_bone, head_screen, view_matrix))
            {
                float radius = std::max<float>(1.0f, 30.0f / distance);
                DrawCircle(head_screen, radius, player_color, 0);
            }
            
            for (const auto& connection : player.SkeletonConnections)
            {
                Vector3 start = player.GetBonePosition(connection.first);
                Vector3 end = player.GetBonePosition(connection.second);

                if (start.invalid() || end.invalid())
                    continue;

                Vector2 start_screen;
                Vector2 end_screen;
                if (Math::WorldToScreen(start, start_screen, view_matrix) && Math::WorldToScreen(end, end_screen, view_matrix))
                {
                    DrawLine(start_screen, end_screen, 1.f, player_color);
                }
            }

			Vector3 min_box = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
			Vector3 max_box = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

            for (int i = 0; i < BoneList::max_bones; i++)
            {
                Vector3 bone_pos = player.GetBonePosition(i);
                if (bone_pos.invalid())
                    continue;

                min_box = min_box._min(bone_pos);
                max_box = max_box._max(bone_pos);
            }

			DrawBoundingBox(min_box, max_box, view_matrix, player_color);
        }
    }
public:
    static void Render()
    {
		RenderEntities();
        RenderPlayers();
    }
};