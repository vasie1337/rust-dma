#pragma once
#include "../../include.hpp"

class Esp : public Drawing
{
private :
    static void RenderEntities()
    {
        auto entity_list = Cache::entities.load();
        auto camera_position = Cache::camera_pos.load();
        auto view_matrix = Cache::view_matrix.load();

        for (Entity& entity : entity_list)
        {
            float distance = camera_position.distance(entity.position);
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
    static void RenderBones(Player& player)
    {
        auto view_matrix = Cache::view_matrix.load();

        for (int i = 1; i < BoneList::max_bones; i++)
        {
            Vector3 bone_position = player.GetBonePosition(i);
            if (bone_position.invalid())
                continue;

            Vector2 screen_position;
            Math::WorldToScreen(bone_position, screen_position, view_matrix);
            {
                DrawCircle(screen_position, 1.0f, ImColor(1.0f, 1.0f, 1.0f, 1.0f), 0);
            }
        }
    }
    static void RenderPlayers()
    {
        auto local_player = Cache::local_player.load();
        auto player_list = Cache::players.load();
        auto camera_position = Cache::camera_pos.load();
        auto view_matrix = Cache::view_matrix.load();

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
                float radius = std::max<float>(1.0f, 50.0f / distance);
                DrawCircle(head_screen, radius, ImColor(1.0f, 1.0f, 1.0f, 1.0f), 0);
            }

            Vector3 foot_middle = Vector3();
            {
                Vector3 right_foot = player.GetBonePosition(BoneList::r_foot);
                Vector3 left_foot = player.GetBonePosition(BoneList::l_foot);

                if (!right_foot.invalid() && !left_foot.invalid())
                {
                    foot_middle = (right_foot + left_foot) / 2.f;
                }
            }

            Vector2 screen_position;
            if (Math::WorldToScreen(foot_middle, screen_position, view_matrix))
            {
                std::string text = player.formatted_name + " [" + std::to_string(static_cast<int>(distance)) + "m]";
                DrawString(screen_position, ImColor(1.0f, 1.0f, 1.0f, 1.0f), text);
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
                    ImColor color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
                    DrawLine(start_screen, end_screen, 1.f, color);
                }
            }
        }
    }
public:
    static void Render()
    {
		//RenderEntities();
        RenderPlayers();
    }
};