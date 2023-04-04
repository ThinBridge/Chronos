#pragma once

#include "CWnd.h"
//�^�u�E�C���h�E�p���b�Z�[�W
#define MYWM_TAB_WINDOW_NOTIFY (WM_APP + 213)

//�^�u�E�C���h�E�p���b�Z�[�W�T�u�R�}���h
enum ETabWndNotifyType
{
	TWNT_REFRESH = 0,      //�ĕ\��
	TWNT_ADD = 1,	       //�E�C���h�E�o�^
	TWNT_DEL = 2,	       //�E�C���h�E�폜
	TWNT_ORDER = 3,	       //�E�C���h�E�����ύX
	TWNT_WNDPL_ADJUST = 4, //�E�B���h�E�ʒu���킹
};
inline LRESULT UpDown_SetRange(HWND hwndCtl, int upper, int lower) { return (LRESULT)(ULONG_PTR)::SendMessage(hwndCtl, UDM_SETRANGE, 0L, MAKELPARAM(upper, lower)); }
inline LRESULT UpDown_GetPos(HWND hwndCtl) { return (LRESULT)(ULONG_PTR)::SendMessage(hwndCtl, UDM_GETPOS, 0L, 0L); }
inline LRESULT UpDown_SetPos(HWND hwndCtl, DWORD value) { return (LRESULT)(ULONG_PTR)::SendMessage(hwndCtl, UDM_SETPOS, 0L, MAKELONG(value, 0)); }

inline BOOL Tooltip_AddTool(HWND hwndCtl, TOOLINFO* info) { return (BOOL)(DWORD)::SendMessage(hwndCtl, TTM_ADDTOOL, 0L, (LPARAM)info); }
inline int Tooltip_SetMaxTipWidth(HWND hwndCtl, int width) { return (int)(DWORD)::SendMessage(hwndCtl, TTM_SETMAXTIPWIDTH, 0L, (LPARAM)width); }
inline void Tooltip_UpdateTipText(HWND hwndCtl, TOOLINFO* info) { ::SendMessage(hwndCtl, TTM_UPDATETIPTEXT, 0L, (LPARAM)info); }
inline void Tooltip_Activate(HWND hwndCtl, BOOL enable) { ::SendMessage(hwndCtl, TTM_ACTIVATE, (WPARAM)enable, (LPARAM)0); }

static void ActivateFrameWindow(HWND hwnd)
{
	if (theApp.m_bTabWndChanging)
		return; // �ؑւ̍Œ�(busy)�͗v���𖳎�����
	theApp.m_bTabWndChanging = TRUE;
	DWORD_PTR dwResult = 0;
	::SendMessageTimeout(
	    hwnd,
	    MYWM_TAB_WINDOW_NOTIFY,
	    TWNT_WNDPL_ADJUST,
	    (LPARAM)NULL,
	    SMTO_NORMAL,
	    10000,
	    &dwResult);

	HWND hwndActivate;
	hwndActivate = hwnd;
	theApp.ShowWnd(hwnd);
	if (::IsIconic(hwnd))
	{
		::ShowWindow(hwnd, SW_RESTORE);
	}
	else if (::IsZoomed(hwnd))
	{
		::ShowWindow(hwnd, SW_MAXIMIZE);
	}
	else
	{
		::ShowWindow(hwnd, SW_SHOW);
	}
	::SetForegroundWindow(hwndActivate);
	::BringWindowToTop(hwndActivate);
	theApp.m_bTabWndChanging = FALSE;
	return;
}

class CMainFrame;
class CChildView;
class CBrowserFrame;

class CTabWnd : public CWndSkr
{
public:
	CTabWnd();
	virtual ~CTabWnd();

	HWND CreateTab(HINSTANCE, CWnd* pFrame, CWnd* pView, HWND hwndParent);
	void Close(void);
	void TabWindowNotify(WPARAM wParam, LPARAM lParam);
	void Refresh(BOOL bEnsureVisible = TRUE, BOOL bRebuild = FALSE);

