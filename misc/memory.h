#pragma once
#pragma warning(disable: C4430)

#include <cstdint>
#include <iostream>
#include <vectors/vector_3d.h>


#define RVA(addr, size) ((uintptr_t)((UINT_PTR)(addr) + *(PINT)((UINT_PTR)(addr) + ((size) - sizeof(INT))) + (size)))

namespace fortnite_utilities
{
	uintptr_t PatternScanEx(uintptr_t pModuleBaseAddress, const char* sSignature, size_t nSelectResultIndex = 0)
	{
		static auto patternToByte = [](const char* pattern)
		{
			auto       bytes = std::vector<int>{};
			const auto start = const_cast<char*>(pattern);
			const auto end = const_cast<char*>(pattern) + strlen(pattern);

			for (auto current = start; current < end; ++current)
			{
				if (*current == '?')
				{
					++current;
					if (*current == '?')
						++current;
					bytes.push_back(-1);
				}
				else
					bytes.push_back(strtoul((const char*)current, &current, 16));
			}
			return bytes;
		};

		const auto dosHeader = (PIMAGE_DOS_HEADER)pModuleBaseAddress;
		const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)pModuleBaseAddress + dosHeader->e_lfanew);

		const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
		auto       patternBytes = patternToByte(sSignature);
		const auto scanBytes = reinterpret_cast<std::uint8_t*>(pModuleBaseAddress);

		const auto s = patternBytes.size();
		const auto d = patternBytes.data();

		size_t nFoundResults = 0;

		for (auto i = 0ul; i < sizeOfImage - s; ++i)
		{
			bool found = true;

			for (auto j = 0ul; j < s; ++j)
			{
				if (scanBytes[i + j] != d[j] && d[j] != -1)
				{
					found = false;
					break;
				}
			}

			if (found)
			{
				if (nSelectResultIndex != 0)
				{
					if (nFoundResults < nSelectResultIndex)
					{
						nFoundResults++;
						found = false;
					}
					else
						return reinterpret_cast<uintptr_t>(&scanBytes[i]);
				}
				else
					return reinterpret_cast<uintptr_t>(&scanBytes[i]);
			}
		}

		return NULL;
	}

	uintptr_t GetCurrentImageBase()
	{
		return *(uintptr_t*)(__readgsqword(0x60) + 0x10);
	}

	uintptr_t ResolveRelativeAddress(uintptr_t Address, int InstructionLength)
	{
		DWORD Offset = *(DWORD*)(Address + (InstructionLength - 4));
		return Address + InstructionLength + Offset;
	}

	uintptr_t PatternScan(const char* sSignature, size_t nSelectResultIndex = 0, int InstructionLength = 0)
	{
		auto ret = PatternScanEx((uintptr_t)SAFE_CALL(GetModuleHandleA)(0), sSignature, nSelectResultIndex);

		if (InstructionLength != 0)
			ret = ResolveRelativeAddress(ret, InstructionLength);

		//std::cout << "Function found of " << sSignature << " at " << "0x" << std::hex << ret - (uintptr_t)GetModuleHandleA(0) << std::endl;
		if (sSignature == "48 89 05 ? ? ? ? 48 8B 4B 78")																		{ std::cout << "[+] Uworld: 0x" << std::hex << ret - (uintptr_t)GetModuleHandleA(0) << std::endl; }
		if (sSignature == "40 53 55 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 33 DB 45 8A F1") { std::cout << "[+] WorldToScreen: 0x" << std::hex << ret - (uintptr_t)GetModuleHandleA(0) << std::endl; }
		if (sSignature == "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 33 C0 49 8B F8 89 02 48 8B F2 41 89 00 48 8B 99")			{ std::cout << "[+] ScreenSize: 0x" << std::hex << ret - (uintptr_t)GetModuleHandleA(0) << std::endl; }
		if (sSignature == "40 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 4C 89 45 DF F3 0F 10 4D")										{ std::cout << "[+] K2_DrawLine: 0x" << std::hex << ret - (uintptr_t)GetModuleHandleA(0) << std::endl; }
		if (sSignature == "48 81 EC A8 00 00 00 F2 41 0F 10 08")										{ std::cout << "[+] K2_DrawLine: 0x" << std::hex << ret - (uintptr_t)GetModuleHandleA(0) << std::endl; }
		if (sSignature == "48 85 C9 0F 84 ? ? ? ? 53 48 83 EC 20 48 89 7C 24 30 48 8B D9 48 8B 3D ? ? ? ? 48 85 FF 0F 84 ? ? ? ? 48 8B 07 4C 8B 40 30 48 8D 05 ? ? ? ? 4C 3B C0")		{ std::cout << "[+] FreeFN: 0x" << std::hex << ret - (uintptr_t)GetModuleHandleA(0) << std::endl; }
		if (sSignature == "48 89 74 24 ? 48 89 7C 24 ? 41 56 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 8B 01 48 8B F2 8B F8")										{ std::cout << "[+] GetNameByIndex: 0x" << std::hex << ret - (uintptr_t)GetModuleHandleA(0) << std::endl; }
		if (sSignature == "48 8B 05 ? ? ? ? 48 8B 0C C8 48 8B 04 D1")										{ std::cout << "[+] UObject: 0x" << std::hex << ret - (uintptr_t)GetModuleHandleA(0) << std::endl; }
		
		if (!ret)
		{
			if (sSignature == "48 89 05 ? ? ? ? 48 8B 4B 78")																		{ std::cout << "[-] UWorld signature not found." << std::endl; for (;;); }
			if (sSignature == "40 53 55 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 33 DB 45 8A F1") { std::cout << "[-] WorldToScreen signature not found." << std::endl; for (;;); }
			if (sSignature == "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 33 C0 49 8B F8 89 02 48 8B F2 41 89 00 48 8B 99")			{ std::cout << "[-] ScreenSize signature not found." << std::endl; for (;;); }
			if (sSignature == "40 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 4C 89 45 DF F3 0F 10 4D")										{ std::cout << "[-] K2_DrawLine signature not found." << std::endl; for (;;); }
			if (sSignature == "48 85 C9 0F 84 ? ? ? ? 53 48 83 EC 20 48 89 7C 24 30 48 8B D9 48 8B 3D ? ? ? ? 48 85 FF 0F 84 ? ? ? ? 48 8B 07 4C 8B 40 30 48 8D 05 ? ? ? ? 4C 3B C0")	{ std::cout << "[-] FreeFN signature not found." << std::endl; for (;;); }
			if (sSignature == "48 89 74 24 ? 48 89 7C 24 ? 41 56 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 8B 01 48 8B F2 8B F8")	{ std::cout << "[-] GetNameByIndex signature not found." << std::endl; for (;;); }
			if (sSignature == "48 8B 05 ? ? ? ? 48 8B 0C C8 48 8B 04 D1")	{ std::cout << "[-] UObject signature not found." << std::endl; for (;;); }
			if (sSignature == "48 81 EC A8 00 00 00 F2 41 0F 10 08")	{ std::cout << "[-] K2_DrawLine signature not found." << std::endl; for (;;); }
			else { std::cout << "Unknown signature not found." << sSignature << std::endl; for (;;); }
		}
		
		return ret;
	}
}

