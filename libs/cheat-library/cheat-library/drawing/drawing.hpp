#pragma once
#include "../include.hpp"
#include "../math/math.hpp"

class Drawing
{
public:
	static void SetDrawList(ImDrawList* DL) { DrawList = DL; }
	static ImDrawList* GetDrawList() { return DrawList; }

	static void DrawBoundingBox(Vector3 min, Vector3 max, Matrix4x4 view_matrix, ImColor color);
	static void DrawCircle(Vector2 center, float radius, ImColor color, int segments = 12);
	static void DrawCircleFilled(Vector2 center, float radius, ImColor color, int segments = 12);
	static void DrawString(Vector2 pos, ImColor color, std::string text, bool center = false);
	static void DrawLine(Vector2 from, Vector2 to, float thickness, ImColor color);

private:
	inline static ImDrawList* DrawList;
};