#pragma once

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif
#include "StdAfx.h"
#include "resource.h" // メイン シンボル

//////////////////////////////////////////////
#include "Psapi.h"
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "Dwmapi.lib")

//////////////////////////////////////////////
#include "fav.h"
#include "sbcommon.h"
#include "DlgSetting.h"
#include "DlgDebugWnd.h"
#include "include\cef_parser.h"
/////////////////////////////////////////////////////////////////////////////
// CSazabi:
//
//////////////////////////////////////////////////////////
#define CEF_PROC_BROWSER  0
#define CEF_PROC_RENDERER 1
#define CEF_PROC_GPU	  2
#define CEF_PROC_OTHER	  3

class CChildView;
class CScriptHost;
class CBrowserFrame;
class CCloseNilButton;
class CNewTabButton;
class CActiveTabLine;
class APIHookC;
class CLogDispatcher;

#include "include/cef_base.h"
#include "include/cef_app.h"
#include "client_app.h"
class CSazabi : public CWinApp
{
public:
	//func
	CSazabi();
	virtual ~CSazabi();
	BOOL PumpMessage();
	CStringW m_strAppIDw;

	//SZB
	BOOL m_bCEFInitialized;
	BOOL m_bMultiThreadedMessageLoop;

	CefRefPtr<ClientApp> m_cefApp;

	CImageList m_imgMenuIcons;
	CImageList m_imgFavIcons;
	void SetFavicon(CImage* img, CBrowserFrame* pwndFrame);
	void SetDefaultFavicon(CBrowserFrame* pwndFrame);
	void SetWarmFavicon(HWND hwndF);

	CFavoriteItemManager m_FavMng;
	APIHookC* m_pAPIHook;
	HWND m_hwndTaskDlg;
	CChildView* GetActiveViewPtr();
	CBrowserFrame* GetActiveBFramePtr();
	HWND GetActiveBFramePtrHWND();
	BOOL m_bTabEnable_Init;
	BOOL m_bShutdownFlg;

	//setting//////////////////////////
	AppSettings m_AppSettings;
	CScriptSrcMgr m_cScriptSrc;

	//Path///////////////////////////////
	CString m_strExeFullPath;
	CString m_strExeFileName;
	CString m_strExeFolderPath;
	CString m_strLogFileFullPath;
	CString m_strRecoveryFileFullPath;
	CString m_strRecoveryFileName;
	CString m_strLogoFileFullPath;

	CString m_strRestoreFileFullPath;
	BOOL m_bAbortFlg;

	CString m_strDBL_EXE_Default_FullPath;
	CString m_strDBL_EXE_FullPath;
	CString m_strDBL_EXE_FolderPath;
	CString m_strSettingFileFullPath;

	CString m_strCEFCachePath;
	CString m_strFaviconCachePath;

	//StringInfo/////////////////////////////////////
	CString m_strThisAppName;
	CString m_strThisAppVersionString;
	CString m_strAtomParam;
	CString m_strCommandParam;
	CString m_strOptionParam;

	BOOL m_bNewInstanceParam;
	BOOL m_bTabWndChanging;
	//ConstString
	CString m_strZoneMessageDBL;
	CString m_strZoneMessageNG;
	CString m_strZoneMessageIE;
	CString m_strZoneMessageFF;
	CString m_strZoneMessageCHR;
	CString m_strZoneMessageEDG;
	CString m_strZoneMessageCustom;

	CString m_strLastSelectFolderPath;
	CString m_strLastSelectUploadFolderPath;

	//funcutil
	TCHAR m_FrmWndClassName[512];

	/* CBrowserFrame::OnCloseDelay */
	HANDLE m_hEventLog;
	CString m_strEventLogName;

	/* CSazabi::IsCacheRedirectFilterNone */
	HANDLE m_hEventLogScript;
	CString m_strEventLogScriptName;

	/* CMainFrame::SaveWindowList */
	HANDLE m_hEventRecovery;
	CString m_strEventRecoveryName;

