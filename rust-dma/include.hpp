#pragma once
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <shared_mutex>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <array>
#include <deque>
#include <queue>
#include <set>
#include <list>
#include <random>
#include <regex>
#include <memory>
#include <codecvt>

// DMA LIB
#include "../libs/dma-library/dma-library/dma/dma.hpp"
#pragma comment(lib, "dma-library.lib")

// OVERLAY LIB
#include "../libs/overlay-library/overlay-library/overlay.hpp"
#pragma comment(lib, "overlay-library.lib")

// CHEAT LIB
#include "../libs/cheat-library/cheat-library/math/math.hpp"
#include "../libs/cheat-library/cheat-library/drawing/drawing.hpp"
#include "../libs/cheat-library/cheat-library/timer/timer.hpp"
#pragma comment(lib, "cheat-library.lib")

// Project
#include "cheat/settings.hpp"
#include "cheat/sdk/offsets/offsets.hpp"
#include "cheat/sdk/decryption/decryption.hpp"
#include "cheat/sdk/transform/transform.hpp"
#include "cheat/filter/category/category.hpp"
#include "cheat/filter/filter.hpp"
#include "cheat/sdk/entity/entity.hpp"
#include "cheat/sdk/player/player.hpp"
#include "cheat/cache/data/data.hpp"
#include "cheat/cache/threads/threads.hpp"
#include "cheat/cache/cache.hpp"
#include "cheat/features/esp.hpp"
#include "cheat/features/aimbot.hpp"
#include "cheat/menu/menu.hpp"
#include "cheat/cheat.hpp"
