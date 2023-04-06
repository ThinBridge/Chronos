#pragma once

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "BroView.h"
#include <afxtempl.h>
#include "MyComboBoxEx.h"
#include "imm.h"
#include "FullScreenTitleBar.h"
#pragma comment(lib, "imm32.lib")
#include "resource.h"

#define CFrameWndBase	     CFrameWndEx
#define nStatusIcon	     0
#define nStatusInfo	     1
#define nStatusProgress	     2
#define nStatusSSL	     3
#define nStatusCompat	     4
#define nStatusZoom	     5
#define SBW_SHOW_MENU_BAR    0x1
#define SBW_SHOW_TOOL_BAR    0x2
#define SBW_SHOW_ADDRESS_BAR 0x4
#define SBW_SHOW_STATUS_BAR  0x8
#include "CTabWnd.h"
class CMyMenuBar;
class CMyReBar;
class CMyToolBar;
class CMyComboBoxEx;
class CMyStatusBar;
class CNilButton;
class CIconEdit;
class CWindowStatusHelper
{
	static BOOL CALLBACK EnumWindowsProc_(HWND hwnd, LPARAM lParam)
	{
		if (lParam == NULL)
			return FALSE;
		return ((CWindowStatusHelper*)lParam)->EnumWindowsProc(hwnd);
	}

public:
	CPtrArray m_ArrayWindowHWND;
	CStringArray m_ArrayWindowTitle;
	DWORD GetWindowList(void)
	{
		m_IETabCnt = 0;
		m_ArrayWindowHWND.RemoveAll();
		m_ArrayWindowTitle.RemoveAll();
		::EnumWindows(EnumWindowsProc_, (LPARAM)this);
		return m_IETabCnt;
	}

	CWindowStatusHelper()
	{
		m_IETabCnt = 0;
	}
	virtual ~CWindowStatusHelper()
	{
	}
	DWORD m_IETabCnt;
	DWORD GetTabCnt(void)
	{
		m_IETabCnt = 0;
		::EnumWindows(EnumWindowsProc_, (LPARAM)this);
		return m_IETabCnt;
	}
	BOOL EnumWindowsProc(HWND hwnd);
};

class CBrowserFrame : public CFrameWndBase
{

public:
	CBrowserFrame();
	CMyMenuBar* m_pwndMenuBar;
	CMyReBar* m_pwndReBar;
	CMyToolBar* m_pwndToolBar;
	CMyComboBoxEx* m_pwndAddress;
	CMyStatusBar* m_pwndStatusBar;
	CNilButton* m_wndpLogo;
	CTabWnd* m_cTabWnd;
	DownloadFaviconCB m_FaviconCB;
	DownloadImageCopyClipboard m_ImageCopy;

	CIconEdit* m_wndEditSearch;
	BOOL m_MenuBarCreateFlg;
	BOOL m_pwndReBarCreateFlg;
	BOOL m_pwndToolBarCreateFlg;
	BOOL m_pwndAddressCreateFlg;
	BOOL m_pwndStatusBarCreateFlg;
	CString m_AtomOptionParam;
	BOOL m_bIME;
	BOOL m_bFullScreen;
	UINT m_FullScreen_ShowCommand;
	CRect m_rFullScreenRect;
	CTitleBar* m_wndTitleBar;
	BOOL m_bOLEViewer;

	BOOL m_bMT_View;
	CWinThread* m_pCurrentThread;
	BOOL m_bIsPopupWindow;

	int m_iStatusIconIndexCache_Zoom;
	void CrateFavoriteTreeData();
	virtual BOOL OnShowPopupMenu(CMFCPopupMenu* pMenuPopup);
	virtual BOOL OnDrawMenuImage(CDC* pDC,
				     const CMFCToolBarMenuButton* pMenuButton,
				     const CRect& rectImage);
	BOOL IsFavoritesMenu(const CMFCToolBarMenuButton* pMenuButton, BOOL bReflect);
	void ChangeFullScreenWindow();
	void ChangeNomalWindow();
	UINT GetWindowStyleSB();
	void SetWindowStyleSB(UINT iParam);

private:
	void CreateRebars();
	void CreateRebarsPopup();
	void CreateStatusbar();

public:
	void CreateView();
	CString m_Glogmsg;
	INT m_nBrowserState;

