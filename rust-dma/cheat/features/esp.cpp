#include "../../include.hpp"

std::string Esp::ws2s(const std::wstring& wstr)
{
#pragma warning(disable: 4996)
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
#pragma warning(default: 4996)
}

void Esp::RenderEntities()
{
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

void Esp::RenderPlayers()
{
    for (Player& player : player_list)
    {
        if (player.is_npc)
			continue;

        if (player_head_circle)
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
                float radius = std::max<float>(1.0f, 40.0f / distance);
                DrawCircle(head_screen, radius, player_color, 0);
            }
        }

        if (player_skeletons)
        {
            for (const auto& connection : SkeletonConnections)
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
        }

        if (player_boxes)
        {
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

        Vector3 head_bone = player.GetBonePosition(BoneList::spine1);
        if (head_bone.invalid())
            continue;
        float distance = camera_position.distance(head_bone);
        if (distance > max_entity_distance)
            continue;

        if (player_names)
        {
            std::string player_name_converted = ws2s(player.player_name);

            Vector2 head_screen;
            if (Math::WorldToScreen(head_bone, head_screen, view_matrix))
            {
                std::string text = player_name_converted + " [" + std::to_string(static_cast<int>(distance)) + "m]";
                DrawString(head_screen, player_color, text);
            }
        }

        if (player_snaplines)
        {
            Vector2 head_screen;
            Math::WorldToScreen(head_bone, head_screen, view_matrix, true);
            
            Vector2 screen_center = Vector2(Math::screen_size.x / 2.f, 0.f);
            DrawLine(screen_center, head_screen, 1.f, player_color);
        }
    }
}