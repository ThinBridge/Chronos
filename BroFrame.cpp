#include "stdafx.h"
#include "Sazabi.h"
#include "BroFrame.h"
#include "Sazabi.h"
#include "MainFrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif
static UINT indicators[] =
    {
	ID_INDICATOR_ICON,    // status icon
	ID_SEPARATOR,	      // status line indicator
	ID_INDICATOR_PROCESS, // progress bar
	ID_INDICATOR_SSL,
	ID_INDICATOR_COMPAT,
	ID_INDICATOR_ZOOM,
};

IMPLEMENT_DYNAMIC(CBrowserFrame, CFrameWndBase)
BEGIN_MESSAGE_MAP(CBrowserFrame, CFrameWndBase)
	//{{AFX_MSG_MAP(CBrowserFrame)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_NCCREATE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_CLOSE()
	ON_WM_ACTIVATE()
	ON_WM_NCACTIVATE()
	ON_WM_MOUSEACTIVATE()
	ON_WM_GETMINMAXINFO()
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_SET_ADDRESSBAR, OnSetAddressbar)
	ON_COMMAND(ID_SET_SEARCHBAR, OnSetSearchbar)

	//}}AFX_MSG_MAP
	ON_MESSAGE(ID_MYCOMBO_OK, OnNewAddressEnter)
	ON_MESSAGE(ID_MYCOMBO_SELENDOK, OnNewAddress)
	ON_MESSAGE(WM_SEL_SEARCH, OnSearchString)
	ON_COMMAND_RANGE(ID_FAV_START, ID_FAV_END, OnFavMenu)
	ON_COMMAND_RANGE(ID_WINDOW_START, ID_WINDOW_END, OnWndMenu)
	ON_COMMAND_RANGE(ID_CLOSE_WINDOW_START, ID_CLOSE_WINDOW_END, OnCloseWndMenu)
	ON_COMMAND_RANGE(ID_SEL_TAB_1, ID_SEL_TAB_LAST, OnSelTab)
	ON_COMMAND(ID_ADD_FAVORITE, OnFavoriteAdd)
	ON_COMMAND(ID_ORGANIZE_FAVORITE, OnFavoriteOrganize)
	ON_REGISTERED_MESSAGE(AFX_WM_RESETTOOLBAR, OnToolbarReset)
	ON_WM_SYSCOMMAND()
	ON_COMMAND(ID_INDICATOR_ICON, OnStatusBarDoubleClick)
	ON_COMMAND(ID_INDICATOR_COMPAT, OnStatusBarCompatClick)
	ON_COMMAND(ID_INDICATOR_ZOOM, OnStatusBarZoomClick)
	//ON_COMMAND(WM_CLOSE_DELAY,OnCloseDelay)
	ON_COMMAND(ID_FULL_SCREEN, OnFullScreen)
	ON_COMMAND(ID_W_CLOSE, OnWClose)
	ON_MESSAGE(WM_ADD_FAVORITE, OnFavoriteAddSendMsg)

	ON_COMMAND(ID_PREV_WND, OnPrevWnd)
	ON_COMMAND(ID_NEXT_WND, OnNextWnd)
	ON_COMMAND(ID_TAB_CLOSE_LEFT, OnTabCloseLeft)
	ON_COMMAND(ID_TAB_CLOSE_RIGHT, OnTabCloseRight)

	ON_COMMAND(ID_RESTORE_WND, OnRestoreWnd)
	ON_COMMAND(ID_SAVE_WND, OnSaveWnd)
	ON_COMMAND(ID_OPEN_THIN_FILER, OpenThinFiler)

	ON_COMMAND(IDC_APP_EXIT, OnAppExitEx)
	ON_COMMAND(IDC_APP_EXIT_BUT_THIS, OnAppExitExBT)
	ON_COMMAND(IDC_APP_DELETE_CACHE, &CBrowserFrame::OnAppDeleteCache)

	ON_WM_SIZING()
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_CAPTURECHANGED()
	ON_WM_ENTERSIZEMOVE()
	ON_WM_EXITSIZEMOVE()
	ON_MESSAGE(MYWM_TAB_WINDOW_NOTIFY, OnTabNotify)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_TAB_LIST, OnTabListShow)
	ON_MESSAGE(WM_POWERBROADCAST, OnPowerBroadcast)
	ON_REGISTERED_MESSAGE(AFX_WM_CHANGEVISUALMANAGER, &CBrowserFrame::OnChangeVisualManager)
	ON_WM_WINDOWPOSCHANGED()

	ON_COMMAND(WM_ACTIVE_FRM, OnActiveFrm)
END_MESSAGE_MAP()

CBrowserFrame::CBrowserFrame()
{
	m_bIME = FALSE;
	m_bFullScreen = FALSE;
	m_FullScreen_ShowCommand = SW_NORMAL;
	m_wndTitleBar = NULL;
	m_iStatusIconIndexCache_Zoom = 0;
	m_pwndMenuBar = NULL;
	m_pwndReBar = NULL;
	m_pwndToolBar = NULL;
	m_pwndAddress = NULL;
	m_pwndStatusBar = NULL;
	m_MenuBarCreateFlg = FALSE;
	m_pwndReBarCreateFlg = FALSE;
	m_pwndToolBarCreateFlg = FALSE;
	m_pwndAddressCreateFlg = FALSE;
	m_pwndStatusBarCreateFlg = FALSE;
	m_wndpLogo = NULL;
	m_wndEditSearch = NULL;
	m_bMT_View = FALSE;
	m_bOLEViewer = FALSE;
	m_pCurrentThread = NULL;
	m_pDlgMsgP = NULL;
	m_bDownloadProgress = FALSE;
	m_bDownloadBlankPage = FALSE;
	m_bIsPopupWindow = FALSE;
	m_cTabWnd = NULL;
	m_RendererPID = 0;
	m_nBrowserState = 0;
}

CBrowserFrame::~CBrowserFrame()
{
	if (m_wndTitleBar)
	{
		delete m_wndTitleBar;
		m_wndTitleBar = NULL;
	}

	if (m_pwndMenuBar)
	{
		delete m_pwndMenuBar;
		m_pwndMenuBar = NULL;
	}

	if (m_pwndReBar)
	{
		delete m_pwndReBar;
		m_pwndReBar = NULL;
	}

	if (m_pwndToolBar)
	{
		delete m_pwndToolBar;
		m_pwndToolBar = NULL;
	}

	if (m_pwndAddress)
	{
		delete m_pwndAddress;
		m_pwndAddress = NULL;
	}

	if (m_pwndStatusBar)
	{
		delete m_pwndStatusBar;
		m_pwndStatusBar = NULL;
	}

	if (m_wndpLogo)
	{
		delete m_wndpLogo;
		m_wndpLogo = NULL;
	}
	if (m_wndEditSearch)
	{
		delete m_wndEditSearch;
		m_wndEditSearch = NULL;
	}
	if (m_pDlgMsgP)
	{
		delete m_pDlgMsgP;
		m_pDlgMsgP = NULL;
	}

	if (m_cTabWnd)
	{
		delete m_cTabWnd;
		m_cTabWnd = NULL;
	}
}
BOOL CBrowserFrame::PreTranslateMessage(MSG* pMsg)
{
	if (theApp.m_bTabEnable_Init)
	{
		if (m_cTabWnd)
		{
			if (pMsg->message == WM_MOUSEMOVE)
			{
				CPoint ptw = pMsg->pt;
				//Tabウインドウにマウスカーソルが載っている
				if (pMsg->hwnd == m_cTabWnd->GetHwnd() || pMsg->hwnd == m_cTabWnd->m_hwndTab)
				{
					//閉じるボタンが作成済み且つこのFrameの場合、カーソルの載っているタブの座標を取得
					if (theApp.IsWnd(theApp.m_wndpClose) && theApp.m_wndpClose->m_pParent == this)
					{
						CRect rc(0, 0, 0, 0);
						theApp.m_wndpClose->GetWindowRect(rc);
						int iWidth = 0, iHeight = 0;
						iWidth = rc.Width();
						iHeight = rc.Height();

						TC_HITTESTINFO kChkHit2 = {0};
						kChkHit2.pt = pMsg->pt;
						::ScreenToClient(m_cTabWnd->m_hwndTab, &kChkHit2.pt);
						kChkHit2.flags = TCHT_ONITEM | TCHT_NOWHERE;

						//閉じるボタンが作成済み且つこのFrameの場合、カーソルの載っているタブの座標を取得
						LRESULT iResult2 = ::SendMessage(m_cTabWnd->m_hwndTab, TCM_HITTEST, 0, (LPARAM)&kChkHit2);
						if (iResult2 > -1)
						{
							CRect rcOver(0, 0, 0, 0);
							::SendMessage(m_cTabWnd->m_hwndTab, TCM_GETITEMRECT, iResult2, (LPARAM)&rcOver);
							rcOver.InflateRect(0, -6, -2, 0);
							::ClientToScreen(m_cTabWnd->m_hwndTab, (LPPOINT)&rcOver.left);
							::ClientToScreen(m_cTabWnd->m_hwndTab, (LPPOINT)&rcOver.right);
							ScreenToClient(rcOver);
							rc.left = rcOver.right - iWidth;
							rc.top = rcOver.top;
							rc.right = rc.left + iWidth;
							rc.bottom = rc.top + iHeight;

							HWND hwndUpDown = {0};
							hwndUpDown = ::FindWindowEx(m_cTabWnd->m_hwndTab, NULL, UPDOWN_CLASS, 0); // タブ内の Up-Down コントロール
							if (hwndUpDown != NULL && ::IsWindowVisible(hwndUpDown))
							{
								CRect rcUp;
								::GetWindowRect(hwndUpDown, &rcUp);
								CRect rcHitTest;
								if (rcHitTest.IntersectRect(rcUp, rc))
									theApp.m_wndpClose->ShowWindow(SW_HIDE);
								else
									//閉じるボタンの位置を移動する。
									theApp.m_wndpClose->MoveWnd(rc);
							}
							else
							{
								//閉じるボタンの位置を移動する。
								theApp.m_wndpClose->MoveWnd(rc);
							}
						}
						//マウスカーソルがタブに載っていないので非表示にする。
						else
						{
							if (theApp.IsWnd(theApp.m_wndpClose))
								theApp.m_wndpClose->ShowWindow(SW_HIDE);
						}
					}
					//Tabウインドウにマウスがあるが、閉じるボタンが未生成orこのFramで作られた物ではない。
					else
					{
						//一度も作成されたことが無い
						if (NULL == theApp.m_wndpClose)
						{
							theApp.m_wndpClose = new CCloseNilButton();
							theApp.m_wndpClose->Init(55, this, this);
						}
						//作成されているが、このFrameではない。
						else if (theApp.m_wndpClose && theApp.m_wndpClose->m_pParent != this)
						{
							theApp.m_wndpClose->DestroyWindow();
							delete theApp.m_wndpClose;
							theApp.m_wndpClose = NULL;
							theApp.m_wndpClose = new CCloseNilButton();
							theApp.m_wndpClose->Init(55, this, this);
						}
					}
				}
				//タブ以外にマウスカーソルが移動した場合は、xボタンを非表示にする。
				else
				{
					if (theApp.IsWnd(theApp.m_wndpClose))
						theApp.m_wndpClose->ShowWindow(SW_HIDE);
				}
				////////////////////////////////////////////////////////////////////////////////////////////////
				//選択されているタブに青いラインを表示する
				////////////////////////////////////////////////////////////////////////////////////////////////
				if (::theApp.IsWndVisible(m_cTabWnd->GetHwnd()))
				{
					//一度も作成されたことが無い
					if (NULL == theApp.m_wndpActiveTabLine)
					{
						theApp.m_wndpActiveTabLine = new CActiveTabLine();
						theApp.m_wndpActiveTabLine->Init(56, this, this);
					}
					//作成済みだが、このFrameではない。
					else if (theApp.m_wndpActiveTabLine && theApp.m_wndpActiveTabLine->m_pParent != this)
					{
						theApp.m_wndpActiveTabLine->DestroyWindow();
						delete theApp.m_wndpActiveTabLine;
						theApp.m_wndpActiveTabLine = NULL;
						theApp.m_wndpActiveTabLine = new CActiveTabLine();
						theApp.m_wndpActiveTabLine->Init(56, this, this);
					}
					//タブラインが作成されているandこのFrameの場合
					if (theApp.IsWnd(theApp.m_wndpActiveTabLine) && theApp.m_wndpActiveTabLine->m_pParent == this)
					{
						CRect rc(0, 0, 0, 0);
						theApp.m_wndpActiveTabLine->GetWindowRect(rc);
						int nCurSel = TabCtrl_GetCurSel(m_cTabWnd->m_hwndTab);
						if (nCurSel > -1)
						{
							CRect rcOver(0, 0, 0, 0);
							::SendMessage(m_cTabWnd->m_hwndTab, TCM_GETITEMRECT, nCurSel, (LPARAM)&rcOver);
							//rcOver.InflateRect(0, -2, -2, 0);
							::ClientToScreen(m_cTabWnd->m_hwndTab, (LPPOINT)&rcOver.left);
							::ClientToScreen(m_cTabWnd->m_hwndTab, (LPPOINT)&rcOver.right);
							ScreenToClient(rcOver);
							rc.left = rcOver.left - 1;   // -4;
							rc.top = rcOver.top - 1;     // -3;
							rc.right = rcOver.right + 1; // +4;
							rc.bottom = rcOver.top + 3;  // +3;

							//Viewにフォーカスをセットする。
							if (theApp.m_wndpActiveTabLine->m_Rc != rc)
							{
								if (this->m_wndView)
									this->m_wndView.bSetCefBrowserFocus();
							}

							if (m_nBrowserState & CEF_BIT_IS_LOADING)
							{
								rc.top -= 2;
								theApp.m_wndpActiveTabLine->MoveWnd(rc);
								if (!theApp.m_wndpActiveTabLine->m_bProgress)
								{
									theApp.m_wndpActiveTabLine->m_bProgress = TRUE;
									theApp.m_wndpActiveTabLine->InvalidateRect(NULL);
								}
							}
							else
							{
								theApp.m_wndpActiveTabLine->MoveWnd(rc);
								if (theApp.m_wndpActiveTabLine->m_bProgress)
								{
									theApp.m_wndpActiveTabLine->m_bProgress = FALSE;
									theApp.m_wndpActiveTabLine->InvalidateRect(NULL);
								}
							}
						}
					}
					//////////////////////////////////////////////////////////////////////////////////
					//NewTabButton
					//一度も作成されたことが無い
					if (NULL == theApp.m_wndpNewTab)
					{
						theApp.m_wndpNewTab = new CNewTabButton();
						theApp.m_wndpNewTab->Init(57, this, this);
					}
					//作成済みだが、このFrameではない。
					else if (theApp.m_wndpNewTab && theApp.m_wndpNewTab->m_pParent != this)
					{
						theApp.m_wndpNewTab->DestroyWindow();
						delete theApp.m_wndpNewTab;
						theApp.m_wndpNewTab = NULL;
						theApp.m_wndpNewTab = new CNewTabButton();
						theApp.m_wndpNewTab->Init(57, this, this);
					}
					//作成されているandこのFrameの場合
					if (theApp.IsWnd(theApp.m_wndpNewTab) && theApp.m_wndpNewTab->m_pParent == this)
					{
						CRect rc(0, 0, 0, 0);
						theApp.m_wndpNewTab->GetWindowRect(rc);
						int iWidth = 0, iHeight = 0;
						iWidth = rc.Width();
						iHeight = rc.Height();

						int nCurSel = TabCtrl_GetItemCount(m_cTabWnd->m_hwndTab);
						if (nCurSel > 0)
						{
							CRect rcOver(0, 0, 0, 0);
							::SendMessage(m_cTabWnd->m_hwndTab, TCM_GETITEMRECT, nCurSel - 1, (LPARAM)&rcOver);
							::ClientToScreen(m_cTabWnd->m_hwndTab, (LPPOINT)&rcOver.left);
							::ClientToScreen(m_cTabWnd->m_hwndTab, (LPPOINT)&rcOver.right);
							ScreenToClient(rcOver);
							rc.left = rcOver.right + 3; //- iWidth;
							rc.top = rcOver.top + 2;
							rc.right = rc.left + iWidth;
							rc.bottom = rc.top + iHeight;

							HWND hwndUpDown = {0};
							hwndUpDown = ::FindWindowEx(m_cTabWnd->m_hwndTab, NULL, UPDOWN_CLASS, 0); // タブ内の Up-Down コントロール
							if (hwndUpDown != NULL && ::IsWindowVisible(hwndUpDown))
							{
								CRect rcUp;
								::GetWindowRect(hwndUpDown, &rcUp);
								CRect rcHitTest;
								if (rcHitTest.IntersectRect(rcUp, rc))
									theApp.m_wndpNewTab->ShowWindow(SW_HIDE);
								else
									theApp.m_wndpNewTab->MoveWnd(rc);
							}
							else
							{
								theApp.m_wndpNewTab->MoveWnd(rc);
							}
						}
					}
				}
			}
			else if (pMsg->message == WM_LBUTTONDOWN)
			{
				CPoint ptw = pMsg->pt;
				if (pMsg->hwnd == m_cTabWnd->GetHwnd() || pMsg->hwnd == m_cTabWnd->m_hwndTab)
				{
					if (theApp.IsWnd(theApp.m_wndpClose) && theApp.m_wndpClose->m_pParent == this)
					{
						POINT pt = {0};
						::GetCursorPos(&pt);
						this->ScreenToClient(&pt);
						if (theApp.m_wndpClose->m_Rc.PtInRect(pt))
						{
							TC_HITTESTINFO kChkHit2 = {0};
							kChkHit2.pt = pMsg->pt;
							::ScreenToClient(m_cTabWnd->m_hwndTab, &kChkHit2.pt);
							kChkHit2.flags = TCHT_ONITEM | TCHT_NOWHERE;
							LRESULT iResult2 = ::SendMessage(m_cTabWnd->m_hwndTab, TCM_HITTEST, 0, (LPARAM)&kChkHit2);
							if (iResult2 > -1)
							{
								TCITEM item = {0};
								item.mask = TCIF_PARAM;
								::SendMessage(m_cTabWnd->m_hwndTab, TCM_GETITEM, iResult2, (LPARAM)&item);
								HWND hCloseWnd = NULL;
								hCloseWnd = (HWND)item.lParam;
								if (IsWindow(hCloseWnd))
								{
									::PostMessage(hCloseWnd, WM_COMMAND, ID_W_CLOSE, 0);
									theApp.m_wndpClose->ShowWindow(SW_HIDE);
									return TRUE;
								}
							}
						}
					}
				}
				if (pMsg->hwnd == m_cTabWnd->GetHwnd() || pMsg->hwnd == m_cTabWnd->m_hwndTab)
				{
					if (theApp.IsWnd(theApp.m_wndpNewTab) && theApp.m_wndpNewTab->m_pParent == this)
					{
						POINT pt = {0};
						::GetCursorPos(&pt);
						this->ScreenToClient(&pt);
						if (theApp.m_wndpNewTab->m_Rc.PtInRect(pt))
						{
							TC_HITTESTINFO kChkHit2 = {0};
							kChkHit2.pt = pMsg->pt;
							::ScreenToClient(m_cTabWnd->m_hwndTab, &kChkHit2.pt);
							kChkHit2.flags = TCHT_ONITEM | TCHT_NOWHERE;
							LRESULT iResult2 = ::SendMessage(m_cTabWnd->m_hwndTab, TCM_HITTEST, 0, (LPARAM)&kChkHit2);
							if (iResult2 < 0)
							{
								::PostMessage(this->m_hWnd, WM_COMMAND, ID_NEW_BLANK, 0);
								return TRUE;
							}
						}
					}
				}
				CPoint ptSpinTab = pMsg->pt;
				if (::IsChild(m_cTabWnd->m_hwndTab, pMsg->hwnd))
				{
					HWND hwndUpDown = {0};
					DWORD nScrollPos = {0};
					hwndUpDown = ::FindWindowEx(m_cTabWnd->m_hwndTab, NULL, UPDOWN_CLASS, 0); // タブ内の Up-Down コントロール
					if (hwndUpDown != NULL && ::IsWindowVisible(hwndUpDown))
					{
						nScrollPos = LOWORD(UpDown_GetPos(hwndUpDown));
						// 現在位置 nScrollPos と画面表示とを一致させる
						((CMainFrame*)theApp.m_pMainWnd)->Tab_HScrollSync(this->GetSafeHwnd(), SB_THUMBPOSITION, nScrollPos, NULL);
					}
				}
			}
		}
	}
	return CFrameWndEx::PreTranslateMessage(pMsg);
}
BOOL CBrowserFrame::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnNcCreate(lpCreateStruct) == -1)
		return FALSE;
	return TRUE;
}

int CBrowserFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	PROC_TIME(OnCreate)
	//Tabあり
	if (theApp.m_bTabEnable_Init)
	{
		WINDOWPLACEMENT zFramePracement = {0};
		zFramePracement = theApp.GetActiveFrameWindowPlacement(); //m_ActiveFramePracement;
		if (zFramePracement.rcNormalPosition.bottom > 0)
		{
			CRect rcNormal(zFramePracement.rcNormalPosition);
			if (zFramePracement.showCmd == SW_MAXIMIZE)
			{
				CRect rcDesktop;
				CWnd* pView = NULL;
				HWND hWnd = {0};
				pView = (CWnd*)theApp.GetActiveViewPtr();
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

				lpCreateStruct->x = rcDesktop.left;
				lpCreateStruct->y = rcDesktop.top;
				lpCreateStruct->cx = rcDesktop.Width();
				lpCreateStruct->cy = rcDesktop.Height();
			}
			else if (zFramePracement.showCmd == SW_NORMAL)
			{
				lpCreateStruct->x = rcNormal.left;
				lpCreateStruct->y = rcNormal.top;
				lpCreateStruct->cx = rcNormal.Width();
				lpCreateStruct->cy = rcNormal.Height();
			}
		}
	}

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CFrameImpl::AddFrame(this);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAppendMenu;
		pSysMenu->AppendMenu(MF_SEPARATOR);
		strAppendMenu.Format(_T("About %s ..."), (LPCTSTR)theApp.m_strThisAppName);
		pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAppendMenu);

		if (theApp.InVirtualEnvironment() != VE_NA && theApp.IsSGMode())
		{
			strAppendMenu = _T("Open Task Manager");
			pSysMenu->AppendMenu(MF_STRING, IDM_TASK_MGR, strAppendMenu);
		}

		strAppendMenu = _T("Close All Window");
		pSysMenu->AppendMenu(MF_STRING, IDM_APP_EXIT, strAppendMenu);
	}
	this->SetWindowText(theApp.m_strThisAppName);
	this->CreateView();

	return 0;
}
void CBrowserFrame::CreateView()
{
	PROC_TIME(CreateView)
	CRect rect;
	GetClientRect(rect);
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
			      rect, this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return;
	}

	//popup
	if (m_bIsPopupWindow)
	{
		//Addressbarのみ必要 Chromium準拠
		CreateRebarsPopup();
		//Statusbarは不要
		return;
	}
	//normal
	else
	{
		if (theApp.m_bTabEnable_Init)
		{
			if (m_bOLEViewer)
				return;
			if (!m_cTabWnd)
				m_cTabWnd = new CTabWnd;
			m_cTabWnd->CreateTab(theApp.m_hInstance, this, &m_wndView, m_wndView.GetSafeHwnd());

			HWND my_hwnd = this->m_hWnd;
			BOOL disable = TRUE;
			DwmSetWindowAttribute(my_hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &disable, sizeof disable);

			m_FaviconCB.SetFramePtr(this);
		}
	}
	CreateRebars();
	CreateStatusbar();
}
void CBrowserFrame::CreateRebarsPopup()
{
	PROC_TIME(CreateRebarsPopup)
	CMenu* pMenu = NULL;
	pMenu = this->GetMenu();
	if (pMenu)
		pMenu->DestroyMenu();
	SetMenu(NULL);

	PROC_TIME_S(CreateRebars_ADDRESSBAR)
	if (theApp.m_AppSettings.IsRebar())
	{
		if (m_pwndAddress == NULL)
			m_pwndAddress = new CMyComboBoxEx;
	}
	if (m_pwndAddress)
	{
		if (!m_pwndAddressCreateFlg)
		{
			// create a combo box for the address bar
			if (!m_pwndAddress->Create(CBS_DROPDOWN | WS_CHILD | CBS_AUTOHSCROLL, CRect(0, 0, 200, 120), this, AFX_IDW_TOOLBAR + 3))
			{
				return; // fail to create
			}
			m_pwndAddressCreateFlg = TRUE;
			m_pwndAddress->SetImageList(&theApp.m_imgMenuIcons);
			if (m_pwndAddress)
			{
				COMBOBOXEXITEM cbi = {0};
				cbi.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
				cbi.iItem = -1;
				cbi.iImage = (int)SBUtil::ICON_INDEX::ICON_IE_FILE_FAV;
				cbi.iSelectedImage = (int)SBUtil::ICON_INDEX::ICON_IE_FILE_FAV;
				m_pwndAddress->SetItem(&cbi);
			}
		}
	}
	PROC_TIME_E(CreateRebars_ADDRESSBAR)

	//--------------
	// Create rebar:
	//--------------
	PROC_TIME_S(CreateRebars_REBAR)
	CString strAddressText;
	strAddressText.LoadString(IDS_STRING_FRM1);

	if (theApp.m_AppSettings.IsRebar())
	{
		if (m_pwndReBar == NULL)
		{
			PROC_TIME_S(CreateRebars_REBAR_NEW)
			m_pwndReBar = new CMyReBar;
			m_pwndReBar->m_bMultiThreaded = TRUE;
			PROC_TIME_E(CreateRebars_REBAR_NEW)
		}
	}
	if (m_pwndReBar)
	{
		if (!m_pwndReBarCreateFlg)
		{
			PROC_TIME_S(CreateRebars_REBAR_Create)
			if (!m_pwndReBar->Create(this, 0))
			{
				TRACE0("Failed to create rebar\n");
				return; // fail to create
			}
			m_pwndReBarCreateFlg = TRUE;
			PROC_TIME_E(CreateRebars_REBAR_Create)

			PROC_TIME_S(CreateRebars_REBAR_AddBar_Address)
			if (m_pwndAddress)
				m_pwndReBar->AddBar(m_pwndAddress, strAddressText, NULL,
						    RBBS_NOGRIPPER | RBBS_BREAK);
			PROC_TIME_E(CreateRebars_REBAR_AddBar_Address)
			EnableDocking(CBRS_ALIGN_TOP);
			m_pwndReBar->EnableDocking(CBRS_TOP);
			DockPane(m_pwndReBar);
		}
	}
	PROC_TIME_E(CreateRebars_REBAR)
}

