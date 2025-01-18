#pragma once
#include "../../../include.hpp"

namespace decryption
{
#pragma warning(disable: 4319)
#pragma warning(disable: 4244)
#pragma optimize("", off)

#define TEST_BITD(x, y) ((x) & (1 << (y)))
#define HANDLE_BASE 0xE209F60
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
		int* v18; // rdx
		int v5; // r8d
		__m128 v25; // [rsp+20h] [rbp-28h] BYREF
		int v6; // eax

		v18 = (int*)&v25;
		v5 = 2;
		v25 = dma.Read<__m128>(Address + 0x18);
		do
		{
			v6 = *v18++;
			*(v18 - 1) = (((v6 + 1968032699) << 12) | ((unsigned int)(v6 + 1968032699) >> 20)) - 1455555583;
			--v5;
		} while (v5);
		signed __int64 v9 = *reinterpret_cast<signed __int64*>(&v25);
		return IL2CppGetHandle(base, v9);
	}

#pragma optimize("", on)
#pragma warning(default: 4319)
#pragma warning(default: 4244)
}