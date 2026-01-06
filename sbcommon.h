#pragma once
#include "resource.h"
#include "comdef.h"
#include "fav.h"
#include "locale.h"
#include <sddl.h>
#include <VersionHelpers.h>
#include <strsafe.h>
#include "include/cef_version.h"

#include "mmsystem.h"
#include <pathcch.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Pathcch.lib")
class CProcessingTimeMon
{
public:
	CProcessingTimeMon(LPCTSTR lpFmt)
	{
		m_timeCount = 0;
		strFmt = lpFmt;
		m_timeCount = timeGetTime();
	}
	~CProcessingTimeMon()
	{
		DWORD timeCount = timeGetTime() - m_timeCount;
		if (timeCount > 20) //20ms以上
		{
			CString strMsg;
			strMsg.Format(_T("#\t%d ms\t%s\n"), timeCount, (LPCTSTR)strFmt);
			::OutputDebugString(strMsg);
		}
	}

private:
	DWORD m_timeCount;
	CString strFmt;
};
#define PROC_TIME(name) CProcessingTimeMon CPTM##name(_T(#name));
#define PROC_TIME_S(name) \
	{                 \
		CProcessingTimeMon CPTM##name(_T(#name));
#define PROC_TIME_E(name) }

#pragma warning(disable : 4996)

#define WM_NEWINSTANCE       WM_USER + 16
#define WM_ACTIVE_HEAD       WM_USER + 40
#define WM_ACTIVE_TAIL       WM_USER + 44
#define WM_SAVE_WND_M        WM_USER + 48
#define WM_AUTH_DBL          WM_USER + 56
#define WM_ACTIVE_FRM        WM_USER + 60
#define WM_SCHEDULE_CEF_WORK WM_USER + 61

#define WND_TYPE_DEV_TOOLS 324929

///////////////////////////////////////////////////////////////////
//アプリ名を変更する場合は、以下を編集する。
static TCHAR gstrThisAppNameR[] = _T("Chronos");
static TCHAR gstrThisAppNameSG[] = _T("Chronos SystemGuard");
static TCHAR sgSZB_UA_START[] = _T("Mozilla/5.0 (");

// clang-format off
#define SB_CHROME_VERSION MAKE_STRING(CHROME_VERSION_MAJOR) "." \
                          MAKE_STRING(CHROME_VERSION_MINOR) "." \
                          MAKE_STRING(CHROME_VERSION_BUILD) "." \
                          MAKE_STRING(CHROME_VERSION_PATCH)
// clang-format on

//2021-01-07Googleにログインできない CEF経由ではNGになった。
//調査結果、FirefoxにすればOK, Edge/87.0.0.0をつけてもOK
//デフォルトのUAをEdgeに変更する対応にする。
//2021-11-30 ↑の対策がNGになっていることに気がついた。UAにEdgeをつけてもNG
static TCHAR sgSZB_UA_END[] = _T(") AppleWebKit/537.36 (KHTML, like Gecko;KA-ZUMA) Chrome/" SB_CHROME_VERSION " Safari/537.36 Chronos/SystemGuard");
#undef SB_CHROME_VERSION

typedef HRESULT(WINAPI* pfnDwmIsCompositionEnabled)(BOOL* pfEnabled);
typedef HRESULT(WINAPI* pfnDwmGetWindowAttribute)(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);

#define WM_VIEW_INIT_OK	      36002
#define WM_DELETE_WINDOW_LIST 36004
//#define WM_CLOSE_DELAY 36009
#define WM_NEW_WINDOW_URL     36011
#define WM_CLOSE_TIME_LIMIT   36014
#define WM_CLOSE_MAX_MEM      36015
#define WM_CLOSE_WAR_MEM      36016
#define WM_ADD_FAVORITE	      36017
#define WM_APP_EXIT	      36019
#define WM_SEL_SEARCH	      36020
#define WM_COPY_IMAGE	      36021
#define ID_MYCOMBO_OK	      33801
#define ID_FAV_START	      40000
#define ID_FAV_END	      45000
#define ID_WINDOW_START	      50000
#define ID_WINDOW_END	      55000
#define ID_CLOSE_WINDOW_START 45001
#define ID_CLOSE_WINDOW_END   45016

#define DEBUG_LOG_LEVEL_OUTPUT_ALL     0
#define DEBUG_LOG_LEVEL_OUTPUT_NO_FILE 1
#define DEBUG_LOG_LEVEL_OUTPUT_URL     2

#define DEBUG_LOG_TYPE_GE  0 //一般情報
#define DEBUG_LOG_TYPE_DE  1 //詳細情報
#define DEBUG_LOG_TYPE_URL 2 //URL情報
#define DEBUG_LOG_TYPE_TR  3 //Browser動作情報
#define DEBUG_LOG_TYPE_CL  4 //Close処理関連情報
#define DEBUG_LOG_TYPE_JS  5 //Javascript関連情報
#define DEBUG_LOG_TYPE_EX  6 //例外処理関連情報
#define DEBUG_LOG_TYPE_AC  7 //操作アクション情報

//IE設定:0
//プロキシ無し(直接):1
//手動設定:2
#define CSG_PROXY_IE 0
#define CSG_PROXY_NA 1
#define CSG_PROXY_TF 2

//VirtualEnv
#define VE_NA	   0
#define VE_THINAPP 1
#define VE_TURBO   2

//RDS
#define RDS_NA	   0
#define RDS_RDP	   1
#define RDS_VMWARE 2
#define RDS_CITRIX 3

#define TF_ALLOW 0
#define TF_DENY	 1

static TCHAR sDEBUG_LOG_TYPE[][4] = {
    _T("GE"),
    _T("DE"),
    _T("URL"),
    _T("TR"),
    _T("CL"),
    _T("JS"),
    _T("EX"),
    _T("AC"),
};

///////////////////////////////////////////////////////////////////
static TCHAR DEF_URLS[][9] = {
    _T("http://"),
    _T("https://"),
    _T("about:"),
    _T("file://"),
    _T("mailto:"),
    _T("chrome:"),
};
static TCHAR DEF_FILE[][8] = {
    _T("file://"),
    _T("\\\\"),
};

static TCHAR DEF_SCRIPT[][11] = {
    _T("javascript"),
    _T("script"),
};

static TCHAR DEF_ETC_PROTOCOLS[][13] = {
    _T("mailto:"),
    _T("vmware-view:"),
    _T("notes:"),
};

#define KEY_COMB_SHIFT 0x00000001
#define KEY_COMB_CTRL  0x00000010
#define KEY_COMB_ALT   0x00000100
#define KEY_COMB_LEFT  0x00001000
#define KEY_COMB_UP    0x00010000
#define KEY_COMB_RIGHT 0x00100000
#define KEY_COMB_DOWN  0x01000000

// ファイル時間から64ビット整数に変換
static ULONGLONG getTimeInt64(LPFILETIME ftTime)
{
	ULARGE_INTEGER u64Time = {0};
	u64Time.u.LowPart = ftTime->dwLowDateTime;
	u64Time.u.HighPart = ftTime->dwHighDateTime;
	return u64Time.QuadPart;
}

// ファイル時間の加算
static LPFILETIME addFileTime(LPFILETIME ftTime1, LPFILETIME ftTime2)
{
	static FILETIME ftTime = {0};
	ULARGE_INTEGER u64Time1 = {0};
	ULARGE_INTEGER u64Time2 = {0};

	// 1セット
	u64Time1.u.LowPart = ftTime1->dwLowDateTime;
	u64Time1.u.HighPart = ftTime1->dwHighDateTime;
	// 2セット
	u64Time2.u.LowPart = ftTime2->dwLowDateTime;
	u64Time2.u.HighPart = ftTime2->dwHighDateTime;
	// 加算
	u64Time1.QuadPart += u64Time2.QuadPart;
	// 変換
	ftTime.dwLowDateTime = u64Time1.u.LowPart;
	ftTime.dwHighDateTime = u64Time1.u.HighPart;
	return &ftTime;
}

// ファイル時間から時間文字列に変換
static void getTimeString(LPFILETIME ftTime, BOOL bLocalTime, CString& str)
{
	str.Empty();
	FILETIME ltTime = {0};
	SYSTEMTIME stTime = {0};

	if (bLocalTime)
	{
		FileTimeToLocalFileTime(ftTime, &ltTime);
	}
	else
	{
		ltTime = *ftTime;
	}
	FileTimeToSystemTime(&ltTime, &stTime);
	str.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"),
		   stTime.wYear,
		   stTime.wMonth,
		   stTime.wDay,
		   stTime.wHour,
		   stTime.wMinute,
		   stTime.wSecond);
	return;
}

static void getTimeStringEx(LPFILETIME ftTime, CString& str)
{
	str.Empty();
	FILETIME ltTime = {0};
	SYSTEMTIME stTime = {0};

	ltTime = *ftTime;
	FileTimeToSystemTime(&ltTime, &stTime);
	str.Format(_T("%02d:%02d:%02d (BASE%04d-%02d-%02d )"),
		   stTime.wHour,
		   stTime.wMinute,
		   stTime.wSecond,
		   stTime.wYear,
		   stTime.wMonth,
		   stTime.wDay);
	return;
}

/////////////////////////////////////////////////////////////////////
#include <winternl.h>
#include <tlhelp32.h>
#include <locale.h>
#pragma comment(lib, "psapi.lib")
#include <psapi.h>
namespace SBUtil
{
	enum class ICON_INDEX
	{
		FOLDER,
		IE_FILE,
		WARNING,
		IE_FILE_FAV,
		LOCK,
		BLANK,
		ZOOM,
		NEW,
		NEW_I,
		RESTORE,
		SAVE_WND,
		CLOSE,
		CUT,
		COPY,
		PASTE,
		SEL_ALL,
		FIND_PAGE,
		IE_OPTION,
		SETTINGS,
		CLOSE_ALL,
		CLOSE_BUT_THIS,
		PREV_WND,
		NEXT_WND,
		CASCADE_WND,
		TILE_V_WND,
		TILE_H_WND,
		MAXIMIZE_V_WND,
		MAXIMIZE_H_WND,
		MINIMIZE_ALL,
		FULL_SCREEN,
		APP_ABOUT,
		TOPMOST_WND,
		TOPMOST_REL_WND,
		ADD_FAVORITE,
		ORGANIZE_FAVORITE,
		GO,
		PASTE_GO,
		PASTE_SEARCH,
		SEARCH,

	};

	static inline void Split(CStringArray* pstrArray, LPCTSTR szTarget, LPCTSTR szDelimiter)
	{
		if (!szTarget)
			return;
		if (!szDelimiter)
			return;
		if (!pstrArray)
			return;

		pstrArray->RemoveAll();

		CString strTarget(szTarget);
		int intDelimiterLen = 0;
		int intStart = 0;
		int intEnd = 0;

		strTarget += szDelimiter;
		intDelimiterLen = (int)_tcslen(szDelimiter);
		intStart = 0;
		while (intEnd = strTarget.Find(szDelimiter, intStart), intEnd >= 0)
		{
			pstrArray->Add(strTarget.Mid(intStart, intEnd - intStart));
			intStart = intEnd + intDelimiterLen;
		}
		return;
	}
	static inline BOOL IsURL(LPCTSTR str)
	{
		if (str == NULL)
			return FALSE;
		CString strCheckStr = str;
		strCheckStr.MakeLower();
		if (strCheckStr.Find(DEF_URLS[0]) == 0 ||
		    strCheckStr.Find(DEF_URLS[1]) == 0 ||
		    strCheckStr.Find(DEF_URLS[2]) == 0 ||
		    strCheckStr.Find(DEF_URLS[3]) == 0 ||
		    strCheckStr.Find(DEF_URLS[4]) == 0 ||
		    strCheckStr.Find(DEF_URLS[5]) == 0)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
		return FALSE;
	}

	static inline BOOL IsURL_HTTP(LPCTSTR str)
	{
		if (str == NULL)
			return FALSE;
		CString strCheckStr = str;
		if (strCheckStr.Find(DEF_URLS[0]) == 0 || //http
		    strCheckStr.Find(DEF_URLS[1]) == 0	  //https
		)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
		return FALSE;
	}
	static inline BOOL IsURL_HTTPS(LPCTSTR str)
	{
		if (str == NULL)
			return FALSE;
		CString strCheckStr = str;
		if (strCheckStr.Find(DEF_URLS[1]) == 0) //https
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
		return FALSE;
	}
	static inline BOOL IsURL_FILE(LPCTSTR str)
	{
		BOOL bRet = FALSE;
		if (str == NULL)
			return FALSE;
		CString strCheckStr = str;
		if (strCheckStr.Find(DEF_FILE[0]) == 0 ||
		    strCheckStr.Find(DEF_FILE[1]) == 0)
		{
			bRet = TRUE;
		}
		else
		{
			//2文字目が:は、パス(c:)
			if (strCheckStr.Find(_T(":")) == 1)
				bRet = TRUE;
		}
		return bRet;
	}
	inline static BOOL IsScript(LPCTSTR strURL)
	{
		if (strURL == NULL)
			return FALSE;
		CString strCheckStr = strURL;
		strCheckStr.MakeLower();
		if (strCheckStr.Find(DEF_SCRIPT[0]) == 0 ||
		    strCheckStr.Find(DEF_SCRIPT[1]) == 0)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
		return FALSE;
	}
	static inline BOOL IsETC_PROTOCOL(LPCTSTR str)
	{
		if (str == NULL)
			return FALSE;

		//	_T("mailto:"),
		//	_T("vmware-view:"),
		//  _T("notes:").
		CString strCheckStr = str;
		if (strCheckStr.Find(DEF_ETC_PROTOCOLS[0]) == 0 ||
		    strCheckStr.Find(DEF_ETC_PROTOCOLS[1]) == 0 ||
		    strCheckStr.Find(DEF_ETC_PROTOCOLS[2]) == 0)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
		return FALSE;
	}

	static inline CString Trim_URLOnly(LPCTSTR str)
	{
		CString strRet;
		CString strTemp;
		CString strTemp2;
		strTemp = str;
		if (!strTemp.IsEmpty())
		{
			int iQPos = 0;
			int iShPos = 0;
			iQPos = strTemp.Find(_T("?"));
			if (iQPos > -1)
			{
				strTemp2 = strTemp.Mid(0, iQPos);
				strTemp = strTemp2;
			}
			iShPos = strTemp.Find(_T("#"));
			if (iShPos > -1)
			{
				strTemp2 = strTemp.Mid(0, iShPos);
				strTemp = strTemp2;
			}
			strRet = strTemp;
		}
		return strRet;
	}

	static void GetDivChar(LPCTSTR str, int size, CString& strRet, BOOL bAppend = TRUE)
	{
		strRet = str;
		CString str1 = str;
		int iTabStrLen = size;
		LPCTSTR szEllipsis = _T("...");
		if (strRet.GetLength() > iTabStrLen)
		{
			if (bAppend)
				strRet = str1.Mid(0, iTabStrLen) + szEllipsis;
			else
				strRet = str1.Mid(0, iTabStrLen);
		}
		return;
	}

	static BOOL Is64BitWindows()
	{
#if defined(_WIN64)
		return TRUE; // 64-bit programs run only on Win64
#elif defined(_WIN32)
		// 32-bit programs run on both 32-bit and 64-bit Windows
		// so must sniff
		BOOL f64 = FALSE;
		LPFN_ISWOW64PROCESS fnIsWow64Process = {0};
		HMODULE hModule = GetModuleHandle(_T("kernel32.dll"));
		if (!hModule)
			return FALSE;
		fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(hModule, "IsWow64Process");
		if (NULL != fnIsWow64Process)
			return fnIsWow64Process(GetCurrentProcess(), &f64) && f64;
		return FALSE;
#else
		return FALSE; // Win64 does not support Win16
#endif
	}

	static inline BOOL IsWindowsServerRDS()
	{
		return IsWindowsServer() && GetSystemMetrics(SM_REMOTESESSION);
	}

	static inline void SafeGetWindowText(HWND hWnd, CString& str)
	{
		CString strText;
		int nLen = ::GetWindowTextLength(hWnd);
		int nRetLen = ::GetWindowText(hWnd, strText.GetBufferSetLength(nLen + 2), nLen + 2);
		strText.ReleaseBuffer();

		if (nRetLen < nLen)
		{
			str.Empty();
			return;
		}
		str = strText;
		return;
	}
	static inline void GetInternetShortcutUrl(LPCTSTR pszFolderURLFile, CString& strVal)
	{
		TCHAR szURL[4096] = {0};
		DWORD nSize = 4096;
		::GetPrivateProfileString(_T("InternetShortcut"), _T("URL"), NULL, szURL, nSize, pszFolderURLFile);
		strVal = szURL;
		if (!SBUtil::IsURL_HTTP(strVal))
		{
			strVal.Empty();
		}
		return;
	}

