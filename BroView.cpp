#include "stdafx.h"
#include "Sazabi.h"
#include "BroView.h"
#include "MainFrm.h"
#include "DlgAuth.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CChildView::CChildView()
{
	m_pwndFrame = NULL;
	m_dbZoomSize = 0.0;
	m_dbZoomSizeDefault = 0.0;
	m_bWndCloseFlg = FALSE;
	//SZB
	m_cefBrowser = nullptr;
	m_popupFeatures = NULL;
	m_pFindDialog = NULL;
	m_pFocusWnd = NULL;
	m_bFirstCallDontClose = FALSE;
	m_nBrowserID = 0;
	m_bDevToolsWnd = FALSE;
}

CChildView::~CChildView()
{
	//SZB
	if (m_cefBrowser)
	{
		m_cefBrowser = nullptr;
	}
	if (m_clientHandler)
	{
		m_clientHandler = nullptr;
	}

	if (m_popupFeatures)
	{
		delete m_popupFeatures;
		m_popupFeatures = NULL;
	}
}

const UINT CChildView::m_pFindDialogMessage = RegisterWindowMessage(FINDMSGSTRING);
BEGIN_MESSAGE_MAP(CChildView,ViewBaseClass )
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_NEW, OnNew)
	ON_COMMAND(ID_NEW_BLANK, OnNewBlank)
	ON_COMMAND(ID_NEW_BLANK_TAB, OnNewBlank)
	ON_COMMAND(ID_REOPEN_CLOSE_TAB,OnReopenCloseTab)
	ON_COMMAND(ID_NEW_SESSION, OnNewSession)
	ON_COMMAND(ID_PRINT, OnPrint)
	ON_COMMAND(ID_PRINT_PDF,OnPrintPDF)
	ON_COMMAND(ID_GO_BACK, OnGoBack)
	ON_COMMAND(ID_SHOW_DEVTOOLS, ShowDevTools)

	ON_UPDATE_COMMAND_UI(ID_GO_BACK, OnUpdateGoBack)
	ON_COMMAND(ID_GO_FORWARD, OnGoForward)
	ON_UPDATE_COMMAND_UI(ID_GO_FORWARD, OnUpdateGoForward)
	ON_COMMAND(ID_GO_START_PAGE, OnGoStartPage)
	ON_UPDATE_COMMAND_UI(ID_GO_START_PAGE, OnUpdateGoStartPage)
	ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	ON_UPDATE_COMMAND_UI(ID_VIEW_REFRESH, OnUpdateViewRefresh)
	ON_COMMAND(ID_VIEW_STOP, OnViewStop)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STOP, OnUpdateViewStop)

	ON_COMMAND(ID_OPEN, OnOpen)
	ON_COMMAND(ID_CUT, OnCut)
	ON_COMMAND(ID_COPY, OnCopy)
	ON_COMMAND(ID_PASTE, OnPaste)
	ON_COMMAND(ID_SEL_ALL, OnSelAll)
	ON_COMMAND(ID_FIND_PAGE, OnFindPage)
	ON_COMMAND(ID_ZOOM_DOWN,OnZoomDown)
	ON_COMMAND(ID_ZOOM_UP,OnZoomUp)
	ON_COMMAND(ID_ZOOM_RESET,OnZoomReset)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_CUT, &CChildView::OnUpdateCut)
	ON_UPDATE_COMMAND_UI(ID_PASTE, &CChildView::OnUpdatePaste)
	ON_COMMAND(ID_SETTINGS, &CChildView::OnSettings)
	ON_COMMAND(ID_APP_ABOUT,&CChildView::OnAppAbout)

	ON_COMMAND_RANGE(ID_BF_BACK1,ID_BF_BACK_END,OnBroBack)
	ON_COMMAND_RANGE(ID_ZOOM_START, ID_ZOOM_END, OnZoom)
	ON_COMMAND_RANGE(ID_BF_FORWARD1,ID_BF_FORWARD_END,OnBroForward)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()

	// CEF
	ON_MESSAGE(WM_APP_CEF_LOAD_START, &CChildView::OnLoadStart)
	ON_MESSAGE(WM_APP_CEF_LOAD_END, &CChildView::OnLoadEnd)
	ON_MESSAGE(WM_APP_CEF_TITLE_CHANGE, &CChildView::OnTitleChange)
	ON_MESSAGE(WM_APP_CEF_FAVICON_MESSAGE, &CChildView::OnFaviconChange)
	ON_MESSAGE(WM_APP_CEF_STATE_CHANGE, &CChildView::OnStateChange)
	ON_MESSAGE(WM_APP_CEF_ADDRESS_CHANGE, &CChildView::OnAddressChange)
	ON_MESSAGE(WM_APP_CEF_STATUS_MESSAGE, &CChildView::OnStatusMessage)
	ON_MESSAGE(WM_APP_CEF_BEFORE_BROWSE, &CChildView::OnBeforeBrowse)
	ON_MESSAGE(WM_APP_CEF_BEFORE_DOWNLOAD, &CChildView::OnBeforeDownload)
	ON_MESSAGE(WM_APP_CEF_DOWNLOAD_UPDATE, &CChildView::OnDownloadUpdate)
	ON_MESSAGE(WM_APP_CEF_SEARCH_URL, &CChildView::OnSearchURL)
	ON_MESSAGE(WM_APP_CEF_CLOSE_BROWSER, &CChildView::OnCloseBrowser)
	//ON_MESSAGE(WM_APP_CEF_NEW_BROWSER, &CChildView::OnNewBrowser)
	ON_MESSAGE(WM_APP_CEF_DOWNLOAD_BLANK_PAGE,&CChildView::OnDownloadBlankPage)
	ON_MESSAGE(WM_APP_CEF_NEW_WINDOW, &CChildView::OnNewWindow)
	ON_MESSAGE(WM_NEW_WINDOW_URL, OnCreateNewBrowserWindow)
	ON_MESSAGE(WM_COPY_IMAGE,&CChildView::OnCopyImage)

	ON_MESSAGE(WM_APP_CEF_AUTHENTICATE, &CChildView::OnAuthenticate)
	ON_MESSAGE(WM_APP_CEF_BAD_CERTIFICATE, &CChildView::OnBadCertificate)
	ON_MESSAGE(WM_APP_CEF_BEFORE_RESOURCE_LOAD,&CChildView::OnBeforeResourceLoad)
	ON_MESSAGE(WM_APP_CEF_FULLSCREEN_MODE_CHANGE,&CChildView::OnFullScreenModeChange)
	ON_MESSAGE(WM_APP_CEF_PROGRESS_CHANGE, &CChildView::OnProgressChange)
	ON_MESSAGE(WM_APP_CEF_WINDOW_ACTIVATE, &CChildView::OnWindowActivate)
	ON_MESSAGE(WM_APP_CEF_SET_RENDERER_PID,&CChildView::OnSetRendererPID)
	
	ON_REGISTERED_MESSAGE(m_pFindDialogMessage, OnFindDialogMessage)
END_MESSAGE_MAP()

BOOL CChildView::PreCreateWindow(CREATESTRUCT &cs)
{
	PROC_TIME(PreCreateWindow)

	if (!ViewBaseClass::PreCreateWindow(cs))
		return FALSE;

	cs.style &= ~WS_BORDER;
	cs.style |= WS_CLIPCHILDREN;
	cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS,
					   ::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW + 1), NULL);

	WNDCLASS wc = {0};
	wc.lpfnWndProc = AfxWndProc;
	wc.cbClsExtra  = 0;
	wc.cbWndExtra  = 0;
	wc.hInstance   = theApp.m_hInstance;
	wc.hIcon       = 0;
	wc.hCursor     = theApp.LoadStandardCursor(IDC_ARROW);
	wc.style = CS_DBLCLKS;
	wc.hbrBackground = HBRUSH(COLOR_WINDOW+1);
	wc.lpszMenuName  = 0;

	wc.lpszClassName = _T("CSGView");
	AfxRegisterClass(&wc);

	cs.lpszClass = _T("CSGView");

	return TRUE;
}