	void SearchAndNavigate(CString strIn);
	void Search(CString strIn, BOOL bNewWindow);

	void SetWindowTitle(LPCTSTR ptr);
	UINT m_RendererPID;
	BOOL OnSetUrlString(LPCTSTR lParam);
	void SetProgress(long nCurr, long nTotal);
	void SetSSLStatus(BOOL bStatus);
	void OnCloseDelay();
	void OnWClose();
	void PostWM_CLOSE();

	CString GetViewLocationURLOnly();
	CPtrArray m_ptrAWnd;

protected:
	DECLARE_DYNAMIC(CBrowserFrame)
public:
	//{{AFX_VIRTUAL(CBrowserFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
public:
	virtual ~CBrowserFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	CChildView m_wndView;
	afx_msg void OnSetAddressbar();
	afx_msg void OnSetSearchbar();

protected:
	//{{AFX_MSG(CBrowserFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnClose();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	//}}AFX_MSG
	afx_msg LRESULT OnToolbarReset(WPARAM, LPARAM);
	afx_msg void OnStatusBarDoubleClick();
	afx_msg void OnStatusBarZoomClick();
	afx_msg void OnStatusBarCompatClick();
	afx_msg void OnAppExitEx();
	afx_msg void OnAppExitExBT();
	LRESULT OnNewAddressEnter(WPARAM wParam, LPARAM lParam);
	LRESULT OnNewAddress(WPARAM wParam, LPARAM lParam);
	LRESULT OnFavoriteAddSendMsg(WPARAM wParam, LPARAM lParam);
	LRESULT OnSearchString(WPARAM wParam, LPARAM lParam);
	LRESULT OnPowerBroadcast(WPARAM wp, LPARAM);
	afx_msg LRESULT OnChangeVisualManager(WPARAM wParam, LPARAM lParam);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnActiveFrm();

	DECLARE_MESSAGE_MAP()
	void OnWndMenu(UINT nID);
	void OnFavMenu(UINT nID);
	void OnCloseWndMenu(UINT nID);
	void OnSelTab(UINT nID);
	CWindowStatusHelper m_WindwMgrClass;

	void CreateWindowMenuSync(CMenu* pMenu);
	void CreateFavoriteMenuSync(CMenu* pMenu);
	void CrateFavoriteMenu(CMenu* pMenu, CFavoriteItem* parentItem);

public:
	void InitVisualManager();
	BOOL m_bDownloadProgress;
	BOOL m_bDownloadBlankPage;
	ProgressDlg* m_pDlgMsgP;
	void Init_MsgDlg(BOOL bShow = TRUE)
	{
		Release_MsgDlg();
		m_pDlgMsgP = new ProgressDlg(this);
		m_pDlgMsgP->Create(MAKEINTRESOURCE(IDD_DIALOG1), this);
		if (bShow)
		{
			if (IsIconic())
				m_pDlgMsgP->ShowWindow(SW_HIDE);
			else if (!theApp.IsWndVisible(this->m_hWnd))
				m_pDlgMsgP->ShowWindow(SW_HIDE);
			else
				m_pDlgMsgP->ShowWindow(SW_SHOW);
		}
	}
	void CountSet_MsgDlg(DWORD dCnt)
	{
		if (m_pDlgMsgP)
		{
			m_pDlgMsgP->m_iCntg = dCnt;
		}
	}
	void SetMessage_MsgDlg(LPCTSTR lpStr)
	{
		if (m_pDlgMsgP)
		{
			m_pDlgMsgP->SetMsg(lpStr);
		}
	}
	void SetMessageExec_MsgDlg(LPCTSTR lpStr)
	{
		if (m_pDlgMsgP)
		{
			m_pDlgMsgP->SetMsgExec(lpStr);
		}
	}
	void Show_MsgDlg(BOOL bShow)
	{
		if (m_pDlgMsgP)
		{
			if (bShow)
			{
				if (IsIconic())
					m_pDlgMsgP->ShowWindow(SW_HIDE);

				else if (!theApp.IsWndVisible(this->m_hWnd))
					m_pDlgMsgP->ShowWindow(SW_HIDE);

				else
					m_pDlgMsgP->ShowWindow(SW_SHOW);
			}
			else
				m_pDlgMsgP->ShowWindow(SW_HIDE);
		}
	}
	void Release_MsgDlg()
	{
		if (m_pDlgMsgP)
		{
			m_pDlgMsgP->DestroyWindow();
			delete m_pDlgMsgP;
			m_pDlgMsgP = NULL;
		}
	}

public:
	void OnFavoriteRefresh();

