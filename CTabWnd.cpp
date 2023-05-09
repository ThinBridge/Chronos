#include "StdAfx.h"
#include "Sazabi.h"
#include "MainFrm.h"
#include "BroFrame.h"
#include "BroView.h"
#include <limits.h>
#include "CWnd.h"
#include "CTabWnd.h"

#ifndef SPI_GETFOREGROUNDLOCKTIMEOUT
#define SPI_GETFOREGROUNDLOCKTIMEOUT 0x2000
#endif
#ifndef SPI_SETFOREGROUNDLOCKTIMEOUT
#define SPI_SETFOREGROUNDLOCKTIMEOUT 0x2001
#endif

#define TAB_MARGIN_TOP	 1
#define TAB_MARGIN_LEFT	 1
#define TAB_MARGIN_RIGHT 1
//#define TAB_FONT_HEIGHT		12
#define TAB_ITEM_HEIGHT	  25
#define TAB_WINDOW_HEIGHT 35
#define MAX_TABITEM_WIDTH 180
#define MIN_TABITEM_WIDTH 26

#define CX_SMICON 16
#define CY_SMICON 16

static const RECT rcBtnBase = {0, 0, 16, 16};

WNDPROC gm_pOldWndProc = NULL;

inline LRESULT CALLBACK DefTabWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (gm_pOldWndProc)
		return ::CallWindowProc(gm_pOldWndProc, hwnd, uMsg, wParam, lParam);
	else
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK TabWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CTabWnd* pcTabWnd = NULL;

	pcTabWnd = (CTabWnd*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (pcTabWnd)
	{
		if (0L == pcTabWnd->TabWndDispatchEvent(hwnd, uMsg, wParam, lParam))
			return 0L;
	}

	return DefTabWndProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CTabWnd::TabWndDispatchEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		return OnTabLButtonDown(wParam, lParam);

	case WM_LBUTTONUP:
		return OnTabLButtonUp(wParam, lParam);

	case WM_MOUSEMOVE:
		return OnTabMouseMove(wParam, lParam);

	case WM_TIMER:
		return OnTabTimer(wParam, lParam);

	case WM_CAPTURECHANGED:
		return OnTabCaptureChanged(wParam, lParam);

	case WM_RBUTTONDOWN:
		return OnTabRButtonDown(wParam, lParam);

	case WM_RBUTTONUP:
		return OnTabRButtonUp(wParam, lParam);

	case WM_MBUTTONDOWN:
		return OnTabMButtonDown(wParam, lParam);

	case WM_MBUTTONUP:
		return OnTabMButtonUp(wParam, lParam);

	case WM_NOTIFY:
		return OnTabNotify(wParam, lParam);

	case WM_HSCROLL:
	{
		::InvalidateRect(GetHwnd(), NULL, TRUE);
		break;
	}
	}
	return 1L;
}

LRESULT CTabWnd::OnTabLButtonDown(WPARAM wParam, LPARAM lParam)
{
	// �{�^���������ꂽ�ʒu���m�F����
	TCHITTESTINFO hitinfo = {0};
	hitinfo.pt.x = LOWORD((DWORD)lParam);
	hitinfo.pt.y = HIWORD((DWORD)lParam);
	int nSrcTab = TabCtrl_HitTest(m_hwndTab, (LPARAM)&hitinfo);
	if (0 > nSrcTab)
	{
		return 1L;
	}

	int nCurSel = TabCtrl_GetCurSel(m_hwndTab);
	if (nCurSel != nSrcTab)
	{
		if (theApp.IsWnd(theApp.m_wndpClose))
		{
			POINT pt = {0};
			::GetCursorPos(&pt);
			m_pwndFrame->ScreenToClient(&pt);
			if (theApp.m_wndpClose->m_Rc.PtInRect(pt))
			{
				HWND hCloseWnd = NULL;
				TCITEM tcitem = {0};
				tcitem.mask = TCIF_PARAM;
				tcitem.lParam = 0;
				TabCtrl_GetItem(m_hwndTab, nSrcTab, &tcitem);
				hCloseWnd = (HWND)tcitem.lParam;
				if (IsWindow(hCloseWnd))
				{
					::PostMessage(hCloseWnd, WM_COMMAND, ID_W_CLOSE, 0);
				}
				return 0L;
			}
		}
	}
	else
	{
		if (theApp.IsWnd(theApp.m_wndpClose))
		{
			POINT pt = {0};
			::GetCursorPos(&pt);
			m_pwndFrame->ScreenToClient(&pt);
			if (theApp.m_wndpClose->m_Rc.PtInRect(pt))
			{
				HWND hCloseWnd = NULL;
				TCITEM tcitem = {0};
				tcitem.mask = TCIF_PARAM;
				tcitem.lParam = 0;
				TabCtrl_GetItem(m_hwndTab, nSrcTab, &tcitem);
				hCloseWnd = (HWND)tcitem.lParam;
				if (IsWindow(hCloseWnd))
				{
					::PostMessage(hCloseWnd, WM_COMMAND, ID_W_CLOSE, 0);
				}
				return 0L;
			}
		}
	}
	// �}�E�X�h���b�O�J�n����
	m_eDragState = DragState::CHECK; // �h���b�O�̃`�F�b�N���J�n
	// �h���b�O���^�u���L������
	m_nSrcTab = nSrcTab;
	::GetCursorPos(&m_ptSrcCursor);
	::SetCapture(m_hwndTab);
	return 0L;
}

