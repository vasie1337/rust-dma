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

		ImGui::Separator();

		ImGui::Text("Entities: %d", Cache::entities.Get().size());
		ImGui::Text("Players: %d", Cache::players.Get().size());

		ImGui::Text("Local Player Pos: %s", Cache::camera_pos.Get().to_string().c_str());

		ImGui::Separator();

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

		if (crosshair)
		{
			DrawCircleFilled({ Math::screen_center.x, Math::screen_center.y }, crosshair_size, crosshair_color, 0);
		}

		Esp::Render();
	}
};