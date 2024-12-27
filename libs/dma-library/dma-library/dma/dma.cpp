#include "dma.hpp"

bool Dma::Initialize(std::string process_name)
{
	std::string memory_ranges_path;
	if (!DumpMemoryMap(memory_ranges_path))
	{
		return false;
	}

	printf("Memory ranges dumped\n");
	printf("Memory ranges path: %s\n", memory_ranges_path.c_str());

	LPCSTR args[4] = { 0 };
	int argc = 0;

	args[argc++] = const_cast<LPCSTR>("-device");
	args[argc++] = const_cast<LPCSTR>("fpga://algo=0");
	args[argc++] = const_cast<LPCSTR>("-memmap");
	args[argc++] = memory_ranges_path.c_str();

	_hDevice = VMMDLL_Initialize(argc, args);

	if (!_hDevice)
	{
		printf("Failed to initialize VMM\n");
		return false;
	}

	#define VMMPROC_UPDATERTHREAD_LOCAL_PERIOD              3
	#define VMMPROC_UPDATERTHREAD_LOCAL_MEM                 3        // 0.3s
	#define VMMPROC_UPDATERTHREAD_LOCAL_TLB                 20       // 2s
	#define VMMPROC_UPDATERTHREAD_LOCAL_PROC_REFRESHLIST    3600     // 1h
	#define VMMPROC_UPDATERTHREAD_LOCAL_PROC_REFRESHTOTAL   3600     // 1h

	VMMDLL_ConfigSet(_hDevice, VMMDLL_OPT_CONFIG_TICK_PERIOD, VMMPROC_UPDATERTHREAD_LOCAL_PERIOD);
	VMMDLL_ConfigSet(_hDevice, VMMDLL_OPT_CONFIG_READCACHE_TICKS, VMMPROC_UPDATERTHREAD_LOCAL_MEM);
	VMMDLL_ConfigSet(_hDevice, VMMDLL_OPT_CONFIG_TLBCACHE_TICKS, VMMPROC_UPDATERTHREAD_LOCAL_TLB);
	VMMDLL_ConfigSet(_hDevice, VMMDLL_OPT_CONFIG_PROCCACHE_TICKS_PARTIAL, VMMPROC_UPDATERTHREAD_LOCAL_PROC_REFRESHLIST);
	VMMDLL_ConfigSet(_hDevice, VMMDLL_OPT_CONFIG_PROCCACHE_TICKS_TOTAL, VMMPROC_UPDATERTHREAD_LOCAL_PROC_REFRESHTOTAL);

	printf("VMM initialized\n");

	if (!SetFPGA())
	{
		printf("Failed to set FPGA\n");
		VMMDLL_Close(_hDevice);
		return false;
	}

	printf("FPGA set\n");

	_process_name = process_name;
	_pid = GetPidFromName(_process_name);
	if (!_pid)
	{
		printf("Failed to get PID\n");
		VMMDLL_Close(_hDevice);
		return false;
	}

	printf("PID: %d\n", _pid);

	if (!FixCR3())
	{
		printf("Failed to fix CR3\n");
		VMMDLL_Close(_hDevice);
		return false;
	}

	_base_address = GetModuleBaseAddress(_process_name);
	if (!_base_address)
	{
		printf("Failed to get base address\n");
		VMMDLL_Close(_hDevice);
		return false;
	}
	return true;
}

VMMDLL_SCATTER_HANDLE Dma::CreateScatterHandle() const
{
	const VMMDLL_SCATTER_HANDLE ScatterHandle = VMMDLL_Scatter_Initialize(_hDevice, _pid, VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOCACHEPUT | VMMDLL_FLAG_ZEROPAD_ON_FAIL | VMMDLL_FLAG_NOPAGING_IO | VMMDLL_FLAG_NOPAGING);
	return ScatterHandle;
}

void Dma::CloseScatterHandle(VMMDLL_SCATTER_HANDLE handle) const
{
	if (handle)
		VMMDLL_Scatter_CloseHandle(handle);
	else
		throw std::runtime_error("Invalid scatter handle");
}

bool Dma::AddScatterRead(VMMDLL_SCATTER_HANDLE handle, uint64_t address, void* buffer, size_t size) const
{
	if (!handle)
	{
		throw std::runtime_error("Invalid scatter handle");
	}

	return VMMDLL_Scatter_PrepareEx(handle, address, static_cast<DWORD>(size), static_cast<PBYTE>(buffer), NULL);
}

