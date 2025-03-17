#include "menu.hpp"

void Menu::Render(Overlay* overlay)
{
	ImGui::Begin("Awhare Rust", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::BeginTabBar("Tabs");
	for (int i = 0; i < IM_ARRAYSIZE(tabs); i++)
	{
		if (ImGui::BeginTabItem(tabs[i]))
		{
			current_tab = i;
			ImGui::EndTabItem();
		}
	}
	ImGui::EndTabBar();

	switch (current_tab)
	{
	case 0:
		RenderVisuals(overlay);
		break;
	case 1:
		RenderSettings(overlay);
		break;
	case 2:
		RenderDebug(overlay);
		break;
	default:
		break;
	};

	ImGui::End();
}

void Menu::RenderVisuals(Overlay* overlay)
{
	ImGui::Text("Players");
	ImGui::Checkbox("Enable", &player_enable);
	ImGui::Checkbox("Boxes", &player_boxes);
	ImGui::Checkbox("Names", &player_names);
	ImGui::Checkbox("Skeletons", &player_skeletons);
	ImGui::Checkbox("Head circle", &player_head_circle);
	ImGui::Checkbox("Snaplines", &player_snaplines);
	ImGui::ColorEdit4("Color", (float*)&player_color);

	ImGui::Separator();

	ImGui::Text("Entities");

	for (auto& category_ref : Filter::categories)
	{
		auto& category = category_ref.get();
		ImGui::Checkbox(category.Name().c_str(), &category.enabled);
		ImGui::SameLine();
		std::string color_label = category.Name() + " Color";
		ImGui::ColorEdit4(color_label.c_str(), (float*)&category.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
	}

	if (ImGui::Button("Select All"))
	{
		for (auto& category_ref : Filter::categories)
		{
			auto& category = category_ref.get();
			category.enabled = true;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Deselect All"))
	{
		for (auto& category_ref : Filter::categories)
		{
			auto& category = category_ref.get();
			category.enabled = false;
		}
	}
}

void Menu::RenderSettings(Overlay* overlay)
{
	ImGui::Checkbox("VSync", &overlay->VSync);
	ImGui::SliderFloat("Max entity distance", &max_entity_distance, 10.f, 300.f);
	ImGui::Checkbox("Crosshair Dot", &crosshair);
	if (crosshair)
	{
		ImGui::ColorEdit4("Crosshair Color", (float*)&crosshair_color);
		ImGui::SliderFloat("Crosshair Size", &crosshair_size, 1.f, 20.f);
	}
}

void Menu::RenderDebug(Overlay* overlay)
{
	ImGui::Text("Base Address: 0x%llX", Cache::base_address);
	ImGui::Text("Camera Object: 0x%llX", Cache::camera_address);
	ImGui::Text("Entity List: 0x%llX", Cache::entity_list_address);

	ImGui::Separator();

	ImGui::Text("Players: %d", Cache::frame_data.players.size());
	ImGui::Text("Entities: %d", Cache::frame_data.entities.size());

	ImGui::Separator();

	ImGui::Separator();

	ImGui::Text("Overlay");

	overlay->RenderPerformanceMetrics();
}
