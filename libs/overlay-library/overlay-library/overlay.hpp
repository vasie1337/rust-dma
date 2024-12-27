#pragma once
#include "include.hpp"

class Globals
{
public:
	ImVec2 ScreenSize = ImVec2(static_cast<float>(GetSystemMetrics(SM_CXSCREEN)), static_cast<float>(GetSystemMetrics(SM_CYSCREEN)));
	ImVec2 ScreenCenter = ImVec2(ScreenSize.x / 2, ScreenSize.y / 2);

	HWND OverlayWindow = nullptr;

	WNDCLASSEXA WindowClass{};
	DXGI_SWAP_CHAIN_DESC SwapChainDesc{};

	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;
	static inline IDXGISwapChain* SwapChain = nullptr;
	ID3D11RenderTargetView* MainRenderTargetView = nullptr;

	bool VSync = false;

	struct PerformanceMetrics {
		float frameTime{};
		float fps{};
		std::chrono::high_resolution_clock::time_point lastFrameTime{};
		std::vector<float> fpsHistory = std::vector<float>(128, 0.0f);
		size_t fpsHistoryIndex{};
	} metrics;
};

class Overlay : public Globals, public NotificationSystem
{
public:
	using Callback = std::function<void(Overlay*)>;

	inline static bool ShouldRun = true;
	ImDrawList* DrawList = nullptr;

	// Create the overlay window
	bool Create(std::string overlay_name = "Overlay");
	// Destroy the overlay window
	void Destroy();
	// Run the overlay
	void Run();
	// Stop the overlay
	void Stop() { ShouldRun = false; }

	void ResetDevice();

	// Callback to handle ImGui style initialization
	void SetStyleCallback(Callback style) { setStyleCallback = style; }
	// Callback to handle ImGui rendering
	void SetRenderCallback(Callback render) { renderCallback = render; }
	// Callback to handle window events
	void SetWindowCallback(Callback window) { windowCallback = window; }

private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	bool CreateWindowAndClass(std::string window_name);
	bool CreateDevice();
	bool CreateRenderTarget();
	bool CreateImGui();
	
	void DestroyWindowAndClass();
	void DestroyDevice();
	void DestroyRenderTarget();
	void DestroyImGui();

	void HandleEvents();

	void UpdatePerformanceMetrics();
	void RenderPerformanceMetrics();

	Callback setStyleCallback = nullptr;
	Callback renderCallback = nullptr;
	Callback windowCallback = nullptr;
};