void CBrowserFrame::CreateRebars()
{
	PROC_TIME(CreateRebars)

	PROC_TIME_S(CreateRebars_MENUBAR)
	if (m_pwndMenuBar == NULL)
		m_pwndMenuBar = new CMyMenuBar;
	if (m_pwndMenuBar)
	{
		if (!m_MenuBarCreateFlg)
		{
			if (!m_pwndMenuBar->Create(this))
			{
				TRACE0("Failed to create menubar\n");
				return; // fail to create
			}
			m_MenuBarCreateFlg = TRUE;
			if (theApp.m_AppSettings.IsRebar())
			{
				m_pwndMenuBar->SetPaneStyle(m_pwndMenuBar->GetPaneStyle() | CBRS_SIZE_DYNAMIC);
				//---------------------------------
				// Set toolbar and menu image size:
				//---------------------------------
				m_pwndMenuBar->SetSizes(CSize(36, 30), CSize(23, 23));
				m_pwndMenuBar->SetMenuSizes(CSize(22, 22), CSize(16, 16));

				//------------------------------------
				// Remove menubar gripper and borders:
				//------------------------------------
				m_pwndMenuBar->SetPaneStyle(m_pwndMenuBar->GetPaneStyle() &
							    ~(CBRS_GRIPPER | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
			}
		}
		if (theApp.m_AppSettings.IsRebar())
			;
		else
			m_pwndMenuBar->ShowPane(FALSE, 0, FALSE);
	}
	PROC_TIME_E(CreateRebars_MENUBAR)

	PROC_TIME_S(CreateRebars_TOOLBAR)
	double dSizeX = 0.0;
	double dSizeY = 0.0;
	int iSizeX = 0;
	int iSizeY = 0;
	dSizeX = 28 * theApp.m_ScaleDPI;
	dSizeY = 28 * theApp.m_ScaleDPI;
	iSizeX = (int)dSizeX;
	iSizeY = (int)dSizeY;

	CClientDC dc(this);
	BOOL bIsHighColor = dc.GetDeviceCaps(BITSPIXEL) > 8;

	UINT uiToolbarHotID = bIsHighColor ? IDB_HOTTOOLBAR : 0;
	UINT uiToolbarColdID = bIsHighColor ? IDB_COLDTOOLBAR : 0;
	UINT uiMenuID = 0;

	if (theApp.m_AppSettings.IsRebar())
	{
		if (m_pwndToolBar == NULL)
		{
			PROC_TIME_S(CreateRebars_TOOLBAR_NEW)
			m_pwndToolBar = new CMyToolBar;
			PROC_TIME_E(CreateRebars_TOOLBAR_NEW)
		}
	}
	if (m_pwndToolBar)
	{
		if (!m_pwndToolBarCreateFlg)
		{
			PROC_TIME_S(CreateRebars_TOOLBAR_Create)
			DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP | CBRS_SIZE_FIXED | CBRS_HIDE_INPLACE;
			if (!m_pwndToolBar->Create(this, dwStyle, AFX_IDW_TOOLBAR))
			{
				TRACE0("Failed to create toolbar\n");
				return; // fail to create
			}
			PROC_TIME_E(CreateRebars_TOOLBAR_Create)

			/////////////////////////////////////////////////////////////////////
			PROC_TIME_S(CreateRebars_TOOLBAR_LoadToolBar)
			PROC_TIME_S(CreateRebars_TOOLBAR_LoadToolBar_STAGE1)
			if (!m_pwndToolBar->LoadToolBar(IDR_MAINFRAME, uiToolbarColdID, uiMenuID, FALSE /* Not locked */, 0, 0, uiToolbarHotID))
			{
				TRACE0("Failed to create toolbar\n");
				return; // fail to create
			}
			if (theApp.m_ScaleDPI > 1)
				m_pwndToolBar->SetSizes(CSize(iSizeX, iSizeY), CSize(23, 23));
			m_pwndToolBar->m_bDontScaleImages = TRUE;
			m_pwndToolBar->m_bMultiThreaded = TRUE;
			PROC_TIME_E(CreateRebars_TOOLBAR_LoadToolBar_STAGE1)

			PROC_TIME_S(CreateRebars_TOOLBAR_LoadToolBar_STAGE2)
			int iSearchIndex = 0;
			iSearchIndex = m_pwndToolBar->GetCount();
			iSearchIndex -= 1;
			if (iSearchIndex > 0)
			{
				// セパレータの幅を設定
				m_pwndToolBar->SetButtonInfo(iSearchIndex, IDC_EDIT_SEARCH, TBBS_SEPARATOR, 500);
				PROC_TIME_E(CreateRebars_TOOLBAR_LoadToolBar_STAGE2)

				//ツールバーのセパレータ上にエディットボックスを載せる
				PROC_TIME_S(CreateRebars_TOOLBAR_LoadToolBar_STAGE3)
				CRect rect;
				m_pwndToolBar->GetItemRect(iSearchIndex, &rect);
				rect.DeflateRect(1, 2);
				if (!m_wndEditSearch)
					m_wndEditSearch = new CIconEdit;
				m_wndEditSearch->m_pParentWnd = this;
				if (!m_wndEditSearch->CreateEx(
					WS_EX_CLIENTEDGE, _T("EDIT"), _T(""),
					WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | ES_MULTILINE,
					rect,
					m_pwndToolBar,
					IDC_EDIT_SEARCH))
				{
					TRACE0("Failed to create edit-box\n");
					return;
				}
				m_wndEditSearch->SetIcon(theApp.m_imgMenuIcons.ExtractIcon((int)SBUtil::ICON_INDEX::ICON_FIND_PAGE));
			}

			PROC_TIME_E(CreateRebars_TOOLBAR_LoadToolBar_STAGE3)

			PROC_TIME_E(CreateRebars_TOOLBAR_LoadToolBar)

			m_pwndToolBarCreateFlg = TRUE;
			m_pwndToolBar->SetWindowText(_T("Standard"));
			m_pwndToolBar->SetBorders();

			//------------------------------------
			// Remove toolbar gripper and borders:
			//------------------------------------
			m_pwndToolBar->SetPaneStyle(m_pwndToolBar->GetPaneStyle() &
						    ~(CBRS_GRIPPER | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
		}
	}
	PROC_TIME_E(CreateRebars_TOOLBAR)

	PROC_TIME_S(CreateRebars_ADDRESSBAR)
	if (theApp.m_AppSettings.IsRebar())
	{
		if (m_pwndAddress == NULL)
			m_pwndAddress = new CMyComboBoxEx;
	}
	if (m_pwndAddress)
	{
		if (!m_pwndAddressCreateFlg)
		{
			// create a combo box for the address bar
			if (!m_pwndAddress->Create(CBS_DROPDOWN | WS_CHILD | CBS_AUTOHSCROLL, CRect(0, 0, 200, 120), this, AFX_IDW_TOOLBAR + 3))
			{
				return; // fail to create
			}
			m_pwndAddressCreateFlg = TRUE;
			m_pwndAddress->SetImageList(&theApp.m_imgMenuIcons);
			if (m_pwndAddress)
			{
				COMBOBOXEXITEM cbi = {0};
				cbi.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
				cbi.iItem = -1;
				cbi.iImage = (int)SBUtil::ICON_INDEX::ICON_IE_FILE_FAV;
				cbi.iSelectedImage = (int)SBUtil::ICON_INDEX::ICON_IE_FILE_FAV;
				m_pwndAddress->SetItem(&cbi);
			}
		}
	}
	PROC_TIME_E(CreateRebars_ADDRESSBAR)

	//--------------
	// Create rebar:
	//--------------
	PROC_TIME_S(CreateRebars_REBAR)
	CString strAddressText;
	strAddressText.LoadString(IDS_STRING_FRM1);

	if (theApp.m_AppSettings.IsRebar())
	{
		if (m_pwndReBar == NULL)
		{
			PROC_TIME_S(CreateRebars_REBAR_NEW)
			m_pwndReBar = new CMyReBar;
			m_pwndReBar->m_bMultiThreaded = TRUE;
			PROC_TIME_E(CreateRebars_REBAR_NEW)
		}
	}
	if (m_pwndReBar)
	{
		if (!m_pwndReBarCreateFlg)
		{
			PROC_TIME_S(CreateRebars_REBAR_Create)
			if (!m_pwndReBar->Create(this, 0))
			{
				TRACE0("Failed to create rebar\n");
				return; // fail to create
			}
			m_pwndReBarCreateFlg = TRUE;
			PROC_TIME_E(CreateRebars_REBAR_Create)

			PROC_TIME_S(CreateRebars_REBAR_AddBar_MENU)
			if (m_pwndMenuBar)
			{
				m_pwndMenuBar->ShowPane(TRUE, 0, FALSE);
				m_pwndReBar->AddBar(m_pwndMenuBar, NULL, NULL,
						    RBBS_NOGRIPPER | RBBS_BREAK);
			}
			PROC_TIME_E(CreateRebars_REBAR_AddBar_MENU)

			PROC_TIME_S(CreateRebars_REBAR_AddBar_ToolBar)
			if (m_pwndToolBar)
			{
				m_pwndReBar->AddBar(m_pwndToolBar, NULL, NULL,
						    RBBS_NOGRIPPER | RBBS_BREAK);
			}
			PROC_TIME_E(CreateRebars_REBAR_AddBar_ToolBar)

			PROC_TIME_S(CreateRebars_REBAR_AddBar_Address)
			if (m_pwndAddress)
				m_pwndReBar->AddBar(m_pwndAddress, strAddressText, NULL,
						    RBBS_NOGRIPPER | RBBS_BREAK);
			PROC_TIME_E(CreateRebars_REBAR_AddBar_Address)

			if (m_pwndToolBar)
			{
				EnableDocking(CBRS_ALIGN_TOP);
				m_pwndReBar->EnableDocking(CBRS_TOP);
				DockPane(m_pwndReBar);

				if (theApp.m_AppSettings.IsShowLogo())
				{
					m_wndpLogo = new CNilButton;
					if (!m_wndpLogo->Init(53, m_pwndToolBar, this))
					{
						if (m_wndpLogo)
						{
							delete m_wndpLogo;
							m_wndpLogo = NULL;
						}
					}
				}
			}
			if (theApp.m_bTabEnable_Init)
			{
				if (!m_pwndReBar)
				{
					if (!theApp.m_AppSettings.IsRebar())
					{
						if (m_pwndReBar == NULL)
						{
							m_pwndReBar = new CMyReBar;
							m_pwndReBar->m_bMultiThreaded = TRUE;
							if (!m_pwndReBar->Create(this))
							{
								TRACE0("Failed to create rebar\n");
								return; // fail to create
							}
							m_pwndReBarCreateFlg = TRUE;
							EnableDocking(CBRS_ALIGN_TOP);
							m_pwndReBar->EnableDocking(CBRS_TOP);
							DockPane(m_pwndReBar);
						}
					}
				}
				if (m_cTabWnd)
				{
					m_pwndReBar->AddBar(CWnd::FromHandle(m_cTabWnd->GetHwnd()), NULL, NULL,
							    RBBS_NOGRIPPER | RBBS_BREAK);
				}
			}
		}
	}
	PROC_TIME_E(CreateRebars_REBAR)
}
void CBrowserFrame::CreateStatusbar()
{
	PROC_TIME(CreateStatusbar)

	//-------------------
	// Create status bar:
	//-------------------
	if (theApp.m_AppSettings.IsStatusbar())
	{
		if (m_pwndStatusBar == NULL)
			m_pwndStatusBar = new CMyStatusBar;
	}

	if (!m_pwndStatusBar)
		return;

	if (m_pwndStatusBarCreateFlg)
		return;
	else
	{
		if (!m_pwndStatusBar->Create(this))
		{
			TRACE0("Failed to create status bar\n");
			return; // fail to create
		}
		m_pwndStatusBarCreateFlg = TRUE;
	}

	if (!m_pwndStatusBar->SetIndicators(indicators,
					    sizeof(indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return; // fail to create
	}

	//------------------------------
	// First pane - image/animation:
	//------------------------------
	m_pwndStatusBar->SetPaneIcon(nStatusIcon, NULL, FALSE, FALSE);
	m_pwndStatusBar->SetPaneStyle(nStatusIcon, SBPS_NOBORDERS);
	m_pwndStatusBar->SetPaneText(nStatusIcon, NULL);
	m_pwndStatusBar->SetPaneWidth(nStatusIcon, 0);
	m_pwndStatusBar->SetPaneStyle(nStatusInfo, SBPS_STRETCH | SBPS_NOBORDERS);
	m_pwndStatusBar->SetPaneStyle(nStatusSSL, SBPS_STRETCH | SBPS_NOBORDERS);
	m_pwndStatusBar->SetPaneIcon(nStatusSSL, NULL, FALSE, FALSE);
	m_pwndStatusBar->SetPaneText(nStatusSSL, NULL);
	m_pwndStatusBar->SetPaneWidth(nStatusSSL, 0);
	m_pwndStatusBar->SetPaneStyle(nStatusCompat, SBPS_STRETCH | SBPS_NOBORDERS);
	m_pwndStatusBar->SetPaneIcon(nStatusCompat, NULL, FALSE, FALSE);
	m_pwndStatusBar->SetPaneText(nStatusCompat, NULL);
	m_pwndStatusBar->SetPaneWidth(nStatusCompat, 20);
	m_pwndStatusBar->SetPaneStyle(nStatusZoom, SBPS_STRETCH | SBPS_NOBORDERS);
	m_pwndStatusBar->SetPaneText(nStatusZoom, NULL);
	m_pwndStatusBar->SetPaneWidth(nStatusZoom, 70);

	CIconHelper ICoHelper;
	ICoHelper = theApp.m_imgMenuIcons.ExtractIcon((int)SBUtil::ICON_INDEX::ICON_ZOOM);
	m_iStatusIconIndexCache_Zoom = (int)SBUtil::ICON_INDEX::ICON_ZOOM;
	m_pwndStatusBar->SetPaneIcon(nStatusZoom, ICoHelper, FALSE);
	m_pwndStatusBar->SetPaneWidth(nStatusProgress, 160);
	m_pwndStatusBar->EnablePaneDoubleClick(TRUE);
}

BOOL CBrowserFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	m_dockManager.Create(this);
	m_Impl.SetDockingManager(&m_dockManager);

	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
	CIconHelper ICoHelper;
	CCursorHelper ICurHelper;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;

	WNDCLASS wc = {0};

	wc.lpfnWndProc = AfxWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = theApp.m_hInstance;
	ICoHelper = theApp.LoadIcon(IDR_MAINFRAME);
	wc.hIcon = ICoHelper;

	ICurHelper = theApp.LoadStandardCursor(IDC_ARROW);
	wc.hCursor = ICurHelper;

	wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wc.hbrBackground = 0;
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINFRAME);

	wc.lpszClassName = _T("CSGFrame");

	cs.style |= WS_CLIPCHILDREN;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	cs.style &= ~CBRS_ALIGN_BOTTOM;
	cs.style &= ~CBRS_ALIGN_RIGHT;

	AfxRegisterClass(&wc);

	cs.lpszClass = _T("CSGFrame");
	//Tabあり
	if (theApp.m_bTabEnable_Init)
	{
		WINDOWPLACEMENT zFramePracement = {0};
		zFramePracement = theApp.GetActiveFrameWindowPlacement(); //m_ActiveFramePracement;
		if (zFramePracement.rcNormalPosition.bottom > 0)
		{
			CRect rcNormal(zFramePracement.rcNormalPosition);
			if (zFramePracement.showCmd == SW_MAXIMIZE)
			{
				CRect rcDesktop;
				CWnd* pView = NULL;
				HWND hWnd = {0};
				pView = (CWnd*)theApp.GetActiveViewPtr();
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
				cs.x = rcDesktop.left;
				cs.y = rcDesktop.top;
				cs.cx = rcDesktop.Width();
				cs.cy = rcDesktop.Height();
			}
			else if (zFramePracement.showCmd == SW_NORMAL)
			{
				cs.x = rcNormal.left;
				cs.y = rcNormal.top;
				cs.cx = rcNormal.Width();
				cs.cy = rcNormal.Height();
			}
		}
	}
	return TRUE;
}
LRESULT CBrowserFrame::OnSearchString(WPARAM wParam, LPARAM lParam)
{
	CString str;
	if (lParam)
	{
		str = (TCHAR*)lParam;
		if (!str.IsEmpty())
		{
			Search(str, wParam == 0 ? TRUE : FALSE);
			return 0;
		}
	}
	if (str.IsEmpty())
	{
		if (theApp.IsWnd(m_pwndAddress))
		{
			m_pwndAddress->GetWindowText(str);
		}
		if (!str.IsEmpty())
			Search(str, FALSE);
	}
	return 0;
}

void CBrowserFrame::Search(CString strIn, BOOL bNewWindow)
{
	PROC_TIME(Search)

	strIn.TrimLeft();
	strIn.TrimRight();
	if (strIn.IsEmpty()) return;

	CString strFindURL(_T("https://www.google.co.jp/search?hl=ja&q={searchTerms}"));
	CString strEncU;
	strEncU = theApp.ConvertStr2UTF8UrlEncode(strIn);
	strFindURL.Replace(_T("{searchTerms}"), strEncU);
	if (bNewWindow)
	{
		if (this->m_wndView)
			m_wndView.CreateNewBrowserWindow(strFindURL);
	}
	else
	{
		if (this->m_wndView)
			m_wndView.Navigate(strFindURL);
	}
	return;
}
void CBrowserFrame::SearchAndNavigate(CString strIn)
{
	strIn.TrimLeft();
	strIn.TrimRight();
	if (strIn.IsEmpty()) return;

	if (SBUtil::IsURL(strIn) ||
	    strIn.Find(_T(":")) == 1 ||
	    strIn.Find(_T("about:")) == 0 ||
	    strIn.Find(_T("chrome:")) == 0)
	{
		if (this->m_wndView)
			m_wndView.Navigate(strIn);
	}
	//検索プロバイダー
	else
	{
		this->Search(strIn, FALSE);
	}
}
LRESULT CBrowserFrame::OnNewAddressEnter(WPARAM wParam, LPARAM lParam)
{
	CString str;
	if (theApp.IsWnd(m_pwndAddress))
	{
		m_pwndAddress->GetWindowText(str);
		SearchAndNavigate(str);
		m_pwndAddress->AppendString(str.GetBuffer(0));
		CString logmsg;
		logmsg.Format(_T("BF_WND:0x%08p OnNewAddressEnter:%s"), theApp.SafeWnd(this->m_hWnd), (LPCTSTR)str);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	}
	return 0;
}
LRESULT CBrowserFrame::OnNewAddress(WPARAM wParam, LPARAM lParam)
{
	CMyComboBoxEx* ExP = (CMyComboBoxEx*)lParam;
	if (ExP == NULL) return 0;
	CString str;
	if (theApp.IsWnd(m_pwndAddress))
	{
		if (ExP == m_pwndAddress)
		{
			CString logmsg;
			logmsg.Format(_T("BF_WND:0x%08p OnNewAddress:%s"), theApp.SafeWnd(this->m_hWnd), (LPCTSTR)str);
			theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
			m_pwndAddress->GetLBText(m_pwndAddress->GetCurSel(), str);
			SearchAndNavigate(str);
		}
	}
	return 0;
}

#ifdef _DEBUG
void CBrowserFrame::AssertValid() const
{
	CFrameWndBase::AssertValid();
}

void CBrowserFrame::Dump(CDumpContext& dc) const
{
	CFrameWndBase::Dump(dc);
}

#endif //_DEBUG

void CBrowserFrame::OnSetFocus(CWnd* pOldWnd)
{
	CFrameWndBase::OnSetFocus(pOldWnd);
}
void CBrowserFrame::OnKillFocus(CWnd* pNewWnd)
{
	CFrameWndBase::OnKillFocus(pNewWnd);
}

BOOL CBrowserFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (this->m_wndView)
	{
		if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}
	return CFrameWndBase::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CBrowserFrame::OnClose()
{
	try
	{
		if (theApp.m_pMainWnd)
		{
			if (::IsWindow(theApp.m_pMainWnd->m_hWnd))
			{
				theApp.m_pMainWnd->SendMessage(WM_DELETE_WINDOW_LIST, 0, (INT_PTR)this);
				if (m_cTabWnd)
				{
					CString strAllCloseFlg;
					((CMainFrame*)theApp.m_pMainWnd)->GetWindowText(strAllCloseFlg);
					if (strAllCloseFlg == theApp.m_strThisAppName)
					{
						CFrameWndBase::OnClose();
						return;
					}

					if (theApp.m_bTabEnable_Init)
					{
						//連続して閉じる場合、自信がアクティブでない場合は、不要
						if (theApp.GetActiveBFramePtr() == this)
						{
							CBrowserFrame* pWnd = NULL;
							pWnd = theApp.GetNextGenerationActiveWindow(this);
							if (theApp.IsWnd(pWnd))
							{
								((CMainFrame*)theApp.m_pMainWnd)->TabWindowMsgBSend(TWNT_DEL, pWnd->GetSafeHwnd());
							}
						}
					}
					((CMainFrame*)theApp.m_pMainWnd)->Delete_TabWindow(this);
					((CMainFrame*)theApp.m_pMainWnd)->TabWindowMsgBSend(TWNT_REFRESH, NULL);

					delete m_cTabWnd;
					m_cTabWnd = NULL;
				}
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	CFrameWndBase::OnClose();
}
void CBrowserFrame::SetWindowTitle(LPCTSTR ptr)
{
	this->SetWindowText(ptr);
	if (m_wndTitleBar)
	{
		m_wndTitleBar->m_strText = ptr;
	}
	if (m_cTabWnd)
	{
		((CMainFrame*)theApp.m_pMainWnd)->TabWindowMsgBSend(TWNT_REFRESH, NULL);
	}
}
void CBrowserFrame::OnActiveFrm()
{
	if (theApp.IsWnd(this))
	{
		SBUtil::SetAbsoluteForegroundWindow(this->m_hWnd, FALSE);
		if (m_wndView)
			m_wndView.SetFocusApp();
	}
}
void CBrowserFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	if (nState == WA_ACTIVE || nState == WA_CLICKACTIVE)
	{
		if (this->m_wndView)
		{
			m_wndView.m_dbZoomSize = m_wndView.GetZoomSizeEx();
			m_wndView.ZoomTo(m_wndView.m_dbZoomSize);
			m_wndView.SetFocusApp();
		}
		if (m_cTabWnd)
		{
			if (((CMainFrame*)theApp.m_pMainWnd)->SetActiveFramePtr(this))
			{
				((CMainFrame*)theApp.m_pMainWnd)->TabWindowMsgBSend(TWNT_ORDER, this->GetSafeHwnd());
			}
		}
		if (::IsWindow(theApp.m_hwndTaskDlg) && ::IsWindowEnabled(theApp.m_hwndTaskDlg))
		{
			if (::IsIconic(theApp.m_hwndTaskDlg))
			{
				::ShowWindow(theApp.m_hwndTaskDlg, SW_HIDE);
				::ShowWindow(theApp.m_hwndTaskDlg, SW_NORMAL);
			}
			::SetWindowPos(theApp.m_hwndTaskDlg, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		}
	}
	else if (nState == WA_INACTIVE)
	{
		if (pWndOther)
		{
			if (this->m_wndView)
				m_wndView.EnableWinEx(FALSE);
		}
	}

	CFrameWndBase::OnActivate(nState, pWndOther, bMinimized);
}

BOOL CBrowserFrame::OnNcActivate(BOOL bActive)
{
	return CFrameWndBase::OnNcActivate(bActive);
}

BOOL CBrowserFrame::OnSetUrlString(LPCTSTR lParam)
{
	BOOL bSSL = FALSE;
	BOOL bRet = FALSE;
	CString strURL;
	strURL = (LPCTSTR)lParam;
	CString logmsg;
	DebugWndLogData dwLogData;
	dwLogData.mHWND.Format(_T("BF_WND:0x%08p"), theApp.SafeWnd(this->m_hWnd));
	dwLogData.mFUNCTION_NAME = _T("OnSetUrlString");
	dwLogData.mMESSAGE1 = strURL;
	theApp.AppendDebugViewLog(dwLogData);
	logmsg = dwLogData.GetString();
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_URL);

	if (!strURL.IsEmpty())
	{
		if (!SBUtil::IsScript(strURL))
		{
			if (SBUtil::IsURL_HTTPS(strURL))
			{
				bSSL = TRUE;
			}
			if (theApp.IsWnd(m_pwndAddress))
			{
				HWND hFocus = NULL;
				hFocus = ::GetFocus();
				if (hFocus == m_pwndAddress->m_hWnd ||
				    hFocus == m_pwndAddress->m_Edit.m_hWnd)
				{
					;
				}
				else
				{
					m_pwndAddress->SetWindowText(strURL);
					bRet = TRUE;
				}
			}
		}
	}
	SetSSLStatus(bSSL);
	return bRet;
}

void CBrowserFrame::OnSetAddressbar()
{
	if (theApp.IsWnd(m_pwndAddress))
		m_pwndAddress->SetFocus();
}
void CBrowserFrame::OnSetSearchbar()
{
	if (theApp.IsWnd(m_wndEditSearch))
		m_wndEditSearch->SetFocus();
}
void CBrowserFrame::OnSelTab(UINT nID)
{
	int itemp = nID - ID_SEL_TAB_1;
	if (itemp < 0)
		itemp = 0;
	if (theApp.m_bTabEnable_Init)
	{
		CStringArray strATitle;
		((CMainFrame*)theApp.m_pMainWnd)->Get_TabWindowInfoArray(strATitle, m_ptrAWnd);
		int iMax = (int)m_ptrAWnd.GetCount();
		if (nID == ID_SEL_TAB_LAST)
			itemp = iMax - 1;
		if (iMax > itemp)
		{
			HWND hWnd = {0};
			hWnd = (HWND)m_ptrAWnd.GetAt(itemp);
			if (hWnd)
			{
				if (theApp.SafeWnd(hWnd))
				{
					if (::IsIconic(hWnd))
					{
						::ShowWindow(hWnd, SW_HIDE);
						::ShowWindow(hWnd, SW_NORMAL);
					}
					::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
				}
			}
		}
		return;
	}
}
void CBrowserFrame::OnCloseWndMenu(UINT nID)
{
	int itemp = nID - ID_CLOSE_WINDOW_START;
	if (itemp < 0)
		itemp = 0;

	if (theApp.m_listCloseWindowURL.GetCount() > itemp)
	{
		CString strURL;
		int iCnt = 0;
		POSITION pos1 = {0};
		//既に登録されているかチェック
		for (pos1 = theApp.m_listCloseWindowURL.GetHeadPosition(); pos1 != NULL; theApp.m_listCloseWindowURL.GetNext(pos1))
		{
			strURL.Empty();
			if (iCnt == itemp)
			{
				strURL = theApp.m_listCloseWindowURL.GetAt(pos1);
				break;
			}
			iCnt++;
		}
		if (!strURL.IsEmpty())
		{
			((CMainFrame*)theApp.m_pMainWnd)->CreateNewWindow(strURL);
		}
	}
}
void CBrowserFrame::OnWndMenu(UINT nID)
{
	int itemp = nID - ID_WINDOW_START;
	if (itemp < 0)
		itemp = 0;
	if (theApp.m_bTabEnable_Init)
	{
		if (m_ptrAWnd.GetCount() > itemp)
		{
			HWND hWnd = {0};
			hWnd = (HWND)m_ptrAWnd.GetAt(itemp);
			if (hWnd)
			{
				if (theApp.SafeWnd(hWnd))
				{
					if (::IsIconic(hWnd))
					{
						::ShowWindow(hWnd, SW_HIDE);
						::ShowWindow(hWnd, SW_NORMAL);
					}
					::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
				}
			}
		}
		return;
	}
	if (m_WindwMgrClass.m_ArrayWindowHWND.GetCount() > itemp)
	{
		HWND hWnd = {0};
		hWnd = (HWND)m_WindwMgrClass.m_ArrayWindowHWND.GetAt(itemp);
		if (hWnd)
		{
			if (theApp.SafeWnd(hWnd))
			{
				if (::IsIconic(hWnd))
				{
					::ShowWindow(hWnd, SW_HIDE);
					::ShowWindow(hWnd, SW_NORMAL);
				}
				::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			}
		}
	}
}
void CBrowserFrame::OnFavMenu(UINT nID)
{
	int itemp = nID - ID_FAV_START;
	if (itemp < 0)
		itemp = 0;

	CString strURL;
	CFavoriteItem* tmp1 = NULL;
	tmp1 = theApp.m_FavMng.GetItemData(itemp);
	if (!tmp1)
		return;

	if (tmp1->GetType() == IEFavURL)
	{
		strURL = tmp1->strURL;

		if (strURL.IsEmpty())
			return;
		if (this->m_wndView)
			m_wndView.Navigate(strURL);
	}
}

void CBrowserFrame::CreateWindowMenuSync(CMenu* pMenu)
{
	if (theApp.m_bTabEnable_Init)
	{
		CString strTitle;
		CStringArray strATitle;
		((CMainFrame*)theApp.m_pMainWnd)->Get_TabWindowInfoArray(strATitle, m_ptrAWnd);
		size_t iSize = 0;
		iSize = strATitle.GetCount();
		for (size_t i = 0; i < iSize; i++)
		{
			strTitle = strATitle.GetAt(i);
			strTitle.TrimLeft();
			strTitle.TrimRight();
			// 文字のカット
			CString str;
			SBUtil::GetDivChar(strTitle, 160, str);
			strTitle = str;
			strTitle.Replace(_T("&"), _T("&&"));
			if (m_ptrAWnd.GetAt(i) == this->GetSafeHwnd())
			{
				pMenu->AppendMenu(MF_BYPOSITION | MF_CHECKED | MF_STRING | MF_ENABLED, ID_WINDOW_START + i, strTitle);
			}
			else
				pMenu->AppendMenu(MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_WINDOW_START + i, strTitle);
		}
	}
	else
	{
		((CMainFrame*)theApp.m_pMainWnd)->m_MultiProcessHWND.RemoveAll();
		((CMainFrame*)theApp.m_pMainWnd)->GetMultiProcessWnd(::GetDesktopWindow());

		CString strTitle;
		m_WindwMgrClass.GetWindowList();
		size_t iSize = 0;
		iSize = m_WindwMgrClass.m_ArrayWindowHWND.GetCount();
		for (size_t i = 0; i < iSize; i++)
		{
			strTitle = m_WindwMgrClass.m_ArrayWindowTitle.GetAt(i);
			strTitle.TrimLeft();
			strTitle.TrimRight();
			CString strMag(_T("　　　　"));
			if (strTitle.Find(strMag) == strTitle.GetLength() - strMag.GetLength())
			{
				CString str1;
				SBUtil::GetDivChar(strTitle, strTitle.GetLength() - strMag.GetLength(), str1, FALSE);
				strTitle = str1;
			}

			// 文字のカット
			CString str;
			SBUtil::GetDivChar(strTitle, 160, str);
			strTitle = str;
			strTitle.Replace(_T("&"), _T("&&"));
			if (i == 0)
			{
				pMenu->AppendMenu(MF_BYPOSITION | MF_CHECKED | MF_STRING | MF_ENABLED, ID_WINDOW_START + i, strTitle);
				pMenu->AppendMenu(MF_SEPARATOR);
			}
			else
				pMenu->AppendMenu(MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_WINDOW_START + i, strTitle);
		}
	}
	DrawMenuBar();
}

void CBrowserFrame::CreateFavoriteMenuSync(CMenu* pMenu)
{
	CFavoriteItem* parentItem = NULL;
	parentItem = theApp.m_FavMng.GetRootItem();
	CrateFavoriteMenu(pMenu, parentItem);
	DrawMenuBar();
}

void CBrowserFrame::CrateFavoriteMenu(CMenu* pMenu, CFavoriteItem* parentItem)
{
	PROC_TIME(CrateFavoriteMenu)

	if (!pMenu) return;
	if (!parentItem) return;
	CString strTitle;

	if (parentItem->GetType() == IEFavDIR ||
	    parentItem->GetType() == IEFavROOT)
	{
		INT_PTR iMax = parentItem->GetSize();
		for (INT_PTR i = 0; i < iMax; i++)
		{
			CFavoriteItem* tmp = NULL;
			tmp = (CFavoriteItem*)parentItem->GetItem(i);
			if (tmp)
			{
				if (tmp->GetType() == IEFavDIR)
				{
					SBUtil::GetDivChar(tmp->strTitle, 60, strTitle);
					strTitle.Replace(_T("&"), _T("&&"));

					CMenu* pSubMenu = NULL;
					pSubMenu = new CMenu;
					pSubMenu->CreatePopupMenu();
					pMenu->AppendMenu(MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)pSubMenu->m_hMenu, strTitle);
					CrateFavoriteMenu(pSubMenu, tmp);
					int iSubCnt = 0;
					iSubCnt = pSubMenu->GetMenuItemCount();
					if (iSubCnt <= 0)
					{
						CString strNone;
						strNone.LoadString(IDS_STRING_FAV_NANE);
						pSubMenu->AppendMenu(MF_BYPOSITION | MF_STRING | MF_DISABLED, ID_FAV_START - 1, strNone);
					}
					pSubMenu->Detach();
					delete pSubMenu;
				}
				else
				{
					CrateFavoriteMenu(pMenu, tmp);
				}
			}
		}
	}
	else
	{
		strTitle = parentItem->strTitle;
		if (parentItem->GetType() == IEFavURL)
		{
			strTitle = strTitle.Mid(0, strTitle.ReverseFind(_T('.')));
		}
		else if (parentItem->GetType() == IEFavFILE)
		{
			strTitle = strTitle.Mid(0, strTitle.ReverseFind(_T('.')));
		}
		// 文字のカット
		CString str;
		SBUtil::GetDivChar(strTitle, 120, str);
		strTitle = str;
		strTitle.Replace(_T("&"), _T("&&"));
		int iCnt = 0;
		iCnt = pMenu->GetMenuItemCount();
		pMenu->AppendMenu(MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_FAV_START + parentItem->commandID, strTitle);
	}
}

void CBrowserFrame::CrateFavoriteTreeData()
{
	if (theApp.bCreateFavDone)
		return;
	theApp.bCreateFavDone = TRUE;
	TCHAR szFolder[MAX_PATH] = {0};
	CString fvFolder;
	::SHGetSpecialFolderPath(NULL, szFolder, CSIDL_FAVORITES, FALSE);
	fvFolder = szFolder;
	theApp.m_FavMng.CreateFavoriteTree(fvFolder, TRUE);
}

void CBrowserFrame::OnFavoriteRefresh()
{
	theApp.bCreateFavDone = FALSE;
}

LRESULT CBrowserFrame::OnFavoriteAddSendMsg(WPARAM wParam, LPARAM lParam)
{
	PROC_TIME(OnFavoriteAddSendMsg)

	CString str1;
	CString str2;

	if (wParam)
	{
		str1 = (TCHAR*)wParam;
	}
	if (lParam)
	{
		str2 = (TCHAR*)lParam;
	}
	if (!str1.IsEmpty() && !str2.IsEmpty())
	{
		CString strCommand;
		CString strParam;
		theApp.CopyDBLEXEToTempEx();

		if (!SBUtil::IsURL_HTTP(str2))
		{
			return 0;
		}
		int iLen = str2.GetLength();

		//2083
		if (iLen > INTERNET_MAX_URL_LENGTH)
			return 0;

		strCommand.Format(_T("\"%s\" -AddFav \"%s|@@|%s\""), (LPCTSTR)theApp.m_strDBL_EXE_FullPath, (LPCTSTR)str1, (LPCTSTR)str2);
		strParam.Format(_T("-AddFav \"%s|@@|%s\""), (LPCTSTR)str1, (LPCTSTR)str2);

		CString logmsg;
		logmsg.Format(_T("BF_WND:0x%08p OnFavoriteAddSendMsg %s"), theApp.SafeWnd(this->m_hWnd), (LPCTSTR)strCommand);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);

		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi = {0};
		si.cb = sizeof(si);
		unsigned long ecode = 0;
		if (!CreateProcess(NULL, (LPTSTR)(LPCTSTR)strCommand, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
		{
			logmsg.Format(_T("BF_WND:0x%08p OnFavoriteAddSendMsg CreateProcess Failed"), theApp.SafeWnd(this->m_hWnd));
			theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
			SetLastError(NO_ERROR);
			//Retry
			if (!CreateProcess(theApp.m_strDBL_EXE_FullPath, (LPTSTR)(LPCTSTR)strParam, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
			{
				logmsg.Format(_T("BF_WND:0x%08p OnFavoriteAddSendMsg CreateProcess Failed2"), theApp.SafeWnd(this->m_hWnd));
				theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
				SetLastError(NO_ERROR);
				if (::ShellExecute(NULL, _T("open"), theApp.m_strDBL_EXE_FullPath, strParam, NULL, SW_SHOW) <= HINSTANCE(32))
				{
					logmsg.Format(_T("BF_WND:0x%08p OnFavoriteAddSendMsg ShellExecute Failed3"), theApp.SafeWnd(this->m_hWnd));
					theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
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
	return 0;
}

void CBrowserFrame::OnFavoriteAdd()
{
	OnFavoriteRefresh();
	CString logmsg;
	logmsg.Format(_T("BF_WND:0x%08p OnFavoriteAdd"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	if (this->m_wndView)
		OnFavoriteAddSendMsg(((WPARAM)(LPCTSTR)m_wndView.m_strTitle), ((WPARAM)(LPCTSTR)m_wndView.GetLocationURL()));
}

void CBrowserFrame::OnFavoriteOrganize()
{
	OnFavoriteRefresh();
	CString logmsg;
	logmsg.Format(_T("BF_WND:0x%08p OnFavoriteOrganize"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	BOOL retval = FALSE;
	TCHAR szFolder[MAX_PATH] = {0};
	BOOL bProc = FALSE;

	CString strCommand;
	CString strParam;
	theApp.CopyDBLEXEToTempEx();

	strCommand.Format(_T("\"%s\" -FavOrganize"), (LPCTSTR)theApp.m_strDBL_EXE_FullPath);
	strParam.Format(_T("-FavOrganize"));
	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi = {0};
	si.cb = sizeof(si);
	unsigned long ecode = 0;
	if (!CreateProcess(NULL, (LPTSTR)(LPCTSTR)strCommand, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
	{
		SetLastError(NO_ERROR);
		//Retry
		if (!CreateProcess(theApp.m_strDBL_EXE_FullPath, (LPTSTR)(LPCTSTR)strParam, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
		{
			SetLastError(NO_ERROR);
			if (::ShellExecute(NULL, _T("open"), theApp.m_strDBL_EXE_FullPath, strParam, NULL, SW_SHOW) <= HINSTANCE(32))
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

void CBrowserFrame::OnFullScreen()
{
	CString logmsg;
	//fullscreen mode 解除
	if (m_bFullScreen)
	{
		ChangeNomalWindow();
		logmsg.Format(_T("BF_WND:0x%08p OnFullScreen_OFF"), theApp.SafeWnd(this->m_hWnd));
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
	}
	//fullscreen mode に変更
	else
	{
		//Popupの場合は、フルスクリーンモードは、無効
		if (this->m_wndView)
		{
			if (!m_wndView.IsPopupWindow())
			{
				ChangeFullScreenWindow();
				logmsg.Format(_T("BF_WND:0x%08p OnFullScreen_ON"), theApp.SafeWnd(this->m_hWnd));
				theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
			}
		}
	}
}
void CBrowserFrame::ChangeFullScreenWindow()
{
	m_bFullScreen = TRUE;
	if (m_wndTitleBar == NULL)
	{
		m_wndTitleBar = new CTitleBar(theApp.m_hInstance, this->m_hWnd);
		CString strTitle;
		GetWindowText(strTitle);
		m_wndTitleBar->m_strText = strTitle;
		m_wndTitleBar->DisplayWindow(TRUE, TRUE);
	}
	SIZE_T style = GetWindowLongPtr(m_hWnd, GWL_STYLE);

	WINDOWPLACEMENT wp = {0};
	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wp);
	m_FullScreen_ShowCommand = wp.showCmd;
	m_rFullScreenRect = wp.rcNormalPosition;
	ShowWindow(SW_MAXIMIZE);

	if (m_pwndReBar)
	{
		CReBarCtrl& rbCtrl = m_pwndReBar->GetReBarCtrl();
		REBARBANDINFO rbbi = {0};
		REBARBANDINFO rbbiNull = {0};

		UINT cbCompileSize = 0;

		cbCompileSize = sizeof(REBARBANDINFO);
		rbbi.cbSize = cbCompileSize;

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
	if (theApp.IsWnd(m_pwndStatusBar))
		m_pwndStatusBar->ShowWindow(SW_HIDE);

	if (m_cTabWnd)
	{
		::ShowWindow(m_cTabWnd->GetHwnd(), SW_HIDE);
		if (theApp.IsWnd(theApp.m_wndpActiveTabLine) && theApp.m_wndpActiveTabLine->m_pParent == this)
		{
			theApp.m_wndpActiveTabLine->DestroyWindow();
			delete theApp.m_wndpActiveTabLine;
			theApp.m_wndpActiveTabLine = NULL;
		}
		if (theApp.IsWnd(theApp.m_wndpNewTab) && theApp.m_wndpNewTab->m_pParent == this)
		{
			theApp.m_wndpNewTab->DestroyWindow();
			delete theApp.m_wndpNewTab;
			theApp.m_wndpNewTab = NULL;
		}
	}

	style = GetWindowLongPtr(this->m_hWnd, GWL_STYLE);
	style &= ~(WS_DLGFRAME | WS_THICKFRAME);
	SetWindowLongPtr(this->m_hWnd, GWL_STYLE, style);
	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);
	::SetWindowPos(this->m_hWnd, HWND_TOP, -1, -1, cx + 3, cy + 3, SWP_FRAMECHANGED);
	::InvalidateRect(this->m_hWnd, NULL, TRUE);
}
void CBrowserFrame::ChangeNomalWindow()
{
	m_bFullScreen = FALSE;
	SIZE_T style = GetWindowLongPtr(m_hWnd, GWL_STYLE);

	if (m_pwndReBar)
	{
		CReBarCtrl& rbCtrl = m_pwndReBar->GetReBarCtrl();
		REBARBANDINFO rbbi = {0};
		REBARBANDINFO rbbiNull = {0};

		UINT cbCompileSize = 0;

		cbCompileSize = sizeof(REBARBANDINFO);
		rbbi.cbSize = cbCompileSize;

		int iBandCnt = 0;
		iBandCnt = rbCtrl.GetBandCount();

		for (int i = 0; i < iBandCnt; i++)
		{
			rbbi = rbbiNull;
			rbbi.fMask = RBBIM_STYLE;
			VERIFY(rbCtrl.GetBandInfo(i, &rbbi));
			VERIFY(rbCtrl.ShowBand(i, TRUE));
		}
	}
	if (theApp.IsWnd(m_pwndStatusBar))
		m_pwndStatusBar->ShowWindow(SW_SHOW);
	if (m_cTabWnd)
	{
		::ShowWindow(m_cTabWnd->GetHwnd(), SW_SHOW);
	}

	style |= WS_DLGFRAME | WS_THICKFRAME;
	SetWindowLongPtr(this->m_hWnd, GWL_STYLE, style);

	::SetWindowPos(this->m_hWnd, HWND_NOTOPMOST, 0, 0, m_rFullScreenRect.Width(), m_rFullScreenRect.Height(), SWP_NOMOVE | SWP_NOSIZE);
	if (SW_MAXIMIZE == m_FullScreen_ShowCommand)
	{
		ANIMATIONINFO info = {0};
		int iMinAnimate = 0;
		info.cbSize = sizeof(ANIMATIONINFO);
		::SystemParametersInfo(SPI_GETANIMATION, sizeof(info), (PVOID)&info, 0);
		if (info.iMinAnimate)
		{
			info.iMinAnimate = 0;
			::SystemParametersInfo(SPI_SETANIMATION, sizeof(info), (PVOID)&info,
					       SPIF_SENDCHANGE);
			ShowWindow(SW_NORMAL);
			ShowWindow(m_FullScreen_ShowCommand);
			info.iMinAnimate = 1;
			::SystemParametersInfo(SPI_SETANIMATION, sizeof(info), (PVOID)&info,
					       SPIF_SENDCHANGE);
		}
	}
	else
	{
		ShowWindow(SW_NORMAL);
	}
	if (m_wndTitleBar)
	{
		m_wndTitleBar->DisplayWindow(FALSE, TRUE);
		delete m_wndTitleBar;
		m_wndTitleBar = NULL;
	}

	::InvalidateRect(this->m_hWnd, NULL, TRUE);
}

UINT CBrowserFrame::GetWindowStyleSB()
{
	UINT iRet = 0;
	if (!theApp.IsWnd(this))
		return iRet;
	CString logmsg;
	logmsg.Format(_T("BF_WND:0x%08p GetWindowStyleSB"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);

	if (m_pwndReBar)
	{
		CReBarCtrl& rbCtrl = m_pwndReBar->GetReBarCtrl();
		REBARBANDINFO rbbi = {0};
		REBARBANDINFO rbbiNull = {0};
		UINT cbCompileSize = 0;

		cbCompileSize = sizeof(REBARBANDINFO);
		rbbi.cbSize = cbCompileSize;

		rbbiNull = rbbi;
		rbbi.fMask = RBBIM_STYLE;

		//menu
		VERIFY(rbCtrl.GetBandInfo(0, &rbbi));
		//表示されている
		if (!(rbbi.fStyle & RBBS_HIDDEN))
		{
			iRet = iRet | SBW_SHOW_MENU_BAR;
		}
		//toolbar
		rbbi = rbbiNull;
		rbbi.fMask = RBBIM_STYLE;
		VERIFY(rbCtrl.GetBandInfo(1, &rbbi));
		//表示されている
		if (!(rbbi.fStyle & RBBS_HIDDEN))
		{
			iRet = iRet | SBW_SHOW_TOOL_BAR;
		}

		//addressbar
		rbbi = rbbiNull;
		rbbi.fMask = RBBIM_STYLE;
		VERIFY(rbCtrl.GetBandInfo(2, &rbbi));
		//表示されている
		if (!(rbbi.fStyle & RBBS_HIDDEN))
		{
			iRet = iRet | SBW_SHOW_ADDRESS_BAR;
		}
	}
	//statusbar
	if (theApp.IsWnd(m_pwndStatusBar))
	{
		//表示されている
		if (m_pwndStatusBar->IsWindowVisible())
		{
			iRet = iRet | SBW_SHOW_STATUS_BAR;
		}
	}
	return iRet;
}
void CBrowserFrame::SetWindowStyleSB(UINT iParam)
{
	if (!theApp.IsWnd(this))
		return;

	CString logmsg;
	logmsg.Format(_T("BF_WND:0x%08p SetWindowStyleSB(%d)"), theApp.SafeWnd(this->m_hWnd), iParam);
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);

	if (m_pwndReBar)
	{
		CReBarCtrl& rbCtrl = m_pwndReBar->GetReBarCtrl();
		REBARBANDINFO rbbi = {0};
		REBARBANDINFO rbbiNull = {0};
		UINT cbCompileSize = 0;

		cbCompileSize = sizeof(REBARBANDINFO);
		rbbi.cbSize = cbCompileSize;

		rbbiNull = rbbi;
		rbbi.fMask = RBBIM_STYLE;

		//menu
		VERIFY(rbCtrl.GetBandInfo(0, &rbbi));
		if (iParam & SBW_SHOW_MENU_BAR)
			VERIFY(rbCtrl.ShowBand(0, TRUE));
		else
			VERIFY(rbCtrl.ShowBand(0, FALSE));

		int iBandCnt = 0;
		iBandCnt = rbCtrl.GetBandCount();
		if (iBandCnt > 1)
		{
			//toolbar
			rbbi = rbbiNull;
			rbbi.fMask = RBBIM_STYLE;
			VERIFY(rbCtrl.GetBandInfo(1, &rbbi));
			if (iParam & SBW_SHOW_TOOL_BAR)
				VERIFY(rbCtrl.ShowBand(1, TRUE));
			else
				VERIFY(rbCtrl.ShowBand(1, FALSE));

			//addressbar
			rbbi = rbbiNull;
			rbbi.fMask = RBBIM_STYLE;
			VERIFY(rbCtrl.GetBandInfo(2, &rbbi));
			if (iParam & SBW_SHOW_ADDRESS_BAR)
				VERIFY(rbCtrl.ShowBand(2, TRUE));
			else
				VERIFY(rbCtrl.ShowBand(2, FALSE));
		}
	}
	//statusbar
	if (theApp.IsWnd(m_pwndStatusBar))
	{
		if (iParam & SBW_SHOW_STATUS_BAR)
			m_pwndStatusBar->ShowWindow(SW_SHOW);
		else
			m_pwndStatusBar->ShowWindow(SW_HIDE);
	}
	return;
}

BOOL CBrowserFrame::OnShowPopupMenu(CMFCPopupMenu* pMenuPopup)
{
	PROC_TIME(OnShowPopupMenu)
	CWaitCursor wcur;
	try
	{
		if (!pMenuPopup)
		{
			return FALSE;
		}
		CString logmsg;
		CMFCPopupMenuBar* pMenuBar = pMenuPopup->GetMenuBar();
		int index = pMenuBar->CommandToIndex(ID_FAVORITE_DUMMY);
		if (index >= 0)
		{
			CrateFavoriteTreeData();
			CMenu Menu;
			Menu.CreatePopupMenu();
			CString strTitle;

			strTitle.LoadString(ID_ADD_FAVORITE);
			Menu.AppendMenu(MF_STRING | MF_ENABLED, ID_ADD_FAVORITE, strTitle);

			strTitle.LoadString(ID_ORGANIZE_FAVORITE);
			Menu.AppendMenu(MF_STRING | MF_ENABLED, ID_ORGANIZE_FAVORITE, strTitle);

			Menu.AppendMenu(MF_SEPARATOR);
			CreateFavoriteMenuSync(&Menu);
			pMenuBar->ImportFromMenu(Menu);
			return CFrameWndBase::OnShowPopupMenu(pMenuPopup);
		}
		index = pMenuBar->CommandToIndex(ID_WINDOW_DUMMY);
		if (index >= 0)
		{
			CMenu Menu;
			Menu.CreatePopupMenu();
			CString strTitle;

			strTitle.LoadString(ID_W_CLOSE);
			Menu.AppendMenu(MF_STRING | MF_ENABLED, ID_W_CLOSE, strTitle);

			strTitle.LoadString(IDC_APP_EXIT);
			Menu.AppendMenu(MF_STRING | MF_ENABLED, IDC_APP_EXIT, strTitle);

			strTitle.LoadString(IDC_APP_EXIT_BUT_THIS);
			Menu.AppendMenu(MF_STRING | MF_ENABLED, IDC_APP_EXIT_BUT_THIS, strTitle);

			strTitle.LoadString(ID_NEXT_WND);
			Menu.AppendMenu(MF_STRING | MF_ENABLED, ID_NEXT_WND, strTitle);

			strTitle.LoadString(ID_PREV_WND);
			Menu.AppendMenu(MF_STRING | MF_ENABLED, ID_PREV_WND, strTitle);
			Menu.AppendMenu(MF_SEPARATOR);

			CreateWindowMenuSync(&Menu);
			pMenuBar->ImportFromMenu(Menu);
			return CFrameWndBase::OnShowPopupMenu(pMenuPopup);
		}

		index = pMenuBar->CommandToIndex(ID_CLOSE_WND_HISTORY_DUMMY);
		if (index >= 0)
		{
			CMenu Menu;
			Menu.CreatePopupMenu();

			CString strTitle;

			POSITION pos1 = {0};
			BOOL bFound = FALSE;
			int iCnt = 0;
			//既に登録されているかチェック
			for (pos1 = theApp.m_listCloseWindowTitle.GetHeadPosition(); pos1 != NULL; theApp.m_listCloseWindowTitle.GetNext(pos1))
			{
				CString strTempStr;
				CString strTempStr2;
				strTempStr = theApp.m_listCloseWindowTitle.GetAt(pos1);
				// 文字のカット
				SBUtil::GetDivChar(strTempStr, 80, strTempStr2);
				strTempStr2.Replace(_T("&"), _T("&&"));
				strTitle.Format(_T("&%X  %s"), iCnt, (LPCTSTR)strTempStr2);
				Menu.AppendMenu(MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_CLOSE_WINDOW_START + iCnt, strTitle);
				iCnt++;
			}
			if (iCnt == 0)
			{
				strTitle.LoadString(IDS_STRING_FAV_NANE);
				Menu.AppendMenu(MF_BYPOSITION | MF_STRING | MF_GRAYED, ID_CLOSE_WND_HISTORY_DUMMY, strTitle);
			}
			pMenuBar->ImportFromMenu(Menu);
			return CFrameWndBase::OnShowPopupMenu(pMenuPopup);
		}

		index = pMenuBar->CommandToIndex(ID_OPEN_THIN_FILER);
		if (index >= 0)
		{
			if (!theApp.IsSGMode())
			{
				CMenu Menu;
				Menu.CreatePopupMenu();

				CString settingsMenuLabel;
				settingsMenuLabel.LoadString(ID_SETTINGS_MENU_LABEL);
				Menu.AppendMenu(MF_STRING | MF_ENABLED, ID_SETTINGS, settingsMenuLabel);
				pMenuBar->ImportFromMenu(Menu);
				return CFrameWndBase::OnShowPopupMenu(pMenuPopup);
			}
		}
		do
		{
			CMenu menu2;
			int iBack = pMenuBar->CommandToIndex(ID_BF_BACK1);
			int iForward = pMenuBar->CommandToIndex(ID_BF_FORWARD1);
			if (iBack < 0 && iForward < 0)
				break;
			BOOL bBack = FALSE;
			if (iBack >= 0)
				bBack = TRUE;
			if (bBack)
			{
				menu2.LoadMenu(IDR_MENU_BACK);
				logmsg.Format(_T("BF_WND:0x%08p TravelLog_Back"), theApp.SafeWnd(this->m_hWnd));
				theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
			}
			else
			{
				menu2.LoadMenu(IDR_MENU_FORWARD);
				logmsg.Format(_T("BF_WND:0x%08p TravelLog_Forward"), theApp.SafeWnd(this->m_hWnd));
				theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
			}
			CMenu* pPopup;
			pPopup = menu2.GetSubMenu(0);
			if (!theApp.IsWnd(&m_wndView))
				break;
			if (m_wndView.IsBrowserNull())
				break;

			CStringArray ArrayList;
			m_wndView.GetTravelLog(ArrayList, bBack);

			if (pPopup != NULL)
				while (pPopup->DeleteMenu(0, MF_BYPOSITION))
					;

			int iCnt = 0;
			int iListMax = (int)ArrayList.GetSize();
			for (iCnt = 0; iCnt < iListMax; iCnt++)
			{
				CString strTitle;
				CString strTitle2;
				strTitle2 = ArrayList.GetAt(iCnt);
				SBUtil::GetDivChar(strTitle2, 80, strTitle);
				strTitle.Replace(_T("&"), _T("&&"));
				strTitle.TrimLeft();
				strTitle.TrimRight();
				if (strTitle.IsEmpty()) continue;
				if (bBack)
					pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_BF_BACK1 + iCnt, strTitle);
				else
					pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_BF_FORWARD1 + iCnt, strTitle);
				if (bBack)
				{
					if (iCnt >= ID_BF_BACK_END - ID_BF_BACK1)
						break;
				}
				else
				{
					if (iCnt >= ID_BF_FORWARD_END - ID_BF_FORWARD1)
						break;
				}
			}
			pMenuBar->ImportFromMenu(pPopup->Detach());
			break;

			break;
		} while (0);
		return CFrameWndBase::OnShowPopupMenu(pMenuPopup);
	}
	catch (...)
	{
		return CFrameWndBase::OnShowPopupMenu(pMenuPopup);
	}
}

void CBrowserFrame::SetProgress(long nCurr, long nTotal)
{
	if (!theApp.IsWnd(m_pwndStatusBar))
		return;
	m_pwndStatusBar->EnablePaneProgressBar(nStatusProgress, nTotal);

	if (nTotal >= 0)
	{
		m_pwndStatusBar->SetPaneProgress(nStatusProgress, min(nTotal, max(0, nCurr)));
	}
	else
	{
		m_pwndStatusBar->RedrawWindow();
	}
}
BOOL CBrowserFrame::OnDrawMenuImage(CDC* pDC,
				    const CMFCToolBarMenuButton* pMenuButton,
				    const CRect& rectImage)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pMenuButton);

	int iIcon = -1;

	if (pMenuButton->m_nID >= ID_FAV_START &&
	    pMenuButton->m_nID <= ID_FAV_END)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_IE_FILE_FAV;
	}
	else if (pMenuButton->m_nID >= ID_WINDOW_START &&
		 pMenuButton->m_nID <= ID_WINDOW_END)
	{
		if (theApp.m_imgFavIcons.GetImageCount())
		{
			int iFaviconIndex = pMenuButton->m_nID - ID_WINDOW_START;
			::ImageList_Draw(theApp.m_imgFavIcons, iFaviconIndex,
					 pDC->GetSafeHdc(),
					 rectImage.left + (rectImage.Width() - 16) / 2,
					 rectImage.top + (rectImage.Height() - 16) / 2, ILD_TRANSPARENT);
			return TRUE;
		}
		else
			iIcon = (int)SBUtil::ICON_INDEX::ICON_IE_FILE_FAV;
	}
	else if (pMenuButton->m_nID >= ID_CLOSE_WINDOW_START &&
		 pMenuButton->m_nID <= ID_CLOSE_WINDOW_END)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_IE_FILE_FAV;
	}
	else if (pMenuButton->m_nID == ID_NEW)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_SEL_ALL;
	}
	else if (pMenuButton->m_nID == ID_NEW_BLANK)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_NEW;
	}
	else if (pMenuButton->m_nID == ID_NEW_SESSION)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_NEW_I;
	}
	else if (pMenuButton->m_nID == ID_RESTORE_WND)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_RESTORE;
	}
	else if (pMenuButton->m_nID == ID_SAVE_WND)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_SAVE_WND;
	}
	else if (pMenuButton->m_nID == ID_W_CLOSE)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_CLOSE;
	}
	else if (pMenuButton->m_nID == ID_CUT)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_CUT;
	}
	else if (pMenuButton->m_nID == ID_COPY)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_COPY;
	}
	else if (pMenuButton->m_nID == ID_PASTE)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_PASTE;
	}
	else if (pMenuButton->m_nID == ID_SEL_ALL)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_SEL_ALL;
	}
	else if (pMenuButton->m_nID == ID_FIND_PAGE)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_FIND_PAGE;
	}
	else if (pMenuButton->m_nID == ID_SETTINGS)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_SETTINGS;
	}
	else if (pMenuButton->m_nID == IDC_APP_EXIT)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_CLOSE_ALL;
	}
	else if (pMenuButton->m_nID == IDC_APP_EXIT_BUT_THIS)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_CLOSE_BUT_THIS;
	}
	else if (pMenuButton->m_nID == ID_PREV_WND)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_PREV_WND;
	}
	else if (pMenuButton->m_nID == ID_NEXT_WND)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_NEXT_WND;
	}
	else if (pMenuButton->m_nID == ID_FULL_SCREEN)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_FULL_SCREEN;
	}
	else if (pMenuButton->m_nID == ID_APP_ABOUT)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_APP_ABOUT;
	}
	else if (pMenuButton->m_nID == ID_ADD_FAVORITE)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_ADD_FAVORITE;
	}
	else if (pMenuButton->m_nID == ID_ORGANIZE_FAVORITE)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_ORGANIZE_FAVORITE;
	}
	else if (pMenuButton->m_nID == IDC_APP_DELETE_CACHE)
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_IE_OPTION;
	}

	else if (IsFavoritesMenu(pMenuButton, FALSE))
	{
		iIcon = (int)SBUtil::ICON_INDEX::ICON_FOLDER;
	}

	//////////////////////////////////////////////
	if (iIcon == -1)
	{
		return FALSE;
	}

	::ImageList_Draw(theApp.m_imgMenuIcons, iIcon,
			 pDC->GetSafeHdc(),
			 rectImage.left + (rectImage.Width() - 16) / 2,
			 rectImage.top + (rectImage.Height() - 16) / 2, ILD_DPISCALE | ILD_TRANSPARENT);

	return TRUE;
}
afx_msg LRESULT CBrowserFrame::OnToolbarReset(WPARAM wp, LPARAM)
{
	UINT uiToolBarId = (UINT)wp;
	if (uiToolBarId == IDR_MAINFRAME)
	{
		CString str;

		CMenu menuHistoryBack;
		CMenu* pmenuHistoryBackSub = NULL;
		menuHistoryBack.LoadMenu(IDR_MENU_BACK);
		pmenuHistoryBackSub = menuHistoryBack.GetSubMenu(0);

		if (m_pwndToolBar)
		{
			m_pwndToolBar->ReplaceButton(ID_GO_BACK,
						     CMFCToolBarMenuButton(ID_GO_BACK, pmenuHistoryBackSub->Detach(),
									   GetCmdMgr()->GetCmdImage(ID_GO_BACK), str));

			CMenu menuHistoryForward;
			CMenu* pmenuHistoryForwardSub = NULL;
			menuHistoryForward.LoadMenu(IDR_MENU_FORWARD);
			pmenuHistoryForwardSub = menuHistoryForward.GetSubMenu(0);
			m_pwndToolBar->ReplaceButton(ID_GO_FORWARD,
						     CMFCToolBarMenuButton(ID_GO_FORWARD, pmenuHistoryForwardSub->Detach(),
									   GetCmdMgr()->GetCmdImage(ID_GO_FORWARD), str));
			if (!theApp.IsSGMode())
			{
				m_pwndToolBar->RemoveButton(m_pwndToolBar->CommandToIndex(ID_OPEN_THIN_FILER));
			}
		}
	}
	return 0;
}
BOOL CBrowserFrame::IsFavoritesMenu(const CMFCToolBarMenuButton* pMenuButton, BOOL bReflect)
{
	if (pMenuButton == NULL)
	{
		return FALSE;
	}
	if (pMenuButton->m_nID != (UINT)-1)
	{
		CString strNone;
		strNone.LoadString(IDS_STRING_FAV_NANE);
		if (bReflect && pMenuButton->m_strText == strNone)
			return TRUE;
		return FALSE;
	}

	ASSERT_VALID(pMenuButton);
	const CObList& lstCommands = pMenuButton->GetCommands();

	for (POSITION pos = lstCommands.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pCmd = (CMFCToolBarButton*)lstCommands.GetNext(pos);
		ASSERT_VALID(pCmd);

		if ((pCmd->m_nID >= ID_FAV_START &&
		     pCmd->m_nID <= ID_FAV_END) ||
		    IsFavoritesMenu(DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pCmd), TRUE))
		{
			return TRUE;
		}
	}

	return FALSE;
}
void CBrowserFrame::SetSSLStatus(BOOL bStatus)
{
	if (!theApp.IsWnd(m_pwndStatusBar))
		return;

	if (m_pwndAddress)
	{
		int iconIndex = 0;
		iconIndex = bStatus ? (int)SBUtil::ICON_INDEX::ICON_LOCK : (int)SBUtil::ICON_INDEX::ICON_IE_FILE_FAV;
		COMBOBOXEXITEM cbi = {0};
		cbi.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
		cbi.iItem = -1;
		cbi.iImage = iconIndex;
		cbi.iSelectedImage = iconIndex;
		m_pwndAddress->SetItem(&cbi);
	}
}

