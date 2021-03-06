#include "stdafx.h"
#include "APIHook.h"
#include "minhook\hook.hh"
#include "Windows.h"

#define API_H_TRY try{
#define API_H_CATCH }catch(...){ATLASSERT(0);}



///////////////////////////////////////////////////////////////////////////////////////////
//@@COM
///////////////////////////////////////////////////////////////////////////////////////////
//TypeDef
typedef HRESULT(WINAPI* ORG_CoCreateInstance)(
	_In_  REFCLSID  rclsid,
	_In_  LPUNKNOWN pUnkOuter,
	_In_  DWORD     dwClsContext,
	_In_  REFIID    riid,
	_Out_ LPVOID    *ppv
	);
static ORG_CoCreateInstance pORG_CoCreateInstance = NULL;

////////////////////////////////////////////////////////////////
//HookFunction
static HRESULT WINAPI Hook_CoCreateInstance(
	_In_  REFCLSID  rclsid,
	_In_  LPUNKNOWN pUnkOuter,
	_In_  DWORD     dwClsContext,
	_In_  REFIID    riid,
	_Out_ LPVOID    *ppv
)
{
	PROC_TIME(Hook_CoCreateInstance)
	API_H_TRY
	if (theApp.IsSGMode())
	{
		if (rclsid == CLSID_FileOpenDialog || rclsid == CLSID_FileSaveDialog)
		{
			::SetLastError(ERROR_ACCESS_DENIED);
			return REGDB_E_CLASSNOTREG;
		}
	}
	API_H_CATCH
	HRESULT hRet = {0};
	hRet = pORG_CoCreateInstance(
		rclsid,
		pUnkOuter,
		dwClsContext,
		riid,
		ppv
	);
	return hRet;
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
//@@ComDlg32
///////////////////////////////////////////////////////////////////////////////////////////
//TypeDef
typedef BOOL(WINAPI* FuncGetSaveFileNameW)(LPOPENFILENAMEW lpofn);
static FuncGetSaveFileNameW	pORG_GetSaveFileNameW = NULL;

typedef BOOL(WINAPI* FuncGetOpenFileNameW)(LPOPENFILENAMEW lpofn);
static FuncGetSaveFileNameW	pORG_GetOpenFileNameW = NULL;

typedef PIDLIST_ABSOLUTE(WINAPI* FuncSHBrowseForFolderW)(LPBROWSEINFOW lpbi);
static FuncSHBrowseForFolderW	pORG_SHBrowseForFolderW = NULL;

////////////////////////////////////////////////////////////////
//HookFunction
static BOOL WINAPI Hook_GetSaveFileNameW(
	LPOPENFILENAMEW lpofn
)
{
	BOOL bRet = FALSE;
	try
	{
		//Download???~
		if (theApp.m_AppSettings.IsEnableDownloadRestriction())
		{
			return FALSE;
		}

		if (!theApp.IsSGMode())
			return pORG_GetSaveFileNameW(lpofn);

		CString strPath;
		strPath = theApp.m_AppSettings.GetRootPath();
		if (strPath.IsEmpty())
			strPath = _T("B:\\");
		strPath = strPath.TrimRight('\\');
		strPath += _T("\\");

		//?t?b?N??????????
		lpofn->Flags &= ~OFN_ENABLEHOOK;
		//?_?C?A???O?e???v???[?g????
		lpofn->Flags &= ~OFN_ENABLETEMPLATE;

		//Long?t?@?C??????????
		lpofn->Flags |= OFN_LONGNAMES;
		//?l?b?g???[?N?{?^?????B??
		lpofn->Flags |= OFN_NONETWORKBUTTON;

		//?????g?????t?@?C??????????????
		lpofn->Flags |= OFN_DONTADDTORECENT;
		//?v???[?X?o?[??????
		lpofn->FlagsEx |= OFN_EX_NOPLACESBAR;

		//?t?@?C???????????????????????m?F?????v?????v?g???\??
		lpofn->Flags |= OFN_OVERWRITEPROMPT;

		CStringW strCaption(theApp.m_strThisAppName);
		CStringW strMsg;
		for (;;)
		{
			WCHAR szSelPath[MAX_PATH + 1] = {0};
			bRet = pORG_GetSaveFileNameW(lpofn);
			if (bRet)
			{
				memset(szSelPath, 0x00, sizeof(WCHAR) * MAX_PATH);
				lstrcpynW(szSelPath, lpofn->lpstrFile, MAX_PATH);
				CStringW strRoot(strPath);
				CStringW strSelPath(szSelPath);
				strRoot.MakeUpper();
				strSelPath.MakeUpper();
				if (strSelPath.IsEmpty()) return bRet;
				if (strSelPath.Find(strRoot) == 0)
				{
					CStringW strTSG_Upload;
					strTSG_Upload = strRoot + L"UPLOAD\\";
					if (strSelPath.Find(strTSG_Upload) == 0)
					{
						strMsg.Format(L"?A?b?v???[?h?t?H???_?[[%s]???????????????????B\n\n?w?????????????????????B\n\n?I????????????[%s]", strTSG_Upload, szSelPath);
						::MessageBoxW(lpofn->hwndOwner, strMsg, strCaption, MB_OK | MB_ICONWARNING);
						continue;
					}
					return bRet;
				}
				else
				{
					strMsg.Format(L"%s?h???C?u???O???w?????????????B\n\n????????????????%s???w?????????????????????B\n\n?I????????????[%s]", strRoot, strRoot, szSelPath);
					::MessageBoxW(lpofn->hwndOwner, strMsg, strCaption, MB_OK | MB_ICONWARNING);
					continue;
				}
			}
			else
				return bRet;
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return bRet;
}

static BOOL WINAPI Hook_GetOpenFileNameW(
	LPOPENFILENAMEW lpofn
)
{
	BOOL bRet = FALSE;
	try
	{
		//?????o?????????m?F?A?e???e??NULL????????Chronos??????????????
		HWND hWindowOwner = GetParent(lpofn->hwndOwner);
		HWND hWindowParent = {0};
		if (hWindowOwner)
			hWindowParent = GetParent(hWindowOwner);
		//hWindowParent??NULL?????????A????????
		if (!hWindowParent)
		{
			bRet = pORG_GetOpenFileNameW(lpofn);
			return bRet;
		}

		//Upload???~
		if (theApp.m_AppSettings.IsEnableUploadRestriction())
		{
			CStringW strCaption(theApp.m_strThisAppName);
			CStringW strMsg;
			strMsg = (L"?t?@?C?? ?A?b?v???[?h???A?V?X?e???????????????????????????????B");
			if (hWindowParent)
				theApp.SB_MessageBox(hWindowParent, strMsg, NULL, MB_OK | MB_ICONWARNING, TRUE);
			else
				::MessageBoxW(lpofn->hwndOwner, strMsg, strCaption, MB_OK | MB_ICONWARNING);
			return FALSE;
		}

		if (theApp.m_AppSettings.IsAdvancedLogMode())
		{
			CString strLogmsg;
			strLogmsg = _T("Hook_GetOpenFileNameW");
			theApp.WriteDebugTraceDateTime(strLogmsg, DEBUG_LOG_TYPE_DE);
		}
		CString strPath;
		if (theApp.IsSGMode())
		{
			//UploadTab???g?????????AB:\\Upload??????
			if (theApp.m_AppSettings.IsShowUploadTab())
			{
				strPath = theApp.m_AppSettings.GetRootPath();
				if (strPath.IsEmpty())
					strPath = _T("B:\\");
				strPath += _T("UpLoad");
				if (!theApp.IsFolderExists(strPath))
					strPath = _T("B:\\");
			}
			//UploadTab???g?????????????AO:\\??????
			else
			{
				strPath = theApp.m_AppSettings.GetUploadBasePath();
				if (strPath.IsEmpty())
					strPath = _T("B:\\");
			}
			//?t?b?N??????????
			lpofn->Flags &= ~OFN_ENABLEHOOK;
			//?_?C?A???O?e???v???[?g????
			lpofn->Flags &= ~OFN_ENABLETEMPLATE;

			//Long?t?@?C??????????
			lpofn->Flags |= OFN_LONGNAMES;
			//?l?b?g???[?N?{?^?????B??
			lpofn->Flags |= OFN_NONETWORKBUTTON;

			//?????g?????t?@?C??????????????
			lpofn->Flags |= OFN_DONTADDTORECENT;
			//?v???[?X?o?[??????
			lpofn->FlagsEx |= OFN_EX_NOPLACESBAR;

			//?t?@?C???????????????????????m?F?????v?????v?g???\??
			lpofn->Flags |= OFN_OVERWRITEPROMPT;
		}
		else
		{
			strPath = SBUtil::GetDownloadFolderPath();
		}
		strPath = strPath.TrimRight('\\');
		strPath += _T("\\");

		if (!theApp.m_strLastSelectUploadFolderPath.IsEmpty())
		{
			if (theApp.IsFolderExists(theApp.m_strLastSelectUploadFolderPath))
			{
				strPath = theApp.m_strLastSelectUploadFolderPath;
			}
		}
		lpofn->lpstrInitialDir = strPath.GetString();
		bRet = pORG_GetOpenFileNameW(lpofn);
		if (bRet)
		{
			WCHAR szSelFolderPath[MAX_PATH + 1] = {0};
			lstrcpynW(szSelFolderPath, lpofn->lpstrFile, MAX_PATH);
			PathRemoveFileSpec(szSelFolderPath);
			theApp.m_strLastSelectUploadFolderPath = szSelFolderPath;

			if (theApp.m_AppSettings.IsEnableLogging() && theApp.m_AppSettings.IsEnableUploadLogging())
			{
				WCHAR szSelPath[MAX_PATH + 1] = {0};
				lstrcpynW(szSelPath, lpofn->lpstrFile, MAX_PATH);
				CString strFileName;
				WCHAR* ptrFile = NULL;
				ptrFile = PathFindFileNameW(szSelPath);
				if (ptrFile)
				{
					strFileName = ptrFile;
				}
				theApp.SendLoggingMsg(LOG_UPLOAD, strFileName, lpofn->hwndOwner);
			}
		}
		return bRet;
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return bRet;
}
static PIDLIST_ABSOLUTE WINAPI Hook_SHBrowseForFolderW(
	LPBROWSEINFOW lpbi
)
{
	CStringW strCaption(theApp.m_strThisAppName);
	CStringW strMsg;
	strMsg.Format(L"?t?H???_?[???Q???@?\?????p???????????B");
	::MessageBoxW(lpbi->hwndOwner, strMsg, strCaption, MB_OK | MB_ICONWARNING);

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////
//@@APIHook////////////////////////////////////////////////////////////////////////////////////
APIHookC::APIHookC()
{
	m_bInitFlg = FALSE;
};

APIHookC::~APIHookC()
{
	if (m_bInitFlg)
	{
		m_bInitFlg = FALSE;
		MH_Uninitialize();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void APIHookC::DoHookComDlgAPI()
{
	PROC_TIME(DoHookComDlgAPI)

	LPVOID pTargetA = {0};
	LPVOID pTargetW = {0};
	HMODULE hModule = {0};

	if (!m_bInitFlg)
	{
		if (MH_Initialize() != MH_OK)
			return;
		m_bInitFlg = TRUE;
	}

	hModule = GetModuleHandleW(L"comdlg32.dll");

	if (!pORG_GetSaveFileNameW)
	{
		if (hModule)
			pTargetW = GetProcAddress(hModule, "GetSaveFileNameW");
		if (MH_CreateHookApiEx(
			L"comdlg32.dll", "GetSaveFileNameW", &Hook_GetSaveFileNameW, &pORG_GetSaveFileNameW) != MH_OK)
			return;

		if (pTargetW == NULL) return;
		if (MH_EnableHook(pTargetW) != MH_OK)
			return;
	}

	if (!pORG_GetOpenFileNameW)
	{
		if (hModule)
			pTargetW = GetProcAddress(hModule, "GetOpenFileNameW");
		if (MH_CreateHookApiEx(
			L"comdlg32.dll", "GetOpenFileNameW", &Hook_GetOpenFileNameW, &pORG_GetOpenFileNameW) != MH_OK)
			return;

		if (pTargetW == NULL) return;
		if (MH_EnableHook(pTargetW) != MH_OK)
			return;
	}

	////////////////////////////////////////////////////////////
	hModule = GetModuleHandleW(L"shell32.dll");
	if (!pORG_SHBrowseForFolderW)
	{
		if (hModule)
			pTargetW = GetProcAddress(hModule, "SHBrowseForFolderW");
		if (MH_CreateHookApiEx(
			L"shell32.dll", "SHBrowseForFolderW", &Hook_SHBrowseForFolderW, &pORG_SHBrowseForFolderW) != MH_OK)
			return;

		if (pTargetW == NULL) return;
		if (MH_EnableHook(pTargetW) != MH_OK)
			return;
	}
	if (!pORG_CoCreateInstance)
	{
		if (MH_CreateHookApiEx(
			L"ole32.dll", "CoCreateInstance", &Hook_CoCreateInstance, &pORG_CoCreateInstance) != MH_OK)
			return;

		if (MH_EnableHook(&CoCreateInstance) != MH_OK)
			return;
	}
}
