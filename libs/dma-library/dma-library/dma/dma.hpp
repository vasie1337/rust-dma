#include "../include.hpp"

#pragma warning(disable: 4200)
#include "../libs/vmmdll.h"
#include "../libs/leechcore.h"
#pragma warning(default: 4200)

class Dma 
{
public:
	std::string _process_name;
	uint32_t _pid = 0;
	uint64_t _base_address = 0;

	Dma()
	{
		_vmm = LoadLibraryA("vmm.dll");
		_FTD3XX = LoadLibraryA("FTD3XX.dll");
		_leechcore = LoadLibraryA("leechcore.dll");

		if (!_vmm || !_FTD3XX || !_leechcore)
		{
			std::cout << "Failed to load required libraries" << std::endl;
			exit(1);
		}
	}
	~Dma()
	{
		VMMDLL_Close(_hDevice);

		if (_vmm)
			FreeLibrary(_vmm);
		if (_FTD3XX)
			FreeLibrary(_FTD3XX);
		if (_leechcore)
			FreeLibrary(_leechcore);
	}

	bool Initialize(std::string process_name);
	bool FixCR3() const;

	uint32_t GetPidFromName(const std::string& process_name) const;
	uint64_t GetModuleBaseAddress(const std::string& module_name) const;

	void PrintAllModules() const;

	bool Read(void* address, void* buffer, size_t size) const;
	bool Write(uint64_t address, void* buffer, size_t size) const;

	template <typename T>
	T Read(uint64_t address) const
	{
		T buffer;
		Read((void*)address, &buffer, sizeof(T));
		return buffer;
	}

	template <typename T>
	bool Write(uint64_t address, T value) const
	{
		return Write(address, &value, sizeof(T));
	}

	VMMDLL_SCATTER_HANDLE CreateScatterHandle() const;

	void CloseScatterHandle(VMMDLL_SCATTER_HANDLE handle) const;
	bool AddScatterRead(VMMDLL_SCATTER_HANDLE handle, uint64_t address, void* buffer, size_t size) const;
	bool ExecuteScatterRead(VMMDLL_SCATTER_HANDLE handle) const;

private:
	HMODULE _vmm = nullptr;
	HMODULE _FTD3XX = nullptr;
	HMODULE _leechcore = nullptr;

	VMM_HANDLE _hDevice = nullptr;

	bool DumpMemoryMap(std::string& path);
	bool SetFPGA() const;
};

inline Dma dma;