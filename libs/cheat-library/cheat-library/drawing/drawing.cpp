#include "drawing.hpp"

void Drawing::DrawBoundingBox(Vector3 min, Vector3 max, Matrix4x4 view_matrix, ImColor color)
{
	Vector3 points[] = {
		{min.x, min.y, min.z},
		{min.x, max.y, min.z},
		{max.x, max.y, min.z},
		{max.x, min.y, min.z},
		{max.x, max.y, max.z},
		{min.x, max.y, max.z},
		{min.x, min.y, max.z},
		{max.x, min.y, max.z}
	};

	Vector2 screenPoints[8];
	bool valid = true;

	for (int i = 0; i < 8; ++i)
	{
		if (!Math::WorldToScreen(points[i], screenPoints[i], view_matrix))
		{
			valid = false;
			break;
		}
	}

	if (!valid)
		return;

	float left = FLT_MAX;
	float top = FLT_MIN;
	float right = FLT_MIN;
	float bottom = FLT_MAX;

	for (const auto& sp : screenPoints)
	{
		if (left > sp.x) left = sp.x;
		if (top < sp.y) top = sp.y;
		if (right < sp.x) right = sp.x;
		if (bottom > sp.y) bottom = sp.y;
	}

	DrawList->AddRect(ImVec2(left, top), ImVec2(right, bottom), color);
}

void Drawing::DrawCircle(Vector2 center, float radius, ImColor color, int segments)
{
	DrawList->AddCircle(ImVec2(center.x, center.y), radius, color, segments);
}

void Drawing::DrawCircleFilled(Vector2 center, float radius, ImColor color, int segments)
{
	DrawList->AddCircleFilled(ImVec2(center.x, center.y), radius, color, segments);
}

void Drawing::DrawString(Vector2 pos, ImColor color, std::string text, bool center)
{
	if (center)
	{
		ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
		pos.x -= textSize.x / 2;
		pos.y -= textSize.y / 2;
	}

	DrawList->AddText(ImVec2(pos.x, pos.y), color, text.c_str());
}

void Drawing::DrawLine(Vector2 from, Vector2 to, float thickness, ImColor color)
{
	DrawList->AddLine(ImVec2(from.x, from.y), ImVec2(to.x, to.y), color, thickness);
}