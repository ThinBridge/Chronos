#include "stdafx.h"
#include "APIHook.h"
#pragma warning(push, 0)
#include <codeanalysis/warnings.h>
#pragma warning(disable \
		: ALL_CODE_ANALYSIS_WARNINGS)
#include "minhook\hook.hh"
#pragma warning(pop)
#include "Windows.h"
#include <mutex>

#define API_H_TRY \
	try       \
	{
#define API_H_CATCH \
	}           \
	catch (...) { ATLASSERT(0); }

static std::mutex ChronosDCMutex;
static std::set<HDC> ChronosMicrosoftPrintToPdfHDCSet;

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
	ChronosFileOpenDialog(CComPtr<IFileOpenDialog> originalDialog)
	{
		if (theApp.m_AppSettings.IsAdvancedLogMode())
		{
			theApp.WriteDebugTraceDateTime(_T("Construct ChronosFileOpenDialog"), DEBUG_LOG_TYPE_DE);
		}
		m_originalDialog = originalDialog;
	}

	~ChronosFileOpenDialog()
	{
		if (theApp.m_AppSettings.IsAdvancedLogMode())
		{
			theApp.WriteDebugTraceDateTime(_T("Destruct ChronosFileOpenDialog"), DEBUG_LOG_TYPE_DE);
		}
	}

	HRESULT SetUp()
	{

		CString strPath;
		HRESULT hresult = S_OK;

		if (theApp.IsSGMode())
		{
			CString strRootPath;
			if (theApp.m_AppSettings.IsShowUploadTab())
			{
				strRootPath = theApp.m_AppSettings.GetRootPath();
				if (strRootPath.IsEmpty())
				{
					strRootPath = _T("B:\\");
				}
				strRootPath += _T("UpLoad");
				if (!theApp.IsFolderExists(strRootPath))
				{
					strRootPath = _T("B:\\");
				}
			}
			else
			{
				strRootPath = theApp.m_AppSettings.GetUploadBasePath();
				if (strRootPath.IsEmpty())
				{
					strRootPath = _T("B:\\");
				}
			}
			m_strRootPath = strRootPath;
			strPath = strRootPath;

			FILEOPENDIALOGOPTIONS option = 0;
			this->GetOptions(&option);
			option |= FOS_HIDEMRUPLACES;
			option |= FOS_OVERWRITEPROMPT;
			option |= FOS_HIDEPINNEDPLACES;

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

		CComPtr<IShellItem> psi;
		hresult = ::SHCreateItemFromIDList(pidl, IID_PPV_ARGS(&psi));
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
		//呼び出し元を確認。親を辿っていき、CSGView（BroView）が登場しないなら、設定画面からの呼び出し。
		//設定画面からの呼び出しかつSGモードでないときは、そのまま無条件で開いて良い。
		HWND hWindowParent = hwndOwner;
		while (hWindowParent)
		{
			TCHAR pszName[64] = {0};
			if (!::GetClassName(hWindowParent, pszName, 64))
				return E_FAIL;
			if (::_tcscmp(pszName, _T("CSGView")) == 0)
				break;
			hWindowParent = GetParent(hWindowParent);
		}

		if (!hWindowParent && !theApp.IsSGMode())
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
				::MessageBoxW(hwndOwner, strMsg, theApp.m_strThisAppName, MB_OK | MB_ICONWARNING);
			}
			return E_ACCESSDENIED;
		}

		HRESULT hresult = SetUp();
		if (FAILED(hresult))
			return hresult;

		for (;;)
		{
			hresult = m_originalDialog->Show(hwndOwner);
			if (FAILED(hresult))
			{
				return hresult;
			}

			LPWSTR wstrSelPath;
			CComPtr<IShellItem> psi;
			hresult = this->GetResult(&psi);

			if (FAILED(hresult))
			{
				return hresult;
			}

			hresult = psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEEDITING, &wstrSelPath);
			if (FAILED(hresult))
			{
				return hresult;
			}

			CString strSelPath(wstrSelPath);
			CString strSelPathOriginal(wstrSelPath);
			CoTaskMemFree(wstrSelPath);
			strSelPath.MakeUpper();
			if (strSelPath.IsEmpty())
			{
				return hresult;
			}

			if (theApp.IsSGMode())
			{
				CString strRoot(m_strRootPath);
				strRoot.MakeUpper();
				if (strSelPath.Find(strRoot) != 0)
				{
					CString strCaption(theApp.m_strThisAppName);
					CString strMsg;
					strMsg.Format(L"アップロードフォルダー[%s]以外からはアップロードできません。\n\n指定しなおしてください。\n\n選択された場所[%s]", (LPCWSTR)m_strRootPath, (LPCWSTR)strSelPathOriginal);
					::MessageBoxW(hwndOwner, strMsg, strCaption, MB_OK | MB_ICONWARNING);
					continue;
				}
			}

			if (theApp.m_AppSettings.IsEnableLogging() && theApp.m_AppSettings.IsEnableUploadLogging())
			{
				WCHAR* ptrFile = NULL;
				ptrFile = PathFindFileNameW(strSelPathOriginal);
				CString strFileName(ptrFile ? ptrFile : strSelPathOriginal);
				theApp.SendLoggingMsg(LOG_UPLOAD, strFileName, hwndOwner);
			}

			PathCchRemoveFileSpec(strSelPath.GetBuffer(), strSelPath.GetLength() + 1);
			strSelPath.ReleaseBuffer();
			theApp.m_strLastSelectUploadFolderPath = strSelPath;

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
		return InterlockedIncrement(&m_referenceCount);
	}

	ULONG STDMETHODCALLTYPE Release(void)
	{
		ULONG referenceCount = InterlockedDecrement(&m_referenceCount);
		if (referenceCount == 0)
		{
			delete this;
		}
		return referenceCount;
	}

