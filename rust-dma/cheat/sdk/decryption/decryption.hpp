#pragma once
#include "../../../include.hpp"

namespace decryption
{
#pragma warning(disable: 4319)
#pragma warning(disable: 4244)
#pragma optimize("", off)

#define TEST_BITD(x, y) ((x) & (1 << (y)))
#define HANDLE_BASE 0xE1A5BF0
#define LODWORD(x) (*((unsigned long*)&(x)))

	// __int64 __fastcall il2cpp_gchandle_get_target_0(unsigned int a1)
	inline ULONG64 IL2CppGetHandle(uintptr_t base, int32_t ObjectHandleID) {

		uint64_t rdi_1 = ObjectHandleID >> 3;
		uint64_t rcx_1 = (ObjectHandleID & 7) - 1;
		uint64_t baseAddr = base + HANDLE_BASE + rcx_1 * 0x28;
		uint32_t limit = dma.Read<uint32_t>(baseAddr + 0x10);
		if (rdi_1 < limit) {
			uintptr_t objAddr = dma.Read<uintptr_t>(baseAddr);
			uint32_t bitMask = dma.Read<uint32_t>(objAddr + ((rdi_1 >> 5) << 2));
			if (TEST_BITD(bitMask, rdi_1 & 0x1f)) {
				uintptr_t ObjectArray = dma.Read<uintptr_t>(baseAddr + 0x8) + (rdi_1 << 3);
				return dma.Read<BYTE>(baseAddr + 0x14) > 1
					? dma.Read<uintptr_t>(ObjectArray)
					: ~dma.Read<uint32_t>(ObjectArray);
			}
		}

		printf("IL2CppGetHandle: Failed to decrypt address\n");
		return 0;
	}

	inline ULONG64 BaseNetworkable(uintptr_t base, ULONG64 Address)
	{
		int* v17; // rdx
		int v18; // r8d
		int v19; // eax
		__m128 v36; // [rsp+68h] [rbp+20h] OVERLAPPED BYREF

		v17 = (int*)&v36;
		v18 = 2;
		v36 = dma.Read<__m128>(Address + 24);
		do
		{
			v19 = *v17++;
			*(v17 - 1) = ((((v19 + 2088045315) << 20) | ((unsigned int)(v19 + 2088045315) >> 12)) + 62779575) ^ 0xEA3B039;
			--v18;
		} while (v18);
		signed __int64 v9 = *reinterpret_cast<signed __int64*>(&v36);
		return IL2CppGetHandle(base, v9);
	}

#pragma optimize("", on)
#pragma warning(default: 4319)
#pragma warning(default: 4244)
}