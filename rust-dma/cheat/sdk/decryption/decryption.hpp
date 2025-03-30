#pragma once
#include "../../../include.hpp"

#pragma warning(disable: 4319)
#pragma warning(disable: 4244)
#pragma warning(disable: 4005)
#pragma warning(disable: 4101)
#pragma optimize("", off)

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

#define TEST_BITD(x, y) ((x) & (1 << (y)))

namespace decryption
{
	// __int64 __fastcall il2cpp_gchandle_get_target_0(unsigned int a1)
	inline ULONG64 Il2cppGetHandle(uintptr_t base, int32_t ObjectHandleID) {

		uint64_t rdi_1 = ObjectHandleID >> 3;
		uint64_t rcx_1 = (ObjectHandleID & 7) - 1;
		uint64_t baseAddr = base + 0xBF9ABA0 + rcx_1 * 0x28;
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

    inline uint64_t networkable_key(uintptr_t base, uint64_t pointer) {
        std::uint64_t* v4; // rdx
        std::uint64_t* a2; // rdx
        int v5; // r8d
        int v6; // eax
        std::uint64_t v22; // [rsp+20h] [rbp-28h] BYREF
        std::uint64_t v23; // [rsp+20h] [rbp-28h] BYREF
        std::uint64_t a3; // [rsp+20h] [rbp-28h] BYREF
        char v7;
        a2 = &v23;
        v23 = dma.Read<uintptr_t>(pointer + 24);
        LODWORD(a3) = 2;
        do
        {
            v5 = *(_DWORD*)a2;
            v6 = *(_DWORD*)a2;
            a2 = (std::uint64_t*)((char*)a2 + 4);
            *((_DWORD*)a2 - 1) = ((((v6 << 11) | (v5 >> 21)) + 1490921071) << 17) | ((((v6 << 11) | (v5 >> 21)) + 1490921071) >> 15);
            v7 = (_DWORD)a3 == 1;
            a3 = (unsigned int)(a3 - 1);
        } while (!v7);

        const auto handle = Il2cppGetHandle(base, static_cast<std::int32_t>(v23));
        if (!handle)
            return 0;

        return handle;
    }


    inline uint64_t networkable_key2(uintptr_t base, uint64_t pointer) {
        std::uint64_t* v4; // rdx
        std::uint64_t* a2; // rdx
        int v5; // r8d
        int v6; // eax
        std::uint64_t v22; // [rsp+20h] [rbp-28h] BYREF
        std::uint64_t v23; // [rsp+20h] [rbp-28h] BYREF
        std::uint64_t a3; // [rsp+20h] [rbp-28h] BYREF
        char v7;
        a2 = &v23;
        v23 = dma.Read<uintptr_t>(pointer + 24);
        LODWORD(a3) = 2;
        do
        {
            v6 = *(_DWORD*)a2;
            a2 = (std::uint64_t*)((char*)a2 + 4);
            *((_DWORD*)a2 - 1) = (((v6 - 232845115) << 13) | ((unsigned int)(v6 - 232845115) >> 19)) - 1716329210;
            v7 = (_DWORD)a3 == 1;
            a3 = (unsigned int)(a3 - 1);
        } while (!v7);

        const auto handle = Il2cppGetHandle(base, static_cast<std::int32_t>(v23));
        if (!handle)
            return 0;

        return handle;

    }
}

#pragma optimize("", on)
#pragma warning(default: 4101)
#pragma warning(default: 4319)
#pragma warning(default: 4244)
#pragma warning(default: 4005)