	CStringList m_listCloseWindowURL;
	CStringList m_listCloseWindowTitle;

	CMapStringToPtr m_CacheRedirectFilter_None;
	BOOL IsCacheRedirectFilterNone(LPCTSTR pURL);
	void AddCacheRedirectFilterNone(LPCTSTR pURL);

	/* CSazabi::IsCacheRedirectFilterAllow */
	HANDLE m_hEventURLFilterAllow;
	CString m_strEventURLFilterAllow;
	CMapStringToPtr m_CacheURLFilter_Allow;
	BOOL IsCacheURLFilterAllow(LPCTSTR pURL);
	void AddCacheURLFilterAllow(LPCTSTR pURL);

	/* CSazabi::IsCacheRedirectFilterDeny */
	HANDLE m_hEventURLFilterDeny;
	CString m_strEventURLFilterDeny;
	CFilterURLList m_cDomainFilterList;
	CString m_strDomainFilterFileFullPath;

	CCustomScriptList m_cCustomScriptList;
	CString m_strCustomScriptConfFullPath;

	CMapStringToPtr m_CacheURLFilter_Deny;
	BOOL IsCacheURLFilterDeny(LPCTSTR pURL);
	void AddCacheURLFilterDeny(LPCTSTR pURL);

	BOOL IsURLFilterAllow(LPCTSTR sURL,
			      LPCTSTR sSchme,
			      LPCTSTR sHost,
			      LPCTSTR sPath);

	BOOL bCreateFavDone;
	BOOL m_IsSGMode;
	BOOL IsSGMode()
	{
		return m_IsSGMode;
	}
	///////////////////////////////////////
	BOOL m_bFirstInstance;
	BOOL IsFirstInstance()
	{
		return m_bFirstInstance;
	}
	BOOL IsExistsAnotherInstance()
	{
		BOOL bRet = FALSE;
		//既に起動しているか？
		HWND hWndCap = FindWindow(m_FrmWndClassName, NULL); //APのハンドル取得
								    //起動している。
		if (hWndCap != NULL)
		{
			TCHAR szTitleMultipleInstance[260] = {0};
			::GetWindowText(hWndCap, szTitleMultipleInstance, 259);
			CString strTitleMultiple;
			strTitleMultiple = szTitleMultipleInstance;
			if (strTitleMultiple != m_FrmWndClassName)
			{
				bRet = FALSE;
			}
			else
				bRet = TRUE;
		}
		else
		{
			bRet = FALSE;
		}
		return bRet;
	}

	//ChildViewのポインタをリストから返す。
	CChildView* GetChildViewPtr(HWND hWnd);

	DWORD m_dwProcessId;
	CHandle m_hProcess;
	int m_iWinOSVersion;
	int m_iWinOSBuildVersion;
	double m_ScaleDPI;

	////////////////////////////////////////////////
	virtual BOOL InitInstance();
	BOOL InitFunc_Base();
	BOOL InitFunc_Events();
	BOOL InitFunc_Paths();
	BOOL InitFunc_ExecOnVOS();
	BOOL InitFunc_Settings();
	BOOL InitFunc_SGMode();
	void InitProcessSetting();
	void InitLogWrite();
	void InitParseCommandLine();
	BOOL InitMultipleInstance();
	void InitReadConfSetting();
	void InitializeCef();
	////////////////////////////////////////////////
	virtual int ExitInstance();
	void UnInitializeObjects();
	void UnInitializeCef();
	////////////////////////////////////////////////

	void SetThisAppVersionString();
	CString GetUserAgent();
	CString GetOSInfo();
	CString GetOSKernelVersion();
	CString GetVOSInfo();
	CString GetVOSVersionFromNT0_DLLStr();
	CString GetVOSProcessString(BOOL bCurrent = TRUE, DWORD* pdwCnt = NULL, BOOL bNeedCmdLine = TRUE);

	CString GetTurboVMInfo();