LRESULT CTabWnd::OnTabLButtonUp(WPARAM wParam, LPARAM lParam)
{
	TCHITTESTINFO hitinfo = {0};
	hitinfo.pt.x = LOWORD((DWORD)lParam);
	hitinfo.pt.y = HIWORD((DWORD)lParam);
	int nDstTab = TabCtrl_HitTest(m_hwndTab, (LPARAM)&hitinfo);
	int nSelfTab = FindTabIndexByHWND(m_pwndFrame->m_hWnd);

	// �}�E�X�h���b�v����
	switch (m_eDragState)
	{
	case DragState::CHECK:
	{
		int nCurSel = TabCtrl_GetCurSel(m_hwndTab);
		if (nCurSel != nDstTab)
		{
			BOOL bActive = TRUE;
			if (theApp.IsWnd(theApp.m_wndpClose))
			{
				POINT pt = {0};
				::GetCursorPos(&pt);
				m_pwndFrame->ScreenToClient(&pt);
				if (theApp.m_wndpClose->m_Rc.PtInRect(pt))
				{
					bActive = FALSE;
					BreakDrag();
				}
			}
			if (bActive)
			{
				//�w��̃E�C���h�E���A�N�e�B�u��
				TCITEM tcitem = {0};
				tcitem.mask = TCIF_PARAM;
				tcitem.lParam = 0;
				TabCtrl_GetItem(m_hwndTab, nDstTab, &tcitem);
				ShowTabWindow((HWND)tcitem.lParam);
				SendMessage((HWND)tcitem.lParam, WM_SETREDRAW, true, 0);
				BreakDrag();
				return 1;
			}
		}
		break;
	}
	case DragState::DRAG:
		m_pwndMainFrame->TabWindowMsgBSend(TWNT_REFRESH, NULL);

		if (m_nTabBorderArray)
		{
			delete[] m_nTabBorderArray;
			m_nTabBorderArray = NULL;
		}
		break;

	default:
		break;
	}
	BreakDrag();
	return 0L;
}

LRESULT CTabWnd::OnTabMouseMove(WPARAM wParam, LPARAM lParam)
{
	TCHITTESTINFO hitinfo = {0};
	int i = 0;
	int nTabCount = 0;
	hitinfo.pt.x = LOWORD((DWORD)lParam);
	hitinfo.pt.y = HIWORD((DWORD)lParam);
	int nDstTab = TabCtrl_HitTest(m_hwndTab, (LPARAM)&hitinfo);

	if (::GetCapture() != m_hwndTab)
	{
		int nTabHoverPrev = m_nTabHover;
		int nTabHoverCur = nDstTab;
		RECT rcPrev = {0};
		RECT rcCur = {0};
		TabCtrl_GetItemRect(m_hwndTab, nTabHoverPrev, &rcPrev);
		TabCtrl_GetItemRect(m_hwndTab, nTabHoverCur, &rcCur);

		m_nTabHover = nTabHoverCur;
		if (nTabHoverCur >= 0)
		{
			if (nTabHoverPrev < 0)
			{
				::SetTimer(m_hwndTab, 1, 200, NULL);
			}
		}
		else
		{
			::KillTimer(m_hwndTab, 1);
		}
	}

	// �}�E�X�h���b�O���̏���
	switch (m_eDragState)
	{
	case DragState::CHECK:
		// ���̃^�u���痣�ꂽ��h���b�O�J�n
		if (m_nSrcTab == nDstTab)
			break;
		m_eDragState = DragState::DRAG;
		m_hDefaultCursor = ::GetCursor();

		// ���݂̃^�u���E�ʒu���L������
		nTabCount = TabCtrl_GetItemCount(m_hwndTab);
		if (m_nTabBorderArray)
		{
			delete[] m_nTabBorderArray;
		}
		m_nTabBorderArray = new LONG[nTabCount];
		memset(m_nTabBorderArray, 0x00, sizeof(LONG) * nTabCount);
		for (i = 0; i < nTabCount - 1; i++)
		{
			RECT rc = {0};
			TabCtrl_GetItemRect(m_hwndTab, i, &rc);
			m_nTabBorderArray[i] = rc.right;
		}
		m_nTabBorderArray[i] = 0; // �Ō�̗v�f�͔ԕ�
					  // �����ɗ�����h���b�O�J�n�Ȃ̂� break ���Ȃ��ł��̂܂� DRAG_DRAG �����ɓ���

	case DragState::DRAG:
		// �h���b�O���̃}�E�X�J�[�\����\������
		HINSTANCE hInstance;
		LPCTSTR lpCursorName;
		lpCursorName = IDC_NO; // �֎~�J�[�\��
		if (0 <= nDstTab)      // �^�u�̏�ɃJ�[�\��������
		{
			lpCursorName = NULL; // �J�n���J�[�\���w��

			// �h���b�O�J�n���̃^�u�ʒu�ňړ���^�u���Čv�Z
			for (nDstTab = 0; m_nTabBorderArray[nDstTab] != 0; nDstTab++)
			{
				if (hitinfo.pt.x < m_nTabBorderArray[nDstTab])
				{
					break;
				}
			}
			// �h���b�O���ɑ����ړ�
			if (m_nSrcTab != nDstTab)
			{
				RECT rc = {0};
				TabCtrl_GetItemRect(m_hwndTab, nDstTab, &rc);
				if (rc.left > 0)
				{
					ReorderTab(m_nSrcTab, nDstTab);
					Refresh(FALSE);
					m_nSrcTab = nDstTab;
					::InvalidateRect(GetHwnd(), NULL, TRUE);
				}
			}
		}
		if (lpCursorName)
		{
			hInstance = (lpCursorName == IDC_NO) ? NULL : ::GetModuleHandle(NULL);
			::SetCursor(::LoadCursor(hInstance, lpCursorName));
		}
		else
		{
			::SetCursor(m_hDefaultCursor);
		}
		break;

	default:
		return 1L;
	}
	return 0L;
}

LRESULT CTabWnd::OnTabTimer(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 1)
	{
		// �J�[�\�����^�u�O�ɂ���ꍇ�ɂ� WM_MOUSEMOVE �𑗂�
		TCHITTESTINFO hitinfo = {0};
		::GetCursorPos(&hitinfo.pt);
		::ScreenToClient(m_hwndTab, &hitinfo.pt);
		int nDstTab = TabCtrl_HitTest(m_hwndTab, (LPARAM)&hitinfo);
		if (nDstTab < 0)
			::SendMessage(m_hwndTab, WM_MOUSEMOVE, 0, MAKELONG(hitinfo.pt.x, hitinfo.pt.y));
	}

	return 0L;
}

