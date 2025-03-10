#pragma once
#include "../../../include.hpp"

typedef          char   int8;
typedef   signed char   sint8;
typedef unsigned char   uint8;
typedef          short  int16;
typedef   signed short  sint16;
typedef unsigned short  uint16;
typedef          int    int32;
typedef   signed int    sint32;
typedef unsigned int    uint32;

#define _BYTE  uint8
#define _WORD  uint16
#define _DWORD uint32
#define _QWORD uint64

#define LOBYTE(x)   (*((_BYTE*)&(x)))   // low byte
#define LOWORD(x)   (*((_WORD*)&(x)))   // low word
#define LODWORD(x)  (*((_DWORD*)&(x)))  // low dword
#define HIBYTE(x)   (*((_BYTE*)&(x)+1))
#define HIWORD(x)   (*((_WORD*)&(x)+1))
#define HIDWORD(x)  (*((_DWORD*)&(x)+1))
#define BYTEn(x, n)   (*((_BYTE*)&(x)+n))
#define WORDn(x, n)   (*((_WORD*)&(x)+n))


namespace decryption
{
#pragma warning(disable: 4319)
#pragma warning(disable: 4244)
#pragma optimize("", off)

#define TEST_BITD(x, y) ((x) & (1 << (y)))

	// __int64 __fastcall il2cpp_gchandle_get_target_0(unsigned int a1)
	inline ULONG64 IL2CppGetHandle(uintptr_t base, int32_t ObjectHandleID) {

		uint64_t rdi_1 = ObjectHandleID >> 3;
		uint64_t rcx_1 = (ObjectHandleID & 7) - 1;
		uint64_t baseAddr = base + 0xBEB5B60 + rcx_1 * 0x28;
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
		return 0;
	}

	inline ULONG64 BaseNetworkable(uintptr_t base, ULONG64 Address)
	{
		__int64* v4; // rdx
		int v5; // r8d
		unsigned int v6; // ecx
		unsigned int v7; // eax
		__int64 v8; // kr00_8
		__int64 v9; // rcx
		__int64 v26; // [rsp+20h] [rbp-28h] BYREF

		v4 = &v26;
		v26 = dma.Read<uintptr_t>(Address + 0x18);
		v5 = 2;
		do
		{
			v6 = *(DWORD*)v4;
			v7 = *(DWORD*)v4;
			v4 = (__int64*)((char*)v4 + 4);
			v8 = 8i64 * ((((v6 >> 3) | (v7 << 29)) ^ 0x1BD6DA3F) + 812328851);
			*((DWORD*)v4 - 1) = v8 | HIDWORD(v8);
			--v5;
		} while (v5);
		v9 = v26;

		return IL2CppGetHandle(base, v26);
	}

	inline uint64_t DecryptList(uintptr_t base, uint64_t pointer)
	{
		__int64* v4; // rdx
		int v5; // r8d
		unsigned int v6; // ecx
		unsigned int v7; // eax
		__int64 v8; // rcx
		__int64 v23; // [rsp+20h] [rbp-28h] BYREF

		v4 = &v23;
		v23 = dma.Read<uintptr_t>(pointer + 24);
		v5 = 2;
		do
		{
			v6 = *(_DWORD*)v4;
			v7 = *(_DWORD*)v4;
			v4 = (__int64*)((char*)v4 + 4);
			*((_DWORD*)v4 - 1) = (((v7 << 23) | (v6 >> 9)) - 2051686922) ^ 0x51B6E58C;
			--v5;
		} while (v5);
		v8 = v23;

		return IL2CppGetHandle(base, v23);
	}

#pragma optimize("", on)
#pragma warning(default: 4319)
#pragma warning(default: 4244)
}