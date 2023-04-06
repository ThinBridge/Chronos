#pragma once
#include "sbcommon.h"
#include "Sazabi.h"
#include "minhook\MinHook.h"

template <typename T>
inline MH_STATUS MH_CreateHookApiEx(
    LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, T** ppOriginal)
{
	return MH_CreateHookApi(
	    pszModule, pszProcName, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}

#define RVA2PTR(base, rva) (((PBYTE)base) + rva)

class APIHookC //:public CObject
{
public:
	APIHookC();
	virtual ~APIHookC();
	BOOL m_bInitFlg;
	void DoHookComDlgAPI();
};