LRESULT CTabWnd::OnTabCaptureChanged(WPARAM wParam, LPARAM lParam)
{
	if (m_eDragState != DragState::NONE)
		m_eDragState = DragState::NONE;
	return 0L;
}

LRESULT CTabWnd::OnTabRButtonDown(WPARAM wParam, LPARAM lParam)
{
	BreakDrag();
	return 0L;
}

LRESULT CTabWnd::OnTabRButtonUp(WPARAM wParam, LPARAM lParam)
{
	TCHITTESTINFO hitinfo = {0};
	int i = 0;
	int nTabCount = 0;
	hitinfo.pt.x = LOWORD((DWORD)lParam);
	hitinfo.pt.y = HIWORD((DWORD)lParam);
	int nDstTab = TabCtrl_HitTest(m_hwndTab, (LPARAM)&hitinfo);
	int nCurSel = TabCtrl_GetCurSel(m_hwndTab);

	if (nDstTab == nCurSel)
	{
		CRect rcOver(0, 0, 0, 0);
		::SendMessage(m_hwndTab, TCM_GETITEMRECT, nDstTab, (LPARAM)&rcOver);
		::ClientToScreen(m_hwndTab, (LPPOINT)&rcOver.left);
		::ClientToScreen(m_hwndTab, (LPPOINT)&rcOver.right);

		TCITEM tcitem = {0};
		tcitem.mask = TCIF_PARAM;
		tcitem.lParam = 0;
		TabCtrl_GetItem(m_hwndTab, nDstTab, &tcitem);
		CMenu menu;
		menu.LoadMenu(IDR_MENU_TAB);
		CMenu* menuSub = NULL;
		menuSub = menu.GetSubMenu(0);
		int lResult = TrackPopupMenuEx(menu.GetSubMenu(0)->m_hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
					       rcOver.left, rcOver.bottom - 4,
					       (HWND)tcitem.lParam, NULL);
		if (lResult > 0)
		{
			SendMessage((HWND)tcitem.lParam, WM_COMMAND, MAKEWPARAM(LOWORD(lResult), 0x0), 0);
		}
		return 1;
	}
	return 0;
}

LRESULT CTabWnd::OnTabMButtonDown(WPARAM wParam, LPARAM lParam)
{
	BreakDrag();
	return 0L;
}

LRESULT CTabWnd::OnTabMButtonUp(WPARAM wParam, LPARAM lParam)
{
	TCHITTESTINFO hitinfo = {0};
	int i = 0;
	int nTabCount = 0;
	hitinfo.pt.x = LOWORD((DWORD)lParam);
	hitinfo.pt.y = HIWORD((DWORD)lParam);
	int nDstTab = TabCtrl_HitTest(m_hwndTab, (LPARAM)&hitinfo);
	if (nDstTab > -1)
	{
		CRect rcOver(0, 0, 0, 0);
		::SendMessage(m_hwndTab, TCM_GETITEMRECT, nDstTab, (LPARAM)&rcOver);
		::ClientToScreen(m_hwndTab, (LPPOINT)&rcOver.left);
		::ClientToScreen(m_hwndTab, (LPPOINT)&rcOver.right);

		TCITEM tcitem = {0};
		tcitem.mask = TCIF_PARAM;
		tcitem.lParam = 0;
		TabCtrl_GetItem(m_hwndTab, nDstTab, &tcitem);
		HWND hCloseWnd = NULL;
		hCloseWnd = (HWND)tcitem.lParam;
		if (IsWindow(hCloseWnd))
		{
			::PostMessage(hCloseWnd, WM_COMMAND, ID_W_CLOSE, 0);
			return 1;
		}
		return 1;
	}
	return 1;
}

LRESULT CTabWnd::OnTabNotify(WPARAM wParam, LPARAM lParam)
{
	return 1L;
}

BOOL CTabWnd::ReorderTab(int nSrcTab, int nDstTab)
{
	TCITEM tcitem = {0};
	if (0 > nSrcTab || 0 > nDstTab || nSrcTab == nDstTab)
		return FALSE;

	if (m_pwndMainFrame->ReorderTab(nSrcTab, nDstTab))
		return FALSE;
	return TRUE;
}

CTabWnd::CTabWnd()
    : CWndSkr(_T("::CTabWnd")),
      m_eDragState(DragState::NONE),
      m_bHovering(FALSE),
      m_eCaptureSrc(CaptureSrc::NONE),
      m_nTabBorderArray(NULL),
      m_nSrcTab(0)
{
	m_hwndTab = NULL;
	gm_pOldWndProc = NULL;
	memset(m_szTextTip, 0x00, _countof(m_szTextTip));
	m_pwndMainFrame = NULL;
	m_pwndFrame = NULL;
	m_pwndView = NULL;
	m_WheelDCnt = 0;
	m_TAB_ITEM_HEIGHT = TAB_ITEM_HEIGHT;
	m_TAB_WINDOW_HEIGHT = TAB_WINDOW_HEIGHT;
	m_MAX_TABITEM_WIDTH = MAX_TABITEM_WIDTH;
	m_MIN_TABITEM_WIDTH = MIN_TABITEM_WIDTH;
	return;
}

CTabWnd::~CTabWnd()
{
	if (m_nTabBorderArray)
	{
		delete[] m_nTabBorderArray;
		m_nTabBorderArray = NULL;
	}
	return;
}