private:
	CComPtr<IFileOpenDialog> m_originalDialog;
	CString m_strRootPath;
	ULONG m_referenceCount = 0;
};

class ChronosFileSaveDialog : public IFileSaveDialog
{
public:
	ChronosFileSaveDialog(CComPtr<IFileSaveDialog> originalDialog)
	{
		if (theApp.m_AppSettings.IsAdvancedLogMode())
		{
			theApp.WriteDebugTraceDateTime(_T("Construct ChronosFileSaveDialog"), DEBUG_LOG_TYPE_DE);
		}
		m_originalDialog = originalDialog;
	}

	~ChronosFileSaveDialog()
	{
		if (theApp.m_AppSettings.IsAdvancedLogMode())
		{
			theApp.WriteDebugTraceDateTime(_T("Destruct ChronosFileSaveDialog"), DEBUG_LOG_TYPE_DE);
		}
	}

	HRESULT SetUp()
	{
		FILEOPENDIALOGOPTIONS option = 0;
		CString strPath;
		HRESULT hresult = S_OK;

		if (theApp.IsSGMode())
		{
			this->GetOptions(&option);
			option |= FOS_HIDEMRUPLACES;
			option |= FOS_OVERWRITEPROMPT;
			option |= FOS_HIDEPINNEDPLACES;

			hresult = this->SetOptions(option);
			if (FAILED(hresult))
			{
				return hresult;
			}
			
			strPath = theApp.m_AppSettings.GetRootPath();
			if (strPath.IsEmpty())
			{
				strPath = _T("B:\\");
			}
		}
		else
		{
			strPath = SBUtil::GetDownloadFolderPath();
		}

		if (!theApp.m_strLastSelectFolderPath.IsEmpty() && theApp.IsFolderExists(theApp.m_strLastSelectFolderPath))
		{
			strPath = theApp.m_strLastSelectFolderPath;
		}

		strPath = strPath.TrimRight('\\');
		strPath += _T("\\");

		PIDLIST_ABSOLUTE pidl;
		hresult = ::SHParseDisplayName(strPath, 0, &pidl, SFGAO_FOLDER, 0);
		if (FAILED(hresult))
		{
			return hresult;
		}

		CComPtr<IShellItem> psi;
		hresult = ::SHCreateItemFromIDList(pidl, IID_PPV_ARGS(&psi));
		if (SUCCEEDED(hresult))
		{
			this->SetFolder(psi);
			this->SetDefaultFolder(psi);
		}
		ILFree(pidl);
		return hresult;
	}

