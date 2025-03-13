#include "aimbot.hpp"

void Aimbot::Run()
{
	aimbot_thread = std::thread([]()
	{
		while (true)
		{
			Step();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	});
	aimbot_thread.detach();
}

void Aimbot::Step()
{
	if (!GetAsyncKeyState(VK_RBUTTON))
		return;

	{
		std::lock_guard<std::mutex> lock(CacheData::frame_mtx);
		frame_buffer = CacheData::frame_data;
	}

	Player* target = nullptr;
	float best_fov = FLT_MAX;

	for (Player& player : frame_buffer.players)
	{
		if (player.is_npc)
			continue;
		Vector3 head_bone = player.GetBonePosition(BoneList::head);
		if (head_bone.invalid())
			continue;
		Vector2 head_screen;
		if (!Math::WorldToScreen(head_bone, head_screen, frame_buffer.view_matrix))
			continue;
		float fov = head_screen.distance({ Math::screen_center.x, Math::screen_center.y });
		if (fov < best_fov)
		{
			best_fov = fov;
			target = &player;
		}
	}

	if (!target)
		return;

	Vector3 head_bone = target->GetBonePosition(BoneList::head);
	if (head_bone.invalid())
		return;
	Vector2 head_screen;
	if (!Math::WorldToScreen(head_bone, head_screen, frame_buffer.view_matrix))
		return;

	std::cout << "Aiming at: " << target->formatted_name << std::endl;
	std::cout << "Distance: " << frame_buffer.camera_pos.distance(head_bone) << "m" << std::endl;
	std::cout << "FOV: " << best_fov << std::endl;
	std::cout << "Screen Position: " << head_screen.to_string() << std::endl;
}