	static CString GetDownloadFolderPath()
	{
		CString strRet;
		TCHAR* path = 0;
		SHGetKnownFolderPath(FOLDERID_Downloads, 0, NULL, &path);
		strRet = path;
		CoTaskMemFree(path);
		return strRet;
	}
	static CString GetLocalAppDataPath()
	{
		CString strRet;
		TCHAR* path = 0;
		SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path);
		strRet = path;
		CoTaskMemFree(path);
		return strRet;
	}

	static CString GetValidFileName(LPCTSTR lpFileName)
	{
		CString strRet(lpFileName);
		strRet.TrimRight();
		strRet.TrimLeft();
		if (!strRet.IsEmpty())
		{
			//ファイル名に使えない文字を置き換える。
			strRet.Replace(_T("<"), _T("＜"));
			strRet.Replace(_T(">"), _T("＞"));
			strRet.Replace(_T(":"), _T("："));
			strRet.Replace(_T("\""), _T("”"));
			strRet.Replace(_T("/"), _T("-"));
			strRet.Replace(_T("\\"), _T("￥"));
			strRet.Replace(_T("|"), _T("｜"));
			strRet.Replace(_T("?"), _T("？"));
			strRet.Replace(_T("*"), _T("＊"));
		}
		return strRet;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static CString MyConvertSidToStringSid(PSID pSid)
	{
		LPTSTR p = NULL;
		BOOL br = ::ConvertSidToStringSid(pSid, &p);
		if (!br)
		{
			return _T("");
		}
		CString strResult = p;
		::LocalFree(p);
		return strResult;
	}
	static CString GetTokenStringSid(HANDLE hToken)
	{
		DWORD dwTokenUserLength = 0;
		::GetTokenInformation(hToken, TokenUser, 0, 0, &dwTokenUserLength);
		TOKEN_USER* pTokenUser = (TOKEN_USER*)::LocalAlloc(LMEM_FIXED, dwTokenUserLength);
		if (pTokenUser == NULL)
		{
			return _T("");
		}
		BOOL br = ::GetTokenInformation(hToken, TokenUser, pTokenUser, dwTokenUserLength, &dwTokenUserLength);
		if (!br)
		{
			::LocalFree(pTokenUser);
			return _T("");
		}
		CString strResult = MyConvertSidToStringSid(pTokenUser->User.Sid);
		::LocalFree(pTokenUser);
		return strResult;
	}
	static CString GetProcessStringSid(HANDLE hProcess)
	{
		HANDLE hToken = NULL;
		BOOL br = ::OpenProcessToken(hProcess, TOKEN_QUERY, &hToken);
		if (!br)
		{
			return _T("");
		}
		CString strResult = GetTokenStringSid(hToken);
		::CloseHandle(hToken);
		return strResult;
	}
	// 現在の(プロセスの)ユーザのSIDの取得
	static CString GetCurrentProcessStringSid()
	{
		HANDLE hProcess = ::GetCurrentProcess();
		return GetProcessStringSid(hProcess);
	}

	static bool GetMonitorWorkRectM(HMONITOR hMon, LPRECT prcWork, LPRECT prcMonitor = NULL)
	{
		MONITORINFO mi = {0};
		::ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		::GetMonitorInfo(hMon, &mi);
		if (NULL != prcWork)
			*prcWork = mi.rcWork;
		if (NULL != prcMonitor)
			*prcMonitor = mi.rcMonitor;
		return (mi.dwFlags == MONITORINFOF_PRIMARY) ? true : false;
	}
	static bool GetMonitorWorkRect(HWND hWnd, LPRECT prcWork, LPRECT prcMonitor = NULL)
	{
		HMONITOR hMon = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		return GetMonitorWorkRectM(hMon, prcWork, prcMonitor);
	}

	static bool GetMonitorWorkRectRC(LPCRECT prc, LPRECT prcWork, LPRECT prcMonitor = NULL)
	{
		HMONITOR hMon = ::MonitorFromRect(prc, MONITOR_DEFAULTTONEAREST);
		return GetMonitorWorkRectM(hMon, prcWork, prcMonitor);
	}

	static bool GetMonitorWorkRectPT(POINT pt, LPRECT prcWork, LPRECT prcMonitor = NULL)
	{
		HMONITOR hMon = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		return GetMonitorWorkRectM(hMon, prcWork, prcMonitor);
	}
	static void SetAbsoluteForegroundWindow(HWND hWnd, BOOL bEnforceActive)
	{
		int nTargetID, nForegroundID = 0;
		INT_PTR sp_time = 0;

		// フォアグラウンドウィンドウを作成したスレッドのIDを取得
		nForegroundID = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
		// 目的のウィンドウを作成したスレッドのIDを取得
		nTargetID = GetWindowThreadProcessId(hWnd, NULL);

		// スレッドのインプット状態を結び付ける
		AttachThreadInput(nTargetID, nForegroundID, TRUE); // TRUE で結び付け

		// 現在の設定を sp_time に保存
		SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &sp_time, 0);
		// ウィンドウの切り替え時間を 0ms にする
		SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)0, 0);

		if (bEnforceActive)
		{
			// ウィンドウをフォアグラウンドに持ってくる
			if (::IsIconic(hWnd))
			{
				ShowWindow(hWnd, SW_HIDE);
				ShowWindow(hWnd, SW_NORMAL);
			}
			else if (::IsZoomed(hWnd))
			{
				ShowWindow(hWnd, SW_HIDE);
				ShowWindow(hWnd, SW_MAXIMIZE);
			}
			else
			{
				ShowWindow(hWnd, SW_HIDE);
				ShowWindow(hWnd, SW_NORMAL);
			}
		}
		SetForegroundWindow(hWnd);

		// 設定を元に戻す
		SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)sp_time, 0);

		// スレッドのインプット状態を切り離す
		AttachThreadInput(nTargetID, nForegroundID, FALSE); // FALSE で切り離し
	}

	// Usually IsWindowsXXOrGreater version helper API should be used,
	// but there is no IsWindows11OrGreater API yet.
	// NOTE: GetVersionEx return information which depends on manifest,
	// so use RtlGetVersion API.
	static inline bool IsWindows11OrLater()
	{
		auto versionInfo = RTL_OSVERSIONINFOW{sizeof(RTL_OSVERSIONINFOW), 0, 0, 0, 0, 0};
		auto hModule = GetModuleHandle(L"ntdll.dll");
		if (!hModule)
		{
			return false;
		}

		typedef NTSTATUS(WINAPI * RtlGetVersionPtr)(PRTL_OSVERSIONINFOW lpVersionInformation);
		auto RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hModule, "RtlGetVersion");
		if (!RtlGetVersion)
		{
			return false;
		}
		if (RtlGetVersion(&versionInfo))
		{
			return false;
		}
		return (versionInfo.dwMajorVersion >= 11 || // for future version
			(versionInfo.dwMajorVersion == 10 && versionInfo.dwBuildNumber >= 22000)); // for Windows 11
	}

	/////////////////////////////////////////////////////////////////
	struct RTL_USER_PROCESS_PARAMETERS_I
	{
		BYTE Reserved1[16];
		PVOID Reserved2[10];
		UNICODE_STRING ImagePathName;
		UNICODE_STRING CommandLine;
	};
	struct PEB_INTERNAL
	{
		BYTE Reserved1[2];
		BYTE BeingDebugged;
		BYTE Reserved2[1];
		PVOID Reserved3[2];
		struct PEB_LDR_DATA* Ldr;
		RTL_USER_PROCESS_PARAMETERS_I* ProcessParameters;
		BYTE Reserved4[104];
		PVOID Reserved5[52];
		struct PS_POST_PROCESS_INIT_ROUTINE* PostProcessInitRoutine;
		BYTE Reserved6[128];
		PVOID Reserved7[1];
		ULONG SessionId;
	};
	typedef NTSTATUS(NTAPI* NtQueryInformationProcessPtr)(
	    IN HANDLE ProcessHandle,
	    IN PROCESSINFOCLASS ProcessInformationClass,
	    OUT PVOID ProcessInformation,
	    IN ULONG ProcessInformationLength,
	    OUT PULONG ReturnLength OPTIONAL);

	typedef ULONG(NTAPI* RtlNtStatusToDosErrorPtr)(NTSTATUS Status);
	static SIZE_T GetRemoteCommandLineW(HANDLE hProcess, LPWSTR pszBuffer, UINT bufferLength)
	{
		HINSTANCE hNtDll = GetModuleHandleW(L"ntdll.dll");
		if (!hNtDll)
			return 0;
		NtQueryInformationProcessPtr NtQueryInformationProcess = (NtQueryInformationProcessPtr)GetProcAddress(hNtDll, "NtQueryInformationProcess");
		RtlNtStatusToDosErrorPtr RtlNtStatusToDosError = (RtlNtStatusToDosErrorPtr)GetProcAddress(hNtDll, "RtlNtStatusToDosError");

		if (!NtQueryInformationProcess || !RtlNtStatusToDosError)
		{
			return 0;
		}

		// Get PROCESS_BASIC_INFORMATION
		PROCESS_BASIC_INFORMATION pbi = {0};
		ULONG len = 0;
		NTSTATUS status = NtQueryInformationProcess(
		    hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &len);
		SetLastError(RtlNtStatusToDosError(status));

		if (NT_ERROR(status) || !pbi.PebBaseAddress)
		{
			return 0;
		}

		// Read PEB memory block
		SIZE_T bytesRead = 0;
		PEB_INTERNAL peb = {0};
		if (!ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(peb), &bytesRead))
		{
			return 0;
		}

		// Obtain size of commandline string
		RTL_USER_PROCESS_PARAMETERS_I upp = {0};
		if (!ReadProcessMemory(hProcess, peb.ProcessParameters, &upp, sizeof(upp), &bytesRead))
		{
			return 0;
		}

		if (!upp.CommandLine.Length)
		{
			return 0;
		}

		// Check the buffer size
		DWORD dwNeedLength = (upp.CommandLine.Length + 1) / sizeof(wchar_t) + 1;
		if (bufferLength < dwNeedLength)
		{
			return dwNeedLength;
		}

		pszBuffer[dwNeedLength - 1] = L'\0';
		if (!ReadProcessMemory(hProcess, upp.CommandLine.Buffer, pszBuffer, upp.CommandLine.Length, &bytesRead))
		{
			return 0;
		}
		return bytesRead / sizeof(wchar_t);
	}
	static CString GetCommandLineData(DWORD dwPID)
	{
		CString strRet;

		if (dwPID == 0) return strRet;
		try
		{
			HANDLE processHandle = {0};
			std::unique_ptr<WCHAR> szCommandLine(new WCHAR[8192]());

			processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, dwPID);
			if (processHandle)
			{
				LPWSTR rawSzCommandLine = szCommandLine.get();
				GetRemoteCommandLineW(processHandle, rawSzCommandLine, 8192);
				CloseHandle(processHandle);
				CStringW strW(rawSzCommandLine);
				strW.TrimLeft();
				strW.TrimRight();
				if (!strW.IsEmpty())
					strRet = strW;
			}
		}
		catch (...)
		{
			ATLASSERT(0);
		}
		return strRet;
	}
	static CString GetFileExt(LPCTSTR lpPath)
	{
		CString strRet;
		if (!lpPath) return strRet;

		CString strParam(lpPath);
		TCHAR szExt[MAX_PATH] = {0};
		StringCchCopy(szExt, MAX_PATH, strParam);

		TCHAR* ptrExt = NULL;
		ptrExt = PathFindExtension(szExt);
		if (ptrExt)
		{
			strRet = ptrExt;
		}
		return strRet;
	}
	static CString ExpandEnvironmentStringsEx(LPCTSTR lpSrc)
	{
		CString strRet;
		if (!lpSrc)
		{
			return strRet;
		}
		DWORD dwSize = ExpandEnvironmentStrings(lpSrc, NULL, 0);
		if (dwSize == 0)
		{
			return lpSrc;
		}
		LPWSTR pBuf = strRet.GetBuffer(dwSize);
		dwSize = ExpandEnvironmentStrings(lpSrc, pBuf, dwSize);
		strRet.ReleaseBuffer();
		if (dwSize == 0)
		{
			return lpSrc;
		}
		return strRet;
	}


}; // namespace SBUtil
//////////////////////////////////////////////////////////////////////
static int wildcmp(const char* wild, const char* string)
{
	const char *cp = NULL, *mp = NULL;

	while ((*string) && (*wild != '*'))
	{
		if ((*wild != *string) && (*wild != '?'))
		{
			return 0;
		}
		wild++;
		string++;
	}

	while (*string)
	{
		if (*wild == '*')
		{
			if (!*++wild)
			{
				return 1;
			}
			mp = wild;
			cp = string + 1;
		}
		else if ((*wild == *string) || (*wild == '?'))
		{
			wild++;
			string++;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}

	while (*wild == '*')
	{
		wild++;
	}
	return !*wild;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXTVAL(name) _ExTxtVal(_T(#name), name);

static TCHAR REG_SETTING_KEY[] = _T("SOFTWARE\\ChronosSystemGuard");
class AppSettings
{
public:
	AppSettings()
	{
		m_IsSGMode = FALSE;
		Clear();
	}
	~AppSettings() {}

	enum class MediaAccessPermission
	{
		NO_MEDIA_ACCESS,
		MANUAL_MEDIA_APPROVAL,
		DEFAULT_MEDIA_APPROVAL
	};
	enum class EnumUploadLoggingURLType
	{
		LAST_BROWSED_URL,
		MAIN_FRAME,
		ACTIVE_FRAME
	};
	void Clear()
	{
		EnableAdvancedLogMode = 0;
		EnableAdvancedLogVerboseMode = 0;
		AdvancedLogLevel = 0;
		ShowLogo = 0;
		EnableTab = 0;
		EnablePDFExtension = 0;
		EnablePDFApplicationSelection = 0;
		EnableMemcache = 0;
		EnableGPURendering = 0;
		EnableCrashRecovery = 0;
		EnableMultipleInstance = 0;
		EnableRebar = 0;
		EnableStatusbar = 0;

		WideMargin = 0;
		HeightMargin = 0;
		DefaultZoomSize = 0;
		RedirectMsgTimeout = 0;
		KeyCombination = 0;
		ProxyType = CSG_PROXY_IE; //0

		StartURL.Empty();
		EnforceInitParam.Empty();
		CustomBrowser.Empty();
		CustomBrowser2.Empty();
		CustomBrowser3.Empty();
		CustomBrowser4.Empty();
		CustomBrowser5.Empty();
		InitMessage.Empty();
		CEFCommandLine.Empty();
		ProxyAddress.Empty();
		ProxyBypassAddress.Empty();
		UserAgentAppendStr.Empty();

		MemoryUsageLimit = 0;
		WindowCountLimit = 0;
		EnableMediaAccessPermission = static_cast<int>(AppSettings::MediaAccessPermission::NO_MEDIA_ACCESS);

		EnableDownloadRestriction = 0;
		EnableUploadRestriction = 0;
		EnableFileExtChangeRestriction = 0;
		EnableRunningTime = 0;
		RunningLimitTime = 0;
		EnableURLRedirect = 0;
		EnableURLFilter = 0;
		EnablePopupFilter = 0;
		EnableCustomScript = 0;

		EnableLogging = 0;
		EnableUploadLogging = 0;
		UploadLoggingURLType = 0;
		EnableDownloadLogging = 0;
		EnableBrowsingLogging = 0;
		EnableAccessAllLogging = 0;

		LogServerURL.Empty();
		LogMethod.Empty();
		RequestHeader.Empty();

		//ChFiler---------------------------------
		RootPath.Empty();
		UploadBasePath.Empty();
		ExtFilter.Empty();
		EnableOpenedOp = 0;
		EnableTransferLog = 0;
		EnableUploadSync = 0;
		EnableUploadSyncMirror = 0;
		UploadSyncInterval = 0;
		EnableAutoTransfer = 0;
		DisableOpenedOpAlert = 0;
		DisableExitOpAlert = 0;
		ConfirmAutoRefresh = 0;
		TransferPath.Empty();
		TransferSubFolder.Empty();
		ShowUploadTab = 0;
		UploadPath.Empty();
		DisallowExt.Empty();
		DisallowTransferringExt.Empty();
		unZipMessage.Empty();
		ExitMessage.Empty();

		//ChTaskMGR---------------------------------
		LABEL_TYPE = 0;
		LABEL_CHK_INTERVAL = 0;
		LABEL_ALPHA_BLEND = 0;
		TASK_LIST_TYPE = 0;
		TASK_LIST_MODE_DETAIL = 0;

		//Config file-------------------------------
		EnableUserConfig = 0;
	}
	void CopyData(AppSettings& Data)
	{
		Data.EnableAdvancedLogMode = EnableAdvancedLogMode;
		Data.EnableAdvancedLogVerboseMode = EnableAdvancedLogVerboseMode;
		Data.AdvancedLogLevel = AdvancedLogLevel;
		Data.ShowLogo = ShowLogo;
		Data.EnableTab = EnableTab;
		Data.EnablePDFExtension = EnablePDFExtension;
		Data.EnablePDFApplicationSelection = EnablePDFApplicationSelection;
		Data.EnableMemcache = EnableMemcache;
		Data.EnableGPURendering = EnableGPURendering;
		Data.EnableCrashRecovery = EnableCrashRecovery;
		Data.EnableMultipleInstance = EnableMultipleInstance;

		Data.ProxyType = ProxyType;
		Data.EnableRebar = EnableRebar;
		Data.EnableStatusbar = EnableStatusbar;

		Data.WideMargin = WideMargin;
		Data.HeightMargin = HeightMargin;
		Data.DefaultZoomSize = DefaultZoomSize;
		Data.RedirectMsgTimeout = RedirectMsgTimeout;
		Data.KeyCombination = KeyCombination;
		Data.StartURL = StartURL;
		Data.EnforceInitParam = EnforceInitParam;
		Data.CustomBrowser = CustomBrowser;
		Data.CustomBrowser2 = CustomBrowser2;
		Data.CustomBrowser3 = CustomBrowser3;
		Data.CustomBrowser4 = CustomBrowser4;
		Data.CustomBrowser5 = CustomBrowser5;
		Data.InitMessage = InitMessage;
		Data.CEFCommandLine = CEFCommandLine;
		Data.ProxyAddress = ProxyAddress;
		Data.ProxyBypassAddress = ProxyBypassAddress;
		Data.UserAgentAppendStr = UserAgentAppendStr;

		Data.MemoryUsageLimit = MemoryUsageLimit;
		Data.WindowCountLimit = WindowCountLimit;

		Data.EnableDownloadRestriction = EnableDownloadRestriction;
		Data.EnableUploadRestriction = EnableUploadRestriction;
		Data.EnableFileExtChangeRestriction = EnableFileExtChangeRestriction;
		Data.EnableRunningTime = EnableRunningTime;
		Data.RunningLimitTime = RunningLimitTime;
		Data.EnableURLRedirect = EnableURLRedirect;
		Data.EnableURLFilter = EnableURLFilter;
		Data.EnablePopupFilter = EnablePopupFilter;
		Data.EnableCustomScript = EnableCustomScript;

		Data.EnableLogging = EnableLogging;
		Data.EnableUploadLogging = EnableUploadLogging;
		Data.UploadLoggingURLType = UploadLoggingURLType;
		Data.EnableDownloadLogging = EnableDownloadLogging;
		Data.EnableBrowsingLogging = EnableBrowsingLogging;
		Data.EnableAccessAllLogging = EnableAccessAllLogging;

		Data.LogServerURL = LogServerURL;
		Data.RequestHeader = RequestHeader;
		Data.LogMethod = LogMethod;

		//ChFiler---------------------------------
		Data.RootPath = RootPath;
		Data.UploadBasePath = UploadBasePath;
		Data.ExtFilter = ExtFilter;
		Data.EnableOpenedOp = EnableOpenedOp;
		Data.EnableTransferLog = EnableTransferLog;
		Data.EnableUploadSync = EnableUploadSync;
		Data.EnableUploadSyncMirror = EnableUploadSyncMirror;
		Data.UploadSyncInterval = UploadSyncInterval;
		Data.EnableAutoTransfer = EnableAutoTransfer;
		Data.DisableOpenedOpAlert = DisableOpenedOpAlert;
		Data.DisableExitOpAlert = DisableExitOpAlert;
		Data.ConfirmAutoRefresh = ConfirmAutoRefresh;
		Data.TransferPath = TransferPath;
		Data.TransferSubFolder = TransferSubFolder;
		Data.ShowUploadTab = ShowUploadTab;
		Data.UploadPath = UploadPath;
		Data.DisallowExt = DisallowExt;
		Data.DisallowTransferringExt = DisallowTransferringExt;
		Data.unZipMessage = unZipMessage;
		Data.ExitMessage = ExitMessage;

		//ChTaskMGR---------------------------------
		Data.LABEL_TYPE = LABEL_TYPE;
		Data.LABEL_CHK_INTERVAL = LABEL_CHK_INTERVAL;
		Data.LABEL_ALPHA_BLEND = LABEL_ALPHA_BLEND;
		Data.TASK_LIST_TYPE = TASK_LIST_TYPE;
		Data.TASK_LIST_MODE_DETAIL = TASK_LIST_MODE_DETAIL;

		//Config file-------------------------------
		Data.EnableUserConfig = EnableUserConfig;
	}

private:
	//全般設定
	int EnableMultipleInstance;
	int EnableMemcache;
	int EnableCrashRecovery;
	int KeyCombination;

	//画面表示設定
	int EnableTab;
	int EnablePDFExtension;
	int EnablePDFApplicationSelection;

	int ShowLogo;
	int EnableGPURendering;
	int EnableRebar;
	int EnableStatusbar;
	int WideMargin;
	int HeightMargin;
	int DefaultZoomSize;

	//起動関連設定
	CString StartURL;
	CString EnforceInitParam;
	CString InitMessage;
	CString CEFCommandLine;

	//インターネット接続設定
	int ProxyType;
	CString ProxyAddress;
	CString ProxyBypassAddress;
	CString UserAgentAppendStr;

	//制限設定
	int EnableDownloadRestriction;
	int EnableUploadRestriction;
	int EnableFileExtChangeRestriction;
	int EnableRunningTime;
	int RunningLimitTime;
	int MemoryUsageLimit;
	int WindowCountLimit;
	int EnableMediaAccessPermission;

	//リダイレクト設定
	int EnableURLRedirect;
	int RedirectMsgTimeout;
	CString CustomBrowser;
	CString CustomBrowser2;
	CString CustomBrowser3;
	CString CustomBrowser4;
	CString CustomBrowser5;

	//URLフィルター設定
	int EnableURLFilter;

	//ポップアップフィルター設定
	int EnablePopupFilter;

	//CustomScript設定
	int EnableCustomScript;

	//ログ出力設定
	int EnableAdvancedLogMode;
	int EnableAdvancedLogVerboseMode;
	int AdvancedLogLevel;
	//////////////////////////////////////////////////
	int EnableLogging;
	int EnableUploadLogging;
	int UploadLoggingURLType;
	int EnableDownloadLogging;
	int EnableBrowsingLogging;
	int EnableAccessAllLogging;
	CString LogServerURL;
	CString LogMethod;
	CString RequestHeader;
	//////////////////////////////////////////////////

	//ChFiler---------------------------------
	CString RootPath;
	CString ExtFilter;
	int EnableOpenedOp;
	int EnableTransferLog;
	int DisableOpenedOpAlert;
	int DisableExitOpAlert;
	int ConfirmAutoRefresh;
	CString TransferPath;
	CString TransferSubFolder;
	int ShowUploadTab;
	CString UploadPath;
	CString DisallowExt;
	CString DisallowTransferringExt;
	CString unZipMessage;
	CString ExitMessage;
	int EnableUploadSync;
	int EnableUploadSyncMirror;
	int UploadSyncInterval;
	int EnableAutoTransfer;
	CString UploadBasePath;
	//ChTaskMGR---------------------------------
	int LABEL_TYPE;
	int LABEL_CHK_INTERVAL;
	int LABEL_ALPHA_BLEND;
	int TASK_LIST_TYPE;
	int TASK_LIST_MODE_DETAIL;
	//Config file-------------------------------
	int EnableUserConfig;


public:
	//SystemGuardModeの判定用
	BOOL m_IsSGMode;

	inline void LoadDefaultData()
	{
		Clear();
		/////////////////////////////////////////////////////////////////////////////////////////////////
		//全般設定
		EnableMultipleInstance = FALSE;
		EnableMemcache = FALSE;
		EnableCrashRecovery = FALSE;
		KeyCombination = 0;

		/////////////////////////////////////////////////////////////////////////////////////////////////
		//画面表示設定
		EnableTab = TRUE;
		EnableGPURendering = TRUE;
		EnablePDFExtension = TRUE;
		EnablePDFApplicationSelection = FALSE;
		EnableRebar = TRUE;
		ShowLogo = TRUE;
		EnableStatusbar = TRUE;
		WideMargin = 25;
		HeightMargin = 48;
		DefaultZoomSize = 100;

		/////////////////////////////////////////////////////////////////////////////////////////////////
		//起動関連設定
		StartURL = _T("https://www.google.co.jp/");
		EnforceInitParam = _T("");
		InitMessage = _T("");
		CEFCommandLine = _T("");

		/////////////////////////////////////////////////////////////////////////////////////////////////
		//インターネット接続設定
		ProxyType = CSG_PROXY_IE;
		ProxyAddress = _T("");
		ProxyBypassAddress = _T("");
		UserAgentAppendStr = _T("");

		/////////////////////////////////////////////////////////////////////////////////////////////////
		//制限設定
		EnableDownloadRestriction = FALSE;
		EnableUploadRestriction = FALSE;
		EnableFileExtChangeRestriction = FALSE;

		EnableRunningTime = FALSE;
		RunningLimitTime = 1440;
		MemoryUsageLimit = 2048;
		WindowCountLimit = 60;
		EnableMediaAccessPermission = static_cast<int>(AppSettings::MediaAccessPermission::NO_MEDIA_ACCESS);

		/////////////////////////////////////////////////////////////////////////////////////////////////
		//リダイレクト設定
		EnableURLRedirect = FALSE;
		RedirectMsgTimeout = 2500;
		CustomBrowser = _T("");
		CustomBrowser2 = _T("");
		CustomBrowser3 = _T("");
		CustomBrowser4 = _T("");
		CustomBrowser5 = _T("");

		/////////////////////////////////////////////////////////////////////////////////////////////////
		//URLフィルター設定
		EnableURLFilter = FALSE;

		/////////////////////////////////////////////////////////////////////////////////////////////////
		// ポップアップフィルター設定
		EnablePopupFilter = FALSE;

		/////////////////////////////////////////////////////////////////////////////////////////////////
		//CustomScript設定
		EnableCustomScript = FALSE;

		/////////////////////////////////////////////////////////////////////////////////////////////////
		//ログ出力設定
		EnableAdvancedLogMode = FALSE;
		EnableAdvancedLogVerboseMode = FALSE;
		AdvancedLogLevel = 0;
		EnableLogging = 0;
		EnableUploadLogging = 0;
		UploadLoggingURLType = static_cast<int>(AppSettings::EnumUploadLoggingURLType::MAIN_FRAME);
		EnableDownloadLogging = 0;
		EnableBrowsingLogging = 0;
		EnableAccessAllLogging = 0;

		LogServerURL.Empty();
		LogMethod.Empty();
		RequestHeader.Empty();

		//ChFiler---------------------------------
		RootPath = _T("B:\\");
		UploadBasePath = _T("O:\\");
		ExtFilter = _T(".*");
		EnableOpenedOp = 1;
		EnableTransferLog = 0;
		EnableUploadSync = 0;
		EnableUploadSyncMirror = 0;
		UploadSyncInterval = 5000;
		EnableAutoTransfer = 0;
		DisableOpenedOpAlert = 0;
		DisableExitOpAlert = 2;
		ConfirmAutoRefresh = 2;
		TransferPath = _T("");
		TransferSubFolder = _T("%Y-%m-%d %H-%M-%S");
		ShowUploadTab = 1;
		UploadPath = _T("");
		DisallowExt = _T("");
		DisallowTransferringExt = _T("");
		unZipMessage = _T("");
		ExitMessage = _T("");

		//ChTaskMGR---------------------------------
		LABEL_TYPE = 1;
		LABEL_CHK_INTERVAL = 300;
		LABEL_ALPHA_BLEND = 196;
		TASK_LIST_TYPE = 1;
		TASK_LIST_MODE_DETAIL = 1;

		//Config file-------------------------------
		EnableUserConfig = 1;
	}

	BOOL SaveDataToFileEx(LPCTSTR pstrFilePath)
	{
		if (!pstrFilePath)
			return FALSE;
		CString strWriteData;
		strWriteData = ExportTxt();
		_wsetlocale(LC_ALL, _T("jpn"));
		CStdioFile out;
		if (out.Open(pstrFilePath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone))
		{
			out.WriteString(strWriteData);
			out.Close();
			return TRUE;
		}
		return FALSE;
	}
	BOOL LoadDataFromFile(LPCTSTR pstrFilePath)
	{
		if (!pstrFilePath)
			return FALSE;

		//Clearはしない。初期値をベースにファイルの内容で上書きする。
		_wsetlocale(LC_ALL, _T("jpn"));
		CStdioFile in;
		CString strTemp;
		if (!in.Open(pstrFilePath, CFile::modeRead | CFile::shareDenyWrite))
			return FALSE;

		CString strTemp2;
		CString strTemp3;
		CStringArray strArray;
		boolean bEnabledMediaAccessPermission = false;
		while (in.ReadString(strTemp))
		{
			strTemp.TrimLeft();
			strTemp.TrimRight();
			if (strTemp.IsEmpty())
				continue;
			strArray.RemoveAll();
			SBUtil::Split(&strArray, strTemp, _T("="));
			if (strArray.GetSize() >= 2)
			{
				strTemp2 = strArray.GetAt(0);
				strTemp2.TrimLeft();
				strTemp2.TrimRight();
				if (strTemp2.IsEmpty())
					continue;

				strTemp3 = strTemp.Mid(strArray.GetAt(0).GetLength() + 1);
				strTemp3.TrimLeft();
				strTemp3.TrimRight();

				if (strTemp2.Find(_T(";")) == 0)
					continue;
				if (strTemp2.Find(_T("#")) == 0)
					continue;

				//String系は、空白strTemp3を認める。//////////////////////////////////////////////////////////
				if (strTemp2.CompareNoCase(_T("StartURL")) == 0)
				{
					StartURL = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnforceInitParam")) == 0)
				{
					EnforceInitParam = strTemp3;
					continue;
				}

				if (strTemp2.CompareNoCase(_T("ProxyAddress")) == 0)
				{
					ProxyAddress = strTemp3;
					continue;
				}

				if (strTemp2.CompareNoCase(_T("ProxyBypassAddress")) == 0)
				{
					ProxyBypassAddress = strTemp3;
					continue;
				}

				if (strTemp2.CompareNoCase(_T("UserAgentAppendStr")) == 0)
				{
					UserAgentAppendStr = strTemp3;
					continue;
				}

				if (strTemp2.CompareNoCase(_T("CustomBrowser")) == 0)
				{
					CustomBrowser = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("CustomBrowser2")) == 0)
				{
					CustomBrowser2 = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("CustomBrowser3")) == 0)
				{
					CustomBrowser3 = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("CustomBrowser4")) == 0)
				{
					CustomBrowser4 = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("CustomBrowser5")) == 0)
				{
					CustomBrowser5 = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("InitMessage")) == 0)
				{
					InitMessage = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("CEFCommandLine")) == 0)
				{
					CEFCommandLine = strTemp3;
					continue;
				}
				/////////////////////////////////////////////////////////
				if (strTemp2.CompareNoCase(_T("RootPath")) == 0)
				{
					RootPath = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("UploadBasePath")) == 0)
				{
					UploadBasePath = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("ExtFilter")) == 0)
				{
					ExtFilter = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("TransferPath")) == 0)
				{
					TransferPath = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("TransferSubFolder")) == 0)
				{
					TransferSubFolder = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("UploadPath")) == 0)
				{
					UploadPath = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("DisallowExt")) == 0)
				{
					DisallowExt = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("DisallowTransferringExt")) == 0)
				{
					DisallowTransferringExt = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("unZipMessage")) == 0)
				{
					unZipMessage = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("ExitMessage")) == 0)
				{
					ExitMessage = strTemp3;
					continue;
				}

				if (strTemp2.CompareNoCase(_T("LogServerURL")) == 0)
				{
					LogServerURL = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("RequestHeader")) == 0)
				{
					RequestHeader = strTemp3;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("LogMethod")) == 0)
				{
					LogMethod = strTemp3;
					if (LogMethod.CompareNoCase(_T("GET")) == 0)
						LogMethod = _T("GET");
					else if (LogMethod.CompareNoCase(_T("POST")) == 0)
						LogMethod = _T("POST");
					else
						LogMethod = _T("");
					continue;
				}

				//BOOL DWORD系は、空白strTemp3を認めない。//////////////////////////////////////////////////////////
				if (strTemp3.IsEmpty())
					continue;
				if (strTemp2.CompareNoCase(_T("EnableAdvancedLogMode")) == 0)
				{
					EnableAdvancedLogMode = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableAdvancedLogVerboseMode")) == 0)
				{
					EnableAdvancedLogVerboseMode = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("ShowLogo")) == 0)
				{
					ShowLogo = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableTab")) == 0)
				{
					EnableTab = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnablePDFExtension")) == 0)
				{
					EnablePDFExtension = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnablePDFApplicationSelection")) == 0)
				{
					EnablePDFApplicationSelection = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableMemcache")) == 0)
				{
					EnableMemcache = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableGPURendering")) == 0)
				{
					EnableGPURendering = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableCrashRecovery")) == 0)
				{
					EnableCrashRecovery = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableMultipleInstance")) == 0)
				{
					EnableMultipleInstance = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableRebar")) == 0)
				{
					EnableRebar = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableStatusbar")) == 0)
				{
					EnableStatusbar = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("AdvancedLogLevel")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (0 <= iW && iW <= 2)
						AdvancedLogLevel = iW;
					else
						AdvancedLogLevel = 0;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("WideMargin")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (0 <= iW && iW <= 999)
						WideMargin = iW;
					else
						WideMargin = 25;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("HeightMargin")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (0 <= iW && iW <= 999)
						HeightMargin = iW;
					else
						HeightMargin = 48;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("DefaultZoomSize")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (25 <= iW && iW <= 500)
						DefaultZoomSize = iW;
					else
						DefaultZoomSize = 100;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("RedirectMsgTimeout")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (0 <= iW && iW <= 60000)
						RedirectMsgTimeout = iW;
					else
						RedirectMsgTimeout = 2500;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("KeyCombination")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					KeyCombination = iW;
					continue;
				}

				if (strTemp2.CompareNoCase(_T("ProxyType")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (0 <= iW && iW <= 10)
						ProxyType = iW;
					else
						ProxyType = 0;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("MemoryUsageLimit")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (0 <= iW && iW <= 4096)
						MemoryUsageLimit = iW;
					else
						MemoryUsageLimit = 1224;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("WindowCountLimit")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (0 <= iW && iW <= 999)
						WindowCountLimit = iW;
					else
						WindowCountLimit = 999;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableMediaAccessPermission")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (static_cast<int>(AppSettings::MediaAccessPermission::NO_MEDIA_ACCESS) <= iW &&
						iW <= static_cast<int>(AppSettings::MediaAccessPermission::DEFAULT_MEDIA_APPROVAL))
					{
						// 0: None
						// 1: Manual Approval
						// 2: Approval by default  
						EnableMediaAccessPermission = iW;
					}
					else
					{
						// Regard as no permission for invalid value
						EnableMediaAccessPermission = static_cast<int>(AppSettings::MediaAccessPermission::NO_MEDIA_ACCESS);
					}
					bEnabledMediaAccessPermission = true;
					continue;
				}

				if (strTemp2.CompareNoCase(_T("EnableDownloadRestriction")) == 0)
				{
					EnableDownloadRestriction = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}

				if (strTemp2.CompareNoCase(_T("EnableUploadRestriction")) == 0)
				{
					EnableUploadRestriction = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}

				if (strTemp2.CompareNoCase(_T("EnableFileExtChangeRestriction")) == 0)
				{
					EnableFileExtChangeRestriction = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}

				if (strTemp2.CompareNoCase(_T("EnableRunningTime")) == 0)
				{
					EnableRunningTime = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("RunningLimitTime")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					RunningLimitTime = iW;
					continue;
				}

				if (strTemp2.CompareNoCase(_T("EnableURLRedirect")) == 0)
				{
					EnableURLRedirect = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}

				if (strTemp2.CompareNoCase(_T("EnableURLFilter")) == 0)
				{
					EnableURLFilter = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}

				if (strTemp2.CompareNoCase(_T("EnablePopupFilter")) == 0)
				{
					EnablePopupFilter = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}

				if (strTemp2.CompareNoCase(_T("EnableCustomScript")) == 0)
				{
					EnableCustomScript = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}

				if (strTemp2.CompareNoCase(_T("EnableLogging")) == 0)
				{
					EnableLogging = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableUploadLogging")) == 0)
				{
					EnableUploadLogging = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("UploadLoggingURLType")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (static_cast<int>(AppSettings::EnumUploadLoggingURLType::LAST_BROWSED_URL) <= iW &&
					    iW <= static_cast<int>(AppSettings::EnumUploadLoggingURLType::ACTIVE_FRAME))
					{
						UploadLoggingURLType = iW;
					}
					else
					{
						// Regard as no permission for invalid value
						UploadLoggingURLType = static_cast<int>(AppSettings::EnumUploadLoggingURLType::MAIN_FRAME);
					}
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableDownloadLogging")) == 0)
				{
					EnableDownloadLogging = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableBrowsingLogging")) == 0)
				{
					EnableBrowsingLogging = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableAccessAllLogging")) == 0)
				{
					EnableAccessAllLogging = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}

				//ChFiler---------------------------------
				// OpendはOpenedの誤字だが、既に設定ファイルに記載されている値のため、下位互換性のために修正しない。
				if (strTemp2.CompareNoCase(_T("EnableOpendOp")) == 0 || 
					strTemp2.CompareNoCase(_T("EnableOpenedOp")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (0 <= iW && iW <= 2)
						EnableOpenedOp = iW;
					else
						EnableOpenedOp = 0;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableTransferLog")) == 0)
				{
					EnableTransferLog = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				// OpendはOpenedの誤字だが、既に設定ファイルに記載されている値のため、下位互換性のために修正しない。
				if (strTemp2.CompareNoCase(_T("DisableOpendOpAlert")) == 0 ||
				    strTemp2.CompareNoCase(_T("DisableOpenedOpAlert")) == 0)
				{
					DisableOpenedOpAlert = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("DisableExitOpAlert")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (0 <= iW && iW <= 2)
						DisableExitOpAlert = iW;
					else
						DisableExitOpAlert = 0;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("ConfirmAutoRefresh")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (0 <= iW && iW <= 2)
						ConfirmAutoRefresh = iW;
					else
						ConfirmAutoRefresh = 0;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("ShowUploadTab")) == 0)
				{
					ShowUploadTab = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableUploadSync")) == 0)
				{
					EnableUploadSync = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableUploadSyncMirror")) == 0)
				{
					EnableUploadSyncMirror = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("UploadSyncInterval")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (iW < 0)
						iW = 0;
					UploadSyncInterval = iW;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableAutoTransfer")) == 0)
				{
					EnableAutoTransfer = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}

				//ChTaskMGR---------------------------------
				if (strTemp2.CompareNoCase(_T("LABEL_TYPE")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (0 <= iW && iW <= 2)
						LABEL_TYPE = iW;
					else
						LABEL_TYPE = 0;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("LABEL_CHK_INTERVAL")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (0 <= iW && iW <= 5000)
						LABEL_CHK_INTERVAL = iW;
					else
						LABEL_CHK_INTERVAL = 300;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("LABEL_ALPHA_BLEND")) == 0)
				{
					int iW = 0;
					iW = _ttoi(strTemp3);
					if (50 <= iW && iW <= 255)
						LABEL_ALPHA_BLEND = iW;
					else
						LABEL_ALPHA_BLEND = 196;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("TASK_LIST_TYPE")) == 0)
				{
					TASK_LIST_TYPE = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("TASK_LIST_MODE_DETAIL")) == 0)
				{
					TASK_LIST_MODE_DETAIL = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
				if (strTemp2.CompareNoCase(_T("EnableUserConfig")) == 0)
				{
					EnableUserConfig = (strTemp3 == _T("1")) ? TRUE : FALSE;
					continue;
				}
			}
		}
		in.Close();

		//監査ログ設定をRegに登録する。
		if (m_IsSGMode)
		{
			if (this->EnableLogging)
			{
				_SetBaseFunc(_T("EnableLogging"), EnableLogging);
				_SetBaseFunc_String(_T("LogServerURL"), LogServerURL);
				_SetBaseFunc_String(_T("RequestHeader"), RequestHeader);
				_SetBaseFunc_String(_T("LogMethod"), LogMethod);
			}
			else
			{
				_SetBaseFunc(_T("EnableLogging"), EnableLogging);
				_SetBaseFunc_String(_T("LogServerURL"), _T(""));
				_SetBaseFunc_String(_T("RequestHeader"), _T(""));
				_SetBaseFunc_String(_T("LogMethod"), _T(""));
			}
		}
		//制限時は、TaskMgrの詳細表示は無効
		if (this->KeyCombination != 0)
		{
			this->TASK_LIST_MODE_DETAIL = 0;
		}

		return TRUE;
	}
	inline void _SetBaseFunc_String(LPCTSTR lpRegSub, LPCTSTR str)
	{
		CString strVal;
		strVal = str;
		HKEY hKey = {0};
		DWORD dwDisposition = 0;
		LONG lResult = 0L;
		lResult = RegCreateKeyEx(HKEY_CURRENT_USER, REG_SETTING_KEY,
					 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
		if (lResult == ERROR_SUCCESS)
		{
			int iSize = 0;
			iSize = strVal.GetLength();
			TCHAR* pVal = NULL;
			iSize += 1; //+1=null
			pVal = new TCHAR[iSize];
			memset(pVal, 0x00, sizeof(TCHAR) * iSize);
			StringCchCopy(pVal, iSize, strVal);
			RegSetValueEx(hKey, lpRegSub, 0, REG_SZ, (LPBYTE)pVal, sizeof(TCHAR) * iSize);
			RegCloseKey(hKey);
			delete[] pVal;
		}
	}

	inline void _SetBaseFunc(LPCTSTR lpRegSub, DWORD bVal)
	{
		HKEY hKey = {0};
		DWORD dwDisposition = 0;
		LONG lResult = 0L;
		lResult = RegCreateKeyEx(HKEY_CURRENT_USER, REG_SETTING_KEY,
					 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
		if (lResult == ERROR_SUCCESS)
		{
			RegSetValueEx(hKey, lpRegSub, 0, REG_DWORD, (LPBYTE)&bVal, sizeof(DWORD));
			RegCloseKey(hKey);
		}
	}

	CString _ExTxtVal(LPCTSTR key, int iVal)
	{
		CString strRet;
		strRet.Format(_T("%s=%d\n"), key, iVal);
		return strRet;
	}
	CString _ExTxtVal(LPCTSTR key, LPCTSTR sVal)
	{
		CString strRet;
		strRet.Format(_T("%s=%s\n"), key, sVal);
		return strRet;
	}
	CString ExportTxt()
	{
		CString strRet;

		//全般設定
		strRet += _T("# General\n");
		strRet += EXTVAL(EnableMultipleInstance);
		strRet += EXTVAL(EnableMemcache);
		strRet += EXTVAL(EnableCrashRecovery);
		strRet += EXTVAL(KeyCombination);

		//画面表示設定
		strRet += _T("# Appearance\n");
		strRet += EXTVAL(EnableTab);
		strRet += EXTVAL(EnableGPURendering);
		strRet += EXTVAL(EnablePDFExtension);
		strRet += EXTVAL(EnablePDFApplicationSelection);
		strRet += EXTVAL(EnableRebar);
		strRet += EXTVAL(ShowLogo);
		strRet += EXTVAL(EnableStatusbar);
		strRet += EXTVAL(WideMargin);
		strRet += EXTVAL(HeightMargin);
		strRet += EXTVAL(DefaultZoomSize);

		//起動関連設定
		strRet += _T("# Startup\n");
		strRet += EXTVAL(StartURL);
		strRet += EXTVAL(EnforceInitParam);
		strRet += EXTVAL(InitMessage);

		//インターネット接続設定
		strRet += _T("# Internet Connection\n");
		strRet += EXTVAL(ProxyType);
		strRet += EXTVAL(ProxyAddress);
		strRet += EXTVAL(ProxyBypassAddress);
		strRet += EXTVAL(UserAgentAppendStr);

		//制限設定
		strRet += _T("# Restriction\n");
		strRet += EXTVAL(EnableDownloadRestriction);
		strRet += EXTVAL(EnableUploadRestriction);
		strRet += EXTVAL(EnableFileExtChangeRestriction);
		strRet += EXTVAL(EnableRunningTime);
		strRet += EXTVAL(RunningLimitTime);
		strRet += EXTVAL(MemoryUsageLimit);
		strRet += EXTVAL(WindowCountLimit);

		//リダイレクト設定
		strRet += _T("# Redirection\n");
		strRet += EXTVAL(EnableURLRedirect);
		strRet += EXTVAL(RedirectMsgTimeout);
		strRet += EXTVAL(CustomBrowser);
		strRet += EXTVAL(CustomBrowser2);
		strRet += EXTVAL(CustomBrowser3);
		strRet += EXTVAL(CustomBrowser4);
		strRet += EXTVAL(CustomBrowser5);

		//URLフィルター設定
		strRet += _T("# URL Filtering\n");
		strRet += EXTVAL(EnableURLFilter);

		//ポップアップフィルター設定
		strRet += _T("# Popup Filtering\n");
		strRet += EXTVAL(EnablePopupFilter);

		//CustomScript設定
		strRet += _T("# Custom Scripts\n");
		strRet += EXTVAL(EnableCustomScript);

		//ログ出力設定
		strRet += _T("# Logging\n");
		strRet += EXTVAL(EnableAdvancedLogMode);
		strRet += EXTVAL(EnableAdvancedLogVerboseMode);
		strRet += EXTVAL(AdvancedLogLevel);
		strRet += EXTVAL(EnableLogging);
		strRet += EXTVAL(LogServerURL);
		strRet += EXTVAL(RequestHeader);
		strRet += EXTVAL(LogMethod);
		strRet += EXTVAL(EnableDownloadLogging);
		strRet += EXTVAL(EnableUploadLogging);
		strRet += EXTVAL(UploadLoggingURLType);
		strRet += EXTVAL(EnableBrowsingLogging);
		strRet += EXTVAL(EnableAccessAllLogging);
	
		//ChFiler---------------------------------
		strRet += _T("# File Manager\n");
		strRet += EXTVAL(RootPath);
		strRet += EXTVAL(ExtFilter);
		strRet += EXTVAL(DisallowExt);
		strRet += EXTVAL(DisallowTransferringExt);
		strRet += EXTVAL(TransferPath);
		strRet += EXTVAL(TransferSubFolder);
		strRet += EXTVAL(UploadPath);
		strRet += EXTVAL(ShowUploadTab);
		strRet += EXTVAL(EnableUploadSync);
		strRet += EXTVAL(EnableUploadSyncMirror);
		strRet += EXTVAL(UploadSyncInterval);
		strRet += EXTVAL(EnableAutoTransfer);
		strRet += EXTVAL(EnableOpenedOp);
		strRet += EXTVAL(DisableOpenedOpAlert);

		strRet += _T("# non GUI parameters\n");
		strRet += EXTVAL(CEFCommandLine);
		strRet += EXTVAL(EnableMediaAccessPermission);
		strRet += EXTVAL(UploadBasePath);
		strRet += EXTVAL(ExitMessage);
		strRet += EXTVAL(unZipMessage);
		strRet += EXTVAL(EnableTransferLog);
		strRet += EXTVAL(DisableExitOpAlert);
		strRet += EXTVAL(ConfirmAutoRefresh);
		//ChTaskMGR---------------------------------
		strRet += EXTVAL(TASK_LIST_TYPE);
		strRet += EXTVAL(TASK_LIST_MODE_DETAIL);
		strRet += EXTVAL(LABEL_TYPE);
		strRet += EXTVAL(LABEL_CHK_INTERVAL);
		strRet += EXTVAL(LABEL_ALPHA_BLEND);
		//Config file-------------------------------
		strRet += EXTVAL(EnableUserConfig);

		return strRet;
	}

public:
	//Is Functions Getter##########################################################
	inline BOOL IsAdvancedLogMode() { return EnableAdvancedLogMode; }
	inline BOOL IsAdvancedLogVerboseMode() { return EnableAdvancedLogVerboseMode; }
	inline BOOL IsShowLogo() { return ShowLogo; }
	inline BOOL IsEnableTab() { return EnableTab; }
	inline BOOL IsEnablePDFExtension() { return EnablePDFExtension; }
	inline BOOL IsEnablePDFApplicationSelection() { return EnablePDFApplicationSelection; }
	inline BOOL IsEnableMemcache() { return EnableMemcache; }
	inline BOOL IsGPURendering() { return EnableGPURendering; }
	inline BOOL IsCrashRecovery() { return EnableCrashRecovery; }
	inline BOOL IsMultipleInstance() { return EnableMultipleInstance; }
	inline int GetProxyType() { return ProxyType; }
	inline CString GetProxyAddress() { return ProxyAddress; }
	inline CString GetProxyBypassAddress() { return ProxyBypassAddress; }
	inline CString GetUserAgentAppendStr() { return UserAgentAppendStr; }
	inline BOOL IsRebar() { return EnableRebar; }
	inline BOOL IsStatusbar() { return EnableStatusbar; }
	inline int GetAdvancedLogLevel() { return AdvancedLogLevel; }
	inline int GetWideMargin() { return WideMargin; }
	inline int GetHeightMargin() { return HeightMargin; }
	inline int GetZoomSize() { return DefaultZoomSize; }
	inline int GetRedirectMsgTimeout() { return RedirectMsgTimeout; }
	inline int GetKeyCombination() { return KeyCombination; }

	inline CString GetStartURL()
	{
		if (StartURL.IsEmpty())
			return _T("about:blank");
		else
			return StartURL;
	}
	inline CString GetEnforceInitParam() { return EnforceInitParam; }
	inline CString GetCEFCommandLine() { return CEFCommandLine; }
	inline CString GetCustomBrowser() { return CustomBrowser; }
	inline CString GetCustomBrowser2() { return CustomBrowser2; }
	inline CString GetCustomBrowser3() { return CustomBrowser3; }
	inline CString GetCustomBrowser4() { return CustomBrowser4; }
	inline CString GetCustomBrowser5() { return CustomBrowser5; }
	inline CString GetInitMessage() { return InitMessage; }

	inline int GetMemoryUsageLimit() { return MemoryUsageLimit; }
	inline int GetWindowCountLimit() { return WindowCountLimit; }
	inline int GetMediaAccessPermission() { return EnableMediaAccessPermission; }

	inline BOOL IsEnableDownloadRestriction() { return EnableDownloadRestriction; }
	inline BOOL IsEnableUploadRestriction() { return EnableUploadRestriction; }
	inline BOOL IsEnableFileExtChangeRestriction() { return EnableFileExtChangeRestriction; }
	inline BOOL IsEnableRunningTime() { return EnableRunningTime; }
	inline int GetRunningLimitTime() { return RunningLimitTime; }

	inline BOOL IsEnableURLRedirect() { return EnableURLRedirect; }
	inline BOOL IsEnableURLFilter() { return EnableURLFilter; }
	inline BOOL IsEnablePopupFilter() { return EnablePopupFilter; }
	inline BOOL IsEnableCustomScript() { return EnableCustomScript; }

	inline BOOL IsEnableLogging() { return EnableLogging; }
	inline BOOL IsEnableUploadLogging() { return EnableUploadLogging; }
	inline int GetUploadLoggingURLType() { return UploadLoggingURLType; }
	inline BOOL IsEnableDownloadLogging() { return EnableDownloadLogging; }
	inline BOOL IsEnableBrowsingLogging() { return EnableBrowsingLogging; }
	inline BOOL IsEnableAccessAllLogging() { return EnableAccessAllLogging; }
	inline CString GetLogServerURL() { return LogServerURL; }
	inline CString GetRequestHeader() { return RequestHeader; }
	inline CString GetLogMethod() { return LogMethod; };

	//ChFiler---------------------------------
	inline CString GetRootPath() { return RootPath; }
	inline CString GetUploadBasePath() { return UploadBasePath; }
	inline CString GetExtFilter() { return ExtFilter; }
	inline CString GetTransferPath() { return TransferPath; }
	inline CString GetTransferSubFolder() { return TransferSubFolder; }
	inline CString GetUploadPath() { return UploadPath; }
	inline CString GetDisallowExt() { return DisallowExt; }
	inline CString GetDisallowTransferringExt() { return DisallowTransferringExt; }
	inline CString GetunZipMessage() { return unZipMessage; }
	inline CString GetExitMessage() { return ExitMessage; }

	//ChFiler---------------------------------
	inline int GetEnableOpenedOp() { return EnableOpenedOp; }
	inline BOOL IsEnableTransferLog() { return EnableTransferLog; }
	inline BOOL IsEnableUploadSync() { return EnableUploadSync; }
	inline BOOL IsEnableUploadSyncMirror() { return EnableUploadSyncMirror; }
	inline int GetUploadSyncInterval() { return UploadSyncInterval; }
	inline BOOL IsEnableAutoTransfer() { return EnableAutoTransfer; }
	inline BOOL IsDisableOpenedOpAlert() { return DisableOpenedOpAlert; }
	inline BOOL IsDisableExitOpAlert() { return DisableExitOpAlert; }
	inline int GetConfirmAutoRefresh() { return ConfirmAutoRefresh; }
	inline BOOL IsShowUploadTab() { return ShowUploadTab; }

	//ChTaskMGR---------------------------------
	inline int GetLABEL_TYPE() { return LABEL_TYPE; }
	inline int GetLABEL_CHK_INTERVAL() { return LABEL_CHK_INTERVAL; }
	inline int GetLABEL_ALPHA_BLEND() { return LABEL_ALPHA_BLEND; }
	inline int GetTASK_LIST_TYPE() { return TASK_LIST_TYPE; }
	inline int GetTASK_LIST_MODE_DETAIL() { return TASK_LIST_MODE_DETAIL; }

	//Config file-------------------------------
	inline BOOL IsEnableUserConfig() { return EnableUserConfig; }

	//Set Functions Setter##########################################################
	inline void SetAdvancedLogMode(DWORD dVal) { EnableAdvancedLogMode = dVal ? 1 : 0; }
	inline void SetAdvancedLogVerboseMode(DWORD dVal) { EnableAdvancedLogVerboseMode = dVal ? 1 : 0; }
	inline void SetShowLogo(DWORD dVal) { ShowLogo = dVal ? 1 : 0; }
	inline void SetEnableTab(DWORD dVal) { EnableTab = dVal ? 1 : 0; }
	inline void SetEnablePDFExtension(DWORD dVal) { EnablePDFExtension = dVal ? 1 : 0; }
	inline void SetEnablePDFApplicationSelection(DWORD dVal) { EnablePDFApplicationSelection = dVal ? 1 : 0; }
	inline void SetEnableMemcache(DWORD dVal) { EnableMemcache = dVal ? 1 : 0; }
	inline void SetGPURendering(DWORD dVal) { EnableGPURendering = dVal ? 1 : 0; }
	inline void SetCrashRecovery(DWORD dVal) { EnableCrashRecovery = dVal ? 1 : 0; }
	inline void SetMultipleInstance(DWORD dVal) { EnableMultipleInstance = dVal ? 1 : 0; }
	inline void SetProxyType(DWORD dVal) { ProxyType = dVal; }
	inline void SetProxyAddress(LPCTSTR str) { ProxyAddress = str; }
	inline void SetProxyBypassAddress(LPCTSTR str) { ProxyBypassAddress = str; }
	inline void SetUserAgentAppendStr(LPCTSTR str) { UserAgentAppendStr = str; }
	inline void SetRebar(DWORD dVal) { EnableRebar = dVal ? 1 : 0; }
	inline void SetStatusbar(DWORD dVal) { EnableStatusbar = dVal ? 1 : 0; }
	inline void SetAdvancedLogLevel(DWORD dVal) { AdvancedLogLevel = dVal; }
	inline void SetWideMargin(DWORD dVal) { WideMargin = dVal; }
	inline void SetHeightMargin(DWORD dVal) { HeightMargin = dVal; }
	inline void SetZoomSize(DWORD dVal) { DefaultZoomSize = dVal; }
	inline void SetRedirectMsgTimeout(DWORD dVal) { RedirectMsgTimeout = dVal; }
	inline void SetKeyCombination(DWORD dVal)
	{
		KeyCombination = dVal;
		//制限時は、TaskMgrの詳細表示は無効
		if (KeyCombination != 0)
		{
			TASK_LIST_MODE_DETAIL = 0;
		}
	}

	inline void SetMemoryUsageLimit(DWORD dVal) { MemoryUsageLimit = dVal; }
	inline void SetWindowCountLimit(DWORD dVal) { WindowCountLimit = dVal; }

	inline void SetStartURL(LPCTSTR str) { StartURL = str; }
	inline void SetEnforceInitParam(LPCTSTR str) { EnforceInitParam = str; }
	inline void SetCustomBrowser(LPCTSTR str) { CustomBrowser = str; }
	inline void SetCustomBrowser2(LPCTSTR str) { CustomBrowser2 = str; }
	inline void SetCustomBrowser3(LPCTSTR str) { CustomBrowser3 = str; }
	inline void SetCustomBrowser4(LPCTSTR str) { CustomBrowser4 = str; }
	inline void SetCustomBrowser5(LPCTSTR str) { CustomBrowser5 = str; }
	inline void SetInitMessage(LPCTSTR str) { InitMessage = str; }

	inline void SetEnableDownloadRestriction(DWORD dVal) { EnableDownloadRestriction = dVal ? 1 : 0; }
	inline void SetEnableUploadRestriction(DWORD dVal) { EnableUploadRestriction = dVal ? 1 : 0; }
	inline void SetEnableFileExtChangeRestriction(DWORD dVal) { EnableFileExtChangeRestriction = dVal; }
	inline void SetEnableRunningTime(DWORD dVal) { EnableRunningTime = dVal ? 1 : 0; }
	inline void SetRunningLimitTime(DWORD dVal) { RunningLimitTime = dVal; }

	inline void SetEnableURLRedirect(DWORD dVal) { EnableURLRedirect = dVal ? 1 : 0; }
	inline void SetEnableURLFilter(DWORD dVal) { EnableURLFilter = dVal ? 1 : 0; }
	inline void SetEnablePopupFilter(DWORD dVal) { EnablePopupFilter = dVal ? 1 : 0; }
	inline void SetEnableCustomScript(DWORD dVal) { EnableCustomScript = dVal ? 1 : 0; }

	inline void SetEnableLogging(DWORD dVal)
	{
		EnableLogging = dVal ? 1 : 0;
		//監査ログ設定をRegに登録する。
		if (m_IsSGMode)
		{
			if (EnableLogging)
			{
				_SetBaseFunc(_T("EnableLogging"), EnableLogging);
				_SetBaseFunc_String(_T("LogServerURL"), LogServerURL);
				_SetBaseFunc_String(_T("RequestHeader"), RequestHeader);
				_SetBaseFunc_String(_T("LogMethod"), LogMethod);
			}
			else
			{
				_SetBaseFunc(_T("EnableLogging"), EnableLogging);
				_SetBaseFunc_String(_T("LogServerURL"), _T(""));
				_SetBaseFunc_String(_T("RequestHeader"), _T(""));
				_SetBaseFunc_String(_T("LogMethod"), _T(""));
			}
		}
	}
	inline void SetEnableUploadLogging(DWORD dVal) { EnableUploadLogging = dVal ? 1 : 0; }
	inline void SetUploadLoggingURLType(DWORD dVal) { UploadLoggingURLType = dVal; }
	inline void SetEnableDownloadLogging(DWORD dVal) { EnableDownloadLogging = dVal ? 1 : 0; }
	inline void SetEnableBrowsingLogging(DWORD dVal) { EnableBrowsingLogging = dVal ? 1 : 0; }
	inline void SetEnableAccessAllLogging(DWORD dVal) { EnableAccessAllLogging = dVal ? 1 : 0; }
	inline void SetLogServerURL(LPCTSTR str) { LogServerURL = str; }
	inline void SetRequestHeader(LPCTSTR str) { RequestHeader = str; }
	inline void SetLogMethod(LPCTSTR str) { LogMethod = str; }

	//ChFiler---------------------------------
	inline void SetRootPath(LPCTSTR str) { RootPath = str; }
	inline void SetUploadBasePath(LPCTSTR str) { UploadBasePath = str; }
	inline void SetExtFilter(LPCTSTR str) { ExtFilter = str; }
	inline void SetTransferPath(LPCTSTR str) { TransferPath = str; }
	inline void SetTransferSubFolder(LPCTSTR str) { TransferSubFolder = str; }
	inline void SetUploadPath(LPCTSTR str) { UploadPath = str; }
	inline void SetDisallowExt(LPCTSTR str) { DisallowExt = str; }
	inline void SetDisallowTransferringExt(LPCTSTR str) { DisallowTransferringExt = str; }
	inline void SetunZipMessage(LPCTSTR str) { unZipMessage = str; }
	inline void SetExitMessage(LPCTSTR str) { ExitMessage = str; }

	//ChFiler---------------------------------
	inline void SetEnableOpenedOp(DWORD dVal) { EnableOpenedOp = dVal; }
	inline void SetEnableTransferLog(DWORD dVal) { EnableTransferLog = dVal; }
	inline void SetEnableUploadSync(DWORD dVal) { EnableUploadSync = dVal; }
	inline void SetEnableUploadSyncMirror(DWORD dVal) { EnableUploadSyncMirror = dVal; }
	inline void SetUploadSyncInterval(DWORD dVal) { UploadSyncInterval = dVal; }
	inline void SetEnableAutoTransfer(DWORD dVal) { EnableAutoTransfer = dVal; }
	inline void SetDisableOpenedOpAlert(DWORD dVal) { DisableOpenedOpAlert = dVal; }
	inline void SetDisableExitOpAlert(DWORD dVal) { DisableExitOpAlert = dVal; }
	inline void SetConfirmAutoRefresh(DWORD dVal) { ConfirmAutoRefresh = dVal; }
	inline void SetShowUploadTab(DWORD dVal) { ShowUploadTab = dVal; }

	//ChTaskMGR---------------------------------
	inline void SetLABEL_TYPE(DWORD dVal) { LABEL_TYPE = dVal; }
	inline void SetLABEL_CHK_INTERVAL(DWORD dVal) { LABEL_CHK_INTERVAL = dVal; }
	inline void SetLABEL_ALPHA_BLEND(DWORD dVal) { LABEL_ALPHA_BLEND = dVal; }
	inline void SetTASK_LIST_TYPE(DWORD dVal) { TASK_LIST_TYPE = dVal; }
	inline void SetTASK_LIST_MODE_DETAIL(DWORD dVal) { TASK_LIST_MODE_DETAIL = dVal; }

	// Config file-------------------------------
	inline int SetEnableUserConfig(DWORD dVal) { EnableUserConfig = dVal; }
};

class CIconHelper
{
private:
	HICON m_hICON;

public:
	CIconHelper() { m_hICON = NULL; }
	~CIconHelper() { Release(); }
	void Release()
	{
		if (m_hICON)
		{
			::DestroyIcon(m_hICON);
			m_hICON = NULL;
		}
	}
	void SetIcon(const HICON ico)
	{
		if (ico)
		{
			Release();
			m_hICON = ico;
		}
	}
	HICON GetIcon()
	{
		return m_hICON;
	}
	operator HICON() { return GetIcon(); }
	HICON operator=(HICON ico)
	{
		SetIcon(ico);
		return GetIcon();
	}
};
class CCursorHelper
{
private:
	HCURSOR m_hICUR;

public:
	CCursorHelper() { m_hICUR = NULL; }
	~CCursorHelper() { Release(); }
	void Release()
	{
		if (m_hICUR)
		{
			::DestroyCursor(m_hICUR);
			m_hICUR = NULL;
		}
	}
	void SetCursor(const HCURSOR cur)
	{
		if (cur)
		{
			Release();
			m_hICUR = cur;
		}
	}
	HICON GetCursor()
	{
		return m_hICUR;
	}
	operator HCURSOR() { return GetCursor(); }
	HCURSOR operator=(HCURSOR cur)
	{
		SetCursor(cur);
		return GetCursor();
	}
};

class CSystemDragFullWindowHelper
{
private:
	int OldVal;
	BOOL bInit;

public:
	void Init()
	{
		if (!bInit)
		{
			OldVal = 0;
			::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &OldVal, 0);
			bInit = TRUE;
		}
	};
	void Lock()
	{
		if (OldVal)
		{
			::SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, FALSE, NULL, SPIF_SENDCHANGE);
		}
	}
	void Release()
	{
		if (bInit)
		{
			if (OldVal)
			{
				::SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, OldVal, NULL, SPIF_SENDCHANGE);
			}
		}
	}
	CSystemDragFullWindowHelper()
	{
		OldVal = 0;
		bInit = FALSE;
	}
	~CSystemDragFullWindowHelper()
	{
		Release();
	}
};
class CReDrawHelper
{
public:
	CReDrawHelper(CWnd* pWnd)
	{
		m_hWnd = NULL;
		if (!pWnd) return;
		m_hWnd = pWnd->GetSafeHwnd();
		if (m_hWnd)
			::SendMessage(m_hWnd, WM_SETREDRAW, (WPARAM)FALSE, (LPARAM)0);
	}
	CReDrawHelper(HWND hWnd)
	{
		m_hWnd = NULL;
		m_hWnd = hWnd;
		if (m_hWnd)
			::SendMessage(m_hWnd, WM_SETREDRAW, (WPARAM)FALSE, (LPARAM)0);
	}
	~CReDrawHelper()
	{
		if (m_hWnd)
			::SendMessage(m_hWnd, WM_SETREDRAW, (WPARAM)TRUE, (LPARAM)0);
	}

protected:
	HWND m_hWnd;
};
class CSystemAnimationHelper
{
private:
	int OldVal;
	BOOL bInit;

public:
	void Init()
	{
		OldVal = 0;
		// アニメーション効果は一時的に OFF にする
		ANIMATIONINFO ai = {0};
		ai.cbSize = sizeof(ANIMATIONINFO);
		::SystemParametersInfo(SPI_GETANIMATION, sizeof(ANIMATIONINFO), &ai, 0);
		OldVal = ai.iMinAnimate;
		ai.iMinAnimate = 0;
		::SystemParametersInfo(SPI_SETANIMATION, sizeof(ANIMATIONINFO), &ai, 0);
		bInit = TRUE;
	};
	void Release()
	{
		if (bInit)
		{
			ANIMATIONINFO ai = {0};
			ai.cbSize = sizeof(ANIMATIONINFO);
			// アニメーション効果を戻す
			ai.iMinAnimate = OldVal;
			::SystemParametersInfo(SPI_SETANIMATION, sizeof(ANIMATIONINFO), &ai, 0);
			bInit = FALSE;
		}
	}
	CSystemAnimationHelper(BOOL bInitOp = FALSE)
	{
		OldVal = 0;
		bInit = FALSE;
		if (bInitOp)
		{
			Init();
		}
	}
	~CSystemAnimationHelper()
	{
		if (bInit)
		{
			ANIMATIONINFO ai = {0};
			// アニメーション効果を戻す
			ai.iMinAnimate = OldVal;
			::SystemParametersInfo(SPI_SETANIMATION, sizeof(ANIMATIONINFO), &ai, 0);
			bInit = FALSE;
		}
	}
};

static int gInfoDlgList[] = {
    ID_INFO_DLG_LIST_HIDE,
    ID_INFO_DLG_LIST_0_5,
    ID_INFO_DLG_LIST_1_0,
    ID_INFO_DLG_LIST_1_5,
    ID_INFO_DLG_LIST_2_0,
    ID_INFO_DLG_LIST_2_5,
    ID_INFO_DLG_LIST_3_0,
    ID_INFO_DLG_LIST_3_5,
    ID_INFO_DLG_LIST_4_0,
    ID_INFO_DLG_LIST_4_5,
    ID_INFO_DLG_LIST_5_0,
    ID_INFO_DLG_LIST_5_5,
    ID_INFO_DLG_LIST_6_0,
    ID_INFO_DLG_LIST_6_5,
    ID_INFO_DLG_LIST_7_0,
    ID_INFO_DLG_LIST_7_5,
    ID_INFO_DLG_LIST_8_0,
    ID_INFO_DLG_LIST_8_5,
    ID_INFO_DLG_LIST_9_0,
    ID_INFO_DLG_LIST_9_5,
    ID_INFO_DLG_LIST_10_0,
};
#define InfoDlgListMaxCnt 21

static int gInfoOpenOp[] = {
    ID_INFO_OPEN_OPTION_HIDE_MARK_AS_OPENED,
    ID_INFO_OPEN_OPTION_SHOW_MARK_AS_OPENED,
    ID_INFO_OPEN_OPTION_SHOW_TRANSFER,
};
#define InfoOpenOpMaxCnt 3

static inline int GetIDInfoFromStr(LPCTSTR str)
{
	int iRet = 0;
	CString str2;
	str2 = str;

	CString strTemp;
	for (int i = 0; i < InfoDlgListMaxCnt; i++)
	{
		strTemp.LoadString(gInfoDlgList[i]);
		if (strTemp == str2)
		{
			iRet = i;
			break;
		}
	}
	return iRet;
}
static inline CString GetStrInfoFromID(int iID)
{
	CString strRet;

	if (iID < 0 || iID >= InfoDlgListMaxCnt)
		iID = 0;
	strRet.LoadString(gInfoDlgList[iID]);
	return strRet;
}

class CScriptSrcMgr
{
public:
	CScriptSrcMgr()
	{
	}
	~CScriptSrcMgr()
	{
	}
	void Clear()
	{
		m_strSrc.Empty();
	}
	void SetFilePathAndSetData(const CString& strFile)
	{
		m_strFilePath = strFile;
		SetData(m_strFilePath);
	}
	void RefreshData()
	{
		SetData(m_strFilePath);
	}

	void SetData(LPCTSTR lPath)
	{
		if (lPath == NULL) return;
		this->Clear();
		_wsetlocale(LC_ALL, _T("jpn"));
		CFile file;
		CString strText;
		if (file.Open(lPath, CFile::modeRead | CFile::shareDenyNone))
		{
			size_t dwLength = (size_t)file.GetLength();
			byte* pBuffer = new byte[dwLength + 2];
			memset(pBuffer, 0x00, dwLength + 2);
			file.Read(pBuffer, (UINT)dwLength);
			strText = pBuffer;
			delete[] pBuffer;
			file.Close();
		}
		m_strSrc = strText;
	}
	CString m_strSrc;
	CString m_strFilePath;
};
class CCustomURLList
{
public:
	CCustomURLList()
	{
	}
	virtual ~CCustomURLList()
	{
	}
	virtual void Clear()
	{
		m_LogMsg.Empty();
		m_arrURL.RemoveAll();
	}
	virtual void SetFilePathAndCreateArrayData(const CString& strFile)
	{
		m_strFilePath = strFile;
		SetArrayData(m_strFilePath);
	}
	virtual void RefreshData()
	{
		SetArrayData(m_strFilePath);
	}
	virtual void SetArrayData(LPCTSTR lPath)
	{
		if (lPath == NULL) return;
		this->Clear();
		_wsetlocale(LC_ALL, _T("jpn"));
		CStdioFile in;
		if (in.Open(lPath, CFile::modeRead | CFile::shareDenyNone))
		{
			m_LogMsg.Format(_T("CustomURLList %s\n===============\n"), lPath);
			CString strTemp;
			while (in.ReadString(strTemp))
			{
				m_LogMsg += strTemp;
				m_LogMsg += _T("\n");

				strTemp.TrimLeft();
				strTemp.TrimRight();
				if (strTemp.IsEmpty())
					continue;

				if (strTemp.Find(_T(";")) == 0)
				{
					;
				}
				else if (strTemp.Find(_T("#")) == 0)
				{
					;
				}
				else
				{
					m_arrURL.Add(strTemp);
				}
			}
			in.Close();
			m_LogMsg += _T("---------------");
		}
	}

	//TRUE HIT
	virtual BOOL IsHitURL(const CString& strSURL)
	{
		BOOL bRet = FALSE;
		CString strTemp;
		CString strTemp2;
		int imax = (int)m_arrURL.GetSize();
		for (int i = 0; i < imax; i++)
		{
			strTemp.Empty();
			strTemp = m_arrURL.GetAt(i);
			//一致する部分があった一部一致
			if (strSURL.Find(strTemp) > -1)
			{
				bRet = TRUE;
				break;
			}
			bRet = FALSE;
		}
		return bRet;
	}
	CStringArray m_arrURL;
	CString m_strFilePath;
	CString m_LogMsg;
};

class CFilterURLList : public CCustomURLList
{
public:
	CFilterURLList()
	{
	}
	virtual ~CFilterURLList()
	{
	}
	void Clear()
	{
		m_arrURL_AD.RemoveAll();
		CCustomURLList::Clear();
	}
	void SetArrayData(LPCTSTR lPath)
	{
		if (lPath == NULL) return;
		this->Clear();
		_wsetlocale(LC_ALL, _T("jpn"));
		CStdioFile in;
		if (in.Open(lPath, CFile::modeRead | CFile::shareDenyNone))
		{
			m_LogMsg.Format(_T("CFilterURLList %s\n===============\n"), lPath);
			CString strTemp;
			CString strTemp2;
			CString strTemp3;
			CStringArray strArray;

			while (in.ReadString(strTemp))
			{
				m_LogMsg += strTemp;
				m_LogMsg += _T("\n");

				strTemp2.Empty();
				strTemp3.Empty();
				strArray.RemoveAll();
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if (strTemp.IsEmpty())
					continue;
				SBUtil::Split(&strArray, strTemp, _T("\t"));
				if (strArray.GetSize() >= 2)
				{
					strTemp2 = strArray.GetAt(0);
					strTemp2.TrimLeft();
					strTemp2.TrimRight();

					strTemp3 = strArray.GetAt(1);
					strTemp3.TrimLeft();
					strTemp3.TrimRight();
					if (strTemp2.Find(_T(";")) == 0)
					{
						;
					}
					else if (strTemp2.Find(_T("#")) == 0)
					{
						;
					}
					else
					{
						if (!strTemp2.IsEmpty())
						{
							m_arrURL.Add(strTemp2);
							INT_PTR iMode = 0;
							iMode = strTemp3 == _T("A") ? TF_ALLOW : TF_DENY;
							m_arrURL_AD.Add((void*)iMode);
						}
					}
				}
			}
			in.Close();
			m_LogMsg += _T("---------------");
		}
	}
	//TRUE HIT
	INT_PTR HitWildCardURL(const CString& strSURL)
	{
		INT_PTR uiRet = TF_ALLOW;
		CStringA strURLA(strSURL);
		CString strTemp;
		CStringA strTempA;

		int imax = (int)m_arrURL.GetSize();
		for (int i = 0; i < imax; i++)
		{
			strTemp.Empty();
			strTemp = m_arrURL.GetAt(i);
			strTempA = strTemp;

			//ワイルドカード対応
			if (wildcmp(strTempA, strURLA))
			{
				uiRet = (INT_PTR)m_arrURL_AD.GetAt(i);
				break;
			}
			uiRet = TF_ALLOW;
		}
		return uiRet;
	}
	CPtrArray m_arrURL_AD;
};

class CPopupFilterURLList : public CCustomURLList
{
public:
	CPopupFilterURLList()
	{
	}
	virtual ~CPopupFilterURLList()
	{
	}
	void Clear()
	{
		m_arrURL_AD.RemoveAll();
		CCustomURLList::Clear();
	}
	void SetArrayData(LPCTSTR lPath)
	{
		if (lPath == NULL) return;
		this->Clear();
		_wsetlocale(LC_ALL, _T("jpn"));
		CStdioFile in;
		if (in.Open(lPath, CFile::modeRead | CFile::shareDenyNone))
		{
			m_LogMsg.Format(_T("CPopupFilterURLList %s\n===============\n"), lPath);
			CString strTemp;
			CString strTemp2;
			CString strTemp3;
			CStringArray strArray;

			while (in.ReadString(strTemp))
			{
				m_LogMsg += strTemp;
				m_LogMsg += _T("\n");

				strTemp2.Empty();
				strTemp3.Empty();
				strArray.RemoveAll();
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if (strTemp.IsEmpty())
					continue;
				SBUtil::Split(&strArray, strTemp, _T("\t"));
				if (strArray.GetSize() >= 2)
				{
					strTemp2 = strArray.GetAt(0);
					strTemp2.TrimLeft();
					strTemp2.TrimRight();

					strTemp3 = strArray.GetAt(1);
					strTemp3.TrimLeft();
					strTemp3.TrimRight();
					if (strTemp2.Find(_T(";")) == 0)
					{
						;
					}
					else if (strTemp2.Find(_T("#")) == 0)
					{
						;
					}
					else
					{
						if (!strTemp2.IsEmpty())
						{
							m_arrURL.Add(strTemp2);
							INT_PTR iMode = 0;
							iMode = strTemp3 == _T("A") ? TF_ALLOW : TF_DENY;
							m_arrURL_AD.Add((void*)iMode);
						}
					}
				}
			}
			in.Close();
			m_LogMsg += _T("---------------");
		}
	}
	// TRUE HIT
	INT_PTR HitWildCardURL(const CString& strSURL)
	{
		INT_PTR uiRet = TF_ALLOW;
		CStringA strURLA(strSURL);
		CString strTemp;
		CStringA strTempA;

		int imax = (int)m_arrURL.GetSize();
		for (int i = 0; i < imax; i++)
		{
			strTemp.Empty();
			strTemp = m_arrURL.GetAt(i);
			strTempA = strTemp;

			// ワイルドカード対応
			if (wildcmp(strTempA, strURLA))
			{
				uiRet = (INT_PTR)m_arrURL_AD.GetAt(i);
				break;
			}
			uiRet = TF_ALLOW;
		}
		return uiRet;
	}
	CPtrArray m_arrURL_AD;
};


class CCustomScriptList : public CCustomURLList
{
public:
	CCustomScriptList()
	{
	}
	virtual ~CCustomScriptList()
	{
	}
	void Clear()
	{
		m_arrURL_FileName.RemoveAll();
		CCustomURLList::Clear();
	}
	void SetArrayData(LPCTSTR lPath)
	{
		if (lPath == NULL) return;
		this->Clear();
		_wsetlocale(LC_ALL, _T("jpn"));
		CStdioFile in;
		if (in.Open(lPath, CFile::modeRead | CFile::shareDenyNone))
		{
			m_LogMsg.Format(_T("CustomScriptList %s\n===============\n"), lPath);
			CString strTemp;
			CString strTemp2;
			CString strTemp3;
			CStringArray strArray;

			while (in.ReadString(strTemp))
			{
				m_LogMsg += strTemp;
				m_LogMsg += _T("\n");

				strTemp2.Empty();
				strTemp3.Empty();
				strArray.RemoveAll();
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if (strTemp.IsEmpty())
					continue;
				SBUtil::Split(&strArray, strTemp, _T("\t"));
				if (strArray.GetSize() >= 2)
				{
					strTemp2 = strArray.GetAt(0);
					strTemp2.TrimLeft();
					strTemp2.TrimRight();

					strTemp3 = strArray.GetAt(1);
					strTemp3.TrimLeft();
					strTemp3.TrimRight();
					if (strTemp2.Find(_T(";")) == 0)
					{
						;
					}
					else if (strTemp2.Find(_T("#")) == 0)
					{
						;
					}
					else
					{
						if (!strTemp2.IsEmpty())
						{
							m_arrURL.Add(strTemp2);
							//ファイル名に使えない文字を置き換える。
							strTemp3 = SBUtil::GetValidFileName(strTemp3);
							m_arrURL_FileName.Add(strTemp3);
						}
					}
				}
			}
			in.Close();
			m_LogMsg += _T("---------------");
		}
	}
	BOOL HitWildCardURL(const CString& strSURL, CStringArray* stArr)
	{
		stArr->RemoveAll();
		BOOL bRet = FALSE;
		CString strRet;
		CStringA strURLA(strSURL);
		CString strTemp;
		CStringA strTempA;

		int imax = (int)m_arrURL.GetSize();
		for (int i = 0; i < imax; i++)
		{
			strTemp.Empty();
			strTemp = m_arrURL.GetAt(i);
			strTempA = strTemp;

			//ワイルドカード対応
			if (wildcmp(strTempA, strURLA))
			{
				strRet = m_arrURL_FileName.GetAt(i);
				stArr->Add(strRet);
				//break;
			}
		}
		if (stArr->GetSize() > 0)
			bRet = TRUE;
		return bRet;
	}
	CStringArray m_arrURL_FileName;
};

#define SECURITY_WIN32
#include "sspi.h"
#include "secext.h"
#pragma comment(lib, "secur32.lib")

#define LOG_UPLOAD     0
#define LOG_DOWNLOAD   1
#define LOG_BROWSING   2
#define LOG_ACCESS_ALL 3
#define LOG_M_GET      0
#define LOG_M_POST     1
class SendLogDataUtil
{
public:
	SendLogDataUtil()
	{
		m_pstrData_UTF8 = NULL;
		m_UTF8Len = 0;
		m_ptrDataURLEncode = NULL;
		m_URLEncodeLen = 0;
	}
	SendLogDataUtil(LPCTSTR lp)
	{
		m_pstrData_UTF8 = NULL;
		m_UTF8Len = 0;
		m_ptrDataURLEncode = NULL;
		m_URLEncodeLen = 0;
		SetData(lp);
	}
	void SetData(LPCTSTR lp)
	{
		if (lp)
		{
			m_strData = lp;
			m_strData.TrimLeft();
			m_strData.TrimRight();
			ConvertUTF8(m_strData);
			URLEncode();
			m_strDataJSON = m_strData;
			m_strDataJSON.Replace(_T("\\"), _T("\\\\"));
			m_strDataJSON.Replace(_T("\""), _T("\\\""));
			m_strDataJSON.Replace(_T("/"), _T("\\/"));
		}
	}
	~SendLogDataUtil()
	{
		if (m_pstrData_UTF8)
		{
			delete[] m_pstrData_UTF8;
			m_pstrData_UTF8 = NULL;
			m_UTF8Len = 0;
		}
		if (m_ptrDataURLEncode)
		{
			delete[] m_ptrDataURLEncode;
			m_ptrDataURLEncode = NULL;
			m_URLEncodeLen = 0;
		}
	}
	CString m_strData;
	CString m_strDataJSON;

	void ConvertUTF8(const CString& src)
	{
		_wsetlocale(LC_ALL, _T("jpn"));
		if (src.IsEmpty()) return;
		try
		{
			if (m_pstrData_UTF8)
			{
				delete[] m_pstrData_UTF8;
				m_pstrData_UTF8 = NULL;
			}
			long size = src.GetLength() + 1;
			wchar_t* lpWideString = new wchar_t[size]{0};
			StringCchCopy(lpWideString, size, src);
			long size2 = src.GetLength() * 3 + 2;
			m_pstrData_UTF8 = new char[size2];
			memset(m_pstrData_UTF8, 0x00, size2);
			m_UTF8Len = WideCharToMultiByte(CP_UTF8, 0, lpWideString, -1, m_pstrData_UTF8, size2, NULL, NULL);
			delete[] lpWideString;
		}
		catch (...)
		{
			ATLASSERT(0);
		}
		return;
	}
	CString GetURLEncode()
	{
		CString strRet;
		if (m_ptrDataURLEncode)
		{
			strRet = m_ptrDataURLEncode;
		}
		return strRet;
	}
	char* m_pstrData_UTF8;
	long m_UTF8Len;

protected:
	char* m_ptrDataURLEncode;
	long m_URLEncodeLen;

public:
	void URLEncode()
	{
		if (!m_pstrData_UTF8)
			return;
		try
		{
			const int nLen = min((int)strlen(m_pstrData_UTF8), m_UTF8Len);
			if (m_ptrDataURLEncode)
			{
				delete[] m_ptrDataURLEncode;
				m_ptrDataURLEncode = NULL;
				m_URLEncodeLen = 0;
			}
			long size = m_UTF8Len * 10;
			m_ptrDataURLEncode = new char[size];
			m_URLEncodeLen = size;
			memset(m_ptrDataURLEncode, 0x00, size);
			int iPos = 0;
			BYTE cText = 0;
			for (int i = 0; i < nLen; ++i)
			{
				cText = m_pstrData_UTF8[i];
				if ((cText >= '0' && cText <= '9') ||
				    (cText >= 'a' && cText <= 'z') ||
				    (cText >= 'A' && cText <= 'Z') ||
				    cText == '-' ||
				    cText == '_' ||
				    cText == '.' ||
				    cText == '!' ||
				    cText == '~' ||
				    cText == '*' ||
				    cText == '\'' ||
				    cText == '(' ||
				    cText == ')')
				{
					memcpy(m_ptrDataURLEncode + iPos, &cText, 1);
					iPos++;
				}
				else if (cText == ' ')
				{
					memcpy(m_ptrDataURLEncode + iPos, "%20", 3);
					iPos += 3;
				}
				else
				{
					char szFmt[4] = {0};
					snprintf(szFmt, sizeof(szFmt), "%%%02X", cText & 0xff);
					memcpy(m_ptrDataURLEncode + iPos, szFmt, 3);
					iPos += 3;
				}
			}
		}
		catch (...)
		{
			ATLASSERT(0);
		}
		return;
	}
};

class CLogDispatcher
{
public:
	CLogDispatcher()
	{
		m_iLogType = 0;
		m_hEventSendMsg = NULL;
		DWORD pidCurrent = GetCurrentProcessId();
		m_strEventSendMsgName.Format(_T("TF_EventSendMsg%08x"), pidCurrent);
		m_hEventSendMsg = CreateEvent(NULL, FALSE, TRUE, m_strEventSendMsgName);
		m_pMonitorThread = NULL;
		m_bStop = FALSE;
		m_iIndexVal = 999;
		m_bStopFin = 0;
	}
	~CLogDispatcher()
	{
		if (m_bStop)
		{
			for (int i = 0; i < 150; i++)
			{
				if (m_bStopFin)
					break;
				::Sleep(100);
			}
		}
		if (m_hEventSendMsg)
		{
			SetEvent(m_hEventSendMsg);
			CloseHandle(m_hEventSendMsg);
			m_hEventSendMsg = NULL;
		}
	}
	CString GetUserNameData();
	CString GetComputerNameData();
	CString GetOpStr(int iLogType);
	CString ConvertUTF8(const CString& srcIn);
	CStringA ConvertUTF8A(char* srcIn);
	CString URLEncode(LPCTSTR strTextP);
	CStringA URLEncodeA(LPCSTR strTextP);
	ULONG m_iIndexVal;
	CString m_strIndexTMP;
	CMapStringToPtr m_MapLogThreadMgr;
	CString m_strIndex;
	CString m_strForceStopIdx;
	BOOL m_bStop;
	BOOL m_bStopFin;
	CWinThread* m_pMonitorThread;
	void Init();
	CString GetIndex()
	{
		CString strRet;
		strRet = _T("TLM:9999");
		try
		{
			strRet.Format(_T("TLM:%08d"), InterlockedIncrement(&m_iIndexVal));
		}
		catch (...)
		{
		}
		return strRet;
	}

	void AddThreadMGR(CWinThread* pThread, LPCTSTR lpIndex)
	{
		try
		{
			if (pThread == NULL) return;
			if (lpIndex == NULL) return;
			CWinThread* pThreadLocal = NULL;
			if (m_MapLogThreadMgr.Lookup(lpIndex, (void*&)pThreadLocal))
			{
				if (pThreadLocal)
				{
					// スレッド終了待ち
					if (::WaitForSingleObject(pThreadLocal->m_hThread, 5000) == WAIT_TIMEOUT)
					{
						// スレッド強制停止
						// (絶対に停止するなら WaitForSingleObjectで INFINITE も可）
#pragma warning(push, 0)
//警告 C6258 TerminateThread を使用すると、正しくスレッドをクリーンアップすることができません。
// -> 普通に停止できなかった場合に強制停止するために使っている。正しい使い方なので警告を無視。
#pragma warning(disable : 6258)
						::TerminateThread(pThreadLocal->m_hThread, 0xffffffff);
#pragma warning(pop)
					}
				}
				ASSERT(FALSE);
				m_MapLogThreadMgr.RemoveKey(lpIndex);
			}
			else
			{
				m_MapLogThreadMgr.SetAt(lpIndex, pThread);
			}
		}
		catch (...)
		{
			ATLASSERT(0);
		}
	}
	void CloseThreadList(LPCTSTR lpIndex)
	{
		try
		{
			void* iVal = 0;
			if (m_MapLogThreadMgr.Lookup(lpIndex, iVal))
			{
				m_MapLogThreadMgr.SetAt(lpIndex, NULL);
			}
		}
		catch (...)
		{
			ATLASSERT(0);
		}
	}
	CString m_logmsg;
	void ChkThread();

	void SendLog(int iLogType, LPCTSTR lpFileName, LPCTSTR lpTargetURL);
	int SendLogThread(int iLogType, LPCTSTR lpFileName, LPCTSTR lpTargetURL);

	int m_iLogType;
	CString m_strFileName;
	CString m_strTargetURL;

protected:
	HANDLE m_hEventSendMsg;
	CString m_strEventSendMsgName;
};
#include <ActivScp.h>
#include <comdef.h>
#pragma comment(lib, "comsuppw.lib")
class CMyObject : public IDispatch
{
private:
	ULONG m_uRef;

public:
	CMyObject()
	    : m_uRef(1){};
	~CMyObject(){};

	//IUnknown
	STDMETHODIMP_(ULONG)
	AddRef()
	{
		return InterlockedIncrement(&m_uRef);
	};

	STDMETHODIMP_(ULONG)
	Release()
	{
		ULONG ulVal = InterlockedDecrement(&m_uRef);
		if (ulVal > 0)
			return ulVal;

		delete this;
		return ulVal;
	};

	STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvOut)
	{
		try
		{
			if (*ppvOut)
				*ppvOut = NULL;

			if (IsEqualIID(riid, IID_IDispatch))
				*ppvOut = (IDispatch*)this;
			else if (IsEqualIID(riid, IID_IUnknown))
				*ppvOut = this;
			else
				return E_NOINTERFACE;

			AddRef();
		}
		catch (...)
		{
			ATLASSERT(0);
		}
		return S_OK;
	};

	// IDispatch
	STDMETHODIMP GetTypeInfoCount(UINT __RPC_FAR* pctinfo)
	{
		return E_NOTIMPL;
	}
	STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo __RPC_FAR* __RPC_FAR* ppTInfo)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP GetIDsOfNames(REFIID riid,
				   LPOLESTR __RPC_FAR* rgszNames,
				   UINT cNames,
				   LCID lcid,
				   DISPID __RPC_FAR* rgDispId)
	{
		HRESULT hRes = NOERROR;
		try
		{
			for (UINT i = 0; i < cNames; i++)
			{
				*(rgDispId + i) = DISPID_UNKNOWN;
				hRes = DISP_E_MEMBERNOTFOUND;
				if (lstrcmpW(*(rgszNames + i), L"Warn") == 0)
				{
					*(rgDispId + i) = 1;
					hRes = S_OK;
					return hRes;
				}
				else if (lstrcmpW(*(rgszNames + i), L"shExpMatch") == 0)
				{
					*(rgDispId + i) = 2;
					hRes = S_OK;
					return hRes;
				}
			}
		}
		catch (...)
		{
			ATLASSERT(0);
		}
		return hRes;
	}
	STDMETHODIMP Invoke(DISPID dispIdMember,
			    REFIID riid,
			    LCID lcid,
			    WORD wFlags,
			    DISPPARAMS FAR* pDispParams,
			    VARIANT FAR* pVarResult,
			    EXCEPINFO FAR* pExcepInfo,
			    unsigned int FAR* puArgErr);
};

//スクリプトで公開するオブジェクト名
#define SCRIPT_OBJECT_NAME L"Message"
class CActiveScriptSite;
class CActiveScriptSite : public IActiveScriptSite
{
private:
	ULONG m_uRef;

public:
	CActiveScriptSite() : m_uRef(1){};
	virtual ~CActiveScriptSite(){};

	//IUnknown
	STDMETHODIMP_(ULONG)
	AddRef()
	{
		return InterlockedIncrement(&m_uRef);
	};
	STDMETHODIMP_(ULONG)
	Release()
	{
		ULONG ulVal = InterlockedDecrement(&m_uRef);
		if (ulVal > 0)
			return ulVal;
		delete this;
		return ulVal;
	};

	STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvOut)
	{
		try
		{
			if (*ppvOut)
				*ppvOut = NULL;

			if (IsEqualIID(riid, IID_IActiveScriptSite))
				*ppvOut = (IActiveScriptSite*)this;
			else if (IsEqualIID(riid, IID_IUnknown))
				*ppvOut = this;
			else
				return E_NOINTERFACE;

			AddRef();
		}
		catch (...)
		{
			ATLASSERT(0);
		}
		return S_OK;
	};

	STDMETHODIMP GetLCID(LCID* plcid)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP GetItemInfo(LPCOLESTR pstrName,
				 DWORD dwReturnMask,
				 IUnknown** ppiunkItem,
				 ITypeInfo** ppti)
	{
		try
		{
			if ((dwReturnMask & SCRIPTINFO_IUNKNOWN))
			{
				if (lstrcmpW(pstrName, SCRIPT_OBJECT_NAME) == 0)
				{
					*ppiunkItem = (IUnknown*)new CMyObject();
					return S_OK;
				}
			}
		}
		catch (...)
		{
			ATLASSERT(0);
		}
		return TYPE_E_ELEMENTNOTFOUND;
	}

	STDMETHODIMP GetDocVersionString(BSTR* pbstrVersion)
	{
		return E_NOTIMPL;
	}
	STDMETHODIMP OnScriptError(IActiveScriptError* pscripterror);
	STDMETHODIMP OnStateChange(SCRIPTSTATE ssScriptState)
	{
		return S_OK;
	}
	STDMETHODIMP OnScriptTerminate(const VARIANT* pvarResult, const EXCEPINFO* pexcepinfo)
	{
		return S_OK;
	}
	STDMETHODIMP OnEnterScript(void)
	{
		return S_OK;
	}
	STDMETHODIMP OnLeaveScript(void)
	{
		return S_OK;
	}
};
//マクロ定義
#define CREATEINSTANCE(C, I, P) (SUCCEEDED(CoCreateInstance((C), NULL, CLSCTX_INPROC_SERVER, (I), (reinterpret_cast<LPVOID*>(P)))))
#define QI(X, Y, Z)		((X)->QueryInterface((Y), (reinterpret_cast<LPVOID*>(Z))))
#define URLBUFFER_SIZE		4096

class CScriptHost
{
public:
	CScriptHost() {}
	~CScriptHost()
	{
	}

	BOOL ExecScript(LPCTSTR sURL, BOOL bTop, LPCTSTR sUserScript, CString& strReturnString)
	{
		PROC_TIME(ExecScript)

		BOOL bRet = FALSE;
		try
		{
			strReturnString.Empty();
			HRESULT hRes = E_INVALIDARG;
			CLSID CLSID_Script;
			CLSIDFromProgID(L"VBScript", &CLSID_Script);

			CComPtr<IActiveScript> pAS;
			CComPtr<IActiveScriptParse> pASP;
			CComPtr<IActiveScriptSite> pASS;
			if (CREATEINSTANCE(CLSID_Script, IID_IActiveScript, &pAS))
			{
				pASS.Attach(new CActiveScriptSite());
				// スクリプトエンジンにSiteを設定
				if (FAILED(hRes = pAS->SetScriptSite(pASS)))
					goto cleanup;
				// スクリプト解析インターフェイスの取得
				if (FAILED(hRes = QI(pAS, IID_IActiveScriptParse, &pASP)))
					goto cleanup;
				// 初回のみ、以後Closeするまで呼び出し不可
				if (FAILED(hRes = pASP->InitNew()))
					goto cleanup;
				//スクリプト側でアクセスできるようにスクリプトのグローバル変数に名前を追加。
				if (FAILED(hRes = pAS->AddNamedItem(SCRIPT_OBJECT_NAME, SCRIPTITEM_GLOBALMEMBERS | SCRIPTITEM_ISVISIBLE)))
					goto cleanup;
				CString vbFormatTemp = sURL;
				vbFormatTemp.Replace(_T("\""), _T("\"\""));

				CString vbGlobalURL;
				vbGlobalURL.Format(_T("Const TB_Global_URL=\"%s\"\r\n"), (LPCTSTR)vbFormatTemp);

				//互換の為 Trueをセット
				CString vbGlobalTopPage;
				vbGlobalTopPage.Format(_T("Const TB_Global_TOP_PAGE=%s\r\n"), bTop ? _T("True") : _T("False"));

				//スクリプトの互換性のためだけに残す。ブランクを設定する。
				CString vbGlobalZone;
				//vbGlobalZone.Format(_T("Const TB_Global_ZONE=\"%s\"\r\n"),_T("INTERNET_ZONE"));
				vbGlobalZone = _T("Const TB_Global_ZONE=\"\"\r\n");

				//互換のためBeforeNavigateをセット
				CString vbGlobalEvent;
				//if(bEventBeforeNavigate)
				{
					vbGlobalEvent.Format(_T("Const TB_Global_EVENT=\"%s\"\r\n"), _T("BeforeNavigate"));
				}

				//Helper追加
				CString strHelper;
				CString strURL;
				strURL = sURL;
				CString TB_Global_SCHEME;
				CString TB_Global_HOSTNAME;
				CString TB_Global_PORT;
				CString TB_Global_URL_PATH;
				CString TB_Global_URL_EXTRAINFO;

				URL_COMPONENTS urlcomponents = {0};
				ZeroMemory(&urlcomponents, sizeof(URL_COMPONENTS));
				urlcomponents.dwStructSize = sizeof(URL_COMPONENTS);

				std::unique_ptr<TCHAR> szSchme(new TCHAR[64]());
				std::unique_ptr<TCHAR> szHostName(new TCHAR[URLBUFFER_SIZE]());
				std::unique_ptr<TCHAR> szUrlPath(new TCHAR[URLBUFFER_SIZE]());
				std::unique_ptr<TCHAR> szUrlExtra(new TCHAR[URLBUFFER_SIZE]());

				urlcomponents.lpszScheme = szSchme.get();
				urlcomponents.dwSchemeLength = 64;

				urlcomponents.lpszHostName = szHostName.get();
				urlcomponents.dwHostNameLength = URLBUFFER_SIZE;

				urlcomponents.lpszUrlPath = szUrlPath.get();
				urlcomponents.dwUrlPathLength = URLBUFFER_SIZE;

				urlcomponents.lpszExtraInfo = szUrlExtra.get();
				urlcomponents.dwExtraInfoLength = URLBUFFER_SIZE;

				InternetCrackUrl(strURL, 0, 0, &urlcomponents);
				TB_Global_SCHEME = urlcomponents.lpszScheme;
				TB_Global_SCHEME.Replace(_T("\""), _T("\"\""));
				TB_Global_HOSTNAME = urlcomponents.lpszHostName;
				TB_Global_HOSTNAME.Replace(_T("\""), _T("\"\""));
				TB_Global_PORT.Format(_T("%d"), urlcomponents.nPort);
				TB_Global_PORT.Replace(_T("\""), _T("\"\""));
				TB_Global_URL_PATH = urlcomponents.lpszUrlPath;
				TB_Global_URL_PATH.Replace(_T("\""), _T("\"\""));
				TB_Global_URL_EXTRAINFO = urlcomponents.lpszExtraInfo;
				TB_Global_URL_EXTRAINFO.Replace(_T("\""), _T("\"\""));

				strHelper.Format(_T("Const TB_Global_SCHEME=\"%s\"\r\nConst TB_Global_HOSTNAME=\"%s\"\r\nConst TB_Global_PORT=\"%s\"\r\nConst TB_Global_URL_PATH=\"%s\"\r\nConst TB_Global_URL_EXTRAINFO=\"%s\"\r\n"),
						 (LPCTSTR)TB_Global_SCHEME,
						 (LPCTSTR)TB_Global_HOSTNAME,
						 (LPCTSTR)TB_Global_PORT,
						 (LPCTSTR)TB_Global_URL_PATH,
						 (LPCTSTR)TB_Global_URL_EXTRAINFO);

				CStringW vbGlobalSettings;
				vbGlobalSettings = vbGlobalURL;
				vbGlobalSettings += vbGlobalTopPage;
				vbGlobalSettings += vbGlobalZone;
				vbGlobalSettings += vbGlobalEvent;
				vbGlobalSettings += strHelper;
				vbGlobalSettings += _T("\r\n\r\n");

				CStringW vbGlobalFunctions;
				vbGlobalFunctions = L"\r\nFunction MsgBox(prompt)\r\nMessage.Warn(prompt)\r\nEnd Function\r\n";
				vbGlobalFunctions += L"\r\nFunction TB_TRACE_LOG(prompt)\r\nMessage.Warn(prompt)\r\nEnd Function\r\n";

				CStringW VBSource;
				VBSource += sUserScript;
				VBSource += L"\r\n\r\n";
				VBSource += vbGlobalSettings;

				VBSource += vbGlobalFunctions;

				LPCWSTR pSource = NULL;
				pSource = VBSource;

				//スクリプトコードのロードと解析。
#pragma warning(push)
//ParseScriptTextの各引数とGetScriptDispatchの第一引数はNULL許容なので、「NULLの可能性がある」警告は無視する。
#pragma warning(disable : 6387)
				if (FAILED(hRes = pASP->ParseScriptText(pSource,
									NULL,
									NULL,
									NULL,
									0,
									1,
									SCRIPTTEXT_ISPERSISTENT | SCRIPTTEXT_ISVISIBLE,
									NULL,
									NULL)))
					goto cleanup;

				//スクリプト実行開始
				if (FAILED(hRes = pAS->SetScriptState(SCRIPTSTATE_CONNECTED)))
					goto cleanup;

				// スクリプトのルートコンテキストを取得
				CComPtr<IDispatch> pScriptDisp;
				if (FAILED(hRes = pAS->GetScriptDispatch(NULL, &pScriptDisp)))
					goto cleanup;
#pragma warning(pop)
				// スクリプト上に定義されているtestfuncの呼び出し
				LPOLESTR szMember[] = {L"OnRedirect", NULL};
				DISPID dispids[1] = {0};

				HRESULT hr = pScriptDisp->GetIDsOfNames(IID_NULL, &szMember[0], 1,
									LOCALE_SYSTEM_DEFAULT, &dispids[0]);
				//ATLASSERT(SUCCEEDED(hr));
				if (FAILED(hr))
				{
					pScriptDisp.Release();
					goto cleanup;
				}

				DISPPARAMS param = {NULL, NULL, 0, 0};
				VARIANT varResult = {0};
				VariantInit(&varResult);

				hr = pScriptDisp->Invoke(dispids[0], IID_NULL, LOCALE_SYSTEM_DEFAULT,
							 DISPATCH_METHOD, &param, &varResult, NULL, NULL);
				if (FAILED(hr))
				{
					pScriptDisp.Release();
					goto cleanup;
				}
				if (!(varResult.vt == VT_EMPTY || varResult.vt == VT_NULL))
				{
					CString strResult = V_BSTR(&varResult);
					strReturnString = strResult;
				}
				VariantClear(&varResult);

				pScriptDisp.Release();

				if (FAILED(hRes = pAS->SetScriptState(SCRIPTSTATE_CLOSED)))
					goto cleanup;

				bRet = TRUE;
			}
		cleanup:
			//後始末
			if (pASP)
			{
				pASP.Release();
			}
			if (pAS)
			{
				pAS.Release();
			}
			if (pASS)
			{
				pASS.Release();
			}
		}
		catch (...)
		{
			ATLASSERT(0);
		}
		return bRet;
	}
	BOOL ExecFilterScript(LPCTSTR sURL,
			      LPCTSTR sSchme,
			      LPCTSTR sHost,
			      LPCTSTR sPath,
			      LPCTSTR sUserScript, CString& strReturnString)
	{
		PROC_TIME(ExecFilterScript)

		BOOL bRet = FALSE;
		try
		{
			strReturnString.Empty();
			HRESULT hRes = E_INVALIDARG;
			CLSID CLSID_Script;
			CLSIDFromProgID(L"VBScript", &CLSID_Script);

			CComPtr<IActiveScript> pAS;
			CComPtr<IActiveScriptParse> pASP;
			CComPtr<IActiveScriptSite> pASS;
			if (CREATEINSTANCE(CLSID_Script, IID_IActiveScript, &pAS))
			{
				pASS.Attach(new CActiveScriptSite());

				// スクリプトエンジンにSiteを設定
				if (FAILED(hRes = pAS->SetScriptSite(pASS)))
					goto cleanup;

				// スクリプト解析インターフェイスの取得
				if (FAILED(hRes = QI(pAS, IID_IActiveScriptParse, &pASP)))
					goto cleanup;

				// 初回のみ、以後Closeするまで呼び出し不可
				if (FAILED(hRes = pASP->InitNew()))
					goto cleanup;

				//スクリプト側でアクセスできるようにスクリプトのグローバル変数に名前を追加。
				if (FAILED(hRes = pAS->AddNamedItem(SCRIPT_OBJECT_NAME, SCRIPTITEM_GLOBALMEMBERS | SCRIPTITEM_ISVISIBLE)))
					goto cleanup;

				CString vbFormatTemp = sURL;
				vbFormatTemp.Replace(_T("\""), _T("\"\""));

				CString strSchme = sSchme;
				strSchme.Trim();
				strSchme.Replace(_T("\""), _T("\"\""));
				CString strHost = sHost;
				strHost.Trim();
				strHost.Replace(_T("\""), _T("\"\""));
				CString strPath = sPath;
				strPath.Trim();
				strPath.Replace(_T("\""), _T("\"\""));

				CString vbGlobalURL;
				//Option Explicit\r\n
				vbGlobalURL.Format(_T("Const TB_Global_URL=\"%s\"\r\nConst TB_Global_Scheme=\"%s\"\r\nConst TB_Global_Host=\"%s\"\r\nConst TB_Global_Path=\"%s\"\r\n\r\n"),
						   (LPCTSTR)vbFormatTemp,
						   (LPCTSTR)strSchme,
						   (LPCTSTR)strHost,
						   (LPCTSTR)strPath);

				CStringW vbGlobalSettings;
				vbGlobalSettings = vbGlobalURL;

				vbGlobalSettings += _T("\r\n\r\n");

				CStringW vbGlobalFunctions;
				vbGlobalFunctions = L"\r\nFunction MsgBox(prompt)\r\nMessage.Warn(prompt)\r\nEnd Function\r\n";
				vbGlobalFunctions += L"\r\nFunction TB_TRACE_LOG(prompt)\r\nMessage.Warn(prompt)\r\nEnd Function\r\n";
				vbGlobalFunctions += L"\r\nFunction shExpMatch(sTR,pA)\r\nshExpMatch=Message.shExpMatch(sTR,pA)\r\nEnd Function\r\n";

				CStringW VBSource;
				VBSource += vbGlobalSettings;
				VBSource += sUserScript;
				VBSource += L"\r\n\r\n";
				VBSource += vbGlobalFunctions;

				LPCWSTR pSource = NULL;
				pSource = VBSource;

#pragma warning(push)
//ParseScriptTextの各引数とGetScriptDispatchの第一引数はNULL許容なので、「NULLの可能性がある」警告は無視する。
#pragma warning(disable : 6387)
				//スクリプトコードのロードと解析。
				if (FAILED(hRes = pASP->ParseScriptText(pSource,
									NULL,
									NULL,
									NULL,
									0,
									1,
									SCRIPTTEXT_ISPERSISTENT | SCRIPTTEXT_ISVISIBLE,
									NULL,
									NULL)))
					goto cleanup;

				//スクリプト実行開始
				if (FAILED(hRes = pAS->SetScriptState(SCRIPTSTATE_CONNECTED)))
					goto cleanup;

				// スクリプトのルートコンテキストを取得
				CComPtr<IDispatch> pScriptDisp;
				if (FAILED(hRes = pAS->GetScriptDispatch(NULL, &pScriptDisp)))
					goto cleanup;
#pragma warning(pop)
				// スクリプト上に定義されているtestfuncの呼び出し
				LPOLESTR szMember[] = {L"URLFilter", NULL};
				DISPID dispids[1] = {0};

				HRESULT hr = pScriptDisp->GetIDsOfNames(IID_NULL, &szMember[0], 1,
									LOCALE_SYSTEM_DEFAULT, &dispids[0]);

				if (FAILED(hr))
				{
					pScriptDisp.Release();
					goto cleanup;
				}

				DISPPARAMS param = {NULL, NULL, 0, 0};
				VARIANT varResult = {0};
				VariantInit(&varResult);

				hr = pScriptDisp->Invoke(dispids[0], IID_NULL, LOCALE_SYSTEM_DEFAULT,
							 DISPATCH_METHOD, &param, &varResult, NULL, NULL);
				if (FAILED(hr))
				{
					pScriptDisp.Release();
					goto cleanup;
				}
				if (!(varResult.vt == VT_EMPTY || varResult.vt == VT_NULL))
				{
					CString strResult = V_BSTR(&varResult);
					strReturnString = strResult;
				}
				VariantClear(&varResult);

				pScriptDisp.Release();

				if (FAILED(hRes = pAS->SetScriptState(SCRIPTSTATE_CLOSED)))
					goto cleanup;

				bRet = TRUE;
			}
		cleanup:
			//後始末
			if (pASP)
			{
				pASP.Release();
			}
			if (pAS)
			{
				pAS.Release();
			}
			if (pASS)
			{
				pASS.Release();
			}
		}
		catch (...)
		{
			ATLASSERT(0);
		}
		return bRet;
	}

private:
};
#include "DlgDL.h"
#include "ShObjIdl.h"
class CTaskbarList3
{
protected:
	ATL::CComPtr<ITaskbarList3> m_pTaskbarList3;
	HWND m_hWnd = nullptr;
	CTaskbarList3(const CTaskbarList3& obj) {}

public:
	CTaskbarList3()
	{
		m_pTaskbarList3 = nullptr;
		m_hWnd = nullptr;
	}
	virtual ~CTaskbarList3() { Uninitialize(); }
	BOOL Initialize(HWND hWnd)
	{
		// ウィンドウハンドルを保持
		m_hWnd = hWnd;
		// すでに作成済みなら成功
		if (NULL != m_pTaskbarList3)
		{
			return TRUE;
		}
		// オブジェクトの作成
		try
		{
			if (FAILED(m_pTaskbarList3.CoCreateInstance(CLSID_TaskbarList)))
			{
				return FALSE;
			}
		}
		catch (COleException* e)
		{
			e->Delete();
			return FALSE;
		}
		catch (COleDispatchException* e)
		{
			e->Delete();
			return FALSE;
		}
		catch (...)
		{
			return FALSE;
		}
		return TRUE;
	}
	void Uninitialize()
	{
		if (NULL != m_pTaskbarList3)
		{
			SetNoProgress();
			m_pTaskbarList3.Release();
			m_pTaskbarList3 = NULL;
		}
	}
	BOOL SetProgress(ULONGLONG ul64Pos, ULONGLONG ul64Max, TBPFLAG tbpFlag)
	{
		HRESULT hResult;
		try
		{
			// 進捗位置変更
			hResult = m_pTaskbarList3->SetProgressValue(m_hWnd, ul64Pos, ul64Max);
			if (FAILED(hResult))
			{
				return FALSE;
			}
			// 進捗色変更
			hResult = m_pTaskbarList3->SetProgressState(m_hWnd, tbpFlag);
			if (FAILED(hResult))
			{
				return FALSE;
			}
		}
		catch (COleException* e)
		{
			e->Delete();
			return FALSE;
		}
		catch (COleDispatchException* e)
		{
			e->Delete();
			return FALSE;
		}
		catch (...)
		{
			return FALSE;
		}
		return TRUE;
	}
	BOOL SetNoProgress()
	{
		HRESULT hResult;
		try
		{
			// 進捗なしに変更
			hResult = m_pTaskbarList3->SetProgressState(m_hWnd, TBPF_NOPROGRESS);
			if (FAILED(hResult))
			{
				return FALSE;
			}
		}
		catch (COleException* e)
		{
			e->Delete();
			return FALSE;
		}
		catch (COleDispatchException* e)
		{
			e->Delete();
			return FALSE;
		}
		catch (...)
		{
			return FALSE;
		}

		return TRUE;
	}
	BOOL SetProgressIndeterminate()
	{
		HRESULT hResult;
		try
		{
			// 進捗なしに変更
			hResult = m_pTaskbarList3->SetProgressState(m_hWnd, TBPF_INDETERMINATE);
			if (FAILED(hResult))
			{
				return FALSE;
			}
		}
		catch (COleException* e)
		{
			e->Delete();
			return FALSE;
		}
		catch (COleDispatchException* e)
		{
			e->Delete();
			return FALSE;
		}
		catch (...)
		{
			return FALSE;
		}

		return TRUE;
	}
};
class CDLManager
{
public:
	CDLManager() {}
	~CDLManager() { ReleaseAll(); }
	CTaskbarList3 m_taskbarList3;
	BOOL IsDlProgress(UINT nBrowserId)
	{
		CDlgDL* pDLDlg = NULL;
		for (int i = 0; i < m_ptrArrDlgBrowserID.GetCount(); i++)
		{
			if (m_ptrArrDlgBrowserID.GetAt(i) == nBrowserId)
			{
				pDLDlg = NULL;
				pDLDlg = (CDlgDL*)m_ptrArrDlgPtr.GetAt(i);
				if (pDLDlg)
				{
					return pDLDlg->m_bDownloadProgress;
				}
			}
		}
		return FALSE;
	}
	void SetDlProgress(UINT nBrowserId, BOOL bProgress)
	{
		CDlgDL* pDLDlg = NULL;
		for (int i = 0; i < m_ptrArrDlgBrowserID.GetCount(); i++)
		{
			if (m_ptrArrDlgBrowserID.GetAt(i) == nBrowserId)
			{
				pDLDlg = NULL;
				pDLDlg = (CDlgDL*)m_ptrArrDlgPtr.GetAt(i);
				if (pDLDlg)
				{
					pDLDlg->m_bDownloadProgress = bProgress;
					return;
				}
			}
		}
	}
	void Init_DLDlg(CWnd* pCWnd, UINT nBrowserId)
	{
		NewDLDlg(pCWnd, nBrowserId);
	}
	void Init_DLDlg(HWND hwnd, UINT nBrowserId)
	{
		CWnd* pCWnd = CWnd::FromHandle(hwnd);
		NewDLDlg(pCWnd, nBrowserId);
	}
	void Show_DLDlg(BOOL bShow, UINT nBrowserId)
	{
		CDlgDL* pDLDlg = NULL;
		pDLDlg = Lookup(nBrowserId);
		if (pDLDlg)
		{
			if (bShow)
			{
				pDLDlg->ShowWindow(SW_SHOW);
				m_taskbarList3.SetProgress((ULONGLONG)1, (ULONGLONG)100, TBPF_NORMAL);
			}
			else
			{
				pDLDlg->ShowWindow(SW_HIDE);
				m_taskbarList3.SetNoProgress();
			}
		}
	}
	void Release_DLDlg(UINT nBrowserId)
	{
		CDlgDL* pDLDlg = NULL;
		for (int i = 0; i < m_ptrArrDlgBrowserID.GetCount(); i++)
		{
			if (m_ptrArrDlgBrowserID.GetAt(i) == nBrowserId)
			{
				pDLDlg = NULL;
				pDLDlg = (CDlgDL*)m_ptrArrDlgPtr.GetAt(i);
				if (pDLDlg)
				{
					pDLDlg->DestroyWindow();
					delete pDLDlg;
					m_ptrArrDlgPtr.SetAt(i, NULL);
					m_ptrArrDlgBrowserID.SetAt(i, 0);
					m_taskbarList3.SetNoProgress();
				}
			}
		}
	}
	void DLComp_DLDlg(UINT nBrowserId, LPCTSTR strFilePath)
	{
		CDlgDL* pDLDlg = NULL;
		for (int i = 0; i < m_ptrArrDlgBrowserID.GetCount(); i++)
		{
			if (m_ptrArrDlgBrowserID.GetAt(i) == nBrowserId)
			{
				pDLDlg = NULL;
				pDLDlg = (CDlgDL*)m_ptrArrDlgPtr.GetAt(i);
				if (pDLDlg)
				{
					pDLDlg->SetCompST(TRUE, strFilePath);
					break;
				}
			}
		}
	}
	void Cancel(UINT nBrowserId)
	{
		for (int i = 0; i < m_ptrArrDlgBrowserID.GetCount(); i++)
		{
			if (m_ptrArrDlgBrowserID.GetAt(i) == nBrowserId)
			{
				CDlgDL* pDLDlg = (CDlgDL*)m_ptrArrDlgPtr.GetAt(i);
				if (pDLDlg)
				{
					pDLDlg->m_bDLCancel = TRUE;
					break;
				}
			}
		}
	}
	BOOL IsCanceled(UINT nBrowserId)
	{
		CDlgDL* pDLDlg = NULL;
		for (int i = 0; i < m_ptrArrDlgBrowserID.GetCount(); i++)
		{
			if (m_ptrArrDlgBrowserID.GetAt(i) == nBrowserId)
			{
				pDLDlg = NULL;
				pDLDlg = (CDlgDL*)m_ptrArrDlgPtr.GetAt(i);
				if (pDLDlg)
				{
					return pDLDlg->m_bDLCancel;
				}
			}
		}
		return TRUE;
	}
	void Set_DLDlgState(UINT nBrowserId, INT nProgress, CString strFileName, CString strMsg, CString strTf)
	{
		CDlgDL* pDLDlg = NULL;
		for (int i = 0; i < m_ptrArrDlgBrowserID.GetCount(); i++)
		{
			if (m_ptrArrDlgBrowserID.GetAt(i) == nBrowserId)
			{
				pDLDlg = NULL;
				pDLDlg = (CDlgDL*)m_ptrArrDlgPtr.GetAt(i);
				if (pDLDlg)
				{
					if (!strFileName.IsEmpty() && nProgress > 1)
					{
						CString progressTitleTemplate;
						progressTitleTemplate.LoadString(ID_DOWNLOAD_PROGRESS_DIALOG_TITLE);
						CString strTitle;
						strTitle.Format(progressTitleTemplate, nProgress, strFileName);
						pDLDlg->SetWindowText(strTitle);
					}

					pDLDlg->m_Prog.SetPos(nProgress);
					pDLDlg->m_FileName.SetWindowText(strFileName);
					pDLDlg->m_Msg.SetWindowText(strMsg);
					pDLDlg->m_Tf.SetWindowText(strTf);
					pDLDlg->m_strFileFullPath = strFileName;
					TCHAR szFolder[MAX_PATH] = {0};
					StringCchCopy(szFolder, MAX_PATH, strFileName);
					PathCchRemoveFileSpec(szFolder, MAX_PATH);
					pDLDlg->m_strFileFolderPath = szFolder;
					m_taskbarList3.SetProgress((ULONGLONG)nProgress, (ULONGLONG)100, TBPF_NORMAL);
					return;
				}
			}
		}
	}

protected:
	CDlgDL* Lookup(UINT nBrowserId)
	{
		CDlgDL* pDLDlg = NULL;
		for (int i = 0; i < m_ptrArrDlgBrowserID.GetCount(); i++)
		{
			if (m_ptrArrDlgBrowserID.GetAt(i) == nBrowserId)
			{
				pDLDlg = NULL;
				pDLDlg = (CDlgDL*)m_ptrArrDlgPtr.GetAt(i);
				if (pDLDlg)
				{
					return pDLDlg;
				}
			}
		}
		return pDLDlg;
	}
	void ReleaseDLCompDlg()
	{
		CDlgDL* pDLDlg = NULL;
		for (int i = 0; i < m_ptrArrDlgPtr.GetCount(); i++)
		{
			pDLDlg = NULL;
			pDLDlg = (CDlgDL*)m_ptrArrDlgPtr.GetAt(i);
			if (pDLDlg)
			{
				if (!IsWindow(pDLDlg->m_hWnd))
				{
					delete pDLDlg;
					m_ptrArrDlgPtr.SetAt(i, NULL);
					m_ptrArrDlgBrowserID.SetAt(i, 0);
				}
				else
				{
					if (!::IsWindowVisible(pDLDlg->m_hWnd) && pDLDlg->GetCompST())
					{
						pDLDlg->DestroyWindow();
						delete pDLDlg;
						m_ptrArrDlgPtr.SetAt(i, NULL);
						m_ptrArrDlgBrowserID.SetAt(i, 0);
					}
				}
			}
		}
	}
	void NewDLDlg(CWnd* pCWnd, UINT nBrowserId)
	{
		ReleaseDLCompDlg();
		CDlgDL* pDLDlg = NULL;
		pDLDlg = new CDlgDL(pCWnd, nBrowserId);
		pDLDlg->Create(MAKEINTRESOURCE(IDD_DLG_DOWNLOAD), pCWnd);
		m_ptrArrDlgPtr.Add(pDLDlg);
		m_ptrArrDlgBrowserID.Add(nBrowserId);
		m_taskbarList3.Initialize(pDLDlg->m_hWnd);
		int iCnt = -1;
		for (int i = 0; i < m_ptrArrDlgPtr.GetCount(); i++)
		{
			CDlgDL* pDLDlg = NULL;
			pDLDlg = (CDlgDL*)m_ptrArrDlgPtr.GetAt(i);
			if (pDLDlg)
			{
				iCnt++;
			}
		}

		//重なり調整
		CRect rt;
		::GetWindowRect(pDLDlg->m_hWnd, &rt);
		rt.OffsetRect(50 * iCnt, 50 * iCnt);
		::SetWindowPos(pDLDlg->m_hWnd, NULL, rt.left, rt.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	}
	void ReleaseAll()
	{
		for (int i = 0; i < m_ptrArrDlgPtr.GetCount(); i++)
		{
			CDlgDL* pDLDlg = NULL;
			pDLDlg = (CDlgDL*)m_ptrArrDlgPtr.GetAt(i);
			if (pDLDlg)
				delete pDLDlg;
			m_ptrArrDlgPtr.SetAt(i, NULL);
			m_ptrArrDlgBrowserID.SetAt(i, 0);
		}
		m_ptrArrDlgPtr.RemoveAll();
		m_ptrArrDlgBrowserID.RemoveAll();
	}
	CPtrArray m_ptrArrDlgPtr;
	CUIntArray m_ptrArrDlgBrowserID;
};
#include <afxtaskdialog.h>
class CMyTaskDlg : public CTaskDialog
{
public:
	CMyTaskDlg(_In_ const CString& strContent, _In_ const CString& strMainInstruction, _In_ const CString& strTitle,
		   _In_ int nCommonButtons = TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON, _In_ int nTaskDialogOptions = TDF_ENABLE_HYPERLINKS | TDF_USE_COMMAND_LINKS,
		   _In_ const CString& strFooter = CString()) : CTaskDialog(strContent, strMainInstruction, strTitle, nCommonButtons, nTaskDialogOptions, strFooter)
	{
		m_lTimer = 0L;
		m_hMyWnd = NULL;
		m_DefButton = 0;
	}
	void SetTimeout(long lTime, DWORD defButton = IDOK)
	{
		if (lTime > 0)
		{
			SetOptions(TDF_CALLBACK_TIMER);
			m_lTimer = lTime;
			m_DefButton = defButton;
		}
	}
	long m_lTimer;
	HWND m_hMyWnd;
	DWORD m_DefButton;
	HRESULT OnInit();
	HRESULT OnDestroy();
	HRESULT OnCreate()
	{
		return S_OK;
	}
	HRESULT OnTimer(_In_ long lTime)
	{
		if (m_lTimer && lTime >= m_lTimer)
		{
			m_lTimer = 0L;
			if (m_hMyWnd)
			{
				SendMessage(m_hMyWnd, TDM_CLICK_BUTTON, m_DefButton, 0);
			}
		}
		return S_OK;
	}
	virtual ~CMyTaskDlg() {}

protected:
};