void CBrowserFrame::OnStatusBarDoubleClick()
{
}
void CBrowserFrame::OnStatusBarCompatClick()
{
	if (!theApp.IsWnd(m_pwndStatusBar))
		return;
	m_wndView.m_dbZoomSize = 0.0;
	m_wndView.ZoomTo(m_wndView.m_dbZoomSize);
}

CString CBrowserFrame::GetViewLocationURLOnly()
{
	CString strRet;
	if (theApp.IsWnd(&m_wndView))
	{
		CString strTemp;
		CString strTemp2;
		strTemp = m_wndView.GetLocationURL();
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
void CBrowserFrame::OnStatusBarZoomClick()
{
	if (!theApp.IsWnd(m_pwndStatusBar))
		return;

	CString logmsg;
	logmsg.Format(_T("BF_WND:0x%08p OnStatusBarZoomClick"), theApp.SafeWnd(this->m_hWnd));
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);

	long lResult = 0;
	CStringArray straMenu;
	straMenu.Add(_T("500%"));
	straMenu.Add(_T("400%"));
	straMenu.Add(_T("300%"));
	straMenu.Add(_T("250%"));
	straMenu.Add(_T("200%"));
	straMenu.Add(_T("175%"));
	straMenu.Add(_T("150%"));
	straMenu.Add(_T("125%"));
	straMenu.Add(_T("110%"));

	straMenu.Add(_T("100%"));

	straMenu.Add(_T("90%"));
	straMenu.Add(_T("80%"));
	straMenu.Add(_T("75%"));
	straMenu.Add(_T("67%"));
	straMenu.Add(_T("50%"));
	straMenu.Add(_T("33%"));
	straMenu.Add(_T("25%"));

	CMenu menu;
	menu.LoadMenu(IDR_MENU_BACK);
	CMenu* pPopup = menu.GetSubMenu(0);
	while (pPopup->DeleteMenu(0, MF_BYPOSITION))
		;

	CString strNowZoom;
	strNowZoom = m_pwndStatusBar->GetPaneText(nStatusZoom);

	logmsg.Format(_T("BF_WND:0x%08p OnStatusBarZoomClick Now_%s"), theApp.SafeWnd(this->m_hWnd), (LPCTSTR)strNowZoom);
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);

	CString strName;
	int iCnt = 0;
	for (int i = 0; i < straMenu.GetSize(); i++)
	{
		strName = straMenu.GetAt(i);
		if (strName == strNowZoom)
		{
			pPopup->AppendMenu(MF_BYPOSITION | MF_STRING | MF_ENABLED | MF_CHECKED, ID_ZOOM_START + iCnt, strName);
		}
		else
		{
			pPopup->AppendMenu(MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_ZOOM_START + iCnt, strName);
		}
		iCnt++;
	}
	POINT point = {0};
	::GetCursorPos(&point);
	lResult = TrackPopupMenuEx(pPopup->m_hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
				   point.x, point.y,
				   this->m_hWnd, NULL);
	if (lResult > 0)
	{
		lResult = lResult - ID_ZOOM_START;
		CString strRet;
		strRet = straMenu.GetAt(lResult);
		if (theApp.IsWnd(&m_wndView))
		{
			if (!strRet.IsEmpty())
			{
				strRet.Replace(_T("%"), _T(""));
				double dZoom = 0.0;
				int iR = _tstoi(strRet);
				switch (iR)
				{
				case 500:
					dZoom = 9.0;
					break;
				case 400:
					dZoom = 8.0;
					break;
				case 300:
					dZoom = 6.0;
					break;
				case 250:
					dZoom = 5.0;
					break;
				case 200:
					dZoom = 4.0;
					break;
				case 175:
					dZoom = 3.5;
					break;
				case 150:
					dZoom = 2.5;
					break;
				case 125:
					dZoom = 1.5;
					break;
				case 110:
					dZoom = 0.5;
					break;

				case 100:
					dZoom = 0.0;
					break;

				case 90:
					dZoom = -0.5;
					break;
				case 80:
					dZoom = -1.0;
					break;
				case 75:
					dZoom = -1.5;
					break;
				case 67:
					dZoom = -2.0;
					break;
				case 50:
					dZoom = -3.5;
					break;
				case 33:
					dZoom = -5.0;
					break;
				case 25:
					dZoom = -6.0;
					break;
				default:
					dZoom = 0.0;
					break;
				}
				m_wndView.ZoomTo(dZoom);
				logmsg.Format(_T("BF_WND:0x%08p OnStatusBarZoomClick %s"), theApp.SafeWnd(this->m_hWnd), (LPCTSTR)strRet);
				theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_AC);
			}
		}
	}
}

