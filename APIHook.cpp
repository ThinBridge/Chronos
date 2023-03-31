#include "stdafx.h"
#include "APIHook.h"
#include "minhook\hook.hh"
#include "Windows.h"

#define API_H_TRY \
	try       \
	{
#define API_H_CATCH \
	}           \
	catch (...) { ATLASSERT(0); }

///////////////////////////////////////////////////////////////////////////////////////////
//@@COM
///////////////////////////////////////////////////////////////////////////////////////////
//TypeDef
typedef HRESULT(WINAPI* ORG_CoCreateInstance)(
    _In_ REFCLSID rclsid,
    _In_ LPUNKNOWN pUnkOuter,
    _In_ DWORD dwClsContext,
    _In_ REFIID riid,
    _Out_ LPVOID* ppv);
static ORG_CoCreateInstance pORG_CoCreateInstance = NULL;

class ChronosFileOpenDialog : public IFileOpenDialog
{
public:
	ChronosFileOpenDialog(IFileOpenDialog* originalDialog)
	{
		if (theApp.m_AppSettings.IsAdvancedLogMode())
		{
			theApp.WriteDebugTraceDateTime(_T("ChronosFileOpenDialog"), DEBUG_LOG_TYPE_DE);
		}

		m_originalDialog = originalDialog;
	}

	~ChronosFileOpenDialog()
	{
		if (m_originalDialog)
		{
			delete m_originalDialog;
		}
	}

