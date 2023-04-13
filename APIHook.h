#pragma once
#include "sbcommon.h"
#include "Sazabi.h"
#pragma warning(push, 0)
#include <codeanalysis/warnings.h>
#pragma warning(disable \
		: ALL_CODE_ANALYSIS_WARNINGS)
#include "minhook\MinHook.h"
#pragma warning(pop)

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