	BOOL CloseVOSProc();
	void CloseVOSProcessReadReg(BOOL bForce);
	void CloseVOSProcessOther();
	HWND GetTopWindowHandle(const DWORD TargetID);
	BOOL IsProcessExists(DWORD dPID);
	CString IsProcessExistsName(DWORD dPID);
	CString GetCefVersionStr();
	CString GetChromiumVersionStr();
	//Setting Dlg///////////////////////////////////
	void ShowSettingDlg(CWnd* pParentWnd);
	CString m_strCurrentURL4DlgSetting;
	CSettingsDialog* m_pSettingDlg;
	AppSettings m_AppSettingsDlgCurrent;
	////////////////////////////////////////////////

	//DebugTrace Dlg///////////////////////////////
	CDlgDebugWnd* m_pDebugDlg;
	void ShowDebugTraceDlg();
	void ShowDevTools();
	BOOL IsShowDevTools();

	void CreateNewWindow(LPCTSTR lpURL, BOOL bActive);

	////////////////////////////////////////////////
	BOOL m_bUseApp;
	////////////////////////////////////////////////

	////////////////////////////////////////////////
	DWORD GetKeyCombi();
	BOOL bValidKeyCombi();
	////////////////////////////////////////////////

	CString GetAllModules();

	///////////////////////////////////////////////////////////
	int GetGDIObjectCnt()
	{
		return ::GetGuiResources(m_hProcess, GR_GDIOBJECTS);
		;
	}
	int GetUserObjectCnt()
	{
		return ::GetGuiResources(m_hProcess, GR_USEROBJECTS);
	}

	unsigned long long GetMemoryUsageSize();
	unsigned long long GetMemoryUsageSizeFromPID(DWORD dwPID);
	void EmptyWorkingSetAll();
	void EmptyWorkingSetSingle(DWORD dwProcessId);

	BOOL IsProcOwner(DWORD dwPID);
	int GetProcessRunningTime();
	///////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////
	CString GetThinAppEntryPointPath();
	CString GetThinAppEntryPointFolderPath();

	UINT GetWindowCount();
	int GetOSVersion();
	void HideRebar(CWnd* pFrame);
	///////////////////////////////////////////////////////////
	void CopyDBLEXEToTempInit();
	void SetRecoveryFilePath();
	void CopyDBLEXEToTempEx();
	void OpenDefaultBrowser(const CString& strURL, DWORD iType, const CString strPath);
	void OpenFileExplorer(const CString& strURL);
	void ExecNewInstance(const CString strURL);

	void TraceLogBackup();
	void WriteDebugTraceDateTime(LPCTSTR msg, int iLogType);
	void AppendDebugViewLog(const DebugWndLogData& Data);
	BOOL SafeTerminateProcess(HANDLE hProcess, INT_PTR uExitCode);
	CString GetActivePageURL();
	BOOL DeleteDirectory(LPCTSTR lpPathName, LPCTSTR lpPat);
	BOOL DeleteDirectoryTempFolder(LPCTSTR lpPathName);

	void ExitKillZombieProcess();
	void OpenChFiler(LPCTSTR lpOpenPath);
	void OpenChTaskMgr();

	//{{AFX_VIRTUAL(CSazabi)
public:
	//}}AFX_VIRTUAL
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

	BOOL IsLimitChkEx();

