#pragma once
#include "../../../include.hpp"

namespace decryption
{
#pragma warning(disable: 4319)
#pragma optimize("", off)

#define TEST_BITD(x, y) ((x) & (1 << (y)))
#define HANDLE_BASE 0xDD06B20

	// __int64 __fastcall il2cpp_gchandle_get_target_0(unsigned int a1)
	inline ULONG64 IL2CppGetHandle(uintptr_t base, int32_t ObjectHandleID)
	{
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
		__m128* v6; // rdx
		int v7; // r8d
		int v8; // ecx
		__m128 v24; // [rsp+20h] [rbp-28h] BYREF
		v6 = &v24;
		v7 = 2;
		v24 = dma.Read<__m128>(Address + 0x18);
		do
		{
			v8 = *(DWORD*)v6;
			v6 = (__m128*)((char*)v6 + 4);
			*((DWORD*)v6 - 1) = ((((v8 << 26) | (v8 >> 6)) - 886443769) << 18) | ((((v8 << 26) | (v8 >> 6)) - 886443769) >> 14);
			--v7;
		} while (v7);
		signed __int64 v9 = *reinterpret_cast<signed __int64*>(&v24);

		if (!v9)
		{
			printf("BaseNetworkable: Failed to decrypt address\n");
		}

		return IL2CppGetHandle(base, static_cast<int32_t>(v9));
	}

#pragma optimize("", on)
#pragma warning(default: 4319)
}