HWND CTabWnd::CreateTab(HINSTANCE hInstance, CWnd* pFrame, CWnd* pView, HWND hwndParent)
{
	m_pwndMainFrame = ((CMainFrame*)theApp.m_pMainWnd);
	m_pwndFrame = ((CBrowserFrame*)pFrame);
	m_pwndView = ((CChildView*)pView);
	LPCTSTR pszClassName = _T("CTabWnd");

	m_hwndTab = NULL;
	gm_pOldWndProc = NULL;
	m_eDragState = DragState::NONE;
	m_bHovering = FALSE;
	m_eCaptureSrc = CaptureSrc::NONE;

	RegisterWC(
	    hInstance,
	    NULL,			   // Handle to the class icon.
	    NULL,			   // Handle to a small icon
	    ::LoadCursor(NULL, IDC_ARROW), // Handle to the class cursor.
	    NULL,			   // Handle to the class background brush.
	    NULL,			   // Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
	    pszClassName		   // Pointer to a null-terminated string or is an atom.
	);

	RECT rcParent = {0};
	::GetWindowRect(hwndParent, &rcParent);

	if (theApp.m_ScaleDPI > 1)
	{
		double dScale = 0.0;
		dScale = TAB_ITEM_HEIGHT * theApp.m_ScaleDPI;
		m_TAB_ITEM_HEIGHT = (int)dScale;
		dScale = TAB_WINDOW_HEIGHT * theApp.m_ScaleDPI;
		m_TAB_WINDOW_HEIGHT = (int)dScale;
		dScale = MAX_TABITEM_WIDTH * theApp.m_ScaleDPI;
		m_MAX_TABITEM_WIDTH = (int)dScale;
		dScale = MIN_TABITEM_WIDTH * theApp.m_ScaleDPI;
		m_MIN_TABITEM_WIDTH = (int)dScale;
	}

	CWndSkr::Create(
	    hwndParent,
	    0,					     // extended window style
	    pszClassName,			     // Pointer to a null-terminated string or is an atom.
	    pszClassName,			     // pointer to window name
	    WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, // window style
	    CW_USEDEFAULT,			     // horizontal position of window
	    0,					     // vertical position of window
	    rcParent.right - rcParent.left,	     // window width
	    m_TAB_WINDOW_HEIGHT,		     // window height
	    NULL				     // handle to menu, or child-window identifier
	);

	//�^�u�E�C���h�E���쐬����B
	m_hwndTab = ::CreateWindow(
	    WC_TABCONTROL,
	    _T(""),
	    WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_TOOLTIPS | TCS_SINGLELINE | TCS_TABS | TCS_FOCUSNEVER | TCS_FORCELABELLEFT | TCS_FIXEDWIDTH,
	    TAB_MARGIN_LEFT,
	    TAB_MARGIN_TOP,
	    rcParent.right - rcParent.left - (TAB_MARGIN_LEFT + TAB_MARGIN_RIGHT),
	    m_TAB_WINDOW_HEIGHT,
	    GetHwnd(),
	    (HMENU)NULL,
	    GetAppInstance(),
	    (LPVOID)NULL);

	if (m_hwndTab)
	{
		::SetWindowLongPtr(m_hwndTab, GWLP_USERDATA, (LONG_PTR)this);
		gm_pOldWndProc = (WNDPROC)::SetWindowLongPtr(m_hwndTab, GWLP_WNDPROC, (LONG_PTR)TabWndProc);

		//�X�^�C����ύX����B
		UINT lngStyle;
		lngStyle = (UINT)::GetWindowLongPtr(m_hwndTab, GWL_STYLE);
		lngStyle &= ~(TCS_BUTTONS | TCS_MULTILINE);
		lngStyle |= TCS_SINGLELINE;
		lngStyle |= TCS_TABS | TCS_FOCUSNEVER | TCS_FIXEDWIDTH | TCS_FORCELABELLEFT;
		::SetWindowLongPtr(m_hwndTab, GWL_STYLE, lngStyle);
		TabCtrl_SetItemSize(m_hwndTab, m_MAX_TABITEM_WIDTH, m_TAB_ITEM_HEIGHT);

		HWND hwndToolTips;
		hwndToolTips = TabCtrl_GetToolTips(m_hwndTab);
		lngStyle = (UINT)::GetWindowLongPtr(hwndToolTips, GWL_STYLE);
		lngStyle |= TTS_ALWAYSTIP | TTS_NOPREFIX;
		::SetWindowLongPtr(hwndToolTips, GWL_STYLE, lngStyle);

		LOGFONT lf = {0};
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
		m_hFont.CreateFontIndirect(&lf);
		::SendMessage(m_hwndTab, WM_SETFONT, (WPARAM)m_hFont.GetSafeHandle(), MAKELPARAM(TRUE, 0));

		//Tab�쐬�����������̂ŁAList�ɒǉ��B
		this->m_pwndMainFrame->Add_TabWindow(m_pwndFrame);
		TabCtrl_SetImageList(m_hwndTab, theApp.m_imgFavIcons);

		Refresh();
	}
	return GetHwnd();
}

void CTabWnd::Close(void)
{
	if (GetHwnd())
	{
		if (gm_pOldWndProc)
		{
			::SetWindowLongPtr(m_hwndTab, GWLP_WNDPROC, (LONG_PTR)gm_pOldWndProc);
			gm_pOldWndProc = NULL;
		}

		::SetWindowLongPtr(m_hwndTab, GWLP_USERDATA, (LONG_PTR)NULL);

		this->DestroyWindow();
	}
}

LRESULT CTabWnd::OnSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (NULL == GetHwnd() || NULL == m_hwndTab) return 0L;
	LayoutTab();
	::InvalidateRect(GetHwnd(), NULL, FALSE);
	return 0L;
}

LRESULT CTabWnd::OnDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_hwndTab)
	{
		::DestroyWindow(m_hwndTab);
		m_hwndTab = NULL;
	}
	::KillTimer(hwnd, 1);
	_SetHwnd(NULL);

	return 0L;
}

LRESULT CTabWnd::OnLButtonDblClk(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CTabWnd::OnCaptureChanged(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_eCaptureSrc != CaptureSrc::NONE)
		m_eCaptureSrc = CaptureSrc::NONE;

	return 0L;
}

