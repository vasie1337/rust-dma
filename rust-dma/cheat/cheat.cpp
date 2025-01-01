#include "../include.hpp"

Cheat::Cheat() : Cache()
{
	if (!dma.Initialize("RustClient.exe"))
	{
		throw std::exception("Failed to initialize DMA");
	}

	Cache::base_address.store(dma.GetModuleBaseAddress("GameAssembly.dll"));
	if (!Cache::base_address.load())
	{
		throw std::exception("Failed to get base address");
	}

	Overlay::SetStyleCallback(StyleCallback);
	Overlay::SetWindowCallback(WindowCallback);
	Overlay::SetRenderCallback(RenderCallback);

	if (!Overlay::Create())
	{
		throw std::exception("Failed to initialize overlay");
	}

	Cache::Run();
	Overlay::Run();
}

Cheat::~Cheat()
{
	Overlay::Stop();
	Cache::Stop();
}

void Cheat::StyleCallback(Overlay* overlay)
{
	ImGuiStyle* style = &ImGui::GetStyle();

	style->ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
}

void Cheat::WindowCallback(Overlay* overlay)
{
	Drawing::SetDrawList(overlay->DrawList);

	Cache::UpdateViewMatrix();

	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		overlay->MenuVisible = !overlay->MenuVisible;
	}

	if (GetAsyncKeyState(VK_END) & 1)
	{
		overlay->Stop();
	}
}

void Cheat::RenderCallback(Overlay* overlay)
{
	Esp::Render();

	if (!overlay->MenuVisible)
		return;

	ImGui::Begin("Awhare Rust", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Checkbox("VSync", &overlay->VSync);
	ImGui::SliderFloat("Max entity distance", &max_entity_distance, 10.f, 300.f);
	ImGui::Checkbox("Crosshair Dot", &crosshair);
	if (crosshair)
	{
		ImGui::ColorEdit4("Crosshair Color", (float*)&crosshair_color);
		ImGui::SliderFloat("Crosshair Size", &crosshair_size, 1.f, 20.f);

		DrawCircleFilled({ Math::screen_center.x, Math::screen_center.y }, crosshair_size, crosshair_color, 0);
	}

	ImGui::Separator();

	ImGui::Checkbox("Player Boxes", &player_boxes);
	ImGui::Checkbox("Player Names", &player_names);
	ImGui::Checkbox("Player Skeletons", &player_skeletons);
	ImGui::Checkbox("Player Head circle", &player_head_circle);
	ImGui::ColorEdit4("Player Color", (float*)&player_color);

	ImGui::Separator();

	for (auto& category_ref : Filter::categories)
	{
		auto& category = category_ref.get();
		ImGui::Checkbox(category.Name().c_str(), &category.enabled);
		ImGui::SameLine();
		std::string color_label = category.Name() + " Color";
		ImGui::ColorEdit4(color_label.c_str(), (float*)&category.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
	}

	ImGui::Separator();

	static bool show_debug;
	static bool show_stats;

	ImGui::Checkbox("Show Debug", &show_debug);
	ImGui::Checkbox("Show Statistics", &show_stats);

	ImGui::End();

	if (show_debug)
	{
		ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Base Address: 0x%llX", Cache::base_address.load());
		ImGui::Text("Camera Object: 0x%llX", Cache::camera_object.load());
		ImGui::Text("Entity List: 0x%llX", Cache::entity_list.load());
		ImGui::Text("Local Player: 0x%llX", Cache::local_player.load().object_ptr);
		ImGui::Text("Camera Position: %.2f %.2f %.2f", Cache::camera_pos.load().x, Cache::camera_pos.load().y, Cache::camera_pos.load().z);
		ImGui::Text("Players: %d", Cache::players.load().size());
		ImGui::Text("Entities: %d", Cache::entities.load().size());
		ImGui::End();
	}
	if (show_stats)
	{
		ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_NoCollapse);

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

		overlay->RenderPerformanceMetrics();

		ImGui::End();
	}
}