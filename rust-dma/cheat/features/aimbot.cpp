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
	{
		std::lock_guard<std::mutex> lock(CacheData::frame_mtx);
		frame_buffer = CacheData::frame_data;
	}

	Player* target = nullptr;
	float best_fov = FLT_MAX;

	for (Player& player : frame_buffer.players)
	{
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


}