LRESULT CTabWnd::OnLButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CTabWnd::OnLButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt = {0};
	RECT rc = {0};
	RECT rcBtn = {0};

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	::GetClientRect(GetHwnd(), &rc);

	if (::GetCapture() == GetHwnd()) // ���E�B���h�E���}�E�X�L���v�`���[���Ă���?
	{
		// �L���v�`���[����
		m_eCaptureSrc = CaptureSrc::NONE;
		::ReleaseCapture();
	}

	return 0L;
}

LRESULT CTabWnd::OnRButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0L;
}

LRESULT CTabWnd::OnMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// �J�[�\�����E�B���h�E���ɓ�������^�C�}�[�N��
	// �E�B���h�E�O�ɏo����^�C�}�[�폜
	POINT pt = {0};
	RECT rc = {0};
	BOOL bHovering = {0};

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	::GetClientRect(hwnd, &rc);
	bHovering = ::PtInRect(&rc, pt);
	if (bHovering != m_bHovering)
	{
		m_bHovering = bHovering;
		if (m_bHovering)
			::SetTimer(hwnd, 1, 200, NULL);
		else
			::KillTimer(hwnd, 1);
	}
	return 0L;
}

LRESULT CTabWnd::OnTimer(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == 1)
	{
		// �J�[�\�����E�B���h�E�O�ɂ���ꍇ�ɂ� WM_MOUSEMOVE �𑗂�
		POINT pt = {0};
		RECT rc = {0};

		::GetCursorPos(&pt);
		::ScreenToClient(hwnd, &pt);
		::GetClientRect(hwnd, &rc);
		if (!::PtInRect(&rc, pt))
			::SendMessage(hwnd, WM_MOUSEMOVE, 0, MAKELONG(pt.x, pt.y));
	}

	return 0L;
}
LRESULT CTabWnd::OnMouseWheel(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	int zDelta = (SHORT)HIWORD(wParam);
	m_WheelDCnt += zDelta;
	if (m_WheelDCnt > 120 * 100)
		m_WheelDCnt = 0;
	else if (m_WheelDCnt < -120 * 100)
		m_WheelDCnt = 0;
	if (m_WheelDCnt > 0)
	{
		//120�P�ʂȂ̂�2��]��1��
		//if(m_WheelDCnt >= 120*2)
		if (m_WheelDCnt >= 120)
		{
			m_pwndFrame->OnPrevWnd();
			m_WheelDCnt = 0;
		}
	}
	else
	{
		//if (m_WheelDCnt <= -120*2)
		if (m_WheelDCnt <= -120)
		{
			m_pwndFrame->OnNextWnd();
			m_WheelDCnt = 0;
		}
	}
	return 1;
}

LRESULT CTabWnd::OnEraseBkgnd(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc = {0};
	PAINTSTRUCT ps = {0};
	RECT rc = {0};

	//�`��Ώ�
	hdc = ::BeginPaint(hwnd, &ps);

	// �w�i��`�悷��
	::GetClientRect(hwnd, &rc);
	::FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW));
	::EndPaint(hwnd, &ps);
	return 1;
}

LRESULT CTabWnd::OnPaint(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc = {0};
	PAINTSTRUCT ps = {0};
	RECT rc = {0};

	//�`��Ώ�
	hdc = ::BeginPaint(hwnd, &ps);
	int nCurSel = TabCtrl_GetCurSel(m_hwndTab);
	nCurSel = FindTabIndexByHWND(this->m_pwndFrame->GetSafeHwnd());
	if (nCurSel >= 0)
	{
		POINT pt = {0};
		RECT rcCurSel = {0};

		TabCtrl_GetItemRect(m_hwndTab, nCurSel, &rcCurSel);
		pt.x = rcCurSel.left;
		pt.y = 0;
		::ClientToScreen(m_hwndTab, &pt);
		::ScreenToClient(GetHwnd(), &pt);
		rcCurSel.right = pt.x + (rcCurSel.right - rcCurSel.left); // - 1;
		rcCurSel.left = pt.x;					  // + 1;
		rcCurSel.top = rc.top + TAB_MARGIN_TOP - 4;
		rcCurSel.bottom = rc.top + TAB_MARGIN_TOP;

		if (rcCurSel.left < rc.left + TAB_MARGIN_LEFT)
			rcCurSel.left = rc.left + TAB_MARGIN_LEFT; // ���[���E�l

		RECT rcTabTop = {0};
		::GetClientRect(this->GetHwnd(), &rcTabTop);
		rcTabTop.bottom = rcTabTop.top + 8;
		HBRUSH hBrb = ::CreateSolidBrush(RGB(207, 214, 229));
		::FillRect(hdc, &rcTabTop, hBrb);
		::DeleteObject(hBrb);

		if (rcCurSel.left < rcCurSel.right)
		{
			HBRUSH hBr = ::CreateSolidBrush(RGB(10, 132, 255)); //( RGB( 255, 128, 0 ) );
			::FillRect(hdc, &rcCurSel, hBr);
			::DeleteObject(hBr);
		}
	}

	::EndPaint(hwnd, &ps);

	return 0L;
}

LRESULT CTabWnd::OnNotify(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	NMHDR* pnmh = (NMHDR*)lParam;
	if (pnmh->hwndFrom == TabCtrl_GetToolTips(m_hwndTab))
	{
		switch (pnmh->code)
		{
		case TTN_GETDISPINFO:
			// �c�[���`�b�v�\������ݒ肷��
			TCITEM tcitem = {0};
			tcitem.mask = TCIF_PARAM;
			tcitem.lParam = (LPARAM)NULL;
			if (TabCtrl_GetItem(m_hwndTab, pnmh->idFrom, &tcitem))
			{
				CString strRet;
				strRet = this->m_pwndMainFrame->Get_TabWindowInfo_Title((HWND)tcitem.lParam);
				lstrcpyn(m_szTextTip, strRet, _countof(m_szTextTip) - 1);

				((NMTTDISPINFO*)pnmh)->lpszText = m_szTextTip;
				((NMTTDISPINFO*)pnmh)->hinst = NULL;
			}
			return 0L;
		}
	}
	return 0L;
}

