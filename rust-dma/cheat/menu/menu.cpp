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
	ImGui::Text("Base Address: 0x%llX", Cache::base_address.load());
	ImGui::Text("Camera Object: 0x%llX", Cache::camera_address.load());
	ImGui::Text("Entity List: 0x%llX", Cache::entity_list_address.load());
	ImGui::Text("Camera Position: %.2f %.2f %.2f", Cache::camera_pos.load().x, Cache::camera_pos.load().y, Cache::camera_pos.load().z);
	ImGui::Text("Players: %d", Cache::players.load().size());
	ImGui::Text("Entities: %d", Cache::entities.load().size());

	ImGui::Separator();

	ImGui::Text("Threads");

	for (auto& thread_ref : Cache::threads)
	{
		auto& thread = thread_ref.get();
		auto stats = thread.GetStatistics();

		std::string header = thread.GetName();

		if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_None))
		{
			ImGui::Indent();

			ImGui::Columns(2, nullptr, false);

			ImGui::BeginGroup();
			ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Current: %.2f ms", stats.current_time);
			ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.8f, 1.0f), "Average: %.2f ms", stats.average_time);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Average over last 128 iterations");

			ImGui::TextColored(ImVec4(0.2f, 0.6f, 0.9f, 1.0f), "Min/Max: %.2f/%.2f ms", stats.min_time, stats.max_time);
			ImGui::EndGroup();

			ImGui::NextColumn();
			ImGui::BeginGroup();

			const auto& times = thread.GetIterationTimes();

			float plot_min = stats.min_time * 0.9f;
			float plot_max = stats.max_time * 1.1f;

			std::string plot_title = "##" + header + "Times";

			ImGui::PlotLines(
				plot_title.c_str(),
				times.data(),
				static_cast<int>(times.size()),
				0,
				"Execution Time (ms)",
				plot_min,
				plot_max,
				ImVec2(ImGui::GetContentRegionAvail().x, 80));

			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Last 128 iterations)");

			ImGui::EndGroup();

			ImGui::Columns(1);
			ImGui::Unindent();

			ImGui::PushID(&thread);

			if (ImGui::Button("Pause")) {
				thread.Pause();
			}
			ImGui::SameLine();
			if (ImGui::Button("Resume")) {
				thread.Resume();
			}

			ImGui::PopID();
		}
		else
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
				"Current: %.2f ms | Avg: %.2f ms",
				stats.current_time,
				stats.average_time
			);
		}
	}

	ImGui::Separator();

	ImGui::Text("Overlay");

	overlay->RenderPerformanceMetrics();
}
