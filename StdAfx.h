#pragma once

#define VC_EXTRALEAN

#include <winsdkver.h>

#ifndef _WIN32_WINNT
// Modify the following define to target a prior platform. Refer to MSDN
// for the latest info on corresponding values for different platforms.
#define _WIN32_WINNT _WIN32_WINNT_MAXVER
#endif
#include <sdkddkver.h>
#include "afxole.h"
#include <afxwin.h>
#include <afxext.h>
#include <afxdisp.h>
#include <afxdtctl.h>
#include "atlbase.h"
#include "tlogstg.h"
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>
#endif			    // _AFX_NO_AFXCMN_SUPPORT
#include <afxcontrolbars.h> // MFC support for ribbons and control bars
#include <locale.h>
#include <Tlhelp32.h>
#include "wininet.h"
#pragma comment(lib, "Wininet.lib")

#undef BOOLAPI
#undef SECURITY_FLAG_IGNORE_CERT_DATE_INVALID
#undef SECURITY_FLAG_IGNORE_CERT_CN_INVALID
#define URL_COMPONENTS	    URL_COMPONENTS_ANOTHER
#define URL_COMPONENTSA	    URL_COMPONENTSA_ANOTHER
#define URL_COMPONENTSW	    URL_COMPONENTSW_ANOTHER
#define LPURL_COMPONENTS    LPURL_COMPONENTS_ANOTHER
#define LPURL_COMPONENTSA   LPURL_COMPONENTS_ANOTHER
#define LPURL_COMPONENTSW   LPURL_COMPONENTS_ANOTHER
#define INTERNET_SCHEME	    INTERNET_SCHEME_ANOTHER
#define LPINTERNET_SCHEME   LPINTERNET_SCHEME_ANOTHER
#define HTTP_VERSION_INFO   HTTP_VERSION_INFO_ANOTHER
#define LPHTTP_VERSION_INFO LPHTTP_VERSION_INFO_ANOTHER

#include <Winhttp.h>
#pragma comment(lib, "Winhttp.lib")
#undef URL_COMPONENTS
#undef URL_COMPONENTSA
#undef URL_COMPONENTSW
#undef LPURL_COMPONENTS
#undef LPURL_COMPONENTSA
#undef LPURL_COMPONENTSW
#undef INTERNET_SCHEME
#undef LPINTERNET_SCHEME
#undef HTTP_VERSION_INFO
#undef LPHTTP_VERSION_INFO

// CEF libraries
#ifdef _DEBUG
#pragma comment(lib, "lib/libcef.lib")
#pragma comment(lib, "lib/libcef_dll_wrapper.lib")
#else
#pragma comment(lib, "rlib/libcef.lib")
#pragma comment(lib, "rlib/libcef_dll_wrapper.lib")
#endif

#pragma warning(push, 0)
#include <codeanalysis/warnings.h>
#pragma warning(disable \
		: ALL_CODE_ANALYSIS_WARNINGS)
#include "include/cef_base.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#pragma warning(pop)
// Set to 0 to disable sandbox support.
#define CEF_ENABLE_SANDBOX 0
#if CEF_ENABLE_SANDBOX
// The cef_sandbox.lib static library is currently built with VS2010. It may not
// link successfully with other VS versions.
#pragma comment(lib, "cef_sandbox.lib")
#endif

#define WM_APP_CEF_LOAD_START	   (WM_APP + 301)
#define WM_APP_CEF_LOAD_END	   (WM_APP + 302)
#define WM_APP_CEF_TITLE_CHANGE	   (WM_APP + 303)
#define WM_APP_CEF_ADDRESS_CHANGE  (WM_APP + 304)
#define WM_APP_CEF_STATE_CHANGE	   (WM_APP + 305)
#define WM_APP_CEF_STATUS_MESSAGE  (WM_APP + 306)
#define WM_APP_CEF_FAVICON_MESSAGE (WM_APP + 307)
#define WM_APP_CEF_NEW_WINDOW	   (WM_APP + 310)
#define WM_APP_CEF_CLOSE_BROWSER   (WM_APP + 312)
//#define WM_APP_CEF_NEW_BROWSER		(WM_APP + 315)
#define WM_APP_CEF_NEW_WINDOW2	    (WM_APP + 316)
#define WM_APP_CEF_SET_RENDERER_PID (WM_APP + 317)

#define WM_APP_CEF_SEARCH_URL		  (WM_APP + 325)
#define WM_APP_CEF_BEFORE_BROWSE	  (WM_APP + 350)
#define WM_APP_CEF_DOWNLOAD_UPDATE	  (WM_APP + 355)
#define WM_APP_CEF_BEFORE_DOWNLOAD	  (WM_APP + 356)
#define WM_APP_CEF_DOWNLOAD_BLANK_PAGE	  (WM_APP + 357)
#define WM_APP_CEF_WINDOW_ACTIVATE	  (WM_APP + 358)
#define WM_APP_CEF_BEFORE_RESOURCE_LOAD	  (WM_APP + 360)
#define WM_APP_CEF_FULLSCREEN_MODE_CHANGE (WM_APP + 365)
#define WM_APP_CEF_PROGRESS_CHANGE	  (WM_APP + 366)

#define WM_APP_CEF_AUTHENTICATE	   (WM_APP + 370)
#define WM_APP_CEF_BAD_CERTIFICATE (WM_APP + 371)

#define CEF_MENU_ID_OPEN_LINK		   (MENU_ID_USER_FIRST + 100)
#define CEF_MENU_ID_OPEN_LINK_TAB	   (MENU_ID_USER_FIRST + 101)
#define CEF_MENU_ID_OPEN_LINK_NEW	   (MENU_ID_USER_FIRST + 102)
#define CEF_MENU_ID_OPEN_LINK_NEW_NOACTIVE (MENU_ID_USER_FIRST + 103)
#define CEF_MENU_ID_SAVE_IMG		   (MENU_ID_USER_FIRST + 105)
#define CEF_MENU_ID_OPEN_IMG		   (MENU_ID_USER_FIRST + 106)
#define CEF_MENU_ID_OPEN_IMG_NOACTIVE	   (MENU_ID_USER_FIRST + 107)
#define CEF_MENU_ID_OPEN_SEARCH		   (MENU_ID_USER_FIRST + 108)
#define CEF_MENU_ID_COPY_LINK		   (MENU_ID_USER_FIRST + 109)
#define CEF_MENU_ID_IMG_COPY_LINK	   (MENU_ID_USER_FIRST + 110)
#define CEF_MENU_ID_IMG_COPY		   (MENU_ID_USER_FIRST + 111)
#define CEF_MENU_ID_SAVE_FILE		   (MENU_ID_USER_FIRST + 112)

#define CEF_BIT_IS_LOADING     0x001
#define CEF_BIT_CAN_GO_BACK    0x002
#define CEF_BIT_CAN_GO_FORWARD 0x004

struct CEFAuthenticationValues
{
	LPCTSTR lpszHost;
	LPCTSTR lpszRealm;
	TCHAR szUserName[1024];
	TCHAR szUserPass[1024];
};

struct CEFDownloadItemValues
{
	BOOL bIsValid;
	BOOL bIsInProgress;
	BOOL bIsComplete;
	BOOL bIsCanceled;
	INT nProgress;
	LONGLONG nSpeed;
	LONGLONG nReceived;
	LONGLONG nTotal;
	TCHAR szFullPath[512];
};

#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