namespace functions
{
	inline Structs::UObject* Font = 0;
	static PVOID FOV;
	inline Structs::UObject* K2_DrawLine = 0;
	inline Structs::UObject* K2_DrawBox = 0;
	inline Structs::UObject* K2_DrawText = 0;
	inline Structs::UObject* K2_DrawTextSize = 0;
}

namespace signatures
{
	/* FORTNITE BASICS */
	uintptr_t UWorld;
	uintptr_t WorldToScreen;
	uintptr_t GetBoneMatrix;
	uintptr_t GetNameByIndex;
	uintptr_t FreeFN;

	/* RENDERING */
	uintptr_t ScreenSize;
	uintptr_t K2_DrawLine;
}

auto Load_Signatures()->void
{
	signatures::UWorld = fortnite_utilities::PatternScan(("48 89 05 ? ? ? ? 48 8B 4B 78"), 0, 7);
	signatures::WorldToScreen = fortnite_utilities::PatternScan("40 53 55 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 33 DB 45 8A F1");
	signatures::GetBoneMatrix = fortnite_utilities::PatternScan(("E8 ? ? ? ? 0F 10 40 68"), 0, 5);
	signatures::FreeFN = fortnite_utilities::PatternScan("48 85 C9 0F 84 ? ? ? ? 53 48 83 EC 20 48 89 7C 24 30 48 8B D9 48 8B 3D ? ? ? ? 48 85 FF 0F 84 ? ? ? ? 48 8B 07 4C 8B 40 30 48 8D 05 ? ? ? ? 4C 3B C0");
	signatures::GetNameByIndex = fortnite_utilities::PatternScan("48 89 74 24 ? 48 89 7C 24 ? 41 56 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 8B 01 48 8B F2 8B F8");

	Structs::objects = (Structs::GObjects*)fortnite_utilities::PatternScan(("48 8B 05 ? ? ? ? 48 8B 0C C8 48 8B 04 D1"), 0, 7);

	signatures::K2_DrawLine = fortnite_utilities::PatternScan("48 81 EC A8 00 00 00 F2 41 0F 10 08");
}

typedef bool(__thiscall* this_ProjectWorldLocationToScreens)(uintptr_t playerController, vec3 pos, vec2* screen, bool bPlayerViewportRelative);
this_ProjectWorldLocationToScreens m_ProjectWorldLocationToScreens;

typedef void(__thiscall* this_GetViewPortSize)(uintptr_t playerController, int32_t& ScreenWidth, int32_t& ScreenHeight);
this_GetViewPortSize m_GetViewPortSize;

bool worldtoscreen(uintptr_t playerController, vec3 pos, vec2* screen, bool bPlayerViewportRelative)
{
	this_ProjectWorldLocationToScreens m_projworld = (this_ProjectWorldLocationToScreens)(DWORD64)signatures::WorldToScreen;
	return m_projworld(playerController, pos, screen, bPlayerViewportRelative);
}

void screensize(uintptr_t playerController, int32_t& ScreenWidth, int32_t& ScreenHeight)
{
	this_GetViewPortSize m_getviewportsize = (this_GetViewPortSize)((DWORD64)signatures::ScreenSize);
	return m_getviewportsize(playerController, ScreenWidth, ScreenHeight);
}

template <typename T> T read(const uintptr_t address)
{
	T Novalue = {};
	if (!IsBadReadPtr((const void*)address, sizeof(T)))
	{
		return *(T*)(address);
	}
	else
	{
		return Novalue;
	}
}