BOOL CBrowserFrame::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}
void CBrowserFrame::OnWClose()
{
	this->OnCloseDelay();
}
void CBrowserFrame::PostWM_CLOSE()
{
	PostMessage(WM_CLOSE);
}
void CBrowserFrame::OnDestroy()
{
	try
	{
		CFrameWndBase::OnDestroy();
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

void CBrowserFrame::OnCloseDelay()
{
	PROC_TIME(OnCloseDelay)
	//ダウンロード中の場合は、警告を表示する。
	if (m_bDownloadProgress)
	{
		CString confirmMsg;
		confirmMsg.LoadString(IDS_STRING_CONFIRM_CANCEL_DOWNLOAD);
		int iRet = theApp.SB_MessageBox(this->m_hWnd, confirmMsg, NULL, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2, TRUE);
		if (iRet != IDYES)
		{
			return;
		}
		INT nBrowserId = m_wndView.GetBrowserId();
		if (nBrowserId >= 0)
			theApp.m_DlMgr.Release_DLDlg(nBrowserId);
	}
	HANDLE hEvent = {0};
	try
	{
		if (theApp.IsWnd(&m_wndView))
		{
			//Popupは除く
			if (!m_wndView.IsPopupWindow())
			{
				hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, theApp.m_strEventLogName);
				DWORD waitRes = WaitForSingleObject(hEvent, 100);
				if (waitRes == WAIT_TIMEOUT)
				{
					;
				}
				else
				{
					CString strURL = m_wndView.GetLocationURL();
					strURL.TrimLeft();
					strURL.TrimRight();
					CString strTitle = m_wndView.m_strTitle;
					strTitle.TrimLeft();
					strTitle.TrimRight();
					if (strTitle.IsEmpty())
					{
						strTitle = _T("about:blank");
					}
					if (SBUtil::IsURL_HTTP(strURL))
					{
						CString strURLDup;
						POSITION pos1 = {0};
						BOOL bFound = FALSE;
						//既に登録されているかチェック
						for (pos1 = theApp.m_listCloseWindowURL.GetHeadPosition(); pos1 != NULL; theApp.m_listCloseWindowURL.GetNext(pos1))
						{
							strURLDup.Empty();
							strURLDup = theApp.m_listCloseWindowURL.GetAt(pos1);
							if (strURLDup == strURL)
							{
								bFound = TRUE;
								break;
							}
						}
						if (!bFound)
						{
							theApp.m_listCloseWindowURL.AddHead(strURL);
							theApp.m_listCloseWindowTitle.AddHead(strTitle);
							int iMaxData = (int)theApp.m_listCloseWindowURL.GetCount();
							if (iMaxData > 16)
							{
								theApp.m_listCloseWindowURL.RemoveTail();
								theApp.m_listCloseWindowTitle.RemoveTail();
							}
						}
					}
					SetEvent(hEvent);
				}
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	if (hEvent)
		CloseHandle(hEvent);

	CString logmsg;
	if (theApp.IsWnd(&m_wndView))
	{
		PostWM_CLOSE();
		return;
	}
	if (theApp.m_bTabEnable_Init)
	{
		CString strAllCloseFlg;
		((CMainFrame*)theApp.m_pMainWnd)->GetWindowText(strAllCloseFlg);
		if (strAllCloseFlg == theApp.m_strThisAppName)
		{
			::SetFocus(NULL);
			m_wndView.EnableWinEx(FALSE);
			this->EnableWindow(FALSE);
			ShowWindow(SW_HIDE);
			return;
		}
	}
	if (m_cTabWnd)
	{
		if (theApp.m_bTabEnable_Init)
		{
			//連続して閉じる場合、自信がアクティブでない場合は、不要
			if (theApp.GetActiveBFramePtr() == this)
			{
				CBrowserFrame* pWnd = NULL;
				pWnd = theApp.GetNextGenerationActiveWindow(this);
				if (pWnd == this)
				{
					//最後の一つ以外には無いはず。
#ifdef _DEBUG
					if (TabCtrl_GetItemCount(this->m_cTabWnd->m_hwndTab) != 1)
						ASSERT(FALSE);
#endif
				}
				else
				{
					if (theApp.IsWnd(pWnd))
					{
#ifdef _DEBUG
						TRACE(_T("##TWNT_DEL##  Close[%s] NGen[%s]\n"), this->m_wndView.m_strTitle, pWnd->m_wndView.m_strTitle);
#endif
						((CMainFrame*)theApp.m_pMainWnd)->TabWindowMsgBSend(TWNT_DEL, pWnd->GetSafeHwnd());
					}
				}
			}
		}
		((CMainFrame*)theApp.m_pMainWnd)->Delete_TabWindow(this);
		((CMainFrame*)theApp.m_pMainWnd)->TabWindowMsgBSend(TWNT_REFRESH, NULL);

		delete m_cTabWnd;
		m_cTabWnd = NULL;
	}

	::SetFocus(NULL);
	this->EnableWindow(FALSE);
	ShowWindow(SW_HIDE);
}
int CBrowserFrame::OnMouseActivate(CWnd* pDetelnetopWnd, UINT nHitTest, UINT message)
{
	int iRet = CFrameWnd::OnMouseActivate(pDetelnetopWnd, nHitTest, message);
	return iRet;
}

/*
 * 位置に応じてカーソルを変える
 */
BOOL CBrowserFrame::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (theApp.m_bTabEnable_Init)
	{
		if (m_cTabWnd)
		{
			if (theApp.IsWnd(theApp.m_wndpClose) && theApp.m_wndpClose->m_pParent == this)
			{
				POINT point = {0};
				::GetCursorPos(&point);
				if (theApp.IsWnd(theApp.m_wndpClose))
				{
					POINT pt = {0};
					::GetCursorPos(&pt);
					this->ScreenToClient(&pt);
					if (theApp.m_wndpClose->m_Rc.PtInRect(pt))
					{
						return theApp.m_wndpClose->OnSetCursor(pWnd, nHitTest, message);
					}
				}
			}
			if (theApp.IsWnd(theApp.m_wndpNewTab) && theApp.m_wndpNewTab->m_pParent == this)
			{
				POINT point = {0};
				::GetCursorPos(&point);
				if (theApp.IsWnd(theApp.m_wndpNewTab))
				{
					POINT pt = {0};
					::GetCursorPos(&pt);
					this->ScreenToClient(&pt);
					if (theApp.m_wndpNewTab->m_Rc.PtInRect(pt))
					{
						return theApp.m_wndpNewTab->OnSetCursor(pWnd, nHitTest, message);
					}
				}
			}
		}
	}
	return CFrameWnd::OnSetCursor(pWnd, nHitTest, message);
}

/*
 * メニュー > タブウィンドウの一覧
 */
void CBrowserFrame::OnTabListShow()
{
	if (theApp.m_bTabEnable_Init)
	{
		CPoint pt;
		GetCursorPos(&pt);
		CRect rcToolBar;
		if (m_pwndToolBar)
		{
			m_pwndToolBar->GetWindowRect(rcToolBar);
			pt.y = rcToolBar.bottom;
		}

		CMenu menu;
		menu.LoadMenu(IDR_MENU_TAB);
		CMenu* menuSub = NULL;
		menuSub = menu.GetSubMenu(0);
		if (!menuSub)
			return;

		while (menuSub->DeleteMenu(0, MF_BYPOSITION))
			;

		CString strTitle;
		CStringArray strATitle;
		((CMainFrame*)theApp.m_pMainWnd)->Get_TabWindowInfoArray(strATitle, m_ptrAWnd);
		size_t iSize = 0;
		iSize = strATitle.GetCount();
		CPtrArray ptBmpArray;
		for (size_t i = 0; i < iSize; i++)
		{
			strTitle = strATitle.GetAt(i);
			strTitle.TrimLeft();
			strTitle.TrimRight();
			// 文字のカット
			CString str;
			SBUtil::GetDivChar(strTitle, 160, str);
			strTitle = str;
			strTitle.Replace(_T("&"), _T("&&"));
			if (m_ptrAWnd.GetAt(i) == this->GetSafeHwnd())
			{
				menuSub->AppendMenu(MF_BYPOSITION | MF_CHECKED | MF_STRING | MF_ENABLED, ID_WINDOW_START + i, strTitle);
			}
			else
			{
				menuSub->AppendMenu(MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_WINDOW_START + i, strTitle);
			}
			HBITMAP hbmp = {0};
			CIconHelper hicon;
			HDC hdcMem = {0};
			HBITMAP hbmpPrev = {0};
			UINT uWidth = GetSystemMetrics(SM_CXSMICON);
			UINT uHeight = GetSystemMetrics(SM_CYSMICON);
			hicon = theApp.m_imgFavIcons.ExtractIcon((int)i);

			LPVOID lpBits = {0};
			BITMAPINFO bmi = {0};
			memset(&bmi, 0, sizeof(BITMAPINFO));
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = uWidth;
			bmi.bmiHeader.biHeight = uHeight;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			hbmp = CreateDIBSection(NULL, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, &lpBits, NULL, 0);
			hdcMem = CreateCompatibleDC(NULL);
			hbmpPrev = (HBITMAP)SelectObject(hdcMem, hbmp);
			DrawIconEx(hdcMem, 0, 0, hicon, uWidth, uHeight, 0, NULL, DI_NORMAL);
			SelectObject(hdcMem, hbmpPrev);
			DeleteDC(hdcMem);
			CBitmap* pBmp = CBitmap::FromHandle((HBITMAP)hbmp);
			ptBmpArray.Add(hbmp);
			menuSub->SetMenuItemBitmaps((int)i, MF_BYPOSITION, pBmp, pBmp);
		}
		BOOL lResult = TrackPopupMenuEx(menuSub->m_hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
						pt.x, pt.y,
						(HWND)this->GetSafeHwnd(), NULL);
		size_t iBmpCnt = ptBmpArray.GetCount();
		for (UINT j = 0; j < iBmpCnt; j++)
		{
			HBITMAP bmpD = {0};
			bmpD = (HBITMAP)ptBmpArray.GetAt(j);
			if (bmpD)
			{
				CBitmap* pBmp = NULL;
				pBmp = CBitmap::FromHandle((HBITMAP)bmpD);
				if (pBmp)
				{
					pBmp->DeleteObject();
				}
			}
		}

		if (lResult > 0)
		{
			SendMessage(WM_COMMAND, MAKEWPARAM(LOWORD(lResult), 0x0), 0);
		}
	}
}

/*
 * ファイル > ウィンドウを復元
 */
void CBrowserFrame::OnRestoreWnd()
{
	int iRet = 0;
	CString strMsg;
	strMsg.LoadString(ID_RESTORE_WND);
	iRet = theApp.SB_MessageBox(this->m_hWnd, strMsg, NULL, MB_ICONQUESTION | MB_YESNO, TRUE);

	if (iRet != IDYES)
		return;

	if (!theApp.IsWnd(theApp.m_pMainWnd))
		return;

	CString strFile(theApp.m_strRestoreFileFullPath);
	((CMainFrame*)theApp.m_pMainWnd)->RestoreWnd(strFile);
}

/*
 * ツール > ファイルマネージャ (SGモードのみ)
 */
void CBrowserFrame::OpenThinFiler()
{
	if (!theApp.IsSGMode()) return;

	CString strFrmWnd;
	strFrmWnd = _T("CFiler:");
	strFrmWnd += theApp.m_FrmWndClassName;
	//既に起動しているか？
	HWND hWndCap = ::FindWindow(strFrmWnd, NULL); //APのハンドル取得

	//起動している。
	if (hWndCap)
	{
		SBUtil::SetAbsoluteForegroundWindow(hWndCap, FALSE);
	}
	else
	{
		Init_MsgDlg();
		CString startingMsg;
		startingMsg.LoadString(IDS_STRING_STARTING_FILEMGR);
		SetMessage_MsgDlg(startingMsg);
		theApp.OpenChFiler(NULL);
		Release_MsgDlg();
	}
}

/*
 * ファイル > ウィンドウの保存
 */
void CBrowserFrame::OnSaveWnd()
{
	int iRet = 0;
	CString strMsg;
	strMsg.LoadString(ID_SAVE_WND);
	iRet = theApp.SB_MessageBox(this->m_hWnd, strMsg, NULL, MB_ICONQUESTION | MB_YESNO, TRUE);

	if (iRet != IDYES)
		return;

	if (!theApp.IsWnd(theApp.m_pMainWnd))
		return;

	CString strFile(theApp.m_strRestoreFileFullPath);
	((CMainFrame*)theApp.m_pMainWnd)->SaveWindowList(strFile);

	((CMainFrame*)theApp.m_pMainWnd)->m_MultiProcessHWND.RemoveAll();
	((CMainFrame*)theApp.m_pMainWnd)->GetMultiProcessWnd(::GetDesktopWindow());
	HWND hWndChild = {0};
	size_t iProcCnt = ((CMainFrame*)theApp.m_pMainWnd)->m_MultiProcessHWND.GetCount();
	if (iProcCnt > 1)
	{
		for (size_t iCol = 0; iCol < iProcCnt; iCol++)
		{
			hWndChild = (HWND)((CMainFrame*)theApp.m_pMainWnd)->m_MultiProcessHWND.GetAt(iCol);
			TCHAR szTitleMultipleInstance[260] = {0};
			::GetWindowText(hWndChild, szTitleMultipleInstance, 259);
			CString strTitleMultiple;
			strTitleMultiple = szTitleMultipleInstance;
			//window名を確認する。
			if (strTitleMultiple == theApp.m_FrmWndClassName)
			{
				if (hWndChild != ((CMainFrame*)theApp.m_pMainWnd)->m_hWnd)
				{
					SendMessageTimeout(hWndChild, WM_SAVE_WND_M, 0, 0, SMTO_NORMAL, 15000, NULL);
				}
			}
		}
	}
}
void CBrowserFrame::OnTabCloseLeft()
{
	if (theApp.IsWnd(theApp.m_pMainWnd))
	{
		//連続して閉じる場合、自信がアクティブでない場合は、不要
		if (theApp.GetActiveBFramePtr() == this)
		{
			((CMainFrame*)theApp.m_pMainWnd)->CloseTabWindowLeft(this);
		}
	}
}
void CBrowserFrame::OnTabCloseRight()
{
	if (theApp.IsWnd(theApp.m_pMainWnd))
	{
		//連続して閉じる場合、自信がアクティブでない場合は、不要
		if (theApp.GetActiveBFramePtr() == this)
		{
			((CMainFrame*)theApp.m_pMainWnd)->CloseTabWindowRight(this);
		}
	}
}

/*
 * ウィンドウ > 次のウィンドウ (Ctrl-Tab)
 */
void CBrowserFrame::OnNextWnd()
{
	CBrowserFrame* ptr = NULL;
	if (theApp.IsWnd(theApp.m_pMainWnd))
	{
		ptr = ((CMainFrame*)theApp.m_pMainWnd)->GetNextFrmWnd(this);
		if (ptr != this)
		{
			if (theApp.IsWnd(ptr))
			{
				if (m_cTabWnd)
				{
					m_cTabWnd->ShowTabWindow(ptr->m_hWnd);
				}
				else
				{
					if (::IsIconic(ptr->m_hWnd))
					{
						::ShowWindow(ptr->m_hWnd, SW_HIDE);
						::ShowWindow(ptr->m_hWnd, SW_NORMAL);
					}
					::SetWindowPos(ptr->m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
				}
			}
		}
	}
}

/*
 * ツールバー > ウィンドウ > 前のウィンドウ (Shift-Ctrl-Tab)
 */
void CBrowserFrame::OnPrevWnd()
{
	CBrowserFrame* ptr = NULL;
	if (theApp.IsWnd(theApp.m_pMainWnd))
	{
		ptr = ((CMainFrame*)theApp.m_pMainWnd)->GetPrevFrmWnd(this);
		if (ptr != this)
		{
			if (theApp.IsWnd(ptr))
			{
				if (m_cTabWnd)
				{
					m_cTabWnd->ShowTabWindow(ptr->m_hWnd);
				}
				else
				{
					if (::IsIconic(ptr->m_hWnd))
					{
						::ShowWindow(ptr->m_hWnd, SW_HIDE);
						::ShowWindow(ptr->m_hWnd, SW_NORMAL);
					}
					::SetWindowPos(ptr->m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
				}
			}
		}
	}
}

/*
 * 同時に複数のタブを閉じる時は確認する
 */
void CBrowserFrame::OnAppExitEx()
{
	if (theApp.IsWnd(theApp.m_pMainWnd))
	{
		if (theApp.m_bTabEnable_Init)
		{
			if (((CMainFrame*)theApp.m_pMainWnd)->Get_TabWindowCount() <= 1)
			{
				theApp.m_pMainWnd->PostMessage(WM_COMMAND, WM_APP_EXIT, NULL);
				return;
			}
		}
		int iRet = 0;
		CString strMsg;
		strMsg.LoadString(IDS_STRING_CLOSE_ALL_WINDOW);
		iRet = theApp.SB_MessageBox(this->m_hWnd, strMsg, NULL, MB_ICONQUESTION | MB_YESNO, TRUE);
		if (iRet != IDYES)
			return;
		theApp.m_pMainWnd->PostMessage(WM_COMMAND, WM_APP_EXIT, NULL);
	}
}

void CBrowserFrame::OnAppExitExBT()
{
	if (theApp.IsWnd(theApp.m_pMainWnd))
	{
		((CMainFrame*)theApp.m_pMainWnd)->AppExitExBT(this);
	}
}

/*
 * ヘルプ > ブラウザーキャッシュを削除する
 */
void CBrowserFrame::OnAppDeleteCache()
{
	if (theApp.IsWnd(theApp.m_pMainWnd))
	{
		int iRet = 0;
		CString confirmMsg;
		confirmMsg.LoadString(IDS_STRING_CONFIRM_CLEAR_CACHE);
		iRet = theApp.SB_MessageBox(this->m_hWnd, confirmMsg, NULL, MB_ICONQUESTION | MB_YESNO, TRUE);
		if (iRet != IDYES)
			return;
		theApp.m_pMainWnd->PostMessage(WM_COMMAND, WM_APP_EXIT, NULL);
		theApp.m_bEnforceDeleteCache = TRUE;
	}
}

/*
 * OnSizing: サイズ変更中はタブのUIを隠す
 */
void CBrowserFrame::OnSizing(UINT fwSide, LPRECT pRect)
{
	if (fwSide == WMSZ_TOP || fwSide == WMSZ_BOTTOM)
		;
	else
	{
		//サイズ変更中は、非表示
		if (theApp.m_bTabEnable_Init)
		{
			//作成済みだが、このFrameではない。
			if (theApp.m_wndpActiveTabLine)
			{
				theApp.m_wndpActiveTabLine->DestroyWindow();
				delete theApp.m_wndpActiveTabLine;
				theApp.m_wndpActiveTabLine = NULL;
			}
			if (theApp.m_wndpNewTab)
			{
				theApp.m_wndpNewTab->DestroyWindow();
				delete theApp.m_wndpNewTab;
				theApp.m_wndpNewTab = NULL;
			}
		}
	}

	CFrameWndEx::OnSizing(fwSide, pRect);
}
void CBrowserFrame::OnMove(int cx, int cy)
{
	CFrameWndBase::OnMove(cx, cy);
}

void CBrowserFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWndBase::OnSize(nType, cx, cy);

	if (m_wndpLogo)
	{
		m_wndpLogo->ShowWindow(SW_SHOW);
		m_wndpLogo->MoveWnd(cx, cy);
	}
	if (theApp.m_bTabEnable_Init)
	{
		if (!m_cTabWnd)
			return;
		//Visibleアクティブなウインドウのはず
		if (!theApp.IsWndVisible(this->m_hWnd))
			return;
		//有効なはず。
		if (!this->m_wndView.IsWindowEnabled())
			return;
		//Popupは除く
		if (this->m_wndView.IsPopupWindow())
			return;

		WINDOWPLACEMENT zFramePracement = {0};
		zFramePracement.length = sizeof(WINDOWPLACEMENT);
		this->GetWindowPlacement(&zFramePracement);
		if (zFramePracement.showCmd == SW_SHOWNORMAL)
		{
			//タスクバーを除く画面サイズを取得
			RECT rcDisp = {0};
			SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcDisp, NULL);
			CRect rc;
			this->GetWindowRect(&rc);
			//タスクバーの位置を考慮する。上、左にあると座標がずれるので。下、右にある分には影響がない。原点座標のxyに影響
			//上にある場合
			if (rcDisp.top > 0)
			{
				rc.top -= rcDisp.top;
				rc.bottom -= rcDisp.top;
			}
			//左にある場合
			if (rcDisp.left > 0)
			{
				rc.left -= rcDisp.left;
				rc.right -= rcDisp.left;
			}
			CRect rcCurrent = zFramePracement.rcNormalPosition;
			CRect rc2(zFramePracement.rcNormalPosition);
			//aerosnap
			if (rc != rc2)
			{
				zFramePracement.rcNormalPosition = rc;
				this->SetWindowPlacement(&zFramePracement);
			}
		}
	}
}

void CBrowserFrame::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CFrameWndBase::OnWindowPosChanged(lpwndpos);
	return;
}

void CBrowserFrame::OnEnterSizeMove()
{
	CFrameWndBase::OnEnterSizeMove();
}

/*
 * OnExitSizeMove: 移動・リサイズ終了後にウィンドウ位置を同期する
 */
void CBrowserFrame::OnExitSizeMove()
{
	//Tab Mode
	if (theApp.m_bTabEnable_Init)
	{
		if (!m_cTabWnd)
			return;
		//Visibleアクティブなウインドウのはず
		if (!theApp.IsWndVisible(this->m_hWnd))
			return;
		//有効なはず。
		if (!this->m_wndView.IsWindowEnabled())
			return;
		//Popupは除く
		if (this->m_wndView.IsPopupWindow())
			return;

		//Activeではない。
		if (theApp.GetActiveBFramePtr() != this)
			return;

		WINDOWPLACEMENT zFramePracement = {0};
		zFramePracement.length = sizeof(WINDOWPLACEMENT);
		this->GetWindowPlacement(&zFramePracement);

		//タスクバーを除く画面サイズを取得
		RECT rcDisp = {0};
		SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcDisp, NULL);
		CRect rc;
		this->GetWindowRect(&rc);
		//タスクバーの位置を考慮する。上、左にあると座標がずれるので。下、右にある分には影響がない。原点座標のxyに影響
		//上にある場合
		if (rcDisp.top > 0)
		{
			rc.top -= rcDisp.top;
			rc.bottom -= rcDisp.top;
		}
		//左にある場合
		if (rcDisp.left > 0)
		{
			rc.left -= rcDisp.left;
			rc.right -= rcDisp.left;
		}
		CRect rcCurrent = zFramePracement.rcNormalPosition;
		if (zFramePracement.showCmd == SW_SHOWNORMAL)
		{
			CRect rc2(zFramePracement.rcNormalPosition);
			//aerosnap
			if (rc != rc2)
			{
				zFramePracement.rcNormalPosition = rc;
				this->SetWindowPlacement(&zFramePracement);
			}
		}
		((CMainFrame*)theApp.m_pMainWnd)->SetWindowPlacementFrm(zFramePracement, this);
	}
	return;
}
LRESULT CBrowserFrame::OnTabNotify(WPARAM wp, LPARAM lp)
{
	if (m_cTabWnd)
	{
		m_cTabWnd->TabWindowNotify(wp, lp);
	}
	return 0;
}

void CBrowserFrame::OnNcLButtonDown(UINT nType, CPoint pt)
{
	CFrameWndBase::OnNcLButtonDown(nType, pt);
}

void CBrowserFrame::OnCaptureChanged(CWnd* pWnd)
{
	CFrameWndBase::OnCaptureChanged(pWnd);
}

void CBrowserFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (LOWORD(nID) == IDM_ABOUTBOX)
	{
		PostMessage(WM_COMMAND, ID_APP_ABOUT);
		return;
	}
	if (LOWORD(nID) == IDM_APP_EXIT)
	{
		PostMessage(WM_COMMAND, IDC_APP_EXIT);
		return;
	}
	if (LOWORD(nID) == IDM_TASK_MGR)
	{
		theApp.OpenChTaskMgr();
		return;
	}

	if (nID == SC_CLOSE)
	{
		if (theApp.m_bTabEnable_Init)
		{
			if (m_cTabWnd)
			{
				//Popupは除く
				if (!m_wndView.IsPopupWindow())
				{
					PostMessage(WM_COMMAND, IDC_APP_EXIT);
					return;
				}
			}
		}

		OnCloseDelay();
		return;
	}
	if (theApp.m_bTabEnable_Init)
	{
		if (nID == SC_RESTORE || nID == SC_MAXIMIZE || nID == SC_MINIMIZE)
		{
			CSystemAnimationHelper anih(TRUE);
			CFrameWndBase::OnSysCommand(nID, lParam);
			return;
		}
	}
	CFrameWndBase::OnSysCommand(nID, lParam);
}

