#pragma once
#include "../include.hpp"

class Cheat : public Cache, public Overlay, public Esp
{
public:
	Cheat(int fetch_delay = 1000) : Cache()
	{
		if (!dma.Initialize("RustClient.exe"))
		{
			throw std::exception("Failed to initialize DMA");
		}
		
		Cache::base_address.Set(dma.GetModuleBaseAddress("GameAssembly.dll"));
		if (!Cache::base_address.Get())
		{
			throw std::exception("Failed to get base address");
		}

		Overlay::SetStyleCallback(Style);
		Overlay::SetWindowCallback(Window);
		Overlay::SetRenderCallback(Render);

		if (!Overlay::Create())
		{
			throw std::exception("Failed to initialize overlay");
		}
	}
	~Cheat()
	{
		Stop();
	}

	void Run()
	{
		Cache::Run();
		Overlay::Run();
	}

	void Stop()
	{
		Overlay::Stop();
		Cache::Stop();
	}

private:
	static void Style(Overlay* overlay)
	{
		ImGuiStyle* style = &ImGui::GetStyle();

		style->ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
	}

	static void Window(Overlay* overlay)
	{
		Drawing::SetDrawList(overlay->DrawList);

		Cache::UpdateViewMatrix();

		if (GetAsyncKeyState(VK_END) & 1)
		{
			overlay->Stop();
		}
	}

	static void Render(Overlay* overlay)
	{
		ImGui::Begin("Awhare Rust", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Checkbox("VSync", &overlay->VSync);
		ImGui::SliderFloat("Max entity distance", &max_entity_distance, 10.f, 1000.f);
		ImGui::Checkbox("PVP Mode", &pvp_mode);
		ImGui::Checkbox("Crosshair Dot", &crosshair);
		if (crosshair)
		{
			ImGui::ColorEdit4("Crosshair Color", (float*)&crosshair_color);
			ImGui::SliderFloat("Crosshair Size", &crosshair_size, 1.f, 20.f);
		}

		ImGui::Text("Base Address: 0x%llX", Cache::base_address.Get());
		ImGui::Text("Camera Object: 0x%llX", Cache::camera_object.Get());
		ImGui::Text("Entity List: 0x%llX", Cache::entity_list.Get());
		ImGui::Text("Local Player: 0x%llX", Cache::local_player.Get().object_ptr);
		ImGui::Text("Camera Position: %.2f %.2f %.2f", Cache::camera_pos.Get().x, Cache::camera_pos.Get().y, Cache::camera_pos.Get().z);

		ImGui::End();

		ImGui::Begin("Filters", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Checkbox("Collectable Ores", &Filter::collectable_ores.enabled); ImGui::SameLine();
		ImGui::ColorEdit4("Collectable Ores Color", (float*)&Filter::collectable_ores.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		ImGui::Checkbox("Food", &Filter::food.enabled); ImGui::SameLine();
		ImGui::ColorEdit4("Food Color", (float*)&Filter::food.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		ImGui::Checkbox("Hemp", &Filter::hemp.enabled); ImGui::SameLine();
		ImGui::ColorEdit4("Hemp Color", (float*)&Filter::hemp.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		ImGui::Checkbox("Loot Containers", &Filter::loot_containers.enabled); ImGui::SameLine();
		ImGui::ColorEdit4("Loot Containers Color", (float*)&Filter::loot_containers.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		ImGui::Checkbox("Berries", &Filter::berries.enabled); ImGui::SameLine();
		ImGui::ColorEdit4("Berries Color", (float*)&Filter::berries.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		ImGui::Checkbox("Ores", &Filter::ores.enabled); ImGui::SameLine();
		ImGui::ColorEdit4("Ores Color", (float*)&Filter::ores.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		ImGui::Checkbox("Misc", &Filter::misc.enabled); ImGui::SameLine();
		ImGui::ColorEdit4("Misc Color", (float*)&Filter::misc.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		ImGui::Checkbox("Vehicles", &Filter::vehicles.enabled); ImGui::SameLine();
		ImGui::ColorEdit4("Vehicles Color", (float*)&Filter::vehicles.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		ImGui::Checkbox("NPC's", &Filter::npcs.enabled); ImGui::SameLine();
		ImGui::ColorEdit4("NPC's Color", (float*)&Filter::npcs.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		ImGui::End();


		ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_NoCollapse);

		ImGui::Text("Cache");

		ImGui::Text("Players: %d", Cache::players.Get().size());
		ImGui::Text("Entities: %d", Cache::entities.Get().size());

		ImGui::Text("Threads");

		for (auto& refthread : Cache::threads)
		{
			auto& thread = refthread.get();
			auto stats = thread.GetStatistics();

			std::string header = thread.GetName();

			if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
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
		ImGui::End();

		if (crosshair)
		{
			DrawCircleFilled({ Math::screen_center.x, Math::screen_center.y }, crosshair_size, crosshair_color, 0);
		}

		Esp::Render();
	}
};