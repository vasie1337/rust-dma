#pragma once
#include "../../include.hpp"

class Esp : public Drawing
{
public:
    static void Render()
    {
        auto view_matrix = Cache::view_matrix.Get();
		auto local_player = Cache::local_player.Get();
		auto camera_position = Cache::camera_pos.Get();
        
        if (pvp_mode)
        {
            auto player_list = Cache::players.Get();

            for (Player& player : player_list)
            {
                Vector3 head_bone = player.GetBonePosition(BoneList::head);
                if (head_bone.IsZero())
                    continue;

                float distance = camera_position.Distance(head_bone);
                if (distance > max_entity_distance)
                    continue;

                Vector2 head_screen;
                if (Math::WorldToScreen(head_bone, head_screen, view_matrix))
                {
                    float radius = std::max<float>(1.0f, 100.0f / distance);
                    DrawCircle(head_screen, radius, ImColor(1.0f, 1.0f, 1.0f, 1.0f), 0);
                }

                Vector3 foot_bone = player.GetBonePosition(BoneList::r_foot);
                Vector2 screen_position;
                if (Math::WorldToScreen(foot_bone, screen_position, view_matrix))
                {
                    std::string text = player.formatted_name + " [" + std::to_string(static_cast<int>(distance)) + "m]";
                    DrawString(screen_position, ImColor(1.0f, 1.0f, 1.0f, 1.0f), text);
                }

                for (const auto& connection : player.SkeletonConnections)
                {
                    Vector3 start = player.GetBonePosition(connection.first);
                    Vector3 end = player.GetBonePosition(connection.second);

                    if (start.IsZero() || end.IsZero())
                        continue;

                    Vector2 start_screen;
                    Vector2 end_screen;
                    if (Math::WorldToScreen(start, start_screen, view_matrix) && Math::WorldToScreen(end, end_screen, view_matrix))
                    {
                        ImColor color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
                        DrawLine(start_screen, end_screen, 1.f, color);
                    }
                }
            }
        }
        else
        {
            auto entity_list = Cache::entities.Get();

            for (Entity& entity : entity_list)
            {
                float distance = camera_position.Distance(entity.position);
                if (distance > max_entity_distance)
                    continue;

                Vector2 screen_position;
                if (Math::WorldToScreen(entity.position, screen_position, view_matrix))
                {
					const auto category = Filter::GetCategory(entity);
					if (!category.Enabled())
						continue;

					std::string text = entity.formatted_name + " [" + std::to_string(static_cast<int>(distance)) + "m]";

                    DrawString(screen_position, category.GetColor(), text);
                }
            }
        }
    }
};