#pragma once

// Standard Library
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <functional>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <utility>
#include <filesystem>

// Windows
#include <Windows.h>

// Imgui
#ifndef IMGUI_ALREADY_INCLUDED
#define IMGUI_ALREADY_INCLUDED
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"
#endif

// Direct3D
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

// Project