void CTabWnd::TabWindowNotify(WPARAM wParam, LPARAM lParam)
{
	if (NULL == m_hwndTab) return;

	bool bFlag = false; //�O�񉽂��^�u���Ȃ��������H
	int nCount = {0};
	int nIndex = {0};
	HWND hwndUpDown = {0};
	DWORD nScrollPos = {0};

	BreakDrag();

	nCount = TabCtrl_GetItemCount(m_hwndTab);
	if (nCount <= 0)
	{
		bFlag = true;
		//�ŏ��̂Ƃ��͂��łɑ��݂���E�C���h�E�̏����o�^����K�v������B
		// �N�����ACTabWnd::Open()����Refresh()�ł͂܂��O���[�v����O�̂��ߊ��ɕʃE�B���h�E�������Ă��^�u�͋�
		if (wParam == TWNT_ADD)
			Refresh(); // ������TWNT_ADD�����Ŏ����ȊO�̃E�B���h�E���B��
	}

	switch (wParam)
	{
	case TWNT_ADD: //�E�C���h�E�o�^
		nIndex = FindTabIndexByHWND((HWND)lParam);
		if (-1 == nIndex)
		{
			TCITEM tcitem = {0};
			TCHAR szName[1024] = {0};

			_tcscpy_s(szName, _T("blank"));

			tcitem.mask = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE;
			tcitem.pszText = szName;
			tcitem.lParam = (LPARAM)lParam;
			tcitem.iImage = nCount;
			TabCtrl_InsertItem(m_hwndTab, nCount, &tcitem);
			nIndex = nCount;
		}
		//�����Ȃ�A�N�e�B�u��
		if (!theApp.IsWndVisible(m_pwndFrame->m_hWnd))
		{
			ShowTabWindow(m_pwndFrame->m_hWnd);
			//�����ɗ����Ƃ������Ƃ͂��łɃA�N�e�B�u
			//�R�}���h���s���̃A�E�g�v�b�g�Ŗ�肪����̂ŃA�N�e�B�u�ɂ���
		}
		TabCtrl_SetCurSel(m_hwndTab, nIndex);
		// �����ȊO���B��
		HideOtherWindows(m_pwndFrame->m_hWnd);
		break;

	case TWNT_DEL: //�E�C���h�E�폜
		nIndex = FindTabIndexByHWND((HWND)lParam);
		if (-1 != nIndex)
		{
			if (!theApp.IsWndVisible(m_pwndFrame->m_hWnd))
			{
				ShowTabWindow(m_pwndFrame->m_hWnd);
				ForceActiveWindow(m_pwndFrame->m_hWnd);
				::InvalidateRect(GetHwnd(), NULL, FALSE);
			}

			// �i�E�[�̂ق��̃^�u�A�C�e�����폜�����Ƃ��A�X�N���[���\�Ȃ̂ɉE�ɗ]�����ł��邱�Ƃւ̑΍�j
			hwndUpDown = ::FindWindowEx(m_hwndTab, NULL, UPDOWN_CLASS, 0); // �^�u���� Up-Down �R���g���[��
			if (hwndUpDown != NULL && ::IsWindowVisible(hwndUpDown))
			{
				nScrollPos = LOWORD(UpDown_GetPos(hwndUpDown));
				// ���݈ʒu nScrollPos �Ɖ�ʕ\���Ƃ���v������
				::SendMessage(m_hwndTab, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, LOWORD(nScrollPos)), (LPARAM)NULL); // �ݒ�ʒu�Ƀ^�u���X�N���[��
			}
		}
		break;

	case TWNT_ORDER: //�E�C���h�E�����ύX
		nIndex = FindTabIndexByHWND((HWND)lParam);
		if (-1 != nIndex)
		{
			//�����Ȃ�A�N�e�B�u��
			if (!theApp.IsWndVisible(m_pwndFrame->m_hWnd))
			{
				ShowTabWindow(m_pwndFrame->m_hWnd);
			}
			RECT rcTab = {0};
			nCount = 0;
			int cx = 0;
			::GetClientRect(m_hwndTab, &rcTab);
			nCount = TabCtrl_GetItemCount(m_hwndTab);
			if (0 < nCount)
			{
				cx = (rcTab.right - rcTab.left - 8) / nCount;
				int min = m_MIN_TABITEM_WIDTH;
				if (m_MAX_TABITEM_WIDTH < cx)
					cx = m_MAX_TABITEM_WIDTH;
				else if (m_MIN_TABITEM_WIDTH > cx)
					cx = m_MIN_TABITEM_WIDTH;

				TabCtrl_SetItemSize(m_hwndTab, cx, m_TAB_ITEM_HEIGHT);
			}
			// ���^�u�A�C�e���������I�ɉ��ʒu�ɂ��邽�߂ɁA
			// ���^�u�A�C�e���I��O�Ɉꎞ�I�ɉ�ʍ��[�̃^�u�A�C�e����I������
			nScrollPos = (hwndUpDown != NULL && ::IsWindowVisible(hwndUpDown)) ? LOWORD(UpDown_GetPos(hwndUpDown)) : 0;
			TabCtrl_SetCurSel(m_hwndTab, nScrollPos);
			TabCtrl_SetCurSel(m_hwndTab, nIndex);
			// �����ȊO���B��
			// �i�A���ؑ֎��� TWNT_ORDER ����ʔ����E�������āH��ʂ����ׂď����Ă��܂����肷��̂�h���j
			HideOtherWindows(m_pwndFrame->m_hWnd);
		}
		break;

	case TWNT_REFRESH: //�ĕ\��
		Refresh((BOOL)lParam);
		if (lParam)
			::InvalidateRect(this->GetHwnd(), NULL, FALSE);
		break;

	case TWNT_WNDPL_ADJUST: // �E�B���h�E�ʒu���킹
		AdjustWindowPlacement();
		LayoutTab();
		::InvalidateRect(this->GetHwnd(), NULL, FALSE);
		return;

	default:
		break;
	}

	return;
}

