#include "overlay.hpp"

bool Overlay::Create(std::string overlay_name)
{
	if (!CreateWindowAndClass(overlay_name))
	{
		std::cerr << "Failed to create window and class." << std::endl;
		return false;
	}

	if (!CreateDevice())
	{
		std::cerr << "Failed to create device." << std::endl;
		return false;
	}

	if (!CreateRenderTarget())
	{
		std::cerr << "Failed to create render target." << std::endl;
		return false;
	}

	if (!CreateImGui())
	{
		std::cerr << "Failed to create ImGui." << std::endl;
		return false;
	}

	return true;
}

void Overlay::Destroy()
{
	DestroyImGui();
	DestroyRenderTarget();
	DestroyDevice();
	DestroyWindowAndClass();
}

void Overlay::Run()
{
	MSG Message;
	ZeroMemory(&Message, sizeof(MSG));

	while (Message.message != WM_QUIT && ShouldRun)
	{
		if (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
			continue;
		}

		if (windowCallback) { windowCallback(this); }

		HandleEvents();
		UpdatePerformanceMetrics();

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		DrawList = ImGui::GetBackgroundDrawList();

		RenderNotifications();
		RenderPerformanceMetrics();

		if (renderCallback) { renderCallback(this); }

		ImGui::Render();

		static constexpr float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		DeviceContext->OMSetRenderTargets(1, &MainRenderTargetView, nullptr);
		DeviceContext->ClearRenderTargetView(MainRenderTargetView, clearColor);

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		HRESULT hr = SwapChain->Present(VSync, 0);
		if (FAILED(hr)) {
			if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
				std::cerr << "Device lost: " << Device->GetDeviceRemovedReason() << std::endl;
				ResetDevice();
				continue;
			}
		}

	}
}

void Overlay::ResetDevice()
{
	if (MainRenderTargetView) { MainRenderTargetView->Release(); MainRenderTargetView = nullptr; }
	if (DeviceContext) { DeviceContext->Release(); DeviceContext = nullptr; }
	if (Device) { Device->Release(); Device = nullptr; }
	if (SwapChain) { SwapChain->Release(); SwapChain = nullptr; }

	if (!CreateDevice()) {
		std::cerr << "Failed to reset device." << std::endl;
		ShouldRun = false;
	}
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Overlay::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
	{
		return true;
	}

	switch (message)
	{
	case WM_SYSCOMMAND:
	{
		if ((wParam & 0xfff0) == SC_KEYMENU)
		{
			return 0;
		}
		break;
	}
	case WM_DESTROY:
	case WM_CLOSE:
	case WM_QUIT:
		ShouldRun = false;
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

bool Overlay::CreateWindowAndClass(std::string window_name)
{
	static WNDCLASSEXA WindowClass = {
		sizeof(WNDCLASSEXA),
		0,
		WndProc,
		0,
		0,
		GetModuleHandle(nullptr),
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		window_name.c_str(),
		nullptr
	};

	WNDCLASSEXA wc;
	if (!GetClassInfoExA(WindowClass.hInstance, window_name.c_str(), &wc))
	{
		if (!RegisterClassExA(&WindowClass))
		{
			return false;
		}
	}

	HMONITOR primaryMonitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
	if (GetMonitorInfo(primaryMonitor, &monitorInfo))
	{
		ScreenSize.x = static_cast<float>(monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);
		ScreenSize.y = static_cast<float>(monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top);
	}

	DWORD exStyle = 0;

#ifdef CLICK_THROUGH
	exStyle |= WS_EX_LAYERED | WS_EX_TRANSPARENT;
#endif

	OverlayWindow = CreateWindowExA(
		exStyle,
		window_name.c_str(),
		nullptr,
		WS_POPUP,
		monitorInfo.rcMonitor.left,
		monitorInfo.rcMonitor.top,
		static_cast<int>(ScreenSize.x),
		static_cast<int>(ScreenSize.y),
		nullptr,
		nullptr,
		WindowClass.hInstance,
		nullptr
	);

	if (!OverlayWindow)
	{
		return false;
	}

	SetWindowLongA(OverlayWindow, GWL_STYLE, WS_POPUP | WS_VISIBLE);

	SetLayeredWindowAttributes(OverlayWindow, RGB(0, 0, 0), 255, LWA_ALPHA);

	DWMNCRENDERINGPOLICY ncrp = DWMNCRP_ENABLED;
	DwmSetWindowAttribute(OverlayWindow, DWMWA_NCRENDERING_POLICY, &ncrp, sizeof(ncrp));

	MARGINS Margin = { -1, -1, -1, -1 };
	DwmExtendFrameIntoClientArea(OverlayWindow, &Margin);

	POINT ptSrc = { 0, 0 };
	BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	UpdateLayeredWindow(OverlayWindow, nullptr, nullptr, nullptr, nullptr, &ptSrc, 0, &blend, ULW_ALPHA);

	ShowWindow(OverlayWindow, SW_SHOWNOACTIVATE);

	return true;
}

bool Overlay::CreateDevice()
{
	static DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};

	SwapChainDesc.BufferCount = 2;

	RECT rect;
	GetClientRect(OverlayWindow, &rect);
	SwapChainDesc.BufferDesc.Width = rect.right;
	SwapChainDesc.BufferDesc.Height = rect.bottom;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;

	static UINT refreshRate;
	if (!refreshRate) {
		HDC hDC = GetDC(this->OverlayWindow);
		refreshRate = GetDeviceCaps(hDC, VREFRESH);
		ReleaseDC(this->OverlayWindow, hDC);
	}

	SwapChainDesc.BufferDesc.RefreshRate.Numerator = refreshRate;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = OverlayWindow;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;

	SwapChainDesc.Windowed = true;

	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	UINT createDeviceFlags = 0;

	static const D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0
	};

	D3D_FEATURE_LEVEL featureLevel;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createDeviceFlags,
		featureLevels,
		_countof(featureLevels),
		D3D11_SDK_VERSION,
		&SwapChainDesc,
		&SwapChain,
		&Device,
		&featureLevel,
		&DeviceContext
	);

	if (FAILED(hr)) {
		std::cerr << "Failed to create device and swap chain. HRESULT: " << hr << std::endl;
		return false;
	}

	hr = SwapChain->SetFullscreenState(false, nullptr);

	if (FAILED(hr)) {
		std::cerr << "Failed to set fullscreen state. HRESULT: " << hr << std::endl;
		return false;
	}

	return true;
}