	HRESULT STDMETHODCALLTYPE SetSaveAsItem(
	    /* [in] */ __RPC__in_opt IShellItem* psi)
	{
		return m_originalDialog->SetSaveAsItem(psi);
	}

	HRESULT STDMETHODCALLTYPE SetProperties(
	    /* [in] */ __RPC__in_opt IPropertyStore* pStore)
	{
		return m_originalDialog->SetProperties(pStore);
	}

	HRESULT STDMETHODCALLTYPE SetCollectedProperties(
	    /* [in] */ __RPC__in_opt IPropertyDescriptionList* pList,
	    /* [in] */ BOOL fAppendDefault)
	{
		return m_originalDialog->SetCollectedProperties(pList, fAppendDefault);
	}

	HRESULT STDMETHODCALLTYPE GetProperties(
	    /* [out] */ __RPC__deref_out_opt IPropertyStore** ppStore)
	{
		return m_originalDialog->GetProperties(ppStore);
	}

	HRESULT STDMETHODCALLTYPE ApplyProperties(
	    /* [in] */ __RPC__in_opt IShellItem* psi,
	    /* [in] */ __RPC__in_opt IPropertyStore* pStore,
	    /* [unique][in] */ __RPC__in_opt HWND hwnd,
	    /* [unique][in] */ __RPC__in_opt IFileOperationProgressSink* pSink)
	{
		return m_originalDialog->ApplyProperties(psi, pStore, hwnd, pSink);
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

	/* [local] */
	HRESULT STDMETHODCALLTYPE Show(
	    /* [annotation][unique][in] */ _In_opt_ HWND hwndOwner)
	{
		if (theApp.m_AppSettings.IsEnableDownloadRestriction())
		{
			return E_ACCESSDENIED;
		}

		HRESULT hresult = SetUp();
		if (FAILED(hresult))
			return hresult;

		CString strRootPath(theApp.m_AppSettings.GetRootPath());
		if (strRootPath.IsEmpty())
		{
			strRootPath = _T("B:\\");
		}
		strRootPath = strRootPath.TrimRight('\\');
		strRootPath += _T("\\");

		LPWSTR wstrOriginalFileName = nullptr;
		hresult = m_originalDialog->GetFileName(&wstrOriginalFileName);
		if (FAILED(hresult))
		{
			return hresult;
		}
		CString strOriginalFileName(wstrOriginalFileName);
		CoTaskMemFree(wstrOriginalFileName);
		CString originalExt = SBUtil::GetFileExt(strOriginalFileName);

		for (;;)
		{
			hresult = m_originalDialog->Show(hwndOwner);
			if (FAILED(hresult))
			{
				return hresult;
			}

			LPWSTR wstrSelPath;
			CComPtr<IShellItem> psi;
			hresult = this->GetResult(&psi);
			if (FAILED(hresult))
			{
				return hresult;
			}

			hresult = psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEEDITING, &wstrSelPath);
			if (FAILED(hresult))
			{
				return hresult;
			}

			CString strSelPath(wstrSelPath);
			CoTaskMemFree(wstrSelPath);

			if (theApp.IsSGMode())
			{
				CString strSelPathUpper(strSelPath);
				strSelPathUpper.MakeUpper();
				if (strSelPathUpper.IsEmpty())
				{
					return hresult;
				}
				CString strRoot(strRootPath);
				strRoot.MakeUpper();
				if (strSelPathUpper.Find(strRoot) != 0)
				{
					CString strMsg;
					strMsg.Format(L"%sドライブ以外は指定できません。\n\n保存する場所から%sを指定しなおしてください。\n\n選択された場所[%s]", (LPCWSTR)strRootPath, (LPCWSTR)strRootPath, (LPCWSTR)strSelPath);
					::MessageBoxW(hwndOwner, strMsg, theApp.m_strThisAppName, MB_OK | MB_ICONWARNING);
					continue;
				}

				CStringW strTSG_Upload = strRootPath + L"Upload\\";
				CStringW strTSG_UploadUpper(strTSG_Upload);
				strTSG_UploadUpper.MakeUpper();
				if (strSelPathUpper.Find(strTSG_UploadUpper) == 0)
				{
					CString strMsg;
					strMsg.Format(L"アップロードフォルダー[%s]には保存できません。\n\n指定しなおしてください。\n\n選択された場所[%s]", (LPCWSTR)strTSG_Upload, (LPCWSTR)strSelPath);
					::MessageBoxW(hwndOwner, strMsg, theApp.m_strThisAppName, MB_OK | MB_ICONWARNING);
					continue;
				}
			}

			if (theApp.m_AppSettings.IsEnableFileExtChangeRestriction())
			{
				CString strSelExt = SBUtil::GetFileExt(strSelPath);
				if (strSelExt.CompareNoCase(originalExt) != 0)
				{
					CString strMsg;
					strMsg.Format(L"拡張子は変更できません。\n\n拡張子に[%s]を指定してください。", (LPCWSTR)originalExt);
					::MessageBoxW(hwndOwner, strMsg, theApp.m_strThisAppName, MB_OK | MB_ICONWARNING);
					// 原因は不明だが、SGモードでは繰り返しm_originalDialog->Show(hwndOwner)を実行しても問題ないが、ネイティブモードでは
					// メモリアクセス違反が発生しクラッシュする。そのため、Nativeモードではダイアログ自体を閉じるようにする。
					if (theApp.IsSGMode())
					{
						continue;
					}
					else
					{
						return E_FAIL;
					}
				}
			}

			PathCchRemoveFileSpec(strSelPath.GetBuffer(), strSelPath.GetLength() + 1);
			strSelPath.ReleaseBuffer();
			theApp.m_strLastSelectFolderPath = strSelPath;
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
		return InterlockedIncrement(&m_referenceCount);
	}

	ULONG STDMETHODCALLTYPE Release(void)
	{
		ULONG referenceCount = InterlockedDecrement(&m_referenceCount);
		if (referenceCount == 0)
		{
			delete this;
		}
		return referenceCount;
	}

private:
	CComPtr<IFileSaveDialog> m_originalDialog;
	ULONG m_referenceCount = 0;
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

	if (!(rclsid == CLSID_FileOpenDialog || rclsid == CLSID_FileSaveDialog))
	{
		return pORG_CoCreateInstance(
		    rclsid,
		    pUnkOuter,
		    dwClsContext,
		    riid,
		    ppv);
	}

	if (rclsid == CLSID_FileOpenDialog)
	{
		CComPtr<IFileOpenDialog> fileOpenDialog;
		hRet = pORG_CoCreateInstance(
		    rclsid,
		    pUnkOuter,
		    dwClsContext,
		    riid,
		    (LPVOID*)&fileOpenDialog);
		if (SUCCEEDED(hRet))
		{
			ChronosFileOpenDialog* chronosFileOpenDialog = new ChronosFileOpenDialog(fileOpenDialog);
			chronosFileOpenDialog->AddRef();
			*ppv = chronosFileOpenDialog;
		}
	}
	else
	{
		CComPtr<IFileSaveDialog> fileSaveDialog;
		hRet = pORG_CoCreateInstance(
		    rclsid,
		    pUnkOuter,
		    dwClsContext,
		    riid,
		    (LPVOID*)&fileSaveDialog);
		if (SUCCEEDED(hRet))
		{
			ChronosFileSaveDialog* chronosFileSaveDialog = new ChronosFileSaveDialog(fileSaveDialog);
			chronosFileSaveDialog->AddRef();
			*ppv = chronosFileSaveDialog;
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
				strMsg.Format(L"%sドライブ以外は指定できません。\n\n保存する場所から%sを指定しなおしてください。\n\n選択された場所[%s]", (LPCWSTR)strPath, (LPCWSTR)strPath, (LPCWSTR)lpofn->lpstrFile);
				::MessageBoxW(lpofn->hwndOwner, strMsg, strCaption, MB_OK | MB_ICONWARNING);
				continue;
			}

			CStringW strTSG_Upload = strPath + L"Upload\\";
			CStringW strTSG_UploadUpper(strTSG_Upload);
			strTSG_UploadUpper.MakeUpper();
			if (strSelPath.Find(strTSG_UploadUpper) == 0)
			{
				strMsg.Format(L"アップロードフォルダー[%s]には保存できません。\n\n指定しなおしてください。\n\n選択された場所[%s]", (LPCWSTR)strTSG_Upload, (LPCWSTR)lpofn->lpstrFile);
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
					strMsg.Format(L"アップロードフォルダー[%s]以外からはアップロードできません。\n\n指定しなおしてください。\n\n選択された場所[%s]", (LPCWSTR)strRootPath, (LPCWSTR)lpofn->lpstrFile);
					::MessageBoxW(lpofn->hwndOwner, strMsg, strCaption, MB_OK | MB_ICONWARNING);
					continue;
				}
			}

			PathCchRemoveFileSpec(strSelPath.GetBuffer(), strSelPath.GetLength() + 1);
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

typedef HDC(WINAPI* ORG_CreateDCW)(
    LPCWSTR pwszDriver,
    LPCWSTR pwszDevice,
    LPCWSTR pszPort,
    const DEVMODEW* pdm);

static ORG_CreateDCW pORG_CreateDCW = nullptr;

static HDC WINAPI Hook_CreateDCW(LPCWSTR pwszDriver, LPCWSTR pwszDevice, LPCWSTR pszPort, const DEVMODEW* pdm)
{
	HDC result = pORG_CreateDCW(pwszDriver, pwszDevice, pszPort, pdm);
	if (theApp.IsSGMode() &&
		lstrcmpW(pwszDevice, L"Microsoft Print to PDF") == 0)
	{
		ChronosDCMutex.lock();
		ChronosMicrosoftPrintToPdfHDCSet.insert(result);
		ChronosDCMutex.unlock();
	}
	return result;
}

typedef BOOL(WINAPI* ORG_DeleteDCW)(
    HDC hdc);

static ORG_DeleteDCW pORG_DeleteDC = nullptr;

static BOOL WINAPI Hook_DeleteDC(HDC hdc)
{
	if (theApp.IsSGMode())
	{
		ChronosDCMutex.lock();
		if (ChronosMicrosoftPrintToPdfHDCSet.find(hdc) != ChronosMicrosoftPrintToPdfHDCSet.end())
		{
			ChronosMicrosoftPrintToPdfHDCSet.erase(hdc);
		}
		ChronosDCMutex.unlock();
	}
	return pORG_DeleteDC(hdc);
}

typedef int(WINAPI* ORG_StartDocW)(HDC hdc, const DOCINFOW* lpdi);

static ORG_StartDocW pORG_StartDocW = nullptr;

static int WINAPI Hook_StartDocW(HDC hdc, const DOCINFOW* lpdi)
{
	// Since Chronos v13.0.112.1(CEF v112), ThinApp-ed Chronos crashes on executing Microsoft Print to PDF,
	// so we prevent to execute Microsoft Print to PDF.
	// Note that the following code requires CEF127+.
	// The code is unstable with CEF126 and below.
	if (theApp.IsSGMode())
	{
		ChronosDCMutex.lock();
		const bool isMicrosoftPrintToPDF = ChronosMicrosoftPrintToPdfHDCSet.find(hdc) != ChronosMicrosoftPrintToPdfHDCSet.end();
		ChronosDCMutex.unlock();
		if (isMicrosoftPrintToPDF)
		{
			::MessageBoxW(theApp.m_pMainWnd->m_hWnd, _T("「Microsoft Print to PDF」は使用できません。代わりに「PDF に保存」を使用してください。"), (LPCWSTR)theApp.m_strThisAppName, MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
			//If the last error is not ERROR_CANCELLED, CEF is crashed.
			SetLastError(ERROR_CANCELLED);
			return 0;
		}
	}
	return pORG_StartDocW(hdc, lpdi);
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
	if (!hModule)
	{
		hModule = LoadLibrary(L"comdlg32.dll");
	}	
	if (hModule)
	{
		if (!pORG_GetSaveFileNameW)
		{
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
			pTargetW = GetProcAddress(hModule, "GetOpenFileNameW");
			if (MH_CreateHookApiEx(
				L"comdlg32.dll", "GetOpenFileNameW", &Hook_GetOpenFileNameW, &pORG_GetOpenFileNameW) != MH_OK)
				return;

			if (pTargetW == NULL) return;
			if (MH_EnableHook(pTargetW) != MH_OK)
				return;
		}
	}

	////////////////////////////////////////////////////////////
	hModule = GetModuleHandleW(L"shell32.dll");
	if (!hModule)
	{
		hModule = LoadLibrary(L"shell32.dll");
	}	
	if (hModule)
	{
		if (!pORG_SHBrowseForFolderW)
		{
			pTargetW = GetProcAddress(hModule, "SHBrowseForFolderW");
			if (MH_CreateHookApiEx(
				L"shell32.dll", "SHBrowseForFolderW", &Hook_SHBrowseForFolderW, &pORG_SHBrowseForFolderW) != MH_OK)
				return;

			if (pTargetW == NULL) return;
			if (MH_EnableHook(pTargetW) != MH_OK)
				return;
		}
	}
	if (!pORG_CoCreateInstance)
	{
		if (MH_CreateHookApiEx(
			L"ole32.dll", "CoCreateInstance", &Hook_CoCreateInstance, &pORG_CoCreateInstance) != MH_OK)
			return;

		if (MH_EnableHook(&CoCreateInstance) != MH_OK)
			return;
	}

	hModule = GetModuleHandle(L"gdi32.dll");
	if (hModule == NULL)
	{
		hModule = LoadLibrary(L"gdi32.dll");
	}
	if (hModule)
	{
		if (!pORG_CreateDCW)
		{
			pTargetW = GetProcAddress(hModule, "CreateDCW");
			if (MH_CreateHookApiEx(
				L"gdi32.dll", "CreateDCW", &Hook_CreateDCW, &pORG_CreateDCW) != MH_OK)
				return;

			if (pTargetW == NULL) return;
			if (MH_EnableHook(pTargetW) != MH_OK) return;
		}
		if (!pORG_DeleteDC)
		{
			pTargetW = GetProcAddress(hModule, "DeleteDC");
			if (MH_CreateHookApiEx(
				L"gdi32.dll", "DeleteDC", &Hook_DeleteDC, &pORG_DeleteDC) != MH_OK)
				return;

			if (pTargetW == NULL) return;
			if (MH_EnableHook(pTargetW) != MH_OK) return;
		}
		if (!pORG_StartDocW)
		{
			pTargetW = GetProcAddress(hModule, "StartDocW");
			if (MH_CreateHookApiEx(
				L"gdi32.dll", "StartDocW", &Hook_StartDocW, &pORG_StartDocW) != MH_OK)
				return;

			if (pTargetW == NULL) return;
			if (MH_EnableHook(pTargetW) != MH_OK) return;
		}
	}
}
