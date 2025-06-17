#pragma once
#include "../include.hpp"
#include <windows.h>
#include <tlhelp32.h>

// only test local when doing a debug build
#ifdef _DEBUG
#define TEST_LOCAL
#endif

class DmaMemory {
public:
	static bool Init(std::string proc_name) {
		if (!mem.Init(proc_name))
		{
			return false;
		}

		if (!mem.GetKeyboard()->InitKeyboard())
		{
			return false;
		}

		if (!mem.FixCr3())
		{
			return false;
		}

		return true;
	}

	static uintptr_t GetBase(std::string mod_name) {
		return mem.GetBaseDaddy(mod_name);
	}

	static VMMDLL_SCATTER_HANDLE CreateScatter() {
		return mem.CreateScatterHandle();
	}

	static void CloseScatter(VMMDLL_SCATTER_HANDLE handle) {
		return mem.CloseScatterHandle(handle);
	}

	static void AddScatter(VMMDLL_SCATTER_HANDLE handle, uintptr_t address, void* buffer, size_t size) {
		return mem.AddScatterReadRequest(handle, address, buffer, size);
	}

	static void ExecuteScatter(VMMDLL_SCATTER_HANDLE handle) {
		return mem.ExecuteReadScatter(handle);
	}

	static bool Read(uintptr_t address, void* buffer, size_t size) {
		return mem.Read(address, buffer, size);
	}

	template<typename t>
	static t Read(uintptr_t address) {
		t buff{};
		if (!Read(address, &buff, sizeof(t))) {
			// idk : log err
		}
		return buff;
	}
};

class LocalMemory {
private:
	inline static HANDLE process_handle;
	inline static DWORD process_id;

public:
	static bool Init(std::string proc_name) {
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (snapshot == INVALID_HANDLE_VALUE) {
			return false;
		}

		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(snapshot, &entry)) {
			do {
				int size = WideCharToMultiByte(CP_UTF8, 0, entry.szExeFile, -1, nullptr, 0, nullptr, nullptr);
				if (size > 0) {
					std::string current_name(size - 1, '\0');
					WideCharToMultiByte(CP_UTF8, 0, entry.szExeFile, -1, &current_name[0], size, nullptr, nullptr);

					if (current_name == proc_name) {
						process_id = entry.th32ProcessID;
						process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
						CloseHandle(snapshot);
						return process_handle != NULL;
					}
				}
			} while (Process32Next(snapshot, &entry));
		}

		CloseHandle(snapshot);
		return false;
	}

	static uintptr_t GetBase(std::string mod_name) {
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);
		if (snapshot == INVALID_HANDLE_VALUE) {
			return 0;
		}

		MODULEENTRY32 entry;
		entry.dwSize = sizeof(MODULEENTRY32);

		if (Module32First(snapshot, &entry)) {
			do {
				// Convert wide string to narrow string using WideCharToMultiByte
				int size = WideCharToMultiByte(CP_UTF8, 0, entry.szModule, -1, nullptr, 0, nullptr, nullptr);
				if (size > 0) {
					std::string current_name(size - 1, '\0');
					WideCharToMultiByte(CP_UTF8, 0, entry.szModule, -1, &current_name[0], size, nullptr, nullptr);

					if (current_name == mod_name) {
						CloseHandle(snapshot);
						return reinterpret_cast<uintptr_t>(entry.modBaseAddr);
					}
				}
			} while (Module32Next(snapshot, &entry));
		}

		CloseHandle(snapshot);
		return 0;
	}

	static VMMDLL_SCATTER_HANDLE CreateScatter() {
		// For local memory, we'll use a simple vector to store scatter requests
		return reinterpret_cast<VMMDLL_SCATTER_HANDLE>(new std::vector<ScatterRequest>());
	}

	static void CloseScatter(VMMDLL_SCATTER_HANDLE handle) {
		if (handle) {
			delete reinterpret_cast<std::vector<ScatterRequest>*>(handle);
		}
	}

	static void AddScatter(VMMDLL_SCATTER_HANDLE handle, uintptr_t address, void* buffer, size_t size) {
		if (handle) {
			auto* requests = reinterpret_cast<std::vector<ScatterRequest>*>(handle);
			requests->push_back({ address, buffer, size });
		}
	}

	static void ExecuteScatter(VMMDLL_SCATTER_HANDLE handle) {
		if (handle) {
			auto* requests = reinterpret_cast<std::vector<ScatterRequest>*>(handle);
			for (const auto& req : *requests) {
				ReadProcessMemory(process_handle, reinterpret_cast<LPCVOID>(req.address), req.buffer, req.size, nullptr);
			}
			requests->clear();
		}
	}

	static bool Read(uintptr_t address, void* buffer, size_t size) {
		if (!process_handle) {
			return false;
		}

		SIZE_T bytes_read;
		return ReadProcessMemory(process_handle, reinterpret_cast<LPCVOID>(address), buffer, size, &bytes_read) && bytes_read == size;
	}

	template<typename t>
	static t Read(uintptr_t address) {
		t buff{};
		if (!Read(address, &buff, sizeof(t))) {
			// idk : log err
		}
		return buff;
	}

private:
	struct ScatterRequest {
		uintptr_t address;
		void* buffer;
		size_t size;
	};
};

#ifdef TEST_LOCAL
class Mem : public LocalMemory {};
#else
class Mem : public DmaMemory {};
inline Mem dma;
#endif