	HRESULT Initialize() {
		HRESULT hresult = S_OK;
		CString strPath;
		if (theApp.IsSGMode())
		{
			CString strRootPath;
			//UploadTabを使う場合は、B:\\Uploadにする
			if (theApp.m_AppSettings.IsShowUploadTab())
			{
				strRootPath = theApp.m_AppSettings.GetRootPath();
				if (strRootPath.IsEmpty())
					strRootPath = _T("B:\\");
				strRootPath += _T("UpLoad");
				if (!theApp.IsFolderExists(strRootPath))
					strRootPath = _T("B:\\");
			}
			//UploadTabを使わない場合は、O:\\にする
			else
			{
				strRootPath = theApp.m_AppSettings.GetUploadBasePath();
				if (strRootPath.IsEmpty())
					strRootPath = _T("B:\\");
			}
			m_strRootPath = strRootPath;
			strPath = strRootPath;

			FILEOPENDIALOGOPTIONS option = 0;
			// フック関数を無効
			option &= ~OFN_ENABLEHOOK;
			//ダイアログテンプレート無効
			option &= ~OFN_ENABLETEMPLATE;
			//Longファイル名を強制
			option |= OFN_LONGNAMES;
			//ネットワークボタンを隠す
			option |= OFN_NONETWORKBUTTON;
			//最近使ったファイルを追加しない
			option |= OFN_DONTADDTORECENT;
			//プレースバーを無効
			option |= OFN_EX_NOPLACESBAR;
			//ファイルを上書きするかどうか確認するプロンプトを表示
			option |= OFN_OVERWRITEPROMPT;

			hresult = this->SetOptions(option);
			if (FAILED(hresult))
			{
				return hresult;
			}
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

		PIDLIST_ABSOLUTE pidl;
		hresult = ::SHParseDisplayName(strPath, 0, &pidl, SFGAO_FOLDER, 0);
		if (FAILED(hresult))
		{
			return hresult;
		}

		IShellItem* psi;
		hresult = ::SHCreateShellItem(NULL, NULL, pidl, &psi);
		if (SUCCEEDED(hresult))
		{
			this->SetFolder(psi);
			this->SetDefaultFolder(psi);
		}
		ILFree(pidl);
		
		return hresult;
	}

	HRESULT STDMETHODCALLTYPE GetResults(/* [out] */ __RPC__deref_out_opt IShellItemArray** ppenum)
	{
		return m_originalDialog->GetResults(ppenum);
	}

	HRESULT STDMETHODCALLTYPE GetSelectedItems(/* [out] */ __RPC__deref_out_opt IShellItemArray** ppsai)
	{
		return m_originalDialog->GetSelectedItems(ppsai);
	}

	HRESULT STDMETHODCALLTYPE SetFileTypes(
	    /* [in] */ UINT cFileTypes,
	    /* [size_is][in] */ __RPC__in_ecount_full(cFileTypes) const COMDLG_FILTERSPEC* rgFilterSpec)
	{
		return m_originalDialog->SetFileTypes(cFileTypes, rgFilterSpec);
	}

	HRESULT STDMETHODCALLTYPE SetFileTypeIndex(
	    /* [in] */ UINT iFileType)
	{
		return m_originalDialog->SetFileTypeIndex(iFileType);
	}

	HRESULT STDMETHODCALLTYPE GetFileTypeIndex(
	    /* [out] */ __RPC__out UINT* piFileType)
	{
		return m_originalDialog->GetFileTypeIndex(piFileType);
	}

	HRESULT STDMETHODCALLTYPE Advise(
	    /* [in] */ __RPC__in_opt IFileDialogEvents* pfde,
	    /* [out] */ __RPC__out DWORD* pdwCookie)
	{
		return m_originalDialog->Advise(pfde, pdwCookie);
	}

	HRESULT STDMETHODCALLTYPE Unadvise(
	    /* [in] */ DWORD dwCookie)
	{
		return m_originalDialog->Unadvise(dwCookie);
	}

	HRESULT STDMETHODCALLTYPE SetOptions(
	    /* [in] */ FILEOPENDIALOGOPTIONS fos)
	{
		return m_originalDialog->SetOptions(fos);
	}

	HRESULT STDMETHODCALLTYPE GetOptions(
	    /* [out] */ __RPC__out FILEOPENDIALOGOPTIONS* pfos)
	{
		return m_originalDialog->GetOptions(pfos);
	}

	HRESULT STDMETHODCALLTYPE SetDefaultFolder(
	    /* [in] */ __RPC__in_opt IShellItem* psi)
	{
		return m_originalDialog->SetDefaultFolder(psi);
	}

	HRESULT STDMETHODCALLTYPE SetFolder(
	    /* [in] */ __RPC__in_opt IShellItem* psi)
	{
		return m_originalDialog->SetFolder(psi);
	}

	HRESULT STDMETHODCALLTYPE GetFolder(
	    /* [out] */ __RPC__deref_out_opt IShellItem** ppsi)
	{
		return m_originalDialog->GetFolder(ppsi);
	}

	HRESULT STDMETHODCALLTYPE GetCurrentSelection(
	    /* [out] */ __RPC__deref_out_opt IShellItem** ppsi)
	{
		return m_originalDialog->GetCurrentSelection(ppsi);
	}

	HRESULT STDMETHODCALLTYPE SetFileName(
	    /* [string][in] */ __RPC__in_string LPCWSTR pszName)
	{
		return m_originalDialog->SetFileName(pszName);
	}

	HRESULT STDMETHODCALLTYPE GetFileName(
	    /* [string][out] */ __RPC__deref_out_opt_string LPWSTR* pszName)
	{
		return m_originalDialog->GetFileName(pszName);
	}

	HRESULT STDMETHODCALLTYPE SetTitle(
	    /* [string][in] */ __RPC__in_string LPCWSTR pszTitle)
	{
		return m_originalDialog->SetTitle(pszTitle);
	}

	HRESULT STDMETHODCALLTYPE SetOkButtonLabel(
	    /* [string][in] */ __RPC__in_string LPCWSTR pszText)
	{
		return m_originalDialog->SetOkButtonLabel(pszText);
	}

	HRESULT STDMETHODCALLTYPE SetFileNameLabel(
	    /* [string][in] */ __RPC__in_string LPCWSTR pszLabel)
	{
		return m_originalDialog->SetFileNameLabel(pszLabel);
	}

	HRESULT STDMETHODCALLTYPE GetResult(
	    /* [out] */ __RPC__deref_out_opt IShellItem** ppsi)
	{
		return m_originalDialog->GetResult(ppsi);
	}

	HRESULT STDMETHODCALLTYPE AddPlace(
	    /* [in] */ __RPC__in_opt IShellItem* psi, /* [in] */ FDAP fdap)
	{
		return m_originalDialog->AddPlace(psi, fdap);
	}

	HRESULT STDMETHODCALLTYPE SetDefaultExtension(
	    /* [string][in] */ __RPC__in_string LPCWSTR pszDefaultExtension)
	{
		return m_originalDialog->SetDefaultExtension(pszDefaultExtension);
	}

	HRESULT STDMETHODCALLTYPE Close(
	    /* [in] */ HRESULT hr)
	{
		return m_originalDialog->Close(hr);
	}

	HRESULT STDMETHODCALLTYPE SetClientGuid(
	    /* [in] */ __RPC__in REFGUID guid)
	{
		return m_originalDialog->SetClientGuid(guid);
	}

	HRESULT STDMETHODCALLTYPE ClearClientData(
	    void)
	{
		return m_originalDialog->ClearClientData();
	}

	HRESULT STDMETHODCALLTYPE SetFilter(
	    /* [in] */ __RPC__in_opt IShellItemFilter* pFilter)
	{
		return m_originalDialog->SetFilter(pFilter);
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE Show(
	    /* [annotation][unique][in] */ _In_opt_ HWND hwndOwner)
	{
		//呼び出しもとを確認、親の親がNULLだったらChronosの設定画面から
		if (!hwndOwner)
		{
			return m_originalDialog->Show(hwndOwner);
		}
		HWND hWindowOwner = GetParent(hwndOwner);
		HWND hWindowParent = {0};
		if (hWindowOwner)
		{
			hWindowParent = GetParent(hWindowOwner);
		}
		//hWindowParentがNULLの場合は、そのまま
		if (!hWindowParent)
		{
			return m_originalDialog->Show(hwndOwner);
		}

		if (theApp.m_AppSettings.IsEnableUploadRestriction())
		{
			CString strMsg(L"ファイル アップロードは、システム管理者により制限されています。");
			if (hWindowParent)
			{
				theApp.SB_MessageBox(hWindowParent, strMsg, NULL, MB_OK | MB_ICONWARNING, TRUE);
			}
			else
			{
				CString strCaption(theApp.m_strThisAppName);
				::MessageBoxW(hwndOwner, strMsg, strCaption, MB_OK | MB_ICONWARNING);
			}

				return E_ACCESSDENIED;
		}
		
		for (;;)
		{
			HRESULT hresult = m_originalDialog->Show(hwndOwner);
			if (FAILED(hresult))
			{
				return hresult;
			}

			LPWSTR wstrSelPath;
			IShellItem *psi;
			hresult = this->GetResult(&psi);
			
			if (FAILED(hresult))
			{
				return hresult;
			}

			psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEEDITING, &wstrSelPath);

			CString strSelPath(wstrSelPath);
			strSelPath.MakeUpper();
			if (strSelPath.IsEmpty())
				return hresult;

			if (theApp.IsSGMode())
			{
				CStringW strRoot(m_strRootPath);
				strRoot.MakeUpper();
				if (strSelPath.Find(strRoot) != 0)
				{
					CString strCaption(theApp.m_strThisAppName);
					CString strMsg;
					strMsg.Format(L"アップロードフォルダー[%s]以外からはアップロードできません。\n\n指定しなおしてください。\n\n選択された場所[%s]", strRoot, (PCWSTR)strSelPath);
					::MessageBoxW(hwndOwner, strMsg, strCaption, MB_OK | MB_ICONWARNING);
					continue;
				}
			}

			PathRemoveFileSpec(strSelPath.GetBuffer());
			strSelPath.ReleaseBuffer();
			theApp.m_strLastSelectUploadFolderPath = strSelPath;

			if (theApp.m_AppSettings.IsEnableLogging() && theApp.m_AppSettings.IsEnableUploadLogging())
			{
				WCHAR* ptrFile = NULL;
				ptrFile = PathFindFileNameW(strSelPath);
				CString strFileName(ptrFile ? ptrFile : strSelPath);
				theApp.SendLoggingMsg(LOG_UPLOAD, strFileName, hwndOwner);
			}
			return hresult;
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(
	    /* [in] */ REFIID riid,
	    /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
	{
		return m_originalDialog->QueryInterface(riid, ppvObject);
	}

	ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return m_originalDialog->AddRef();
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		return m_originalDialog->Release();
	};

private:
	IFileOpenDialog* m_originalDialog = nullptr;
	CString m_strRootPath;
};

////////////////////////////////////////////////////////////////
//HookFunction
static HRESULT WINAPI Hook_CoCreateInstance(
    _In_ REFCLSID rclsid,
    _In_ LPUNKNOWN pUnkOuter,
    _In_ DWORD dwClsContext,
    _In_ REFIID riid,
    _Out_ LPVOID* ppv)
{
	PROC_TIME(Hook_CoCreateInstance)
	HRESULT hRet = {0};
	hRet = pORG_CoCreateInstance(
	    rclsid,
	    pUnkOuter,
	    dwClsContext,
	    riid,
	    ppv);

	if (SUCCEEDED(hRet))
	{
		if (rclsid == CLSID_FileOpenDialog) // || rclsid == CLSID_FileSaveDialog)
		{
			ChronosFileOpenDialog* chronosFileOpenDialog = new ChronosFileOpenDialog(static_cast<IFileOpenDialog*>(*ppv));
			chronosFileOpenDialog->Initialize();
			*ppv = (LPVOID)chronosFileOpenDialog;
		}
	}

	return hRet;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
//@@ComDlg32
///////////////////////////////////////////////////////////////////////////////////////////
//TypeDef
typedef BOOL(WINAPI* FuncGetSaveFileNameW)(LPOPENFILENAMEW lpofn);
static FuncGetSaveFileNameW pORG_GetSaveFileNameW = NULL;

typedef BOOL(WINAPI* FuncGetOpenFileNameW)(LPOPENFILENAMEW lpofn);
static FuncGetSaveFileNameW pORG_GetOpenFileNameW = NULL;

typedef PIDLIST_ABSOLUTE(WINAPI* FuncSHBrowseForFolderW)(LPBROWSEINFOW lpbi);
static FuncSHBrowseForFolderW pORG_SHBrowseForFolderW = NULL;

////////////////////////////////////////////////////////////////
//HookFunction
static BOOL WINAPI Hook_GetSaveFileNameW(
    LPOPENFILENAMEW lpofn)
{
	BOOL bRet = FALSE;
	try
	{
		//Download禁止
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

		//フック関数を無効
		lpofn->Flags &= ~OFN_ENABLEHOOK;
		//ダイアログテンプレート無効
		lpofn->Flags &= ~OFN_ENABLETEMPLATE;

		//Longファイル名を強制
		lpofn->Flags |= OFN_LONGNAMES;
		//ネットワークボタンを隠す
		lpofn->Flags |= OFN_NONETWORKBUTTON;

		//最近使ったファイルを追加しない
		lpofn->Flags |= OFN_DONTADDTORECENT;
		//プレースバーを無効
		lpofn->FlagsEx |= OFN_EX_NOPLACESBAR;

		//ファイルを上書きするかどうか確認するプロンプトを表示
		lpofn->Flags |= OFN_OVERWRITEPROMPT;

		CStringW strCaption(theApp.m_strThisAppName);
		CStringW strMsg;
		for (;;)
		{
			bRet = pORG_GetSaveFileNameW(lpofn);
			if (!bRet)
				return bRet;

			CStringW strSelPath(lpofn->lpstrFile);
			strSelPath.MakeUpper();
			if (strSelPath.IsEmpty())
				return bRet;

			CStringW strRoot(strPath);
			strRoot.MakeUpper();
			if (strSelPath.Find(strRoot) != 0)
			{
				strMsg.Format(L"%sドライブ以外は指定できません。\n\n保存する場所から%sを指定しなおしてください。\n\n選択された場所[%s]", strRoot, strRoot, (PCWSTR)strSelPath);
				::MessageBoxW(lpofn->hwndOwner, strMsg, strCaption, MB_OK | MB_ICONWARNING);
				continue;
			}

			CStringW strTSG_Upload = strRoot + L"UPLOAD\\";
			if (strSelPath.Find(strTSG_Upload) == 0)
			{
				strMsg.Format(L"アップロードフォルダー[%s]には保存できません。\n\n指定しなおしてください。\n\n選択された場所[%s]", strTSG_Upload, (PCWSTR)strSelPath);
				::MessageBoxW(lpofn->hwndOwner, strMsg, strCaption, MB_OK | MB_ICONWARNING);
				continue;
			}
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
    LPOPENFILENAMEW lpofn)
{
	BOOL bRet = FALSE;
	try
	{
		//呼び出しもとを確認、親の親がNULLだったらChronosの設定画面から
		HWND hWindowOwner = GetParent(lpofn->hwndOwner);
		HWND hWindowParent = {0};
		if (hWindowOwner)
			hWindowParent = GetParent(hWindowOwner);
		//hWindowParentがNULLの場合は、そのまま
		if (!hWindowParent)
		{
			bRet = pORG_GetOpenFileNameW(lpofn);
			return bRet;
		}

		CStringW strCaption(theApp.m_strThisAppName);
		CStringW strMsg;

		//Upload禁止
		if (theApp.m_AppSettings.IsEnableUploadRestriction())
		{
			strMsg = (L"ファイル アップロードは、システム管理者により制限されています。");
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
		CString strRootPath;
		if (theApp.IsSGMode())
		{
			//UploadTabを使う場合は、B:\\Uploadにする
			if (theApp.m_AppSettings.IsShowUploadTab())
			{
				strRootPath = theApp.m_AppSettings.GetRootPath();
				if (strRootPath.IsEmpty())
					strRootPath = _T("B:\\");
				strRootPath += _T("UpLoad");
				if (!theApp.IsFolderExists(strRootPath))
					strRootPath = _T("B:\\");
			}
			//UploadTabを使わない場合は、O:\\にする
			else
			{
				strRootPath = theApp.m_AppSettings.GetUploadBasePath();
				if (strRootPath.IsEmpty())
					strRootPath = _T("B:\\");
			}
			strPath = strRootPath;

			//フック関数を無効
			lpofn->Flags &= ~OFN_ENABLEHOOK;
			//ダイアログテンプレート無効
			lpofn->Flags &= ~OFN_ENABLETEMPLATE;

			//Longファイル名を強制
			lpofn->Flags |= OFN_LONGNAMES;
			//ネットワークボタンを隠す
			lpofn->Flags |= OFN_NONETWORKBUTTON;

			//最近使ったファイルを追加しない
			lpofn->Flags |= OFN_DONTADDTORECENT;
			//プレースバーを無効
			lpofn->FlagsEx |= OFN_EX_NOPLACESBAR;

			//ファイルを上書きするかどうか確認するプロンプトを表示
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
		for (;;)
		{
			bRet = pORG_GetOpenFileNameW(lpofn);
			if (!bRet)
				return bRet;

			CStringW strSelPath(lpofn->lpstrFile);
			strSelPath.MakeUpper();
			if (strSelPath.IsEmpty())
				return bRet;

			if (theApp.IsSGMode())
			{
				CStringW strRoot(strRootPath);
				strRoot.MakeUpper();
				if (strSelPath.Find(strRoot) != 0)
				{
					strMsg.Format(L"アップロードフォルダー[%s]以外からはアップロードできません。\n\n指定しなおしてください。\n\n選択された場所[%s]", strRoot, (PCWSTR)strSelPath);
					::MessageBoxW(lpofn->hwndOwner, strMsg, strCaption, MB_OK | MB_ICONWARNING);
					continue;
				}
			}

			PathRemoveFileSpec(strSelPath.GetBuffer());
			strSelPath.ReleaseBuffer();
			theApp.m_strLastSelectUploadFolderPath = strSelPath;

			if (theApp.m_AppSettings.IsEnableLogging() && theApp.m_AppSettings.IsEnableUploadLogging())
			{
				CString strPath(lpofn->lpstrFile);
				WCHAR* ptrFile = NULL;
				ptrFile = PathFindFileNameW(strPath);
				CString strFileName(ptrFile ? ptrFile : strPath);
				theApp.SendLoggingMsg(LOG_UPLOAD, strFileName, lpofn->hwndOwner);
			}
			return bRet;
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return bRet;
}
static PIDLIST_ABSOLUTE WINAPI Hook_SHBrowseForFolderW(
    LPBROWSEINFOW lpbi)
{
	CStringW strCaption(theApp.m_strThisAppName);
	CStringW strMsg;
	strMsg.Format(L"フォルダーの参照機能は利用できません。");
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