/*
 * サスペンド復帰時にフォント崩れを防ぐ
 */
LRESULT CBrowserFrame::OnPowerBroadcast(WPARAM wp, LPARAM)
{
	LRESULT lres = Default();

	if (wp == PBT_APMRESUMESUSPEND)
	{
		LOGFONT lf = {0};
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
		afxGlobalData.SetMenuFont(&lf, true);
	}
	return lres;
}

LRESULT CBrowserFrame::OnChangeVisualManager(WPARAM, LPARAM)
{
	return 0;
}

void CBrowserFrame::InitVisualManager()
{
}

void CBrowserFrame::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	if (m_cTabWnd)
	{
		lpMMI->ptMinTrackSize.x = 850;
		lpMMI->ptMinTrackSize.y = 260;
	}
}

/*
 * CMyStatusBar: ステータスバー
 */
IMPLEMENT_DYNAMIC(CMyStatusBar, CMFCStatusBar)
//{{AFX_MSG_MAP(CMyStatusBar)
BEGIN_MESSAGE_MAP(CMyStatusBar, CMFCStatusBar)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()
//}}AFX_MSG_MAP
void CMyStatusBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_bPaneDoubleClick)
	{
		CMFCStatusBarPaneInfo* pSBP = HitTest(point);
		if (pSBP != NULL)
		{
			if (pSBP->nID == ID_INDICATOR_COMPAT)
			{
				GetOwner()->PostMessage(WM_COMMAND, ID_INDICATOR_COMPAT);
				return;
			}
		}
	}
	CMFCStatusBar::OnLButtonDblClk(nFlags, point);
}
void CMyStatusBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bPaneDoubleClick)
	{
		CMFCStatusBarPaneInfo* pSBP = HitTest(point);
		if (pSBP != NULL)
		{
			GetOwner()->PostMessage(WM_COMMAND, pSBP->nID);
		}
	}
	CMFCStatusBar::OnLButtonUp(nFlags, point);
}
void CMyStatusBar::OnRButtonUp(UINT nFlags, CPoint point)
{
	CMFCStatusBar::OnRButtonUp(nFlags, point);
}

