#pragma once
#include "../../include.hpp"

class Esp : public Drawing
{
private:
    static std::vector<Vector3> CalcBox(Vector3 origin, Vector3 center, Vector3 extents)
    {
        std::vector<Vector3> box;
        box.push_back(Vector3(center.x + extents.x, center.y + extents.y, center.z + extents.z));
        box.push_back(Vector3(center.x + extents.x, center.y + extents.y, center.z - extents.z));
        box.push_back(Vector3(center.x + extents.x, center.y - extents.y, center.z + extents.z));
        box.push_back(Vector3(center.x + extents.x, center.y - extents.y, center.z - extents.z));
        box.push_back(Vector3(center.x - extents.x, center.y + extents.y, center.z + extents.z));
        box.push_back(Vector3(center.x - extents.x, center.y + extents.y, center.z - extents.z));
        box.push_back(Vector3(center.x - extents.x, center.y - extents.y, center.z + extents.z));
        box.push_back(Vector3(center.x - extents.x, center.y - extents.y, center.z - extents.z));
        for (auto& point : box)
            point += origin;
        return box;
    }
	static void DrawBox(const std::vector<Vector3>& box, const ImColor& color, Matrix4x4 view_matrix)
	{
        const std::vector<std::pair<int, int>> box_lines = {
             {0, 1}, {1, 3}, {3, 2}, {2, 0},
             {4, 5}, {5, 7}, {7, 6}, {6, 4},
             {0, 4}, {1, 5}, {2, 6}, {3, 7}
        };

        std::vector<Vector2> screen_box;
        for (const auto& point : box) {
            Vector2 screen_point;
            if (!Math::WorldToScreen(point, screen_point, view_matrix)) {
                screen_box.clear();
                break;
            }
            screen_box.push_back(screen_point);
        }

        if (screen_box.size() == 8) {
            for (const auto& line : box_lines) {
				DrawLine(screen_box[line.first], screen_box[line.second], 1.f, color);
            }
        }
	}
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

		static const ImColor player_color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);

        for (Player& player : player_list)
        {
            Vector3 head_bone = player.GetBonePosition(BoneList::head);
            if (head_bone.invalid())
                continue;
            
            float distance = camera_position.distance(head_bone);
            if (distance > max_entity_distance)
                continue;
            
			// Draw player head
            Vector2 head_screen;
            if (Math::WorldToScreen(head_bone, head_screen, view_matrix))
            {
                float radius = std::max<float>(1.0f, 30.0f / distance);
                DrawCircle(head_screen, radius, player_color, 0);
            }
            
			// Draw player skeleton
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

			// Draw player box
            {
                auto box = CalcBox(player.position, player.bounds.center, player.bounds.extents);
                DrawBox(box, player_color, view_matrix);
            }
        }
    }

public:
    static void Render()
    {
		RenderEntities();
        RenderPlayers();
    }
};