bool Overlay::CreateRenderTarget()
{
	ID3D11Texture2D* BackBuffer = nullptr;

	if (FAILED(SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer))))
	{
		std::cerr << "Failed to get back buffer." << std::endl;
		return false;
	}

	if (FAILED(Device->CreateRenderTargetView(BackBuffer, nullptr, &MainRenderTargetView)))
	{
		std::cerr << "Failed to create render target view." << std::endl;
		BackBuffer->Release();
		return false;
	}

	BackBuffer->Release();

	return true;
}

bool Overlay::CreateImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	ImGui::StyleColorsDark();

	io.Fonts->AddFontDefault();

	float baseFontSize = 16.0f;
	float iconFontSize = baseFontSize * 2.0f / 3.0f;

	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = baseFontSize;
	io.Fonts->AddFontFromMemoryTTF(icon_font_data, sizeof(icon_font_data), baseFontSize, &icons_config, icons_ranges);

	io.Fonts->Build();

	if (setStyleCallback)
	{
		setStyleCallback(this);
	}

	ImGui_ImplWin32_Init(OverlayWindow);
	ImGui_ImplDX11_Init(Device, DeviceContext);

	return true;
}

void Overlay::DestroyWindowAndClass()
{
	DestroyWindow(OverlayWindow);
	UnregisterClassA(WindowClass.lpszClassName, WindowClass.hInstance);
}

void Overlay::DestroyDevice()
{
	if (DeviceContext)
	{
		DeviceContext->Release();
	}

	if (Device)
	{
		Device->Release();
	}

	if (SwapChain)
	{
		SwapChain->Release();
	}
}

void Overlay::DestroyRenderTarget()
{
	if (MainRenderTargetView)
	{
		MainRenderTargetView->Release();
	}
}

void Overlay::DestroyImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	//ImGui::DestroyContext(); // Ez fix ?
}

void Overlay::HandleEvents()
{
	ImGuiIO& io = ImGui::GetIO();

	POINT pos{};
	GetCursorPos(&pos);
	io.MousePos = ImVec2(static_cast<float>(pos.x), static_cast<float>(pos.y));

	constexpr int PRESSED_MASK = 0x8000;
	io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & PRESSED_MASK) != 0;
	io.MouseDown[1] = (GetAsyncKeyState(VK_RBUTTON) & PRESSED_MASK) != 0;
	io.MouseDown[2] = (GetAsyncKeyState(VK_MBUTTON) & PRESSED_MASK) != 0;
	io.MouseDown[3] = (GetAsyncKeyState(VK_XBUTTON1) & PRESSED_MASK) != 0;
	io.MouseDown[4] = (GetAsyncKeyState(VK_XBUTTON2) & PRESSED_MASK) != 0;
}

void Overlay::UpdatePerformanceMetrics()
{
	auto now = std::chrono::high_resolution_clock::now();
	metrics.frameTime = std::chrono::duration<float>(now - metrics.lastFrameTime).count();
	metrics.lastFrameTime = now;

	metrics.fps = 1.0f / metrics.frameTime;
	metrics.fpsHistory[metrics.fpsHistoryIndex] = metrics.fps;
	metrics.fpsHistoryIndex = (metrics.fpsHistoryIndex + 1) % metrics.fpsHistory.size();
}

void Overlay::RenderPerformanceMetrics()
{
	if (ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
		ImGui::Text("FPS: %.1f", metrics.fps);
		ImGui::Text("Frame Time: %.2f ms", metrics.frameTime * 1000.0f);

		float avgFps = std::accumulate(metrics.fpsHistory.begin(), metrics.fpsHistory.end(), 0.0f) / metrics.fpsHistory.size();
		ImGui::Text("Average FPS: %.1f", avgFps);

		ImVec2 size = ImGui::GetContentRegionAvail();
		size.y = 50.0f;
		ImGui::PlotLines("##FPSHistory", metrics.fpsHistory.data(), metrics.fpsHistory.size(), metrics.fpsHistoryIndex, nullptr, 0.0f, 200.0f, size);

		ImGui::End();
	}
}