BOOL CChildView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	PROC_TIME(Create)

	BOOL bRet = CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
	try
	{
		//親フレームのポインターをセット。取得できない場合は、NG
		this->m_pwndFrame = this->GetParent();
		if (!theApp.IsWnd(m_pwndFrame))
			return FALSE;

		//Zoom初期値セット
		UINT iZoomSizeReg = 0;
		iZoomSizeReg = theApp.m_AppSettings.GetZoomSize();
		if (iZoomSizeReg != 100 && m_dbZoomSize == 0.0)
		{
			double dZoom = 0.0;
			switch (iZoomSizeReg)
			{
				case 500:dZoom = 9.0; break;
				case 400:dZoom = 8.0; break;
				case 300:dZoom = 6.0; break;
				case 250:dZoom = 5.0; break;
				case 200:dZoom = 4.0; break;
				case 175:dZoom = 3.5; break;
				case 150:dZoom = 2.5; break;
				case 125:dZoom = 1.5; break;
				case 110:dZoom = 0.5; break;
				case 100:dZoom = 0.0; break;

				case 90:dZoom = -0.5; break;
				case 80:dZoom = -1.0; break;
				case 75:dZoom = -1.5; break;
				case 67:dZoom = -2.0; break;
				case 50:dZoom = -3.5; break;
				case 33:dZoom = -5.0; break;
				case 25:dZoom = -6.0; break;
				default: dZoom = 0.0; break;
			}
			m_dbZoomSizeDefault = dZoom;
		}
		CString logmsg;
		logmsg.Format(_T("ChildView::Create BF_WND:0x%08x CV_WND:0x%08x")
			,theApp.SafeWnd(m_pwndFrame)
			,theApp.SafeWnd(this->m_hWnd)
			);
		theApp.WriteDebugTraceDateTime(logmsg,DEBUG_LOG_TYPE_GE);
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return bRet;
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
	if (m_cefBrowser)
	{
		CefWindowHandle hwnd = m_cefBrowser->GetHost()->GetWindowHandle();
		if (hwnd)
		{
			m_cefBrowser->GetHost()->WasResized();
			CRect rect;
			GetClientRect(rect);
			if (theApp.m_bTabEnable_Init)
			{
				CRect rcTab(0, 0, 0, 0);
				if (theApp.IsWnd(FRM))
				{
					if (FRM->m_cTabWnd)
					{
						if (theApp.SafeWnd(FRM->m_cTabWnd->GetHwnd()))
						{
							::GetClientRect(FRM->m_cTabWnd->GetHwnd(), rcTab);
							::SetWindowPos(FRM->m_cTabWnd->GetHwnd(), NULL, rcTab.left, rcTab.top, rect.Width(), rcTab.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
						}
					}
				}
			}
			::SetWindowPos(hwnd, NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
		}
	}
	else
	{
		//DevTool
		if (m_bDevToolsWnd)
		{
			HWND hwnd2 = ::FindWindowEx(this->m_hWnd, NULL, _T("CefBrowserWindow"), _T(""));
			if (::IsWindow(hwnd2))
			{
				CRect rect;
				GetClientRect(rect);
				::SetWindowPos(hwnd2, NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
			}
		}
	}
}

BOOL CChildView::OnEraseBkgnd(CDC *pDC)
{
	return TRUE;
}

BOOL CChildView::GetToolBar() const
{
	try
	{
		if (m_popupFeatures)
		{
			//常に非表示 2020-01-06
			return FALSE;
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return TRUE;
}
BOOL CChildView::GetMenuBar() const
{
	try
	{
		if (m_popupFeatures)
		{
			//常に非表示 2020-01-06
			return FALSE;
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return TRUE;
}
BOOL CChildView::GetStatusBar() const
{
	try
	{
		if (m_popupFeatures)
		{
			//常に非表示 2020-01-06
			return FALSE;
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return TRUE;
}
BOOL CChildView::GetAddressBar() const
{
	try
	{
		if (m_popupFeatures)
		{
			//chrome 59まではあったが、62ではこのプロパティーは削除されている。
			//if (m_popupFeatures->locationBarVisible)
			return TRUE;
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return TRUE;
}

void CChildView::UpDateAddressBar()
{
	if (theApp.IsWnd(FRM))
	{
		if (this->m_UpDateAddressBarURL_Cache != m_strURL)
		{
			if (FRM->OnSetUrlString((LPCTSTR)m_strURL))
			{
				this->m_UpDateAddressBarURL_Cache = m_strURL;
			}
		}
	}
}
BOOL CChildView::IsFileURINavigation(const CString &strURL)
{
	//Trueの場合は、ヒットしたので、そのまま開いてはダメ
	//Falseの場合は、そのまま開く。
	BOOL bRet = FALSE;
	if (strURL.IsEmpty())
		return FALSE;

	if (!SBUtil::IsURL_FILE(strURL))
		return FALSE;

	if (FRM->m_bOLEViewer)
		return FALSE;

	CString logmsg;
	logmsg.Format(_T("CV_WND:0x%08x IsFileURINavigation:%s"), theApp.SafeWnd(this->m_hWnd), strURL);
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);
	LockSetForegroundWindow(LSFW_UNLOCK);
	theApp.OpenFileExplorer(strURL);
	bRet = TRUE;
	return bRet;
}

BOOL CChildView::IsRedirectScriptEx(LPCTSTR sURL, LPCTSTR sChkURLNoQuery, BOOL bTop)
{
	PROC_TIME(IsRedirectScriptEx)

	CString strURL;
	strURL = sURL;

	//Scriptベースの判定
	if (theApp.m_cScriptSrc.m_strSrc.IsEmpty())
		return FALSE;

	if (!theApp.m_pScHost)
	{
		theApp.m_pScHost = new CScriptHost;
	}

	if (!theApp.m_pScHost) return FALSE;

	CString strRet;
	if (!theApp.m_pScHost->ExecScript(sChkURLNoQuery, bTop, theApp.m_cScriptSrc.m_strSrc, strRet))
		return FALSE;

	strRet.TrimLeft();
	strRet.TrimRight();
	if (strRet.IsEmpty())
		return FALSE;

	CString strTimeoutMessage;
	CString logmsg;

	CString strURLMid;
	strURLMid = strURL;
	SBUtil::GetDivChar(strURL, 260, strURLMid, FALSE);

	if (strRet.CompareNoCase(_T("IE")) == 0)
	{
		strTimeoutMessage.Format(_T("%s\n%s"), theApp.m_strZoneMessageIE, strURLMid);
		logmsg.Format(_T("CV_WND:0x%08x IsRedirectScript:%s"), theApp.SafeWnd(this->m_hWnd), strTimeoutMessage);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);

		if (theApp.m_AppSettings.GetRedirectMsgTimeout() > 0)
			ShowTimeoutMessageBox(strTimeoutMessage, MB_OK | MB_ICONINFORMATION, theApp.m_AppSettings.GetRedirectMsgTimeout());
		LockSetForegroundWindow(LSFW_UNLOCK);
		theApp.OpenDefaultBrowser(strURL, 1, _T(""));
		return TRUE;
	}

	if (strRet.CompareNoCase(_T("Custom")) == 0 || strRet.CompareNoCase(_T("Custom1")) == 0)
	{
		strTimeoutMessage.Format(_T("%s\n%s"), theApp.m_strZoneMessageCustom, strURLMid);
		logmsg.Format(_T("CV_WND:0x%08x IsRedirectScript:%s"), theApp.SafeWnd(this->m_hWnd), strTimeoutMessage);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);

		if (theApp.m_AppSettings.GetRedirectMsgTimeout() > 0)
			ShowTimeoutMessageBox(strTimeoutMessage, MB_OK | MB_ICONINFORMATION, theApp.m_AppSettings.GetRedirectMsgTimeout());
		LockSetForegroundWindow(LSFW_UNLOCK);
		theApp.OpenDefaultBrowser(strURL, 5, theApp.m_AppSettings.GetCustomBrowser());
		return TRUE;
	}
	if (strRet.CompareNoCase(_T("Custom2")) == 0)
	{
		strTimeoutMessage.Format(_T("%s\n%s"), theApp.m_strZoneMessageCustom, strURLMid);
		logmsg.Format(_T("CV_WND:0x%08x IsRedirectScript:%s"), theApp.SafeWnd(this->m_hWnd), strTimeoutMessage);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);

		if (theApp.m_AppSettings.GetRedirectMsgTimeout() > 0)
			ShowTimeoutMessageBox(strTimeoutMessage, MB_OK | MB_ICONINFORMATION, theApp.m_AppSettings.GetRedirectMsgTimeout());
		LockSetForegroundWindow(LSFW_UNLOCK);
		theApp.OpenDefaultBrowser(strURL, 5, theApp.m_AppSettings.GetCustomBrowser2());
		return TRUE;
	}
	if (strRet.CompareNoCase(_T("Custom3")) == 0)
	{
		strTimeoutMessage.Format(_T("%s\n%s"), theApp.m_strZoneMessageCustom, strURLMid);
		logmsg.Format(_T("CV_WND:0x%08x IsRedirectScript:%s"), theApp.SafeWnd(this->m_hWnd), strTimeoutMessage);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);

		if (theApp.m_AppSettings.GetRedirectMsgTimeout() > 0)
			ShowTimeoutMessageBox(strTimeoutMessage, MB_OK | MB_ICONINFORMATION, theApp.m_AppSettings.GetRedirectMsgTimeout());
		LockSetForegroundWindow(LSFW_UNLOCK);
		theApp.OpenDefaultBrowser(strURL, 5, theApp.m_AppSettings.GetCustomBrowser3());
		return TRUE;
	}
	if (strRet.CompareNoCase(_T("Custom4")) == 0)
	{
		strTimeoutMessage.Format(_T("%s\n%s"), theApp.m_strZoneMessageCustom, strURLMid);
		logmsg.Format(_T("CV_WND:0x%08x IsRedirectScript:%s"), theApp.SafeWnd(this->m_hWnd), strTimeoutMessage);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);

		if (theApp.m_AppSettings.GetRedirectMsgTimeout() > 0)
			ShowTimeoutMessageBox(strTimeoutMessage, MB_OK | MB_ICONINFORMATION, theApp.m_AppSettings.GetRedirectMsgTimeout());
		LockSetForegroundWindow(LSFW_UNLOCK);
		theApp.OpenDefaultBrowser(strURL, 5, theApp.m_AppSettings.GetCustomBrowser4());
		return TRUE;
	}
	if (strRet.CompareNoCase(_T("Custom5")) == 0)
	{
		strTimeoutMessage.Format(_T("%s\n%s"), theApp.m_strZoneMessageCustom, strURLMid);
		logmsg.Format(_T("CV_WND:0x%08x IsRedirectScript:%s"), theApp.SafeWnd(this->m_hWnd), strTimeoutMessage);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);

		if (theApp.m_AppSettings.GetRedirectMsgTimeout() > 0)
			ShowTimeoutMessageBox(strTimeoutMessage, MB_OK | MB_ICONINFORMATION, theApp.m_AppSettings.GetRedirectMsgTimeout());
		LockSetForegroundWindow(LSFW_UNLOCK);
		theApp.OpenDefaultBrowser(strURL, 5, theApp.m_AppSettings.GetCustomBrowser5());
		return TRUE;
	}

	if (strRet.CompareNoCase(_T("Block")) == 0)
	{
		strTimeoutMessage.Format(_T("%s\n%s"), theApp.m_strZoneMessageNG, strURLMid);
		logmsg.Format(_T("CV_WND:0x%08x IsRedirectScript:<BLOCK> %s"), theApp.SafeWnd(this->m_hWnd), strTimeoutMessage);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);

		if (theApp.m_AppSettings.GetRedirectMsgTimeout() > 0)
			ShowTimeoutMessageBox(strTimeoutMessage, MB_OK | MB_ICONERROR, theApp.m_AppSettings.GetRedirectMsgTimeout());

		return TRUE;
	}

	if (strRet.CompareNoCase(_T("Default")) == 0)
	{
		strTimeoutMessage.Format(_T("%s\n%s"), theApp.m_strZoneMessageDBL, strURLMid);
		logmsg.Format(_T("CV_WND:0x%08x IsRedirectScript:%s"), theApp.SafeWnd(this->m_hWnd), strTimeoutMessage);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);

		if (theApp.m_AppSettings.GetRedirectMsgTimeout() > 0)
			ShowTimeoutMessageBox(strTimeoutMessage, MB_OK | MB_ICONINFORMATION, theApp.m_AppSettings.GetRedirectMsgTimeout());
		LockSetForegroundWindow(LSFW_UNLOCK);
		theApp.OpenDefaultBrowser(strURL, 0, _T(""));
		return TRUE;
	}

	if (strRet.CompareNoCase(_T("Firefox")) == 0)
	{
		strTimeoutMessage.Format(_T("%s\n%s"), theApp.m_strZoneMessageFF, strURLMid);
		logmsg.Format(_T("CV_WND:0x%08x IsRedirectScript:%s"), theApp.SafeWnd(this->m_hWnd), strTimeoutMessage);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);

		if (theApp.m_AppSettings.GetRedirectMsgTimeout() > 0)
			ShowTimeoutMessageBox(strTimeoutMessage, MB_OK | MB_ICONINFORMATION, theApp.m_AppSettings.GetRedirectMsgTimeout());
		LockSetForegroundWindow(LSFW_UNLOCK);
		theApp.OpenDefaultBrowser(strURL, 2, _T(""));
		return TRUE;
	}

	if (strRet.CompareNoCase(_T("Chrome")) == 0)
	{
		strTimeoutMessage.Format(_T("%s\n%s"), theApp.m_strZoneMessageCHR, strURLMid);
		logmsg.Format(_T("CV_WND:0x%08x IsRedirectScript:%s"), theApp.SafeWnd(this->m_hWnd), strTimeoutMessage);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);

		if (theApp.m_AppSettings.GetRedirectMsgTimeout() > 0)
			ShowTimeoutMessageBox(strTimeoutMessage, MB_OK | MB_ICONINFORMATION, theApp.m_AppSettings.GetRedirectMsgTimeout());
		LockSetForegroundWindow(LSFW_UNLOCK);
		theApp.OpenDefaultBrowser(strURL, 3, _T(""));
		return TRUE;
	}

	if (strRet.CompareNoCase(_T("Edge")) == 0)
	{
		strTimeoutMessage.Format(_T("%s\n%s"), theApp.m_strZoneMessageEDG, strURLMid);
		logmsg.Format(_T("CV_WND:0x%08x IsRedirectScript:%s"), theApp.SafeWnd(this->m_hWnd), strTimeoutMessage);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);

		if (theApp.m_AppSettings.GetRedirectMsgTimeout() > 0)
			ShowTimeoutMessageBox(strTimeoutMessage, MB_OK | MB_ICONINFORMATION, theApp.m_AppSettings.GetRedirectMsgTimeout());
		LockSetForegroundWindow(LSFW_UNLOCK);
		theApp.OpenDefaultBrowser(strURL, 4, _T(""));

		return TRUE;
	}
	return FALSE;
}