bool Dma::ExecuteScatterRead(VMMDLL_SCATTER_HANDLE handle) const
{
	if (!handle)
	{
		throw std::runtime_error("Invalid scatter handle");
	}

	__try
	{
		if (!VMMDLL_Scatter_Execute(handle))
		{
			return false;
		}
		if (!VMMDLL_Scatter_Clear(handle, _pid, VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOCACHEPUT | VMMDLL_FLAG_ZEROPAD_ON_FAIL | VMMDLL_FLAG_NOPAGING_IO | VMMDLL_FLAG_NOPAGING))
		{
			return false;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
	return true;
}

bool Dma::DumpMemoryMap(std::string& path)
{
	LPCSTR args[2] = { 0 };
	int argc = 0;

	args[argc++] = const_cast<LPCSTR>("-device");
	args[argc++] = const_cast<LPCSTR>("fpga://algo=0");

	VMM_HANDLE handle = VMMDLL_Initialize(argc, args);
	if (!handle)
	{
		return false;
	}

	PVMMDLL_MAP_PHYSMEM pPhysMemMap = NULL;
	if (!VMMDLL_Map_GetPhysMem(handle, &pPhysMemMap))
	{
		VMMDLL_Close(handle);
		return false;
	}

	if (pPhysMemMap->dwVersion != VMMDLL_MAP_PHYSMEM_VERSION)
	{
		VMMDLL_MemFree(pPhysMemMap);
		VMMDLL_Close(handle);
		return false;
	}

	if (pPhysMemMap->cMap == 0)
	{
		VMMDLL_MemFree(pPhysMemMap);
		VMMDLL_Close(handle);
		return false;
	}

	std::stringstream sb;
	for (DWORD i = 0; i < pPhysMemMap->cMap; i++)
	{
		sb << std::hex << pPhysMemMap->pMap[i].pa << " " << (pPhysMemMap->pMap[i].pa + pPhysMemMap->pMap[i].cb - 1) << std::endl;
	}

	std::filesystem::path temp_path = std::filesystem::temp_directory_path();
	std::ofstream file(temp_path.string() + "\\memory_ranges.dmp");
	file << sb.str();
	file.close();

	VMMDLL_MemFree(pPhysMemMap);
	VMMDLL_Close(handle);

	path = temp_path.string() + "\\memory_ranges.dmp";

	return true;
}

bool Dma::SetFPGA() const
{
	ULONG64 qwID = 0, qwVersionMajor = 0, qwVersionMinor = 0;
	if (!VMMDLL_ConfigGet(_hDevice, LC_OPT_FPGA_FPGA_ID, &qwID) && VMMDLL_ConfigGet(_hDevice, LC_OPT_FPGA_VERSION_MAJOR, &qwVersionMajor) && VMMDLL_ConfigGet(_hDevice, LC_OPT_FPGA_VERSION_MINOR, &qwVersionMinor))
	{
		return false;
	}

	if ((qwVersionMajor >= 4) && ((qwVersionMajor >= 5) || (qwVersionMinor >= 7)))
	{
		HANDLE handle;
		LC_CONFIG config = { .dwVersion = LC_CONFIG_VERSION, .szDevice = "existing" };
		handle = LcCreate(&config);
		if (!handle)
		{
			return false;
		}

		unsigned char abort2[4] = { 0x10, 0x00, 0x10, 0x00 };

		LcCommand(handle, LC_CMD_FPGA_CFGREGPCIE_MARKWR | 0x002, 4, reinterpret_cast<PBYTE>(&abort2), NULL, NULL);
		LcClose(handle);
	}

	return true;
}

uint32_t Dma::GetPidFromName(const std::string& process_name) const
{
	DWORD pid = 0;
	VMMDLL_PidGetFromName(_hDevice, (LPSTR)process_name.c_str(), &pid);
	return pid;
}

uint64_t Dma::GetModuleBaseAddress(const std::string& module_name) const
{
	std::wstring str(module_name.begin(), module_name.end());

	PVMMDLL_MAP_MODULEENTRY module_info;
	if (!VMMDLL_Map_GetModuleFromNameW(_hDevice, _pid, const_cast<LPWSTR>(str.c_str()), &module_info, VMMDLL_MODULE_FLAG_NORMAL))
	{
		printf("Failed to get module info\n");
		return 0;
	}

	return module_info->vaBase;
}

void Dma::PrintAllModules() const
{
}

bool Dma::Read(void* address, void* buffer, size_t size) const
{
	DWORD read_size = 0;
	__try
	{
		if (!VMMDLL_MemReadEx(_hDevice, _pid, (uint64_t)address, static_cast<PBYTE>(buffer), static_cast<DWORD>(size), &read_size, VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOCACHEPUT | VMMDLL_FLAG_ZEROPAD_ON_FAIL | VMMDLL_FLAG_NOPAGING_IO | VMMDLL_FLAG_NOPAGING))
		{
			return false;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}

	return (read_size == size);
}

bool Dma::Write(uint64_t address, void* buffer, size_t size) const
{
	if (!VMMDLL_MemWrite(_hDevice, _pid, address, static_cast<PBYTE>(buffer), static_cast<DWORD>(size)))
	{
		return false;
	}
	return true;
}

uint64_t cbSize = 0x80000;
VOID cbAddFile(_Inout_ HANDLE h, _In_ LPCSTR uszName, _In_ ULONG64 cb, _In_opt_ PVMMDLL_VFS_FILELIST_EXINFO pExInfo)
{
	if (strcmp(uszName, "dtb.txt") == 0)
		cbSize = cb;
}

bool Dma::FixCR3() const
{
	PVMMDLL_MAP_MODULEENTRY module_entry = NULL;
	bool result = VMMDLL_Map_GetModuleFromNameU(_hDevice, _pid, const_cast<LPSTR>(_process_name.c_str()), &module_entry, NULL);
	if (result)
		return true;

	if (!VMMDLL_InitializePlugins(_hDevice))
	{
		return false;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	while (true)
	{
		BYTE bytes[4] = { 0 };
		DWORD i = 0;
		auto nt = VMMDLL_VfsReadW(_hDevice, const_cast<LPWSTR>(L"\\misc\\procinfo\\progress_percent.txt"), bytes, 3, &i, 0);
		if (nt == VMMDLL_STATUS_SUCCESS && atoi(reinterpret_cast<LPSTR>(bytes)) == 100)
			break;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	VMMDLL_VFS_FILELIST2 VfsFileList;
	VfsFileList.dwVersion = VMMDLL_VFS_FILELIST_VERSION;
	VfsFileList.h = 0;
	VfsFileList.pfnAddDirectory = 0;
	VfsFileList.pfnAddFile = cbAddFile;

	result = VMMDLL_VfsListU(_hDevice, const_cast<LPSTR>("\\misc\\procinfo\\"), &VfsFileList);
	if (!result)
		return false;

	const size_t buffer_size = cbSize;
	std::unique_ptr<BYTE[]> bytes(new BYTE[buffer_size]);
	DWORD j = 0;
	auto nt = VMMDLL_VfsReadW(_hDevice, const_cast<LPWSTR>(L"\\misc\\procinfo\\dtb.txt"), bytes.get(), static_cast<DWORD>(buffer_size - 1), &j, 0);
	if (nt != VMMDLL_STATUS_SUCCESS)
		return false;

	std::vector<uint64_t> possible_dtbs = { };
	std::string lines(reinterpret_cast<char*>(bytes.get()));
	std::istringstream iss(lines);
	std::string line = "";
	struct Info
	{
		uint32_t index;
		uint32_t process_id;
		uint64_t dtb;
		uint64_t kernelAddr;
		std::string name;
	};
	while (std::getline(iss, line))
	{
		Info info = { };

		std::istringstream info_ss(line);
		if (info_ss >> std::hex >> info.index >> std::dec >> info.process_id >> std::hex >> info.dtb >> info.kernelAddr >> info.name)
		{
			if (info.process_id == 0)
				possible_dtbs.push_back(info.dtb);

			if (_process_name.find(info.name) != std::string::npos)
				possible_dtbs.push_back(info.dtb);
		}
	}

	for (size_t i = 0; i < possible_dtbs.size(); i++)
	{
		auto dtb = possible_dtbs[i];
		VMMDLL_ConfigSet(_hDevice, VMMDLL_OPT_PROCESS_DTB | _pid, dtb);
		result = VMMDLL_Map_GetModuleFromNameU(_hDevice, _pid, const_cast<LPSTR>(_process_name.c_str()), &module_entry, NULL);
		if (result)
		{
			static ULONG64 pml4[512];
			DWORD readsize;
			VMMDLL_MemReadEx(_hDevice, -1, dtb, reinterpret_cast<PBYTE>(pml4), sizeof(pml4), (PDWORD)&readsize, VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOPAGING | VMMDLL_FLAG_ZEROPAD_ON_FAIL | VMMDLL_FLAG_NOPAGING_IO);
			VMMDLL_MemReadEx((VMM_HANDLE)-666, 333, (ULONG64)pml4, 0, 0, 0, 0);
			VMMDLL_ConfigSet(_hDevice, VMMDLL_OPT_PROCESS_DTB | _pid, 666);
			return true;
		}
	}

	return false;
}