BOOL CWindowStatusHelper::EnumWindowsProc(HWND hwnd)
{
	int nRet = 0;
	TCHAR pszName[64] = {0};
	//ウィンドウクラスの取得
	nRet = ::GetClassName(hwnd, pszName, 64);
	if (nRet == 0)
		return TRUE;
	nRet = ::_tcscmp(pszName, _T("CSGFrame"));
	if (nRet != 0)
	{
		return TRUE;
	}

	DWORD dwProcessId = 0;
	::GetWindowThreadProcessId(hwnd, &dwProcessId);
	if (::GetCurrentProcessId() != dwProcessId)
	{
		BOOL bMulti = FALSE;
		size_t iCnt = ((CMainFrame*)theApp.m_pMainWnd)->m_MultiProcessHWND.GetCount();
		for (size_t i = 0; i < iCnt; i++)
		{
			HWND hW = (HWND)((CMainFrame*)theApp.m_pMainWnd)->m_MultiProcessHWND.GetAt(i);
			if (hW)
			{
				if (theApp.SafeWnd(hW))
				{
					DWORD dwProcessIdMulti = 0;
					::GetWindowThreadProcessId(hW, &dwProcessIdMulti);
					if (dwProcessIdMulti == dwProcessId)
					{
						bMulti = TRUE;
						break;
					}
				}
			}
		}
		if (!bMulti)
			return TRUE;
	}
	CString strTitle;
	SBUtil::SafeGetWindowText(hwnd, strTitle);
	m_ArrayWindowHWND.Add(hwnd);
	m_ArrayWindowTitle.Add(strTitle);

	return TRUE;
}

