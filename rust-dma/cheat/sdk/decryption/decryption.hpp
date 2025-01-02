#pragma once
#include "../../../include.hpp"

namespace decryption
{
#pragma warning(disable: 4319)
#pragma optimize("", off)

#define TEST_BITD(x, y) ((x) & (1 << (y)))
#define HANDLE_BASE 0xDD06B20
#define LODWORD(x) (*((unsigned long*)&(x)))

	// __int64 __fastcall il2cpp_gchandle_get_target_0(unsigned int a1)
	inline ULONG64 Il2cppGetHandle(uintptr_t base, __int64 ObjectHandleID)
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
		__m128 a1 = dma.Read<__m128>(Address + 0x18);
		unsigned int v5; // ecx
		unsigned int v6; // eax
		bool v7; // zf
		__m128 v24; // [rsp+20h] [rbp-28h] BYREF
		__m128* a2;
		__int64 a3;

		a2 = &v24;
		LODWORD(a3) = 2;
		v24 = a1;
		do
		{
			v5 = *(DWORD*)a2;
			v6 = *(DWORD*)a2;
			a2 = (__m128*)((char*)a2 + 4);
			*((DWORD*)a2 - 1) = ((((v6 << 26) | (v5 >> 6)) - 886443769) << 18) | ((((v6 << 26) | (v5 >> 6)) - 886443769) >> 14);
			v7 = (DWORD)a3 == 1;
			a3 = (unsigned int)(a3 - 1);
		} while (!v7);
		return Il2cppGetHandle(base, *reinterpret_cast<signed __int64*>(&v24));
	}

#pragma optimize("", on)
#pragma warning(default: 4319)
}