	afx_msg void OnDestroy();
	afx_msg void OnEnterSizeMove();
	afx_msg void OnExitSizeMove();
	LRESULT OnTabNotify(WPARAM wp, LPARAM lp);

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int cx, int cy);
	afx_msg void OnNcLButtonDown(UINT nType, CPoint pt);
	afx_msg void OnCaptureChanged(CWnd* pWnd);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);

	afx_msg void OnFavoriteAdd();
	afx_msg void OnFavoriteOrganize();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnFullScreen();
	afx_msg int OnMouseActivate(CWnd* pDetelnetopWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnPrevWnd();
	afx_msg void OnNextWnd();
	afx_msg void OnTabCloseRight();
	afx_msg void OnTabCloseLeft();
	afx_msg void OnTabListShow();
	afx_msg void OnRestoreWnd();
	afx_msg void OnSaveWnd();
	afx_msg void OpenThinFiler();
	afx_msg void OnAppDeleteCache();
};

class CNilButton : public CStatic
{
public:
	CNilButton()
	{
		m_bImage = FALSE;
		m_pParent = NULL;
		m_pView = NULL;
	}
	CWnd* m_pParent;
	CWnd* m_pView;
	BOOL m_bImage;

public:
	CImageList imagelist;
	CSize size;
	BOOL Init(int i, CWnd* pParent, CWnd* pView);
	void MoveWnd(int cx, int cy);
	CRect m_Rc;

public:
	virtual ~CNilButton(){};

protected:
	//{{AFX_MSG(CMDIClientWnd)
	virtual afx_msg void OnPaint();
	virtual afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) { return CStatic::OnSetCursor(pWnd, nHitTest, message); }
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CCloseNilButton : public CNilButton
{
public:
	CCursorHelper m_hCursor;
	CCloseNilButton() {}
	virtual ~CCloseNilButton() {}
	virtual void Init(int i, CWnd* pParent, CWnd* pView)
	{
		CBitmap bitmap;
		bitmap.LoadBitmap(IDB_BITMAP5);
		BITMAP bm = {0};
		if (bitmap.m_hObject != NULL)
		{
			bitmap.GetBitmap(&bm);
			size.cx = bm.bmWidth;
			size.cy = bm.bmHeight;
			imagelist.Create(size.cx, size.cy, ILC_COLOR24 | ILC_MASK, 6, 1);
			imagelist.Add(&bitmap, RGB(255, 0, 255));
			bitmap.DeleteObject();
		}
		m_pParent = pParent;
		m_pView = pView;
		m_bImage = TRUE;
		if (theApp.m_ScaleDPI > 1)
		{
			double dScale = 0.0;
			dScale = size.cx * theApp.m_ScaleDPI;
			size.cx = (int)dScale;
			dScale = size.cy * theApp.m_ScaleDPI;
			size.cy = (int)dScale;
		}
		CRect r(0, 0, size.cx, size.cy);
		CStatic::Create(NULL, WS_CHILD, r, pParent, i);
		m_hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(32649));
	}
	virtual void MoveWnd(CRect& rc)
	{
		this->SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
		if (m_Rc != rc)
		{
			this->InvalidateRect(NULL, TRUE);
			m_Rc = rc;
		}
	}
	virtual void OnPaint()
	{
		if (!m_bImage) return;
		CPaintDC dc(this);
		CPoint pos;
		CRect rect;
		GetClientRect(&rect);
		pos.x = rect.left;
		pos.y = rect.top;
		imagelist.Draw(&dc, 0, pos, ILD_DPISCALE | ILD_NORMAL); //ILD_TRANSPARENT);
	}
	virtual BOOL OnEraseBkgnd(CDC* pDC) { return TRUE; }
	virtual afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
	{
		if (nHitTest == HTCLIENT)
		{
			::SetCursor(m_hCursor);
			return TRUE;
		}
		return CStatic::OnSetCursor(pWnd, nHitTest, message);
	}
};
class CNewTabButton : public CCloseNilButton
{
public:
	CNewTabButton() {}
	virtual ~CNewTabButton() {}
	void Init(int i, CWnd* pParent, CWnd* pView)
	{
		CBitmap bitmap;
		bitmap.LoadBitmap(IDB_BITMAP6);
		BITMAP bm = {0};
		if (bitmap.m_hObject != NULL)
		{
			bitmap.GetBitmap(&bm);
			size.cx = bm.bmWidth;
			size.cy = bm.bmHeight;
			//size=bitmap.GetBitmapDimension();
			imagelist.Create(size.cx, size.cy, ILC_COLOR24 | ILC_MASK, 6, 1);
			imagelist.Add(&bitmap, RGB(255, 0, 255));
			bitmap.DeleteObject();
		}
		if (theApp.m_ScaleDPI > 1)
		{
			double dScale = 0.0;
			dScale = size.cx * theApp.m_ScaleDPI;
			size.cx = (int)dScale;
			dScale = size.cy * theApp.m_ScaleDPI;
			size.cy = (int)dScale;
		}

		m_pParent = pParent;
		m_pView = pView;
		m_bImage = TRUE;
		CRect r(0, 0, size.cx, size.cy);
		CStatic::Create(NULL, WS_CHILD, r, pParent, i);
		m_hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(32649));
	}
	void MoveWnd(CRect& rc)
	{
		this->SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
		if (m_Rc != rc)
		{
			this->InvalidateRect(NULL, TRUE);
			m_Rc = rc;
		}
	}
	void OnPaint()
	{
		if (!m_bImage) return;
		CPaintDC dc(this);
		CPoint pos;
		CRect rect;
		GetClientRect(&rect);
		pos.x = rect.left;
		pos.y = rect.top;
		imagelist.Draw(&dc, 0, pos, ILD_DPISCALE | ILD_TRANSPARENT); //ILD_TRANSPARENT);
	}
	BOOL OnEraseBkgnd(CDC* pDC)
	{
		return TRUE;
	}
	virtual afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
	{
		if (nHitTest == HTCLIENT)
		{
			::SetCursor(m_hCursor);
			return TRUE;
		}
		return CStatic::OnSetCursor(pWnd, nHitTest, message);
	}
};