	///////////////////////////////////////////////////////////
	inline HWND SafeWnd(HWND wnd)
	{
		HWND hRetNULL = {0};
		if (wnd == NULL)
			return hRetNULL;
		TCHAR szLog[128] = {0};
		__try
		{
			if (IsWindow(wnd))
				return wnd;
			else
			{
				return hRetNULL;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return hRetNULL;
		}
		return hRetNULL;
	}
	inline HWND SafeWnd(CWnd* wnd)
	{
		HWND hRetNULL = {0};
		if (wnd == NULL)
			return hRetNULL;
		return SafeWnd(wnd->m_hWnd);
	}
	inline static BOOL IsWnd(CWnd* wnd)
	{
		if (wnd == NULL)
			return FALSE;
		__try
		{
			if (!IsWindow(wnd->m_hWnd))
				return FALSE;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return FALSE;
		}
		return TRUE;
	}
	inline static BOOL Safe_WM_GETTEXT(HWND hWnd, int iBuffLen, LPCTSTR ptrS)
	{
		BOOL bRet = TRUE;
		if (ptrS == NULL) return bRet;

		if (hWnd == NULL) return bRet;

		__try
		{
			if (!IsWindow(hWnd))
				return bRet;
			::SendMessage(hWnd, WM_GETTEXT, iBuffLen, (LPARAM)ptrS);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return FALSE;
		}
		return bRet;
	}
	inline BOOL IsWndVisible(HWND hWnd)
	{
		BOOL bRet = FALSE;
		__try
		{
			if (!IsWindow(hWnd))
				return bRet;
			bRet = ::IsWindowVisible(hWnd);
			if (bRet)
			{
				LONG dwExStyle = 0;
				dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
				if ((dwExStyle & WS_EX_LAYERED) == WS_EX_LAYERED)
				{
					BYTE alpha = 0;
					GetLayeredWindowAttributes(hWnd, NULL, &alpha, NULL);
					if (alpha == 0)
						bRet = FALSE;
				}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return FALSE;
		}
		return bRet;
	}
	inline void HideWnd(HWND hWnd)
	{
		__try
		{
			if (!IsWindow(hWnd))
				return;
			::ShowWindow(hWnd, SW_HIDE);
			return;
			LONG dwExStyle = 0;
			dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
			if ((dwExStyle & WS_EX_LAYERED) != WS_EX_LAYERED)
			{
				dwExStyle = dwExStyle | WS_EX_LAYERED;
				SetWindowLong(hWnd, GWL_EXSTYLE, dwExStyle);
				::SetLayeredWindowAttributes(hWnd, 0, 0, LWA_ALPHA);
			}
			else
			{
				BYTE alpha = 0;
				GetLayeredWindowAttributes(hWnd, NULL, &alpha, NULL);
				if (alpha != 0)
				{
					::SetLayeredWindowAttributes(hWnd, 0, 0, LWA_ALPHA);
				}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return;
		}
		return;
	}

	inline void ShowWnd(HWND hWnd)
	{
		__try
		{
			if (!IsWindow(hWnd))
				return;
			return;

			LONG dwExStyle = 0;
			dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
			if ((dwExStyle & WS_EX_LAYERED) == WS_EX_LAYERED)
			{
				BYTE alpha = 0;
				GetLayeredWindowAttributes(hWnd, NULL, &alpha, NULL);
				if (alpha < 255)
				{
					::SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
				}
				return;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return;
		}
		return;
	}

	CString ConvertStr2UTF8UrlEncode(LPCTSTR str)
	{
		CefString strIn(str);
		CefString strOut;
		strOut = CefURIEncode(strIn, false);
		CString strRet = strOut.c_str();
		return strRet;
	}
	UINT m_VEcache;
	UINT InVirtualEnvironment()
	{
		if (m_VEcache == 9999)
		{
			m_VEcache = VE_NA;
			HMODULE hMods = NULL;
#ifndef _WIN64
			hMods = ::GetModuleHandle(_T("NT0_DLL.DLL"));
#else  //WIN64
			hMods = ::GetModuleHandle(_T("NT0_DLL64.DLL"));
#endif //WIN64
			if (hMods)
				m_VEcache = VE_THINAPP;
			else
			{
				hMods = ::GetModuleHandle(_T("vmx.dll"));
				if (hMods)
					m_VEcache = VE_TURBO;
			}
		}
		return m_VEcache;
	}
	UINT m_tRDS;
	UINT InRDSEnvironment()
	{
		if (m_tRDS == 9999)
		{
			m_tRDS = RDS_NA;
			if (SBUtil::IsWindowsServerRDS())
			{
				m_tRDS = RDS_RDP;
				HMODULE hMods = NULL;
#ifndef _WIN64
				hMods = ::GetModuleHandle(_T("VMToolsHook.DLL"));
#else  //WIN64
				hMods = ::GetModuleHandle(_T("VMToolsHook64.DLL"));
#endif //WIN64
				if (hMods)
					m_tRDS = RDS_VMWARE;
			}
		}
		return m_tRDS;
	}

	int SB_MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, BOOL bBlackOut = FALSE, int iTimeOut = -1);

	CScriptHost* m_pScHost;
	CDLManager m_DlMgr;

	WINDOWPLACEMENT m_NomalWindow_FramePracementCache;

protected:
	WINDOWPLACEMENT m_ActiveFramePracement;

public:
	WINDOWPLACEMENT GetActiveFrameWindowPlacement();
	CBrowserFrame* GetNextGenerationActiveWindow(CBrowserFrame* pTarget);
	CCloseNilButton* m_wndpClose;
	CNewTabButton* m_wndpNewTab;
	CActiveTabLine* m_wndpActiveTabLine;
	CLogDispatcher* m_pLogDisp;
	void SendLoggingMsg(int LOG_TYPE, LPCTSTR lpFileName, HWND hWnd);

public:
	BOOL IsFolderExists(LPCTSTR ptrPath)
	{
		if (!ptrPath) return FALSE;
		BOOL bFolderExists = FALSE;
		//フォルダーの存在チェック
		if (::PathFileExists(ptrPath) && !::PathIsDirectory(ptrPath))
		{
			// 指定されたパスにファイルが存在、かつディレクトリでない
			bFolderExists = FALSE;
		}
		else if (::PathFileExists(ptrPath))
		{
			bFolderExists = TRUE;
		}
		return bFolderExists;
	}
	BOOL MakeDirectoryPath(LPCTSTR lpPathName)
	{
		int nStart = 0;
		BOOL res = FALSE;
		CString strMakePath;
		CString strPathName = lpPathName;
		strPathName.TrimRight('\\');
		strPathName += _T("\\");

		// ディレクトリを1つずつ繰り返し作成
		while (strMakePath + _T("\\") != strPathName)
		{
			// 作成するディレクトリ名を設定
			nStart = strPathName.Find(_T("\\"), nStart + 1);
			strMakePath = strPathName.Left(nStart);

			// ディレクトリが存在するかチェックし無ければ作成
			if (strMakePath.GetLength() > 2)
			{
				if (!IsFolderExists(strMakePath))
					res = ::CreateDirectory(strMakePath, NULL);
			}
		}
		return res;
	}
	BOOL SetPriority(DWORD PID, DWORD uiPR_Class)
	{
		BOOL bRet = FALSE;
		if (PID == 0) return TRUE;

		HANDLE hProcess = {0};
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, PID);
		if (!hProcess)
			return FALSE;

		if (::SetPriorityClass(hProcess, uiPR_Class))
			bRet = TRUE;
		//::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_NORMAL);
		CloseHandle(hProcess);
		return bRet;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	CString GetDriveName(LPCTSTR lpPath)
	{
		CString strRet;
		if (!lpPath) return strRet;

		CString strParam(lpPath);
		TCHAR szPath[MAX_PATH] = {0};
		lstrcpyn(szPath, strParam, MAX_PATH);

		if (PathStripToRoot(szPath))
			strRet = szPath;
		return strRet;
	}

	CString GetSandboxFilePath(LPCTSTR lpFile)
	{
		CString strRet;
		if (InVirtualEnvironment() != VE_THINAPP)
			return strRet;

		CString SBPath;
		HKEY hKey = {0};
		LONG lResult = 0L;
		DWORD dwType = 0;
		lResult = RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\ChronosSystemGuard"),
				       0, KEY_ALL_ACCESS, &hKey);
		if (lResult == ERROR_SUCCESS)
		{
			DWORD iSize = 0;
			TCHAR* pVal = NULL;

			RegQueryValueEx(hKey, _T("SBPath"), NULL, &dwType, NULL, &iSize);
			if (iSize > 0)
			{
				iSize += 1; //+1=null
				pVal = new TCHAR[iSize];
				memset(pVal, 0x00, sizeof(TCHAR) * iSize);
				RegQueryValueEx(hKey, _T("SBPath"), NULL, &dwType, (LPBYTE)pVal, &iSize);
				SBPath = pVal;
				delete[] pVal;
			}
			RegCloseKey(hKey);
		}
		if (!SBPath.IsEmpty())
		{
			SBPath.TrimRight('\\');
			SBPath += _T("\\");
			CString strTemp;
			strTemp = vFS_ReverseExpandPath(lpFile);
			if (!strTemp.IsEmpty())
			{
				SBPath += strTemp;
				strRet = SBPath;
			}
		}
		return strRet;
	}
	CString vFS_ReverseExpandPath(LPCTSTR lpFile)
	{
		CString strFile(lpFile);
		CString strRet;
		if (InVirtualEnvironment() != VE_THINAPP)
			return strRet;
		//現実的にファイルを保存する先のみに絞る。
		CString strDesktopPath;
		CString strPersonalPath;
		CString strProfilePath;
		CString strDrive;
		TCHAR szFolder[1024] = {0};
		//Desktop
		memset(szFolder, 0x00, sizeof(szFolder));
		if (::SHGetSpecialFolderPath(NULL, szFolder, CSIDL_DESKTOP, FALSE))
		{
			strDesktopPath = szFolder;
			if (!strDesktopPath.IsEmpty())
			{
				strDesktopPath.TrimRight('\\');
				strDesktopPath += _T("\\");
			}
		}

		//Personal
		memset(szFolder, 0x00, sizeof(szFolder));
		if (::SHGetSpecialFolderPath(NULL, szFolder, CSIDL_PERSONAL, FALSE))
		{
			strPersonalPath = szFolder;
			if (!strPersonalPath.IsEmpty())
			{
				strPersonalPath.TrimRight('\\');
				strPersonalPath += _T("\\");
			}
		}

		//Profile
		memset(szFolder, 0x00, sizeof(szFolder));
		if (::SHGetSpecialFolderPath(NULL, szFolder, CSIDL_PROFILE, FALSE))
		{
			strProfilePath = szFolder;
			if (!strProfilePath.IsEmpty())
			{
				strProfilePath.TrimRight('\\');
				strProfilePath += _T("\\");
			}
		}
		//Drive
		strDrive = GetDriveName(strFile);
		strDrive.MakeLower();

		//Pathがどの項目に該当するかチェック
		TCHAR szBuffer[MAX_PATH] = {0};
		CString strBuffer;
		CString strTemp;
		BOOL bFindSB = FALSE;
		for (;;)
		{
			//Desktop
			if (PathCommonPrefix(strDesktopPath, strFile, szBuffer))
			{
				strBuffer = szBuffer;
				strBuffer.MakeLower();
				strTemp = strFile;
				strTemp.MakeLower();
				strTemp.Replace(strBuffer, _T("%Desktop%"));
				if (FindSBFile(strTemp))
				{
					bFindSB = TRUE;
					break;
				}
			}
			//Personal
			if (PathCommonPrefix(strPersonalPath, strFile, szBuffer))
			{
				strBuffer = szBuffer;
				strBuffer.MakeLower();
				strTemp = strFile;
				strTemp.MakeLower();
				strTemp.Replace(strBuffer, _T("%Personal%"));
				if (FindSBFile(strTemp))
				{
					bFindSB = TRUE;
					break;
				}
			}

			//Profile
			if (PathCommonPrefix(strProfilePath, strFile, szBuffer))
			{
				strBuffer = szBuffer;
				strBuffer.MakeLower();
				strTemp = strFile;
				strTemp.MakeLower();
				strTemp.Replace(strBuffer, _T("%Profile%"));
				if (FindSBFile(strTemp))
				{
					bFindSB = TRUE;
					break;
				}
			}
			//Drive
			strTemp = strFile;
			strTemp.MakeLower();
			CString strDriveMac;
			strDriveMac = _T("%drive_");
			strDriveMac += strDrive[0];
			strDriveMac += _T("%\\");
			strTemp.Replace(strDrive, strDriveMac);
			if (FindSBFile(strTemp))
			{
				bFindSB = TRUE;
				break;
			}
			else
			{
				//DosDeviceChk
				CString strDiskPath;
				CString strDosDrive;
				strDosDrive = strDrive[0];
				strDosDrive += _T(":");
				TCHAR szTargetPath[4096] = {0};
				if (0 != QueryDosDevice(strDosDrive, szTargetPath, 4096))
				{
					strDiskPath = szTargetPath;
					if (strDiskPath.Find(_T("\\??\\")) == 0)
					{
						strDiskPath.Replace(_T("\\??\\"), _T(""));
						strDiskPath.TrimRight('\\');
						strDiskPath += _T("\\");
						strDiskPath.MakeLower();
						strTemp = strFile;
						strTemp.MakeLower();
						strTemp.Replace(strDrive, strDiskPath);
						strDrive = GetDriveName(strTemp);
						strDrive.MakeLower();
						strDriveMac = _T("%drive_");
						strDriveMac += strDrive[0];
						strDriveMac += _T("%\\");
						strTemp.Replace(strDrive, strDriveMac);
						if (FindSBFile(strTemp))
						{
							bFindSB = TRUE;
							break;
						}
					}
				}
			}
			break;
		}
		if (bFindSB)
			strRet = strTemp;
		return strRet;
	}

	BOOL FindSBFile(LPCTSTR lpMacroRoot)
	{
		CString strSubKey;
		strSubKey = _T("FS\\");
		strSubKey += lpMacroRoot;
		HKEY hKey = {0};
		LONG lResult = 0L;
		DWORD dwType = 0;
		DWORD dwCount = 0;
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey,
				       0, KEY_READ, &hKey);
		if (lResult == ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return TRUE;
		}
		return FALSE;
	}
	void Exec_SB2PYS_COPY(LPCTSTR lpSBFilePath, LPCTSTR lpPYSFilePath)
	{
		try
		{
			if (InVirtualEnvironment() != VE_THINAPP)
				return;

			CString strSBFilePath(lpSBFilePath);
			CString strPYSFilePath(lpPYSFilePath);
			if (!strSBFilePath.IsEmpty() && !strPYSFilePath.IsEmpty())
			{
				if (PathIsRelative(strPYSFilePath))
					return;
				if (PathIsFileSpec(strPYSFilePath))
					return;

				TCHAR szTemp[MAX_PATH + 1] = {0};
				::GetTempPath(MAX_PATH, szTemp);
				CString strTempPath;
				CString strTempPathDir;

				strTempPath = szTemp;
				CString strTempUpper;
				strTempUpper = strTempPath;
				strTempUpper.MakeUpper();
				if (strTempUpper.Find(_T("\\CHRONOSSG\\")) >= 0)
				{
					strTempPath.TrimRight('\\');
				}
				else
					strTempPath += _T("ChronosSG");
				strTempPath += _T("\\DBLC.exe");
				if (::PathFileExists(strTempPath))
				{
					CString strCommand;
					CString strParam;

					strCommand.Format(_T("\"%s\" -SB2PYS \"%s|@@|%s\""), (LPCWSTR)strTempPath, (LPCWSTR)strSBFilePath, (LPCWSTR)strPYSFilePath);
					strParam.Format(_T("-SB2PYS \"%s|@@|%s\""), (LPCWSTR)strSBFilePath, (LPCWSTR)strPYSFilePath);

					STARTUPINFO si = {0};
					PROCESS_INFORMATION pi = {0};
					si.cb = sizeof(si);
					unsigned long ecode = 0;
					if (!CreateProcess(NULL, (LPTSTR)(LPCTSTR)strCommand, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
					{
						SetLastError(NO_ERROR);
						//Retry
						if (!CreateProcess(strTempPath, (LPTSTR)(LPCTSTR)strParam, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
						{
							SetLastError(NO_ERROR);
							if (::ShellExecute(NULL, _T("open"), strTempPath, strParam, NULL, SW_SHOW) <= HINSTANCE(32))
							{
								::ShellExecute(NULL, NULL, strCommand, NULL, NULL, SW_SHOW);
							}
						}
					}
					if (pi.hThread)
					{
						CloseHandle(pi.hThread);
						pi.hThread = 0;
					}
					if (pi.hProcess)
					{
						CloseHandle(pi.hProcess);
						pi.hProcess = 0;
					}
				}
			}
		}
		catch (...)
		{
			ATLASSERT(0);
		}
		return;
	}
	BOOL m_bEnforceDeleteCache;
	void DeleteCEFCache()
	{
		//CEFCacheを削除する。
		if (!m_strCEFCachePath.IsEmpty())
		{
			DeleteDirectory(m_strCEFCachePath, _T("*.*"));
		}
	}
	//Windows10 1903環境で問題発生。対策コード2019-10-15
	BOOL DeleteFileFix(LPCTSTR ptrPath)
	{
		if (!ptrPath)
			return FALSE;

		//ファイルが存在しない。
		if (!::PathFileExists(ptrPath))
			return FALSE;
		//Win32APIのDeleteFileをCall
		//Windows 10 1903 VOSでは、削除されない場合がある。2019-10-18
		::DeleteFile(ptrPath);
		//ファイルが存在しない＝削除された。
		if (!::PathFileExists(ptrPath))
			return TRUE;

		CString strLog;
		strLog.Format(_T("##ChWin32API_DeleteFile FailBack: %s\n"), ptrPath);
		::OutputDebugString(strLog);

		//Windows 10 1903 VOSでは、削除されない場合がある。2019-10-18
		//IFileOperationで消す。
		BOOL bRet = FALSE;
		CStringW strDeleteFilePath(ptrPath);
		HRESULT hr = {0};
		CComPtr<IFileOperation> pfo;
		hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));
		if (SUCCEEDED(hr))
		{
			hr = pfo->SetOperationFlags(FOF_FILESONLY | FOF_NO_CONNECTED_ELEMENTS | FOF_NORECURSEREPARSE | FOF_NOCONFIRMMKDIR | FOF_NO_UI);
			if (SUCCEEDED(hr))
			{
				CComPtr<IShellItem> psiDelete;
				hr = SHCreateItemFromParsingName(strDeleteFilePath, NULL, IID_PPV_ARGS(&psiDelete));
				if (SUCCEEDED(hr))
				{
					hr = pfo->DeleteItems(psiDelete);
				}
				if (SUCCEEDED(hr))
				{
					hr = pfo->PerformOperations();
					if (SUCCEEDED(hr))
					{
						bRet = TRUE;
						//削除成功したのに存在している。。。
						if (::PathFileExists(ptrPath))
						{
							strLog.Format(_T("##ChWin32API_DeleteFile(FAILED) FailBack: %s\n"), ptrPath);
							::OutputDebugString(strLog);
						}
						else
						{
							strLog.Format(_T("##ChWin32API_DeleteFile(SUCCESS) FailBack: %s\n"), ptrPath);
							::OutputDebugString(strLog);
						}
					}
				}
			}
		}
		return bRet;
	}

	DECLARE_MESSAGE_MAP()
};

class CAboutDlg : public CDialog
{
public:
	CAboutDlg(CWnd* pParent = NULL);
	CWnd* pParentFrm;
	enum
	{
		IDD = IDD_ABOUTBOX
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void SetDetailString(BOOL bEnableDetail);
	afx_msg void OnBnClickedOk();
	afx_msg void OnCloseVOSProc();
	afx_msg void OnDetailShow();
	afx_msg void OnBnClickedShowDevTools();
	afx_msg void OnBnClickedCefVersion();
};

extern CSazabi theApp;