BOOL CChildView::IsRedirectURLChk(const CString &strURL, BOOL bTop)
{
	PROC_TIME(IsRedirectURLChk)

	if (!theApp.m_AppSettings.IsEnableURLRedirect())
		return FALSE;

	if (strURL.IsEmpty())
		return FALSE;

	BOOL bRet = FALSE;

	CefString newURL(strURL);
	CefURLParts cfURLparts;
	if (CefParseURL(newURL, cfURLparts))
	{
		CefString cfScheme(&cfURLparts.scheme);
		CefString cfHost(&cfURLparts.host);
		CefString cfPath(&cfURLparts.path);

		CString strScheme(cfScheme.c_str());
		CString strHost(cfHost.c_str());
		CString strPath(cfPath.c_str());

		//http httpsのみフィルターを利用する。about: notes: vwmare-view: mailto:を考慮する。
		if (strScheme.Find(_T("http")) != 0) //http|https
			return FALSE;

		if (strHost.IsEmpty())
			return FALSE;
		if (strPath.IsEmpty())
			strPath = _T("/");

		CString strURLChk; //Queryを除く。無駄な情報を省く。
		strURLChk.Format(_T("%s://%s%s"), strScheme, strHost, strPath);

		//除外にHitした。
		if (theApp.IsCacheRedirectFilterNone(strURLChk))
		{
			return FALSE;
		}
		else
		{
			if (IsRedirectScriptEx(strURL, strURLChk, bTop))
			{
				return TRUE;
			}
			else
			{
				//除外にキャッシュ
				theApp.AddCacheRedirectFilterNone(strURLChk);
			}
		}
	}
	return FALSE;
}
void CChildView::IsRedirectWndAutoCloseChk()
{
	CString logmsg;

	if (!theApp.IsWnd(FRM))
		return;

	//Shiftキー / ESCが押されている場合は、閉じない。
	if (::GetKeyState(VK_SHIFT) < 0 || ::GetKeyState(VK_ESCAPE) < 0)
	{
		logmsg.Format(_T("CV_WND:0x%08x IsRedirectWndAutoCloseChk :AutoClose Shift Key Skip"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_TR);
		return;
	}
	//初めの起動でゾーンフィルターに引っかかると、何もできないので。
	if (m_bFirstCallDontClose)
	{
		m_bFirstCallDontClose = FALSE;
		logmsg.Format(_T("CV_WND:0x%08x IsRedirectWndAutoCloseChk :AutoClose First Call GoHome Skip"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_TR);
		return;
	}
	LockSetForegroundWindow(LSFW_UNLOCK);
	CString strJsStr;
	strJsStr += _T("if (window.self.history.length==0) \n");
	strJsStr += _T("    window.close();\n");
	CefString strCefJsStr(strJsStr);
	if (m_cefBrowser)
		m_cefBrowser->GetMainFrame()->ExecuteJavaScript(strCefJsStr, "", 0);
}

typedef int(__stdcall *TMessageBoxTimeout)(HWND, LPCTSTR, LPCTSTR, UINT, WORD, DWORD);
void CChildView::ShowTimeoutMessageBox(LPCTSTR strMsg, int iType, int iTimeOut)
{
	try
	{
		BOOL bFreeLibrary = FALSE;
		HMODULE hModule = {0};
		hModule = ::GetModuleHandle(_T("user32.dll"));
		if (!hModule)
		{
			hModule = ::LoadLibrary(_T("user32.dll"));
			if (hModule)
				bFreeLibrary = TRUE;
		}

		if (hModule)
		{
			TMessageBoxTimeout MessageBoxTimeout;
			MessageBoxTimeout = (TMessageBoxTimeout)GetProcAddress(hModule, "MessageBoxTimeoutW");
			if (MessageBoxTimeout)
			{
				MessageBoxTimeout(this->m_hWnd, strMsg,
						  theApp.m_strThisAppName, iType, LANG_NEUTRAL, iTimeOut);
			}
			else
			{
				::MessageBox(this->m_hWnd, strMsg, theApp.m_strThisAppName, iType);
			}
			if (bFreeLibrary)
			{

				FreeLibrary(hModule);
				bFreeLibrary = FALSE;
				hModule = NULL;
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

void CChildView::ResizeFrmWindow(RECT &rectClient)
{
	PROC_TIME(ResizeFrmWindow)

	try
	{
		BOOL bResizeAndCenter = FALSE;
		if (theApp.IsWnd(FRM))
		{
			//フルスクリーンモードの場合は、無効
			if (FRM->m_bFullScreen) return;
			WINDOWPLACEMENT zFramePracement = {0};
			zFramePracement.length = sizeof(WINDOWPLACEMENT);
			// 現在取得
			FRM->GetWindowPlacement(&zFramePracement);
			zFramePracement.showCmd = SW_NORMAL;

			zFramePracement.rcNormalPosition = rectClient;

			FRM->SetWindowPlacement(&zFramePracement);

			if (bResizeAndCenter)
			{
				FRM->CenterWindow();
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

void CChildView::ResizeWindowPopup()
{
	try
	{
		ResizeWindowPopupInpl();
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}
void CChildView::ResizeWindowPopupInpl()
{
	PROC_TIME(ResizeWindowPopupInpl)

	try
	{
		CString logmsg;
		logmsg.Format(_T("CV_WND:0x%08x ResizeWindowPopupInpl"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_JS);
		if (!theApp.IsWnd(m_pwndFrame))
			return;

		//DevToolsの場合はツールバー関連非表示
		if (m_bDevToolsWnd)
		{
			if (FRM->m_pwndReBar)
			{
				CReBarCtrl &rbCtrl = FRM->m_pwndReBar->GetReBarCtrl();
				REBARBANDINFO rbbi = {0};
				REBARBANDINFO rbbiNull = {0};
				UINT cbCompileSize = 0;
				cbCompileSize = sizeof(REBARBANDINFO);
				rbbi.cbSize = cbCompileSize;
				rbbiNull = rbbi;
				int iBandCnt = 0;
				iBandCnt = rbCtrl.GetBandCount();
				for (int i = 0; i < iBandCnt; i++)
				{
					rbbi = rbbiNull;
					rbbi.fMask = RBBIM_STYLE;
					VERIFY(rbCtrl.GetBandInfo(i, &rbbi));
					if (!(rbbi.fStyle & RBBS_HIDDEN))
						VERIFY(rbCtrl.ShowBand(i, FALSE));
				}
			}
		}

		if (theApp.IsWnd(FRM) && theApp.IsWnd(FRM->m_pwndStatusBar))
		{
			if (!this->GetStatusBar())
			{
				FRM->m_pwndStatusBar->ShowWindow(SW_HIDE);
			}
			else
			{
				FRM->m_pwndStatusBar->ShowWindow(SW_SHOW);
			}
		}

		WINDOWPLACEMENT zFramePracement = {0};
		zFramePracement.length = sizeof(WINDOWPLACEMENT);

		m_pwndFrame->GetWindowPlacement(&zFramePracement);
		zFramePracement.showCmd = SW_NORMAL;
		CRect rcCurrent = zFramePracement.rcNormalPosition;

		if (m_popupFeatures)
		{
			CRect rcWindow;
			CRect rcClient;
			m_pwndFrame->GetWindowRect(&rcWindow);
			m_pwndFrame->GetClientRect(&rcClient);
			int iAdjustHeight = rcWindow.Height() - rcClient.Height();
			int iAdjustWidth = rcWindow.Width() - rcClient.Width();

			CRect rcDesktop;
			CRect rcFeature;

			//初期値(0,0)
			rcFeature.top = 0;
			rcFeature.left = 0;
			//初期値(アクティブウインドウの高さ)
			rcFeature.bottom = rcCurrent.Height();
			//初期値(アクティブウインドウの幅)
			rcFeature.right = rcCurrent.Width();

			//高さを指定している場合はセット
			if (m_popupFeatures->height)
			{
				rcFeature.bottom = m_popupFeatures->height;
				rcFeature.bottom += iAdjustHeight;
				rcFeature.bottom += theApp.m_AppSettings.GetHeightMargin();
				//DPIによる補正
				if (theApp.m_ScaleDPI > 1)
				{
					double dScale = 0.0;
					int iScale = 0;
					dScale = rcFeature.bottom * theApp.m_ScaleDPI;
					iScale = (int)dScale;
					rcFeature.bottom = iScale;
				}
			}

			//幅を指定している場合はセット
			if (m_popupFeatures->width)
			{
				rcFeature.right = m_popupFeatures->width;
				rcFeature.right += iAdjustWidth;
				rcFeature.right += theApp.m_AppSettings.GetWideMargin();
				//DPIによる補正
				if (theApp.m_ScaleDPI > 1)
				{
					double dScale = 0.0;
					int iScale = 0;
					dScale = rcFeature.right * theApp.m_ScaleDPI;
					iScale = (int)dScale;
					rcFeature.right = iScale;
				}
			}

			//topを指定している場合
			if (m_popupFeatures->y)
				rcFeature.OffsetRect(0, m_popupFeatures->y);
			//leftを指定している場合
			if (m_popupFeatures->x)
				rcFeature.OffsetRect(m_popupFeatures->x, 0);

			//マルチモニター補正
			CWnd *pView = NULL;
			HWND hWnd = {0};
			pView = (CWnd *)theApp.GetActiveViewPtr();
			if (theApp.IsWnd(pView))
			{
				hWnd = pView->m_hWnd;
			}
			if (hWnd == NULL)
				hWnd = ::GetDesktopWindow();
			SBUtil::GetMonitorWorkRect(hWnd, &rcDesktop);
			if (theApp.m_iWinOSVersion >= 100)
			{
				CRect rectAdj;
				rcDesktop.left -= 7;
				rcDesktop.right += 7;
				rcDesktop.bottom += 7;
			}

			//マルチモニター補正
			rcFeature.OffsetRect(rcDesktop.left, rcDesktop.top);

			//最終的な値をセット
			rcCurrent = rcFeature;
		}
		zFramePracement.rcNormalPosition = rcCurrent;
		m_pwndFrame->SetWindowPlacement(&zFramePracement);
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}
void CChildView::OnStatusTextChange(LPCTSTR lpszText)
{
	try
	{
		CString strTemp = lpszText;
		if (theApp.IsWnd(FRM) && theApp.IsWnd(FRM->m_pwndStatusBar))
		{
			FRM->m_pwndStatusBar->SetWindowText(strTemp);
		}

		DebugWndLogData dwLogData;
		dwLogData.mHWND.Format(_T("CV_WND:0x%08x"), theApp.SafeWnd(this->m_hWnd));
		dwLogData.mFUNCTION_NAME = _T("OnStatusTextChange");
		dwLogData.mMESSAGE1 = strTemp;
		theApp.AppendDebugViewLog(dwLogData);
		CString logmsg = dwLogData.GetString();
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_TR);
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

void CChildView::OnQuit()
{
}
void CChildView::ReSetRendererPID()
{
	try
	{
		if (!theApp.IsWnd(this))return;
		if (IsBrowserNull()) return;
		CefRefPtr<CefFrame> frame;
		frame = m_cefBrowser->GetMainFrame();
		if (frame)
		{
			CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("GET_PID");
			CefRefPtr<CefListValue> args = msg->GetArgumentList();
			args->SetString(0, "GET_PID");
			args->SetInt(0, FRM->m_RendererPID);
			frame->SendProcessMessage(PID_RENDERER, msg);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

void CChildView::OnTitleChange(LPCWSTR lpwszText)
{
	PROC_TIME(OnTitleChange)

	try
	{
		CString strTitle(lpwszText);

		m_strTitle = strTitle;
		if (strTitle.IsEmpty())
			strTitle = theApp.m_strThisAppName;
		if (theApp.IsWnd(FRM))
		{
			this->ZoomTo(m_dbZoomSize);
			FRM->SetWindowTitle(strTitle);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

BOOL CChildView::ZoomTo(double lFactor)
{
	BOOL bRet = FALSE;
	try
	{
		if (!m_cefBrowser) return FALSE;
		if (lFactor < -6.0)
			lFactor = -6.0;
		if (lFactor > 9.0)
			lFactor = 9.0;
		if (GetZoomSizeEx() != lFactor)
		{
			m_cefBrowser->GetHost()->SetZoomLevel(lFactor);
		}
		m_dbZoomSize = GetZoomSizeEx();
		bRet = TRUE;
		if (theApp.IsWnd(FRM) && theApp.IsWnd(FRM->m_pwndStatusBar))
		{
			double dScale = 0.0;

			     if (m_dbZoomSize == 9.0)dScale= 500;
			else if (m_dbZoomSize == 8.0)dScale = 400;
			else if (m_dbZoomSize == 6.0)dScale = 300;
			else if (m_dbZoomSize == 5.0)dScale = 250;
			else if (m_dbZoomSize == 4.0)dScale = 200;
			else if (m_dbZoomSize == 3.5)dScale = 175;
			else if (m_dbZoomSize == 2.5)dScale = 150;
			else if (m_dbZoomSize == 1.5)dScale = 125;
			else if (m_dbZoomSize == 0.5)dScale = 110;
			else if (m_dbZoomSize == 0.0)dScale = 100;
			else if (m_dbZoomSize == -0.5)dScale = 90;
			else if (m_dbZoomSize == -1.0)dScale = 80;
			else if (m_dbZoomSize == -1.5)dScale = 75;
			else if (m_dbZoomSize == -2.0)dScale = 67;
			else if (m_dbZoomSize == -3.5)dScale = 50;
			else if (m_dbZoomSize == -5.0)dScale = 33;
			else if (m_dbZoomSize == -6.0)dScale = 25;
			else dScale = 100;
		
			CString strZoomFmt;
			strZoomFmt.Format(_T("%.0f%%"), dScale);
			FRM->m_pwndStatusBar->SetPaneText(nStatusZoom, strZoomFmt);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return bRet;
}
void CChildView::OnZoomDown()
{
	SetWheelZoom(-1);
}
void CChildView::OnZoomUp()
{
	SetWheelZoom(1);
}
void CChildView::OnZoomReset()
{
	m_dbZoomSize = 0.0;
	ZoomTo(m_dbZoomSize);
}
void CChildView::SetWheelZoom(int iDel)
{
	try
	{
		double iZoom = GetZoomSizeEx();
		double iZoomR=0.0;
		if(iZoom >= 9.0)iZoomR = 9.0;
		else if (iZoom >= 8.0)iZoomR = 8.0;
		else if (iZoom >= 6.0)iZoomR = 6.0;
		else if (iZoom >= 5.0)iZoomR = 5.0;
		else if (iZoom >= 4.0)iZoomR = 4.0;
		else if (iZoom >= 3.5)iZoomR = 3.5;
		else if (iZoom >= 2.5)iZoomR = 2.5;
		else if (iZoom >= 1.5)iZoomR = 1.5;
		else if (iZoom >= 0.5)iZoomR = 0.5;
		else if (iZoom >= 0.0)iZoomR = 0.0;
		else if (iZoom >= -0.5)iZoomR = -0.5;
		else if (iZoom >= -1.0)iZoomR = -1.0;
		else if (iZoom >= -1.5)iZoomR = -1.5;
		else if (iZoom >= -2.0)iZoomR = -2.0;
		else if (iZoom >= -3.5)iZoomR = -3.5;
		else if (iZoom >= -5.0)iZoomR = -5.0;
		else if (iZoom >= -6.0)iZoomR = -6.0;
		else if (iZoom < -6.0)	iZoomR = -6.0;
		else iZoomR = 0.0;

		if(iDel >0)
		{
			if(iZoomR == 9.0)return;
			else if (iZoomR == 8.0)iZoomR =9.0;
			else if (iZoomR == 6.0)iZoomR = 8.0;
			else if (iZoomR == 5.0)iZoomR = 6.0;
			else if (iZoomR == 4.0)iZoomR = 5.0;
			else if (iZoomR == 3.5)iZoomR = 4.0;
			else if (iZoomR == 2.5)iZoomR = 3.5;
			else if (iZoomR == 1.5)iZoomR = 2.5;
			else if (iZoomR == 0.5)iZoomR = 1.5;
			else if (iZoomR == 0.0)iZoomR = 0.5;
			else if (iZoomR == -0.5)iZoomR = 0.0;
			else if (iZoomR == -1.0)iZoomR = -0.5;
			else if (iZoomR == -1.5)iZoomR = -1.0;
			else if (iZoomR == -2.0)iZoomR = -1.5;
			else if (iZoomR == -3.5)iZoomR = -2.0;
			else if (iZoomR == -5.0)iZoomR = -3.5;
			else if (iZoomR == -6.0)iZoomR = -5.0;
			else return;
			ZoomTo(iZoomR);
		}
		else
		{
			if (iZoomR == 9.0)iZoomR = 8.0;
			else if (iZoomR == 8.0)iZoomR = 6.0;
			else if (iZoomR == 6.0)iZoomR = 5.0;
			else if (iZoomR == 5.0)iZoomR = 4.0;
			else if (iZoomR == 4.0)iZoomR = 3.5;
			else if (iZoomR == 3.5)iZoomR = 2.5;
			else if (iZoomR == 2.5)iZoomR = 1.5;
			else if (iZoomR == 1.5)iZoomR = 0.5;
			else if (iZoomR == 0.5)iZoomR = 0.0;
			else if (iZoomR == 0.0)iZoomR = -0.5;
			else if (iZoomR == -0.5)iZoomR = -1.0;
			else if (iZoomR == -1.0)iZoomR = -1.5;
			else if (iZoomR == -1.5)iZoomR = -2.0;
			else if (iZoomR == -2.0)iZoomR = -3.5;
			else if (iZoomR == -3.5)iZoomR = -5.0;
			else if (iZoomR == -5.0)iZoomR = -6.0;
			else if (iZoomR == -6.0)return;
			else return;
			ZoomTo(iZoomR);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return;
}
double CChildView::GetZoomSizeEx()
{
	double iRet = m_dbZoomSizeDefault;
	try
	{
		if (!m_cefBrowser) return m_dbZoomSizeDefault;
		iRet = m_cefBrowser->GetHost()->GetZoomLevel();
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return iRet;
}

BOOL CChildView::PreTranslateMessage(MSG *pMsg)
{
	ASSERT(pMsg != NULL);
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);
	if (pMsg == NULL) return FALSE;

	switch (pMsg->message)
	{
		case WM_KEYDOWN:
		{
			switch (pMsg->wParam)
			{
		        case VK_BROWSER_BACK:
				{
					OnGoBack();
					return TRUE;
				}
				case VK_BROWSER_FORWARD:
				{
					OnGoForward();
					return TRUE;
				}
				case VK_BROWSER_REFRESH:
				{
					OnViewRefresh();
					return TRUE;
				}
				case VK_BROWSER_STOP:
				{
					OnViewStop();
					return TRUE;
				}
				case VK_BROWSER_HOME:
				{
					OnGoStartPage();
					return TRUE;
				}
				default:break;
			}
			break;
		}
		case WM_XBUTTONUP:
		{
			int nArg = HIWORD(pMsg->wParam);
			if(nArg==XBUTTON1)
			{
				OnGoBack();
				return TRUE;				
			}
			else if(nArg==XBUTTON2)
			{
				OnGoForward();
				return TRUE;
			}
			break;
		}
		case WM_MOUSEWHEEL:
		{
			//VK_CONTROL
			if(::GetKeyState(VK_CONTROL) < 0)
			{
				int zDelta = (SHORT)HIWORD(pMsg->wParam);
				SetWheelZoom(zDelta);
				return TRUE;
			}
			break;
		}
		default:break;
	}
	return ViewBaseClass::PreTranslateMessage(pMsg);
}

void CChildView::OnNew()
{
	PROC_TIME(OnNew)
	try
	{
		CString logmsg;
		logmsg.Format(_T("CV_WND:0x%08x OnNew"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);

		if (!IsBrowserNull())
		{
			CString strURL;
			strURL = GetLocationURL();
			if (strURL.IsEmpty())
				strURL = theApp.m_AppSettings.GetStartURL();
			if (!strURL.IsEmpty())
			{
				this->CreateNewBrowserWindow(strURL);
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}
void CChildView::OnReopenCloseTab()
{
	PROC_TIME(OnReopenCloseTab)
	try
	{
		CString logmsg;
		logmsg.Format(_T("CV_WND:0x%08x OnReopenCloseTab"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);

		if (!IsBrowserNull())
		{
			if (theApp.m_listCloseWindowURL.GetCount())
			{
				CString strURL;
				POSITION pos1 = {0};
				//既に登録されているかチェック
				for (pos1 = theApp.m_listCloseWindowURL.GetHeadPosition(); pos1 != NULL; theApp.m_listCloseWindowURL.GetNext(pos1))
				{
					strURL.Empty();
					strURL = theApp.m_listCloseWindowURL.GetAt(pos1);
					break;
				}
				if (!strURL.IsEmpty())
				{
					this->CreateNewBrowserWindow(strURL);
				}
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}
void CChildView::OnNewBlank()
{
	PROC_TIME(OnNewBlank)
	try
	{
		CString logmsg;
		logmsg.Format(_T("CV_WND:0x%08x OnNewBlank"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);

		if (!IsBrowserNull())
		{
			CString strURL;
			strURL = theApp.m_AppSettings.GetStartURL();
			if (strURL.IsEmpty())
				strURL = _T("about:blank");
			if (!strURL.IsEmpty())
			{
				this->CreateNewBrowserWindow(strURL);
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

void CChildView::OnNewSession()
{
	PROC_TIME(OnNewSession)
	try
	{
		FRM->Init_MsgDlg();
		CString startingMsg;
		startingMsg.LoadString(IDS_STRING_STARTING_NEW_SESSION);
		FRM->SetMessage_MsgDlg(startingMsg);
		CString logmsg;
		logmsg.Format(_T("CV_WND:0x%08x OnNew"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
		if (!IsBrowserNull())
		{
			CString strURL;
			strURL = GetLocationURL();
			if (strURL.IsEmpty())
				strURL = theApp.m_AppSettings.GetStartURL();
			if (!strURL.IsEmpty())
			{
				theApp.ExecNewInstance(strURL);
			}
		}
		FRM->Release_MsgDlg();
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

void CChildView::OnPrintPDF()
{
	CString logmsg;
	logmsg.Format(_T("CV_WND:0x%08x OnPrintPDF"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	if (m_cefBrowser)
	{
		//Download禁止
		if (theApp.m_AppSettings.IsEnableDownloadRestriction())
		{
			if (theApp.IsWnd(m_pwndFrame)) {
				CString restrictedMsg;
				restrictedMsg.LoadString(ID_MSG_FILE_DOWNLOAD_RESTRICTED);
				theApp.SB_MessageBox(m_pwndFrame->m_hWnd, restrictedMsg, NULL, MB_OK | MB_ICONWARNING, TRUE);
			}
			return;
		}

		CString strFileName;
		strFileName = m_strTitle;
		strFileName.TrimLeft();
		strFileName.TrimRight();
		if (strFileName.IsEmpty()) {
			strFileName.LoadString(ID_DEFAULT_FILE_BASENAME);
		}

		//ファイル名に使えない文字を置き換える。
		strFileName = SBUtil::GetValidFileName(strFileName);
		strFileName += _T(".pdf");

		CString strPath;
		if (theApp.IsSGMode())
		{
			strPath = theApp.m_AppSettings.GetRootPath();
			if (strPath.IsEmpty())
				strPath = _T("B:\\");
		}
		else
		{
			strPath = SBUtil::GetDownloadFolderPath();
		}
		strPath = strPath.TrimRight('\\');
		strPath += _T("\\");
		if (!theApp.m_strLastSelectFolderPath.IsEmpty())
		{
			if (theApp.IsFolderExists(theApp.m_strLastSelectFolderPath))
			{
				strPath = theApp.m_strLastSelectFolderPath;
			}
		}

		CString szFilter;
		szFilter.LoadString(ID_FILE_TYPE_PDF);
		CString strFullPath;
		strFullPath.Format(_T("%s%s"), strPath, strFileName);
		CString strTitle;
		strTitle.LoadString(ID_PRINT_TO_PDF_FILE_CHOOSER_TITLE);
		CStringW strCaption(theApp.m_strThisAppName);
		CStringW strRootDrive(theApp.m_AppSettings.GetRootPath());
		CStringW strMsg;

		CFileDialog *pFileDlg = NULL;
		if (theApp.IsSGMode())
		{
			//SGModeの場合は、Classicダイアログを使用
			pFileDlg = new CFileDialog(FALSE, _T("pdf"), strFileName, OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, szFilter, this, 0, FALSE);
		}
		else
		{
			pFileDlg = new CFileDialog(FALSE, _T("pdf"), strFileName, OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, szFilter, this);
		}

		pFileDlg->m_ofn.lpstrTitle = strTitle.GetString();
		pFileDlg->m_ofn.lpstrInitialDir = strPath;

		INT_PTR bRet = FALSE;
		WCHAR szSelPath[MAX_PATH + 1] = {0};
		bRet = pFileDlg->DoModal();
		if (bRet == IDOK)
		{
			memset(szSelPath, 0x00, sizeof(WCHAR) * MAX_PATH);
			lstrcpynW(szSelPath, pFileDlg->GetPathName(), MAX_PATH);
			WCHAR szSelFolderPath[MAX_PATH + 1] = {0};
			lstrcpynW(szSelFolderPath, pFileDlg->GetPathName(), MAX_PATH);
			PathRemoveFileSpec(szSelFolderPath);
			theApp.m_strLastSelectFolderPath = szSelFolderPath;
			strPath = pFileDlg->GetPathName();
			CefString strPDFPath(strPath);
			CefPdfPrintSettings stPDFSetting;

			if (theApp.m_AppSettings.IsEnableLogging() && theApp.m_AppSettings.IsEnableDownloadLogging())
			{
				CString strFileName;
				TCHAR *ptrFile = NULL;
				ptrFile = PathFindFileName(strPath);
				if (ptrFile)
				{
					strFileName = ptrFile;
				}
				theApp.m_pLogDisp->SendLog(LOG_DOWNLOAD, strFileName, m_strURL);
			}

#if CHROME_VERSION_MAJOR >= 108
			// Since CEF108, cef_pdf_print_settings_t is refactored.
			CefString(&stPDFSetting.header_template) = CefString(this->m_strTitle);
			CefString(&stPDFSetting.footer_template) = CefString(this->m_strURL);
			stPDFSetting.paper_width = 0;
			stPDFSetting.paper_height = 0;
#else
			CefString(&stPDFSetting.header_footer_title) = CefString(this->m_strTitle);
			CefString(&stPDFSetting.header_footer_url) = CefString(this->m_strURL);
			stPDFSetting.page_width = 0;
			stPDFSetting.page_height = 0;
#endif
			stPDFSetting.margin_top = 0;
			stPDFSetting.margin_right = 0;
			stPDFSetting.margin_bottom = 0;
			stPDFSetting.margin_left = 0;
			stPDFSetting.margin_type = PDF_PRINT_MARGIN_DEFAULT;
#if CHROME_VERSION_MAJOR >= 108
			stPDFSetting.display_header_footer = 1;
			CefString(&stPDFSetting.page_ranges) = CefString("");
			stPDFSetting.landscape = 0;
			stPDFSetting.print_background = 1;
#else
			stPDFSetting.header_footer_enabled = 1;
			stPDFSetting.selection_only = 0;
			stPDFSetting.landscape = 0;
			stPDFSetting.backgrounds_enabled = 1;
#endif
			CefRefPtr<CefPdfPrintCallback> callback;

			m_cefBrowser->GetHost()->PrintToPDF(strPDFPath, stPDFSetting, callback);

			CString finishMsg;
			finishMsg.LoadString(ID_PRINT_TO_PDF_FINISH_MSG);
			theApp.SB_MessageBox(m_pwndFrame->m_hWnd, finishMsg, NULL, MB_ICONINFORMATION | MB_OK, TRUE, 5000);
		}
		if (pFileDlg)
		{
			delete pFileDlg;
			pFileDlg = NULL;
		}
	}
}
void CChildView::ShowDevTools()
{
	CString logmsg;
	logmsg.Format(_T("CV_WND:0x%08x ShowDevTools"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	if (m_cefBrowser)
	{
		bool has_devtools = m_cefBrowser->GetHost()->HasDevTools();
		if (!has_devtools)
		{
			CefWindowInfo windowInfo;
			CefBrowserSettings settings;
			OnNewWindow((WPARAM)WND_TYPE_DEV_TOOLS, (LPARAM)&windowInfo);
			CefRefPtr<CefClient> client;
			CefPoint inspect_element_at;
			m_cefBrowser->GetHost()->ShowDevTools(windowInfo, client, settings, inspect_element_at);
		}
	}
}
void CChildView::OnPrint()
{
	CString logmsg;
	logmsg.Format(_T("CV_WND:0x%08x OnPrint"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	if (m_cefBrowser)
	{
		m_cefBrowser->GetHost()->Print();
	}
}
void CChildView::OnGoBack()
{
	CString logmsg;
	logmsg.Format(_T("CV_WND:0x%08x OnGoBack"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	if (!IsBrowserNull())
		GoBack();
}

void CChildView::OnUpdateGoBack(CCmdUI *pCmdUI)
{
	if (m_cefBrowser)
		pCmdUI->Enable(FRM->m_nBrowserState & CEF_BIT_CAN_GO_BACK);
}

void CChildView::OnGoForward()
{
	CString logmsg;
	logmsg.Format(_T("CV_WND:0x%08x OnGoForward"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	if (!IsBrowserNull())
		GoForward();
}

void CChildView::OnUpdateGoForward(CCmdUI *pCmdUI)
{
	if (m_cefBrowser)
		pCmdUI->Enable(FRM->m_nBrowserState & CEF_BIT_CAN_GO_FORWARD);
}

void CChildView::OnGoStartPage()
{
	if (!IsBrowserNull())
	{
		CString logmsg;
		logmsg.Format(_T("CV_WND:0x%08x OnGoStartPage"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
		this->GoHome();
	}
}

void CChildView::OnUpdateGoStartPage(CCmdUI *pCmdUI)
{
}

void CChildView::OnViewRefresh()
{
	CString logmsg;
	logmsg.Format(_T("CV_WND:0x%08x OnViewRefresh"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	if (::GetAsyncKeyState(VK_CONTROL) < 0
		&&  ::GetKeyState(VK_SHIFT) >= 0
		&&  ::GetKeyState(VK_MENU) >= 0)

		Refresh2(REFRESH_COMPLETELY);
	else
		Refresh();
}

void CChildView::OnUpdateViewRefresh(CCmdUI *pCmdUI)
{
}

void CChildView::OnViewStop()
{
	CString logmsg;
	logmsg.Format(_T("CV_WND:0x%08x OnViewStop"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	if (!IsBrowserNull())
		Stop();
}

void CChildView::OnUpdateViewStop(CCmdUI *pCmdUI)
{
	if (m_cefBrowser)
	{
		BOOL bLoading = FALSE;
		if (FRM->m_nBrowserState & CEF_BIT_IS_LOADING)
		{
			bLoading = TRUE;
		}
		pCmdUI->Enable(bLoading);
	}
}

void CChildView::OnOpen()
{
}

void CChildView::OnCut()
{
	HWND hwnd = NULL;
	hwnd = ::GetFocus();
	if (theApp.IsWnd(FRM) && theApp.IsWnd(FRM->m_pwndAddress) && (hwnd == FRM->m_pwndAddress->m_hWnd || hwnd == FRM->m_pwndAddress->m_Edit.m_hWnd))
	{
		FRM->m_pwndAddress->m_Edit.Cut();
	}
	else
	{
		if (m_cefBrowser)
			m_cefBrowser->GetMainFrame()->Cut();
	}
}
void CChildView::OnCopy()
{
	HWND hwnd = NULL;
	hwnd = ::GetFocus();

	if (theApp.IsWnd(FRM) && theApp.IsWnd(FRM->m_pwndAddress) && (hwnd == FRM->m_pwndAddress->m_hWnd || hwnd == FRM->m_pwndAddress->m_Edit.m_hWnd))
	{
		FRM->m_pwndAddress->m_Edit.Copy();
	}
	else
	{
		if (m_cefBrowser)
			m_cefBrowser->GetMainFrame()->Copy();
	}
}

void CChildView::OnPaste()
{
	HWND hwnd = NULL;
	hwnd = ::GetFocus();

	if (theApp.IsWnd(FRM) && theApp.IsWnd(FRM->m_pwndAddress) && (hwnd == FRM->m_pwndAddress->m_hWnd || hwnd == FRM->m_pwndAddress->m_Edit.m_hWnd))
	{
		FRM->m_pwndAddress->m_Edit.Paste();
	}
	else
	{
		if (m_cefBrowser)
			m_cefBrowser->GetMainFrame()->Paste();
	}
}
void CChildView::OnSelAll()
{
	HWND hwnd = NULL;
	hwnd = ::GetFocus();

	if (theApp.IsWnd(FRM) && theApp.IsWnd(FRM->m_pwndAddress) && (hwnd == FRM->m_pwndAddress->m_hWnd || hwnd == FRM->m_pwndAddress->m_Edit.m_hWnd))
	{
		CString str;
		FRM->m_pwndAddress->m_Edit.GetWindowText(str);
		FRM->m_pwndAddress->m_Edit.SetSel(0, str.GetLength());
	}
	else
	{
		if (m_cefBrowser)
			m_cefBrowser->GetMainFrame()->SelectAll();
	}
}

LRESULT CChildView::OnFindDialogMessage(WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_pFindDialog != NULL);
	try
	{
		if (m_pFindDialog->IsTerminating())
		{
			if (m_cefBrowser)
				m_cefBrowser->GetHost()->StopFinding(true);
			m_pFindDialog = NULL;
			m_bFindNext = FALSE;
			return S_OK;
		}

		if (m_pFindDialog->FindNext())
		{
			CString FindName = m_pFindDialog->GetFindString();
			bool bMatchCase = m_pFindDialog->MatchCase() == TRUE ? true : false;
			bool bMatchWholeWord = m_pFindDialog->MatchWholeWord() == TRUE ? true : false;
			bool bSearchDown = m_pFindDialog->SearchDown() == TRUE ? true : false;

			CefString csFind = FindName;
			if (m_cefBrowser)
			{
#if CHROME_VERSION_MAJOR >= 99
				// Since CEF99, no need to specify identifier for Find API
				m_cefBrowser->GetHost()->Find(csFind, bSearchDown, bMatchCase, m_bFindNext);
#else
				INT nBrowserId = m_cefBrowser->GetIdentifier();
				m_cefBrowser->GetHost()->Find(nBrowserId, csFind, bSearchDown, bMatchCase, m_bFindNext);
#endif
			}
			if (!m_bFindNext)
				m_bFindNext = TRUE;
		}
	}
	catch (...)
	{
		;
	}
	return S_OK;
}

void CChildView::OnFindPage()
{
	CString logmsg;
	logmsg.Format(_T("CV_WND:0x%08x OnFindPage"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	if (m_cefBrowser)
	{
		ASSERT(m_pFindDialog == NULL);
		m_pFindDialog = new CFindReplaceDialog();
		m_pFindDialog->Create(TRUE, _T(""), NULL, FR_DOWN, this);
	}
}

void CChildView::OnUpdateCut(CCmdUI *pCmdUI)
{
	BOOL bEnable = FALSE;
	HWND hwnd = NULL;
	hwnd = ::GetFocus();

	if (theApp.IsWnd(FRM) && theApp.IsWnd(FRM->m_pwndAddress) && (hwnd == FRM->m_pwndAddress->m_hWnd || hwnd == FRM->m_pwndAddress->m_Edit.m_hWnd))
	{
		bEnable = TRUE;
	}
	else
	{
		if (m_cefBrowser)
			bEnable = TRUE;
	}
	pCmdUI->Enable(bEnable);
}

void CChildView::OnUpdatePaste(CCmdUI *pCmdUI)
{
	BOOL bEnable = FALSE;
	HWND hwnd = NULL;
	hwnd = ::GetFocus();

	if (theApp.IsWnd(FRM) && theApp.IsWnd(FRM->m_pwndAddress) && (hwnd == FRM->m_pwndAddress->m_hWnd || hwnd == FRM->m_pwndAddress->m_Edit.m_hWnd))
	{
		bEnable = TRUE;
	}
	else
	{
		if (m_cefBrowser)
			bEnable = TRUE;
	}
	pCmdUI->Enable(bEnable);
}
void CChildView::OnAppAbout()
{
	theApp.m_strCurrentURL4DlgSetting = m_strURL;
	CAboutDlg aboutDlg(this);
	aboutDlg.pParentFrm = m_pwndFrame;
	aboutDlg.DoModal();
}

void CChildView::OnSettings()
{
	CString logmsg;
	logmsg.Format(_T("CV_WND:0x%08x OnSettings"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	theApp.m_strCurrentURL4DlgSetting = m_strURL;
	theApp.ShowSettingDlg(m_pwndFrame);
}
void CChildView::OnBroBack(UINT nID)
{
	CString logmsg;
	logmsg.Format(_T("CV_WND:0x%08x OnBroBack"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	int max = nID - ID_BF_BACK1 + 1;
	for (int i = 0; i < max; i++)
		GoBack();
}
void CChildView::OnZoom(UINT nID)
{
	CString logmsg;
	logmsg.Format(_T("CV_WND:0x%08x OnZoom"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	int max = nID - ID_ZOOM_START + 1;
}
void CChildView::OnBroForward(UINT nID)
{
	CString logmsg;
	logmsg.Format(_T("CV_WND:0x%08x OnBroForward"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	int max = nID - ID_BF_FORWARD1 + 1;
	for (int i = 0; i < max; i++)
		GoForward();
}

void CChildView::OnSetFocus(CWnd *pOldWnd)
{
	ViewBaseClass::OnSetFocus(pOldWnd);
}

void CChildView::OnKillFocus(CWnd *pNewWnd)
{
	ViewBaseClass::OnKillFocus(pNewWnd);
}

void CChildView::OnFullScreen(BOOL bFlg)
{
	CString logmsg;
	logmsg.Format(_T("CV_WND:0x%08x OnFullScreen"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	if (bFlg)
	{
		if (theApp.IsWnd(FRM))
		{
			FRM->ChangeFullScreenWindow();
		}
	}
	else
	{
		if (theApp.IsWnd(FRM))
		{
			FRM->ChangeNomalWindow();
		}
	}
}

void CChildView::OnAddressBar(BOOL bFlg)
{
	try
	{
		CString logmsg;
		//非表示
		if (!bFlg)
		{
			logmsg.Format(_T("CV_WND:0x%08x OnAddressBar_HIDE"), theApp.SafeWnd(this->m_hWnd));
			theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_JS);
			if (theApp.IsWnd(m_pwndFrame))
			{
				if (FRM->m_pwndReBar)
				{
					CReBarCtrl &rbCtrl = FRM->m_pwndReBar->GetReBarCtrl();
					REBARBANDINFO rbbi = {0};

					UINT cbCompileSize = 0;

					cbCompileSize = sizeof(REBARBANDINFO);
					rbbi.cbSize = cbCompileSize;
					rbbi.fMask = RBBIM_STYLE;
					VERIFY(rbCtrl.GetBandInfo(2, &rbbi));
					if (!(rbbi.fStyle & RBBS_HIDDEN))
					{
						VERIFY(rbCtrl.ShowBand(2, FALSE));
					}
				}
			}
		}
		else
		{
			logmsg.Format(_T("CV_WND:0x%08x OnAddressBar"), theApp.SafeWnd(this->m_hWnd));
			theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_JS);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

void CChildView::OnToolBar(BOOL bFlg)
{
	CString logmsg;
	//非表示
	if (!bFlg)
	{
		logmsg.Format(_T("CV_WND:0x%08x OnToolBar_HIDE"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_JS);
		if (theApp.IsWnd(m_pwndFrame))
		{
			if (FRM->m_pwndReBar)
			{
				CReBarCtrl &rbCtrl = FRM->m_pwndReBar->GetReBarCtrl();
				REBARBANDINFO rbbi = {0};
				UINT cbCompileSize = 0;

				cbCompileSize = sizeof(REBARBANDINFO);
				rbbi.cbSize = cbCompileSize;
				rbbi.fMask = RBBIM_STYLE;
				VERIFY(rbCtrl.GetBandInfo(1, &rbbi));
				if (!(rbbi.fStyle & RBBS_HIDDEN))
				{
					VERIFY(rbCtrl.ShowBand(1, FALSE));
				}
			}
		}
	}
	else
	{
		logmsg.Format(_T("CV_WND:0x%08x OnToolBar"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_JS);
	}
}

void CChildView::OnMenuBar(BOOL bFlg)
{
	CString logmsg;
	//非表示
	if (!bFlg)
	{
		logmsg.Format(_T("CV_WND:0x%08x OnMenuBar_HIDE"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_JS);
		if (theApp.IsWnd(m_pwndFrame))
		{
			if (FRM->m_pwndReBar)
			{
				CReBarCtrl &rbCtrl = FRM->m_pwndReBar->GetReBarCtrl();
				REBARBANDINFO rbbi = {0};
				UINT cbCompileSize = 0;
				cbCompileSize = sizeof(REBARBANDINFO);
				rbbi.cbSize = cbCompileSize;
				rbbi.fMask = RBBIM_STYLE;
				VERIFY(rbCtrl.GetBandInfo(0, &rbbi));
				if (!(rbbi.fStyle & RBBS_HIDDEN))
				{
					VERIFY(rbCtrl.ShowBand(0, FALSE));
				}
			}
		}
	}
	else
	{
		logmsg.Format(_T("CV_WND:0x%08x OnMenuBar"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_JS);
	}
}

void CChildView::OnStatusBar(BOOL bFlg)
{
	CString logmsg;
	//非表示
	if (!bFlg)
	{
		logmsg.Format(_T("CV_WND:0x%08x OnStatusBar_HIDE"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_JS);
		if (theApp.IsWnd(FRM) && theApp.IsWnd(FRM->m_pwndStatusBar))
		{
			FRM->m_pwndStatusBar->ShowWindow(SW_HIDE);
		}
	}
	else
	{
		logmsg.Format(_T("CV_WND:0x%08x OnStatusBar"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_JS);
	}
}

void CChildView::OnVisible(BOOL bFlg)
{
	CString logmsg;
	//表示
	if (bFlg)
	{
		logmsg.Format(_T("CV_WND:0x%08x OnVisible"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_JS);
	}
	//非表示
	else
	{
		logmsg.Format(_T("CV_WND:0x%08x OnVisible_HIDE"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_JS);
		this->EnableWindow(FALSE);
		if (theApp.IsWnd(m_pwndFrame))
		{
			FRM->ShowWindow(SW_HIDE);
		}
	}
}
void CChildView::OnWindowSetResizable(BOOL bFlg)
{
	if (theApp.IsWnd(m_pwndFrame))
	{
		LONG nOldStyle = ::GetWindowLong(m_pwndFrame->m_hWnd, GWL_STYLE);
		LONG nNewStyle = 0;
		CString logmsg;
		//サイズ変更可能
		if (bFlg)
		{
			logmsg.Format(_T("CV_WND:0x%08x OnWindowSetResizable_TRUE"), theApp.SafeWnd(this->m_hWnd));
			theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_JS);

			nNewStyle = nOldStyle | WS_THICKFRAME | WS_MAXIMIZEBOX;
			::SetWindowLong(m_pwndFrame->m_hWnd, GWL_STYLE, nNewStyle);
			m_pwndFrame->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DRAWFRAME);
		}
		//サイズ変更不可
		else
		{
			logmsg.Format(_T("CV_WND:0x%08x OnWindowSetResizable_FALSE"), theApp.SafeWnd(this->m_hWnd));
			theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_JS);

			nNewStyle = nOldStyle & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
			::SetWindowLong(m_pwndFrame->m_hWnd, GWL_STYLE, nNewStyle);
			m_pwndFrame->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DRAWFRAME);
		}
	}
}

void CChildView::CreateNewBrowserWindow(LPCTSTR lpszUrl, BOOL bActive)
{
	PROC_TIME(CreateNewBrowserWindow)

	try
	{
		if (theApp.m_pMainWnd)
		{
			if (::IsWindow(theApp.m_pMainWnd->m_hWnd))
			{
				CString strURL = lpszUrl;
				CChildView *pCreateView = NULL;
				DWORD bFlags = bActive ? 0 : NWMF_FORCETAB;

				DebugWndLogData dwLogData;
				dwLogData.mHWND.Format(_T("CV_WND:0x%08x"), theApp.SafeWnd(this->m_hWnd));
				dwLogData.mFUNCTION_NAME = _T("CreateNewBrowserWindow");
				dwLogData.mMESSAGE1 = strURL;
				dwLogData.mMESSAGE2.Format(_T("bActive:%s"), bActive ? _T("TRUE") : _T("FALSE"));
				theApp.AppendDebugViewLog(dwLogData);
				CString logmsg = dwLogData.GetString();
				theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_URL);

				pCreateView = ((CMainFrame *)theApp.m_pMainWnd)->NewBrowserWindow(bFlags);
				if (pCreateView)
				{
					if (strURL.IsEmpty())
					{
						strURL = _T("about:blank");
					}
					HWND hWnd = pCreateView->GetSafeHwnd();
					CRect rect;
					pCreateView->GetClientRect(rect);
					CefWindowInfo info;
					CefRect windowBounds;
					windowBounds.Set(rect.right,rect.top,rect.Width(),rect.Height());
					info.SetAsChild(hWnd, windowBounds);
					CefBrowserSettings browserSettings;
					pCreateView->m_clientHandler = new ClientHandler();
					pCreateView->m_clientHandler->CreateBrowser(info, browserSettings, CefString(strURL));
				}
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

LRESULT CChildView::OnNewWindow(WPARAM wParam, LPARAM lParam)
{
	PROC_TIME(OnNewWindow)

	BOOL bDevTools = FALSE;

	CefPopupFeatures *popupFeatures = NULL;
	if (wParam == WND_TYPE_DEV_TOOLS)
	{
		popupFeatures = &CefPopupFeatures();
		bDevTools = TRUE;
	}
	else
	{
		popupFeatures = (CefPopupFeatures *)wParam;
	}
	CefWindowInfo *windowInfo = (CefWindowInfo *)lParam;

	CChildView *pCreateView = NULL;
	if (theApp.m_pMainWnd == NULL)
	{
		return S_OK;
	}
	if (::IsWindow(theApp.m_pMainWnd->m_hWnd) == FALSE)
	{
		return S_OK;
	}

	DWORD dwFlags = 0;
	if (popupFeatures != NULL)
	{
		dwFlags = NWMF_SUGGESTWINDOW;
	}
	pCreateView = ((CMainFrame *)theApp.m_pMainWnd)->NewBrowserWindow(dwFlags);

	if (!pCreateView)
		return S_OK;
	if (bDevTools)
	{
		pCreateView->m_bDevToolsWnd = TRUE;
	}
	if (popupFeatures)
	{
		pCreateView->m_popupFeatures = new CefPopupFeatures;
		pCreateView->m_popupFeatures->Set(*popupFeatures, true);
		pCreateView->ResizeWindowPopup();
	}
	HWND hWnd = pCreateView->GetSafeHwnd();
	CRect rect;
	pCreateView->GetClientRect(rect);
	CefRect windowBounds;
	windowBounds.Set(rect.right, rect.top, rect.Width(), rect.Height());
	windowInfo->SetAsChild(hWnd, windowBounds);
	return S_OK;
}

LRESULT CChildView::OnCreateNewBrowserWindow(WPARAM wParam, LPARAM lParam)
{
	PROC_TIME(OnCreateNewBrowserWindow)

	try
	{
		if (lParam)
		{
			CString str = (TCHAR *)lParam;
			BOOL bActive = true;
			if (wParam)
			{
				switch (wParam)
				{
					case cef_window_open_disposition_t::WOD_NEW_FOREGROUND_TAB:
					{
						bActive = true;
						break;
					}
					case cef_window_open_disposition_t::WOD_NEW_BACKGROUND_TAB:
					{
						bActive = false;
						break;
					}
					case cef_window_open_disposition_t::WOD_NEW_WINDOW:
					{
						bActive = true;
						break;
					}
					default:
						break;
				}
			}
			CreateNewBrowserWindow(str, bActive);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return 0;
}

void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	try
	{
		ViewBaseClass::OnTimer(nIDEvent);
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}
void CChildView::OnDestroy()
{
	try
	{

		if (!IsBrowserNull())
		{
			this->SafeWindowCloseFunction();
		}
		CString logmsg;
		logmsg.Format(_T("CV_WND:0x%08x CChildView::OnDestroy"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_CL);
		ViewBaseClass::OnDestroy();
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

void CChildView::SafeWindowCloseFunction()
{
	//実施済み
	if (m_bWndCloseFlg) return;
	m_bWndCloseFlg = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CChildView::OnBeforeBrowse(WPARAM wParam, LPARAM lParam)
{
	PROC_TIME(OnBeforeBrowse)

	if (wParam)
	{
		CString strURL((LPCTSTR)wParam);
		if (!strURL.IsEmpty())
		{
			BOOL bTopPage = FALSE;
			UINT *pbRet = NULL;

			//TOPページ(Frameなし)
			if (lParam)
			{
				pbRet = (UINT *)lParam;
			}
			if (pbRet)
			{
				if (*pbRet == 1)
				{
					bTopPage = TRUE;
				}
			}
			DebugWndLogData dwLogData;
			dwLogData.mHWND.Format(_T("CV_WND:0x%08x"), theApp.SafeWnd(this->m_hWnd));
			dwLogData.mFUNCTION_NAME = _T("OnBeforeBrowse");
			dwLogData.mMESSAGE1 = strURL;
			dwLogData.mMESSAGE2.Format(_T("TopPage:%s"), bTopPage ? _T("TRUE") : _T("FALSE"));
			theApp.AppendDebugViewLog(dwLogData);
			CString logmsg = dwLogData.GetString();
			theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_URL);

			if (IsRedirectURLChk(strURL, bTopPage))
			{
				*pbRet = 2;
				IsRedirectWndAutoCloseChk();
			}
		}
	}
	return S_OK;
}

LRESULT CChildView::OnBeforeDownload(WPARAM wParam, LPARAM lParam)
{
	if (wParam)
	{
		FRM->m_bDownloadProgress = TRUE;
		return S_OK;
	}
	FRM->m_bDownloadProgress = FALSE;
	return S_OK;
}

LRESULT CChildView::OnDownloadBlankPage(WPARAM wParam, LPARAM lParam)
{
	if (wParam)
	{
		FRM->m_bDownloadBlankPage = TRUE;
		if (FRM->m_cTabWnd)
		{
			if (theApp.IsWndVisible(this->m_hWnd))
			{
				if (theApp.GetActiveViewPtr() == this)
				{
					CBrowserFrame *pWnd = NULL;
					pWnd = theApp.GetNextGenerationActiveWindow(FRM);
					if (theApp.IsWnd(pWnd))
					{
						FRM->m_cTabWnd->ShowTabWindow(pWnd->GetSafeHwnd());
					}
				}
			}
		}
		//非表示
		FRM->EnableWindow(FALSE);
		FRM->ShowWindow(SW_HIDE);
		return S_OK;
	}
	FRM->m_bDownloadBlankPage = FALSE;
	return S_OK;
}
LRESULT CChildView::OnDownloadUpdate(WPARAM wParam, LPARAM lParam)
{
	if (wParam)
	{
		FRM->m_bDownloadProgress = TRUE;
		return S_OK;
	}
	FRM->m_bDownloadProgress = FALSE;
	return S_OK;
}
LRESULT CChildView::OnBeforeResourceLoad(WPARAM wParam, LPARAM lParam)
{
	if (!lParam)
		return S_OK;

	CString strURL((LPCTSTR)lParam);
	if (strURL.IsEmpty())
		return S_OK;

	DebugWndLogData dwLogData;
	dwLogData.mHWND.Format(_T("CV_WND:0x%08x"), theApp.SafeWnd(this->m_hWnd));
	dwLogData.mFUNCTION_NAME = _T("OnBeforeResourceLoad");
	dwLogData.mMESSAGE1 = strURL;
	theApp.AppendDebugViewLog(dwLogData);
	CString logmsg = dwLogData.GetString();
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_URL);

	return S_OK;
}

LRESULT CChildView::OnLoadStart(WPARAM wParam, LPARAM lParam)
{
	PROC_TIME(OnLoadStart)
	m_strTitle.Empty();
	if (!theApp.IsWnd(FRM))
		return S_OK;

	if (!theApp.IsWnd(FRM->m_pwndStatusBar))
		return S_OK;

	FRM->m_pwndStatusBar->EnablePaneProgressBar(nStatusProgress, 100);

	DebugWndLogData dwLogData;
	dwLogData.mHWND.Format(_T("CV_WND:0x%08x"), theApp.SafeWnd(this->m_hWnd));
	dwLogData.mFUNCTION_NAME = _T("OnLoadStart");
	theApp.AppendDebugViewLog(dwLogData);
	CString logmsg = dwLogData.GetString();
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_TR);

	return S_OK;
}

LRESULT CChildView::OnLoadEnd(WPARAM wParam, LPARAM lParam)
{
	PROC_TIME(OnLoadEnd)
	if (theApp.IsWnd(FRM))
	{
		if (theApp.IsWnd(FRM->m_pwndStatusBar))
		{
			FRM->SetProgress(0, -1);

			DebugWndLogData dwLogData;
			dwLogData.mHWND.Format(_T("CV_WND:0x%08x"), theApp.SafeWnd(this->m_hWnd));
			dwLogData.mFUNCTION_NAME = _T("OnLoadEnd");
			theApp.AppendDebugViewLog(dwLogData);
			CString logmsg = dwLogData.GetString();
			theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_TR);
		}
	}
	if (theApp.m_AppSettings.IsEnableLogging() && theApp.m_AppSettings.IsEnableBrowsingLogging())
	{
		if (SBUtil::IsURL_HTTP(m_strURL))
		{
			CString strHost;
			strHost = m_strTitle;
			strHost.TrimLeft();
			strHost.TrimRight();

			if (strHost.IsEmpty())
			{
				CefURLParts cfURLpa;
				CefString cefURL(m_strURL);
				if (CefParseURL(cefURL, cfURLpa))
				{
					CefString cfHost(&cfURLpa.host);
					strHost = cfHost.c_str();
				}
			}
			if (theApp.m_pLogDisp)
				theApp.m_pLogDisp->SendLog(LOG_BROWSING, strHost, m_strURL);
		}
	}

	return S_OK;
}

LRESULT CChildView::OnTitleChange(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR lpszTitle = (LPCTSTR)wParam;
	CStringW strText(lpszTitle);
	OnTitleChange(strText);
	return S_OK;
}

LRESULT CChildView::OnFaviconChange(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR pszFavURL = (LPCTSTR)wParam;
	if (pszFavURL)
	{
		if (FRM->m_cTabWnd)
		{
			if (m_cefBrowser)
				m_cefBrowser->GetHost()->DownloadImage(pszFavURL, true, 100, false, &FRM->m_FaviconCB);
		}
	}
	return S_OK;
}
LRESULT CChildView::OnCopyImage(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR lpszURL = (LPCTSTR)wParam;
	CString strURL(lpszURL);
	if (!strURL.IsEmpty())
	{
		if (m_cefBrowser)
			m_cefBrowser->GetHost()->DownloadImage(lpszURL, false, 0, false, &FRM->m_ImageCopy);
	}
	return S_OK;
}

LRESULT CChildView::OnFullScreenModeChange(WPARAM wParam, LPARAM lParam)
{
	PROC_TIME(OnFullScreenModeChange)
	BOOL bMode = FALSE;
	if (wParam)
		bMode = TRUE;
	else
		bMode = FALSE;

	OnFullScreen(bMode);
	return S_OK;
}

LRESULT CChildView::OnProgressChange(WPARAM wParam, LPARAM lParam)
{
	PROC_TIME(OnProgressChange)
	try
	{
		if (theApp.IsWnd(FRM))
		{
			if (theApp.IsWnd(FRM->m_pwndStatusBar))
			{
				DWORD dwProgress = 0;
				dwProgress = (DWORD)wParam;
				if (dwProgress >= 100)
				{
					FRM->SetProgress(0, -1);
					return S_OK;
				}
				if (FRM->m_pwndStatusBar->GetPainProgressTotal(nStatusProgress) < 0)
				{
					FRM->m_pwndStatusBar->EnablePaneProgressBar(nStatusProgress, 100);
					return S_OK;
				}
				FRM->m_pwndStatusBar->SetPaneProgress(nStatusProgress, min(100, max(0, dwProgress)));

				DebugWndLogData dwLogData;
				dwLogData.mHWND.Format(_T("CV_WND:0x%08x"), theApp.SafeWnd(this->m_hWnd));
				dwLogData.mFUNCTION_NAME = _T("OnProgressChange");
				dwLogData.mMESSAGE1.Format(_T("%d"), dwProgress);
				theApp.AppendDebugViewLog(dwLogData);
				CString logmsg = dwLogData.GetString();
				theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_TR);
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return S_OK;
}

LRESULT CChildView::OnStateChange(WPARAM wParam, LPARAM lParam)
{
	FRM->m_nBrowserState = (INT)wParam;
	if (!lParam)
		return S_OK;

	CString strURL((LPCTSTR)lParam);
	if (strURL.IsEmpty())
		return S_OK;

	DebugWndLogData dwLogData;
	dwLogData.mHWND.Format(_T("CV_WND:0x%08x"), theApp.SafeWnd(this->m_hWnd));
	dwLogData.mFUNCTION_NAME = _T("OnStateChange");
	dwLogData.mMESSAGE1 = strURL;
	dwLogData.mMESSAGE2.Format(_T("Loading:%s"), FRM->m_nBrowserState & CEF_BIT_IS_LOADING ? _T("TRUE") : _T("FALSE"));
	theApp.AppendDebugViewLog(dwLogData);
	CString logmsg = dwLogData.GetString();
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_URL);

	return S_OK;
}

LRESULT CChildView::OnWindowActivate(WPARAM wParam, LPARAM lParam)
{
	if (FRM->m_cTabWnd)
	{
		FRM->m_cTabWnd->ShowTabWindow((HWND)FRM->GetSafeHwnd());
	}
	else
	{
		if (::IsIconic(FRM->m_hWnd))
		{
			::ShowWindow(FRM->m_hWnd, SW_HIDE);
			::ShowWindow(FRM->m_hWnd, SW_NORMAL);
		}
		::SetWindowPos(FRM->m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
	return S_OK;
}
LRESULT CChildView::OnSetRendererPID(WPARAM wParam, LPARAM lParam)
{
	FRM->m_RendererPID = (UINT)wParam;
	CBrowserFrame *pFrm = NULL;
	if (FRM->m_RendererPID > 0)
	{
		pFrm = theApp.GetActiveBFramePtr();
		if (pFrm)
		{
			//優先度を下げる
			if (pFrm != FRM)
			{
				if (pFrm->m_RendererPID > 0)
				{
					if (pFrm->m_RendererPID != FRM->m_RendererPID)
					{
						//低
						if (!theApp.SetPriority(FRM->m_RendererPID, IDLE_PRIORITY_CLASS))
						{
							//ReSetRendererPID();
						}
					}
				}
			}
		}
	}
	return S_OK;
}

LRESULT CChildView::OnAddressChange(WPARAM wParam, LPARAM lParam)
{
	CString strURL((LPCTSTR)wParam);
	m_strURL = strURL;
	UpDateAddressBar();
	if (!strURL.IsEmpty())
	{
		BOOL bTopPage = TRUE;
		if (IsRedirectURLChk(strURL, bTopPage))
		{
			IsRedirectWndAutoCloseChk();
		}
	}
	return S_OK;
}

LRESULT CChildView::OnStatusMessage(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR lpszStatus = (LPCTSTR)wParam;
	OnStatusTextChange(lpszStatus);
	return S_OK;
}

LRESULT CChildView::OnSearchURL(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR lpszURL = (LPCTSTR)wParam;
	FRM->Search(lpszURL, (BOOL)lParam);
	return S_OK;
}

LRESULT CChildView::OnCloseBrowser(WPARAM wParam, LPARAM lParam)
{
	INT nBrowserID = (INT)wParam;
	return S_OK;
}

//LRESULT CChildView::OnNewBrowser(WPARAM wParam, LPARAM lParam)
//{
//	PROC_TIME(OnNewBrowser)
//	m_nBrowserID = (INT)wParam;
//	CefBrowser* browser = (CefBrowser *)lParam;
//	m_cefBrowser = browser;
//	this->PostMessage(WM_SIZE);
//	return S_OK;
//}
void CChildView::SetBrowserPtr(INT nBrowserId,CefRefPtr<CefBrowser> browser)
{
	ASSERT(!m_nBrowserID);
	m_cefBrowser = browser;
	m_nBrowserID = nBrowserId;
	this->PostMessage(WM_SIZE);
}

LRESULT CChildView::OnAuthenticate(WPARAM wParam, LPARAM lParam)
{
	CEFAuthenticationValues *lpValues = (CEFAuthenticationValues *)wParam;
	CString strHost(lpValues->lpszHost);
	CDlgAuth Dlg(this);
	CString authRequiredMsg;
	authRequiredMsg.LoadString(IDS_STRING_HOST_AUTH_REQUIRED);
	Dlg.m_strMsgTxt.Format(authRequiredMsg, strHost);

	Dlg.Create(IDD_DLG_AUTH, this);
	Dlg.CenterWindow(this);
	Dlg.ShowWindow(SW_SHOW);
	int iResult = Dlg.RunModalLoop(0);
	Dlg.DestroyWindow();

	if (iResult == IDOK)
	{
		_tcscpy_s(lpValues->szUserName, Dlg.m_strID);
		_tcscpy_s(lpValues->szUserPass, Dlg.m_strPW);
		return S_OK;
	}
	return S_FALSE;
}

LRESULT CChildView::OnBadCertificate(WPARAM wParam, LPARAM lParam)
{
	return S_OK;
}
void CChildView::Navigate(LPCTSTR pszURL)
{
	PROC_TIME(Navigate)

	if (IsBrowserNull())
	{
		auto rect = RECT{0};
		GetClientRect(&rect);

		CefWindowInfo info;
		CefRect windowBounds;
		windowBounds.Set(rect.right, rect.top,rect.right-rect.left,rect.bottom-rect.top);
		info.SetAsChild(GetSafeHwnd(), windowBounds);

		CefBrowserSettings browserSettings;

		m_clientHandler = new ClientHandler();
		m_clientHandler->CreateBrowser(info, browserSettings, CefString(pszURL));
		return;
	}
	else
	{
		this->Stop();
		m_cefBrowser->GetMainFrame()->LoadURL(pszURL);
		::SetFocus(m_cefBrowser->GetHost()->GetWindowHandle());
		m_cefBrowser->GetHost()->SetFocus(true);
	}
}
BOOL CChildView::bSetCefBrowserFocus()
{
	BOOL bRet = FALSE;
	if (!IsBrowserNull())
	{
		if (m_cefBrowser->GetHost())
		{
			m_cefBrowser->GetHost()->SetFocus(true);
			bRet = TRUE;
		}
	}
	return bRet;
}