class CActiveTabLine : public CCloseNilButton
{
public:
	CActiveTabLine() { m_bProgress = FALSE; }
	virtual ~CActiveTabLine() {}
	BOOL m_bProgress;
	void Init(int i, CWnd* pParent, CWnd* pView)
	{
		m_pParent = pParent;
		m_pView = pView;
		m_bImage = TRUE;
		CRect r(0, 0, 8, 100);
		CStatic::Create(NULL, WS_CHILD, r, pParent, i);
	}
	void MoveWnd(CRect& rc)
	{
		if (m_Rc != rc)
		{
			this->InvalidateRect(NULL, TRUE);
			this->SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
			m_Rc = rc;
		}
	}
	void OnPaint()
	{
		if (!m_bImage) return;

		CPaintDC dc(this);
		CPoint pos;
		CRect rect;
		GetClientRect(&rect);
		pos.x = rect.left;
		pos.y = rect.top;
		if (m_bProgress)
		{
			// ブラシの作成
			CBrush brush(RGB(34, 177, 76));
			// 新しいブラシを指定(元のブラシの保存)
			CBrush* pOldBrush = dc.SelectObject(&brush);
			// 塗りつぶす
			dc.PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
			// ブラシを元に戻す
			dc.SelectObject(pOldBrush);
		}
		else
		{
			// ブラシの作成
			CBrush brush(RGB(10, 132, 255));
			// 新しいブラシを指定(元のブラシの保存)
			CBrush* pOldBrush = dc.SelectObject(&brush);
			// 塗りつぶす
			dc.PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
			// ブラシを元に戻す
			dc.SelectObject(pOldBrush);
		}
	}
	BOOL OnEraseBkgnd(CDC* pDC)
	{
		return TRUE;
	}
};