	LRESULT TabWndDispatchEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT TabListMenu(POINT pt, BOOL bSel = TRUE, BOOL bFull = FALSE, BOOL bOtherGroup = TRUE);

	void OnSize()
	{
		OnSize(GetHwnd(), WM_SIZE, 0, 0);
	}
	void UpdateStyle();

	void ShowTabWindow(HWND hwnd);
	void LayoutTab(void);

protected:
	int FindTabIndexByHWND(HWND hWnd);
	void AdjustWindowPlacement(void);
	int SetCarmWindowPlacement(HWND hwnd, const WINDOWPLACEMENT* pWndpl);
	void HideOtherWindows(HWND hwndExclude);
	void ForceActiveWindow(HWND hwnd);
	void TabWnd_ActivateFrameWindow(HWND hwnd);

	virtual void AfterCreateWindow(void) {}

	virtual LRESULT OnSize(HWND, UINT, WPARAM, LPARAM);
	virtual LRESULT OnDestroy(HWND, UINT, WPARAM, LPARAM);
	virtual LRESULT OnNotify(HWND, UINT, WPARAM, LPARAM);
	virtual LRESULT OnPaint(HWND, UINT, WPARAM, LPARAM);
	virtual LRESULT OnEraseBkgnd(HWND, UINT, WPARAM, LPARAM);
	virtual LRESULT OnCaptureChanged(HWND, UINT, WPARAM, LPARAM);
	virtual LRESULT OnLButtonDown(HWND, UINT, WPARAM, LPARAM);
	virtual LRESULT OnLButtonUp(HWND, UINT, WPARAM, LPARAM);
	virtual LRESULT OnRButtonDown(HWND, UINT, WPARAM, LPARAM);
	virtual LRESULT OnLButtonDblClk(HWND, UINT, WPARAM, LPARAM);
	virtual LRESULT OnMouseMove(HWND, UINT, WPARAM, LPARAM);
	virtual LRESULT OnTimer(HWND, UINT, WPARAM, LPARAM);
	virtual LRESULT OnMouseWheel(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	int m_WheelDCnt;
	LRESULT OnTabLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnTabLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnTabMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnTabTimer(WPARAM wParam, LPARAM lParam);
	LRESULT OnTabCaptureChanged(WPARAM wParam, LPARAM lParam);
	LRESULT OnTabRButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnTabRButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnTabMButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnTabMButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnTabNotify(WPARAM wParam, LPARAM lParam);

	void BreakDrag(void)
	{
		if (::GetCapture() == m_hwndTab) ::ReleaseCapture();
		m_eDragState = DRAG_NONE;
	}
	BOOL ReorderTab(int nSrcTab, int nDstTab);

protected:
	enum DragState
	{
		DRAG_NONE,
		DRAG_CHECK,
		DRAG_DRAG
	};
	enum CaptureSrc
	{
		CAPT_NONE
	};

public:
	CFont m_hFont;
	HWND m_hwndTab;
	TCHAR m_szTextTip[1024];
	int m_TAB_ITEM_HEIGHT;
	int m_TAB_WINDOW_HEIGHT;
	int m_MAX_TABITEM_WIDTH;
	int m_MIN_TABITEM_WIDTH;

private:
	DragState m_eDragState;
	int m_nSrcTab;
	POINT m_ptSrcCursor;
	HCURSOR m_hDefaultCursor;
	BOOL m_bHovering;
	CaptureSrc m_eCaptureSrc;
	LONG* m_nTabBorderArray;
	//LOGFONT		m_lf;

	int m_nTabHover;
	CMainFrame* m_pwndMainFrame;
	CBrowserFrame* m_pwndFrame;
	CChildView* m_pwndView;

private:
	DISALLOW_COPY_AND_ASSIGN(CTabWnd);
};
