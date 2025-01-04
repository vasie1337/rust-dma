#include "../include.hpp"

Cheat::Cheat()
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

	if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_Insert))
	{
		overlay->MenuVisible = !overlay->MenuVisible;
	}

	if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_End))
	{
		overlay->Stop();
	}
}

void Cheat::RenderCallback(Overlay* overlay)
{
	Esp::Render();

	if (crosshair)
		DrawCircleFilled({ Math::screen_center.x, Math::screen_center.y }, crosshair_size, crosshair_color, 0);

	if (overlay->MenuVisible)
		Menu::Render(overlay);
}