class CMyStatusBar : public CMFCStatusBar
{
	DECLARE_DYNAMIC(CMyStatusBar)
public:
	CMyStatusBar() {}
	virtual ~CMyStatusBar() {}
	void SetPaneIcon(int nIndex, HICON hIcon, BOOL bUpdate, BOOL bShow = TRUE)
	{
		if (!bShow)
			return;
		CMFCStatusBar::SetPaneIcon(nIndex, hIcon, bUpdate);
		return;
	}
	long GetPainProgressTotal(int Index)
	{
		CMFCStatusBarPaneInfo* pSBP = _GetPanePtr(Index);
		return pSBP->nProgressTotal;
	}

protected:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};
class CMyToolBar : public CMFCToolBar
{
	DECLARE_DYNAMIC(CMyToolBar)
public:
	CMyToolBar() {}
	virtual ~CMyToolBar() {}
	virtual BOOL Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
	{
		return CreateEx(pParentWnd, TBSTYLE_FLAT, dwStyle, CRect(0, 0, 0, 0), nID);
	}
	virtual BOOL CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle, DWORD dwStyle, CRect rcBorders, UINT nID)
	{
		SetBorders(rcBorders);

		// Save the style:
		m_dwStyle = (dwStyle & CBRS_ALL);

		SetPaneAlignment(m_dwStyle);
		if (nID == AFX_IDW_TOOLBAR)
		{
			m_dwStyle |= CBRS_HIDE_INPLACE;
		}

		dwStyle &= ~CBRS_ALL;
		dwStyle = dwStyle | CBRS_SIZE_FIXED;
		dwStyle |= CCS_NOPARENTALIGN | CCS_NOMOVEY | CCS_NODIVIDER | CCS_NORESIZE;
		dwStyle |= dwCtrlStyle;

		CRect rect;
		rect.SetRectEmpty();

		if (!CMFCBaseToolBar::Create(
			GetGlobalData()->RegisterWindowClass(_T("CSG:ToolBar")), dwStyle, rect, pParentWnd, nID, 0))
		{
			return FALSE;
		}

		return TRUE;
	}

protected:
	DECLARE_MESSAGE_MAP()
	void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
};
class CMyMenuBar : public CMFCMenuBar
{
	DECLARE_DYNAMIC(CMyMenuBar)
public:
	CMyMenuBar() {}
	virtual ~CMyMenuBar() {}
	DECLARE_MESSAGE_MAP()
	void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
};
class CMyReBar : public CMFCReBar
{
	DECLARE_DYNAMIC(CMyReBar)
public:
	CMyReBar() {}
	virtual ~CMyReBar() {}
	DECLARE_MESSAGE_MAP()
protected:
	virtual BOOL OnEraseBkgnd(CDC* pDC)
	{
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
		//return FALSE;
	}
};