int CTabWnd::FindTabIndexByHWND(HWND hWnd)
{
	int i = {0};
	int nCount = {0};
	TCITEM tcitem = {0};

	if (NULL == m_hwndTab) return -1;

	nCount = TabCtrl_GetItemCount(m_hwndTab);
	for (i = 0; i < nCount; i++)
	{
		tcitem.mask = TCIF_PARAM;
		tcitem.lParam = (LPARAM)0;
		TabCtrl_GetItem(m_hwndTab, i, &tcitem);

		if ((HWND)tcitem.lParam == hWnd) return i;
	}

	return -1;
}

void CTabWnd::Refresh(BOOL bEnsureVisible /* = TRUE*/, BOOL bRebuild /* = FALSE*/)
{
	TCITEM tcitem = {0};
	int nCount = 0;
	int nGroup = 0;
	int nTab = 0;
	int nSel = 0;
	int nCurTab = 0;
	int nCurSel = 0;
	int i = 0;
	int j = 0;

	if (NULL == m_hwndTab) return;

	if (!m_pwndMainFrame)
		return;

	nCount = m_pwndMainFrame->Get_TabWindowCount();

	if (bRebuild)
		TabCtrl_DeleteAllItems(m_hwndTab); // �쐬���Ȃ���

	nTab = nCount;
	nSel = m_pwndMainFrame->Get_TabWindowIndex(this->m_pwndFrame);

	TCHAR szName[2048] = {0};
	_tcscpy_s(szName, _T("New Tab"));
	tcitem.mask = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE;
	tcitem.pszText = szName;
	tcitem.lParam = (LPARAM)m_pwndFrame->m_hWnd;
	if (TabCtrl_GetItemCount(m_hwndTab) == 0)
	{
		tcitem.iImage = 0;
		TabCtrl_InsertItem(m_hwndTab, 0, &tcitem);
		TabCtrl_SetCurSel(m_hwndTab, 0);
	}

	// �I���^�u�����O�̉ߕs���𒲐�����
	// �i�I���^�u�̒��O�ʒu�ւ̒ǉ��^�폜���J��Ԃ����ƂŃX�N���[��������ጸ�j
	nCurSel = TabCtrl_GetCurSel(m_hwndTab); // ���݂̑I���^�u�ʒu
	if (nCurSel > nSel)
	{
		for (i = 0; i < nCurSel - nSel; i++)
			TabCtrl_DeleteItem(m_hwndTab, nCurSel - 1 - i); // �]�����폜
	}
	else
	{
		for (i = 0; i < nSel - nCurSel; i++)
		{
			tcitem.iImage = nCurSel + i;
			TabCtrl_InsertItem(m_hwndTab, nCurSel + i, &tcitem); // �s����ǉ�
		}
	}

	// �I���^�u������̉ߕs���𒲐�����
	nCurTab = TabCtrl_GetItemCount(m_hwndTab); // ���݂̃^�u��
	if (nCurTab > nTab)
	{
		for (i = 0; i < nCurTab - nTab; i++)
			TabCtrl_DeleteItem(m_hwndTab, nSel + 1); // �]�����폜
	}
	else
	{
		for (i = 0; i < nTab - nCurTab; i++)
		{
			tcitem.iImage = nSel + 1;
			TabCtrl_InsertItem(m_hwndTab, nSel + 1, &tcitem); // �s����ǉ�
		}
	}

	// �쐬�����^�u�Ɋe�E�B���h�E����ݒ肷��
	CStringArray strATitle;
	CPtrArray ptrAWnd;
	m_pwndMainFrame->Get_TabWindowInfoArray(strATitle, ptrAWnd);

	size_t iTitleCnt = 0;
	iTitleCnt = strATitle.GetCount();
	size_t iWndCnt = 0;
	iWndCnt = ptrAWnd.GetCount();

	if (iTitleCnt == iWndCnt && iTitleCnt == nCount)
	{
		for (i = 0, j = 0; i < nCount; i++)
		{
			tcitem.mask = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE;
			CString strTempTitle = strATitle.GetAt(i);
			strTempTitle.Replace(_T("&"), _T("&&"));
			//�����̃J�b�g
			SBUtil::GetDivChar(strTempTitle, 24, strTempTitle, FALSE);
			lstrcpyn(szName, strTempTitle, _countof(szName) - 1);

			tcitem.pszText = szName;
			tcitem.iImage = j;
			HWND hWndTmp = {0};
			hWndTmp = (HWND)ptrAWnd.GetAt(i);
			tcitem.lParam = (LPARAM)hWndTmp;

			TabCtrl_SetItem(m_hwndTab, j, &tcitem);
			j++;
		}
	}
	else
	{
		ASSERT(FALSE);
	}

	RECT rcTab = {0};
	nCount = 0;
	int cx = 0;
	::GetClientRect(m_hwndTab, &rcTab);
	nCount = TabCtrl_GetItemCount(m_hwndTab);
	if (0 < nCount)
	{
		cx = (rcTab.right - rcTab.left - 8) / nCount;
		int min = m_MIN_TABITEM_WIDTH;
		if (m_MAX_TABITEM_WIDTH < cx)
			cx = m_MAX_TABITEM_WIDTH;
		else if (m_MIN_TABITEM_WIDTH > cx)
			cx = m_MIN_TABITEM_WIDTH;

		TabCtrl_SetItemSize(m_hwndTab, cx, m_TAB_ITEM_HEIGHT);
	}
	return;
}

void CTabWnd::AdjustWindowPlacement(void)
{
	HWND hwnd = m_pwndFrame->m_hWnd; // ���g�̕ҏW�E�B���h�E
	WINDOWPLACEMENT wp = {0};
	if (!theApp.IsWndVisible(hwnd)) // ��������Ƃ����������p��
	{
		theApp.ShowWnd(hwnd);
		wp.length = sizeof(wp);
		wp = theApp.GetActiveFrameWindowPlacement(); //m_ActiveFramePracement;
		if (wp.showCmd == SW_SHOWMINIMIZED)
			wp.showCmd = SW_RESTORE;
		//HWND hwndInsertAfter={0};
		//hwndInsertAfter = theApp.GetActiveBFramePtrHWND();
		//::SetWindowPos(hwnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		SetCarmWindowPlacement(hwnd, &wp); // �ʒu�𕜌�����
		//::UpdateWindow(hwnd);	// �����`��
	}
}