/*
 * CNilButton: ウィンドウ右上のSazabiアイコン
 */
BEGIN_MESSAGE_MAP(CNilButton, CStatic)
	//{{AFX_MSG_MAP(CNilButton)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNilButton::Init(int i, CWnd* pParent, CWnd* pView)
{
	m_pParent = pParent;
	m_pView = pView;
	CString szPathData;
	szPathData = theApp.m_strLogoFileFullPath;
	CBitmap bitmap;
	bitmap.Attach(::LoadImage(AfxGetInstanceHandle(), szPathData, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
	BITMAP bm = {0};
	if (bitmap.m_hObject != NULL)
	{
		bitmap.GetBitmap(&bm);
		size.cx = bm.bmWidth;
		size.cy = bm.bmHeight;
		imagelist.Create(size.cx, size.cy, ILC_COLOR24 | ILC_MASK, 1, 1);
		imagelist.Add(&bitmap, RGB(255, 0, 255));

		bitmap.DeleteObject();
		m_bImage = TRUE;
		CRect r(0, 0, size.cx, size.cy);
		CStatic::Create(NULL, WS_CHILD, r, pParent, i);
	}
	else
	{
		CImage pngImage;
		if (S_OK == pngImage.Load(szPathData))
		{
			bitmap.Attach(pngImage.Detach());
			if (bitmap.m_hObject != NULL)
			{
				bitmap.GetBitmap(&bm);
				size.cx = bm.bmWidth;
				size.cy = bm.bmHeight;
				imagelist.Create(size.cx, size.cy, ILC_COLOR24 | ILC_MASK, 1, 1);
				imagelist.Add(&bitmap, RGB(255, 255, 255));

				bitmap.DeleteObject();
				m_bImage = TRUE;
				CRect r(0, 0, size.cx, size.cy);
				CStatic::Create(NULL, WS_CHILD, r, pParent, i);
				return m_bImage;
			}
		}
		CIconHelper ICoHelper;
		size.cx = 32;
		size.cy = 32;
		ICoHelper = theApp.LoadIcon(IDR_MAINFRAME);
		imagelist.Create(size.cx, size.cy, ILC_COLOR32 | ILC_MASK, 1, 1);
		imagelist.Add(ICoHelper);
		m_bImage = TRUE;
		CRect r(0, 0, size.cx, size.cy);
		CStatic::Create(NULL, WS_CHILD, r, pParent, i);
	}
	return m_bImage;
}
void CNilButton::MoveWnd(int cx, int cy)
{
	if (!m_bImage) return;
	if (!theApp.IsWnd(m_pParent)) return;
	if (!theApp.IsWnd(m_pView)) return;

	CRect rect;
	GetClientRect(&rect);
	rect.left = cx - size.cx;
	if (rect.left < 350)
	{
		rect.left = 350;
	}
	rect.right = rect.left + size.cx;
	rect.top = 0;
	rect.bottom = rect.Height();
	this->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

	this->InvalidateRect(NULL, FALSE);
}
void CNilButton::OnPaint()
{
	if (!m_bImage) return;
	if (!theApp.IsWnd(m_pParent)) return;
	if (!theApp.IsWnd(m_pView)) return;

	CPaintDC dc(this);
	CPoint pos;
	CRect rect;
	GetClientRect(&rect);
	pos.x = rect.left;
	pos.y = rect.top;
	imagelist.Draw(&dc, 0, pos, ILD_NORMAL);
}
BOOL CNilButton::OnEraseBkgnd(CDC* pDC)
{
	if (!m_bImage) return TRUE;
	if (!theApp.IsWnd(m_pParent)) return TRUE;
	if (!theApp.IsWnd(m_pView)) return TRUE;

	// ブラシの作成
	CBrush brush;
	brush.CreateSysColorBrush(COLOR_3DHILIGHT);
	// 新しいブラシを指定(元のブラシの保存)
	CBrush* pOldBrush = pDC->SelectObject(&brush);

	// 塗りつぶす領域を取得
	CRect rect;
	GetClientRect(&rect);
	//pDC->GetClipBox(&rect);

	// 塗りつぶす
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);

	// ブラシを元に戻す
	pDC->SelectObject(pOldBrush);

	return TRUE;
}

/*
 * CMyToolBar: ツールバー
 */
IMPLEMENT_DYNAMIC(CMyToolBar, CMFCToolBar)
//{{AFX_MSG_MAP(CMyToolBar)
BEGIN_MESSAGE_MAP(CMyToolBar, CMFCToolBar)
	ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()
//}}AFX_MSG_MAP
void CMyToolBar::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
}

/*
 * CMyMenuBar: メニューバー
 */
IMPLEMENT_DYNAMIC(CMyMenuBar, CMFCMenuBar)
//{{AFX_MSG_MAP(CMyMenuBar)
BEGIN_MESSAGE_MAP(CMyMenuBar, CMFCMenuBar)
	ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()
//}}AFX_MSG_MAP
void CMyMenuBar::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
}

/*
 * CMyMenuBar: メニューコンテナ (ツール/メニュー)
 */
IMPLEMENT_DYNAMIC(CMyReBar, CMFCReBar)
//{{AFX_MSG_MAP(CMyReBar)
BEGIN_MESSAGE_MAP(CMyReBar, CMFCReBar)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()
//}}AFX_MSG_MAP