int CTabWnd::SetCarmWindowPlacement(HWND hwnd, const WINDOWPLACEMENT* pWndpl)
{
	WINDOWPLACEMENT wp = *pWndpl;
	if (wp.showCmd == SW_SHOWMAXIMIZED && ::IsZoomed(hwnd))
	{
		WINDOWPLACEMENT wpCur = {0};
		wpCur.length = sizeof(WINDOWPLACEMENT);
		::GetWindowPlacement(hwnd, &wpCur);
		if (!::EqualRect(&wp.rcNormalPosition, &wpCur.rcNormalPosition))
		{
			wp.showCmd = SW_SHOWNOACTIVATE;
			::SetWindowPlacement(hwnd, &wp);
			wp.showCmd = SW_SHOWMAXIMIZED;
		}
		else
		{
			wp.showCmd = SW_SHOWNA; // ���̂܂܍ő�\��
		}
	}
	else if (wp.showCmd != SW_SHOWMAXIMIZED)
	{
		wp.showCmd = SW_SHOWNOACTIVATE;
	}
	::SetWindowPlacement(hwnd, &wp);
	return wp.showCmd;
}

void CTabWnd::ShowTabWindow(HWND hwnd)
{
	if (NULL == hwnd) return;

	if (theApp.m_bTabWndChanging)
		return;			 // �ؑւ̍Œ�(busy)�͗v���𖳎�����
	theApp.m_bTabWndChanging = TRUE; //�E�B���h�E�ؑ֒�ON
	DWORD_PTR dwResult = {0};
	::SendMessageTimeout(hwnd, MYWM_TAB_WINDOW_NOTIFY, TWNT_WNDPL_ADJUST, (LPARAM)NULL,
			     SMTO_NORMAL, 10000, &dwResult);
	TabWnd_ActivateFrameWindow(hwnd);
	theApp.m_bTabWndChanging = FALSE; // �E�B���h�E�ؑ֒�OFF
	return;
}

void CTabWnd::HideOtherWindows(HWND hwndExclude)
{
	HWND hwnd = {0};
	if (!hwndExclude) return;

	this->m_pwndMainFrame->HideOtherTabWindows(hwndExclude);
	return;
}

void CTabWnd::ForceActiveWindow(HWND hwnd)
{
	int nId1 = {0};
	int nId2 = {0};
	DWORD dwTime = {0};

	nId2 = ::GetWindowThreadProcessId(::GetForegroundWindow(), NULL);
	nId1 = ::GetWindowThreadProcessId(hwnd, NULL);

	::AttachThreadInput(nId1, nId2, TRUE);

	::SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &dwTime, 0);
	::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)0, 0);

	theApp.ShowWnd(hwnd);
	//�E�B���h�E���t�H�A�O���E���h�ɂ���
	::SetForegroundWindow(hwnd);
	::BringWindowToTop(hwnd);

	::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)(INT_PTR)dwTime, 0);

	::AttachThreadInput(nId1, nId2, FALSE);
}

void CTabWnd::TabWnd_ActivateFrameWindow(HWND hwnd)
{
	// �Ώۂ�disable�̂Ƃ��͍ŋ߂̃|�b�v�A�b�v���t�H�A�O���E���h������
	HWND hwndActivate = {0};
	hwndActivate = ::IsWindowEnabled(hwnd) ? hwnd : ::GetLastActivePopup(hwnd);

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
	return;
}

void CTabWnd::LayoutTab(void)
{
	RECT rcTab = {0};
	int nCount = 0;
	int cx = 0;
	::GetClientRect(m_hwndTab, &rcTab);
	nCount = TabCtrl_GetItemCount(m_hwndTab);
	if (0 < nCount)
	{
		cx = (rcTab.right - rcTab.left - 8) / nCount;
		int min = m_MIN_TABITEM_WIDTH;
		if (m_MAX_TABITEM_WIDTH < cx)
			cx = m_MAX_TABITEM_WIDTH;
		else if (m_MIN_TABITEM_WIDTH > cx)
			cx = m_MIN_TABITEM_WIDTH;

		TabCtrl_SetItemSize(m_hwndTab, cx, m_TAB_ITEM_HEIGHT);
	}

	RECT rcWnd = {0};
	::GetWindowRect(GetHwnd(), &rcWnd);

	int nHeight = m_TAB_WINDOW_HEIGHT;
	::GetWindowRect(m_hwndTab, &rcTab);

	RECT rcDisp = rcTab;
	rcDisp.left = TAB_MARGIN_LEFT;
	rcDisp.right = rcTab.left + (rcWnd.right - rcWnd.left) - (TAB_MARGIN_LEFT + TAB_MARGIN_RIGHT);
	TabCtrl_AdjustRect(m_hwndTab, FALSE, &rcDisp);
	nHeight = (rcDisp.top - rcTab.top) + TAB_MARGIN_TOP + 2 + 10;
	::SetWindowPos(GetHwnd(), NULL, 0, 0, rcWnd.right - rcWnd.left, m_TAB_WINDOW_HEIGHT, SWP_NOMOVE | SWP_NOZORDER);
	int nWidth = (rcWnd.right - rcWnd.left) - (TAB_MARGIN_LEFT + TAB_MARGIN_RIGHT);
	if ((nWidth != rcTab.right - rcTab.left) || (nHeight != rcTab.bottom - rcTab.top))
	{
		::MoveWindow(m_hwndTab, TAB_MARGIN_LEFT, TAB_MARGIN_TOP, nWidth, nHeight, TRUE);
	}
}
