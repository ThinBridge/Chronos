#include "stdafx.h"
#include "Sazabi.h"
#include "MyComboBoxEx.h"
#include "BroFrame.h"
#include "BroView.h"
#include "MainFrm.h"
#include "sbcommon.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMyComboBoxEx::CMyComboBoxEx()
{
	m_pParentWnd = NULL;
}

CMyComboBoxEx::~CMyComboBoxEx()
{
}

BEGIN_MESSAGE_MAP(CMyComboBoxEx, CComboBoxEx)
	//{{AFX_MSG_MAP(CMyComboBoxEx)
	ON_CONTROL_REFLECT(CBN_SELENDOK, OnSelendok)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMyComboBoxEx::OnSelendok()
{
	m_pParentWnd->SendMessage(ID_MYCOMBO_SELENDOK, 0, (LPARAM)this);
}

BOOL CMyComboBoxEx::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			m_pParentWnd->SendMessage(ID_MYCOMBO_OK, 0, (LPARAM)this);
			if (GetDroppedState())
			{
				ShowDropDown(false);
			}
			GetParentFrame()->GetActiveFrame()->SetFocus();
			return TRUE;
		}
		else if (pMsg->wParam == VK_ESCAPE)
		{
			if (GetDroppedState())
			{
				ShowDropDown(false);
			}
			CChildView* pView = (CChildView*)&((CBrowserFrame*)m_pParentWnd)->m_wndView;
			if (theApp.IsWnd(pView))
			{
				CString strURL;
				strURL = pView->GetLocationURL();
				m_Edit.SetWindowText(strURL);
				int length = strURL.GetLength();
				m_Edit.SetSel(length, length, FALSE);
				return TRUE;
			}
			else
			{
				GetParentFrame()->GetActiveFrame()->SetFocus();
				return TRUE;
			}
		}
		else if (pMsg->wParam == VK_TAB)
		{
			if (!GetDroppedState())
			{
				CChildView* pView = (CChildView*)&((CBrowserFrame*)m_pParentWnd)->m_wndView;
				if (theApp.IsWnd(pView))
				{
					pView->bSetCefBrowserFocus();
					return TRUE;
				}
			}
		}
	}
	return CComboBoxEx::PreTranslateMessage(pMsg);
}

IMPLEMENT_DYNCREATE(CMyComboEdit, CEdit)
BEGIN_MESSAGE_MAP(CMyComboEdit, CEdit)
	//{{AFX_MSG_MAP(CMyComboEdit)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#define ME_SELECTALL    WM_USER + 0x7000
#define ME_PASTE_GO     WM_USER + 0x7001
#define ME_PASTE_SEARCH WM_USER + 0x7002
#define ME_GO			WM_USER + 0x7004
#define ME_SEARCH		WM_USER + 0x7008

///////////////////////////////////////////////////////////////////////////////
void CMyComboEdit::OnContextMenu(CWnd* pWnd, CPoint point)
{
	SetFocus();
	CMenu menu;
	CString strMsg;
	int iIndex = 0;
	menu.CreatePopupMenu();
	BOOL bReadOnly = GetStyle() & ES_READONLY;
	DWORD flags = CanUndo() && !bReadOnly ? 0 : MF_GRAYED;
	strMsg.LoadString(IDS_STRING_UNDO);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, EM_UNDO, strMsg);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | MF_SEPARATOR);

	DWORD sel = GetSel();
	flags = LOWORD(sel) == HIWORD(sel) ? MF_GRAYED : 0;
	strMsg.LoadString(IDS_STRING_COPY);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, WM_COPY, strMsg);

	flags = (flags == MF_GRAYED || bReadOnly) ? MF_GRAYED : 0;
	strMsg.LoadString(IDS_STRING_CUT);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, WM_CUT, strMsg);

	strMsg.LoadString(IDS_STRING_DELETE);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, WM_CLEAR, strMsg);

	menu.InsertMenu(iIndex++, MF_BYPOSITION | MF_SEPARATOR);
	flags = IsClipboardFormatAvailable(CF_TEXT) && !bReadOnly ? 0 : MF_GRAYED;
	strMsg.LoadString(IDS_STRING_PASTE);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, WM_PASTE, strMsg);
	strMsg.LoadString(IDS_STRING_PASTE_GO);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, ME_PASTE_GO, strMsg);

	strMsg.LoadString(IDS_STRING_PASTE_SEARCH);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, ME_PASTE_SEARCH, strMsg);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | MF_SEPARATOR);
	strMsg.LoadString(IDS_STRING_GO);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, ME_GO, strMsg);

	strMsg.LoadString(IDS_STRING_SEARCH);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, ME_SEARCH, strMsg);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | MF_SEPARATOR);

	int len = GetWindowTextLength();
	flags = (!len || (LOWORD(sel) == 0 && HIWORD(sel) == len)) ? MF_GRAYED : 0;
	strMsg.LoadString(IDS_STRING_SELECTALL);

	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, ME_SELECTALL, strMsg);

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y, this);
}

BOOL CMyComboEdit::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case EM_UNDO:
	case WM_CUT:
	case WM_COPY:
	case WM_CLEAR:
	case WM_PASTE:
		return SendMessage(LOWORD(wParam)) ? TRUE : FALSE;
	case ME_SELECTALL:
		return SendMessage(EM_SETSEL, 0, -1) ? TRUE : FALSE;
	case ME_PASTE_GO:
	{
		SendMessage(LOWORD(WM_PASTE));
		if (m_pParentFrmWnd)
		{
			m_pParentFrmWnd->SendMessage(ID_MYCOMBO_OK, 0, 0);
		}
		return TRUE;
	}
	case ME_PASTE_SEARCH:
	{
		SendMessage(LOWORD(WM_PASTE));
		if (m_pParentFrmWnd)
		{
			m_pParentFrmWnd->SendMessage(WM_SEL_SEARCH, 0, 0);
		}
		return TRUE;
	}
	case ME_GO:
	{
		if (m_pParentFrmWnd)
		{
			m_pParentFrmWnd->SendMessage(ID_MYCOMBO_OK, 0, 0);
		}
		return TRUE;
	}
	case ME_SEARCH:
	{
		if (m_pParentFrmWnd)
		{
			m_pParentFrmWnd->SendMessage(WM_SEL_SEARCH, 0, 0);
		}
		return TRUE;
	}
	default:
		return CEdit::OnCommand(wParam, lParam);
	}
}
/////////////////////////////////////////////////////////////////////////////
// CIconEdit
IMPLEMENT_DYNCREATE(CIconEdit, CEdit)
CIconEdit::CIconEdit()
{
	m_internalIcon = NULL;
	m_icon.m_hWnd = NULL;
	m_pParentWnd = NULL;
	m_bFontCreated = FALSE;
}

CIconEdit::~CIconEdit()
{
	if (m_internalIcon)
		::DestroyIcon(m_internalIcon);
}

BEGIN_MESSAGE_MAP(CIconEdit, CEdit)
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void CIconEdit::PreSubclassWindow()
{
	ASSERT(GetStyle() & ES_MULTILINE);
	Prepare();
}

/////////////////////////////////////////////////////////////////////////////
// CIconEdit implementation

void CIconEdit::SetIcon(HICON icon)
{
	CreateIcon();
	m_icon.SetIcon(icon);
}

void CIconEdit::SetIcon(UINT iconres)
{
	CreateIcon();
	if (m_internalIcon)
		::DestroyIcon(m_internalIcon);
	m_internalIcon = (HICON)::LoadImage(AfxGetResourceHandle(),
					    MAKEINTRESOURCE(iconres),
					    IMAGE_ICON,
					    16,
					    16,
					    LR_DEFAULTCOLOR);
	ASSERT(m_internalIcon != NULL);
	m_icon.SetIcon(m_internalIcon);
}

/////////////////////////////////////////////////////////////////////////////
// CIconEdit implementation

void CIconEdit::Prepare()
{
	int width = GetSystemMetrics(SM_CXSMICON);
	CRect editRect;
	GetRect(&editRect);
	editRect.left += width + 2;
	SetRect(&editRect);
}

/////////////////////////////////////////////////////////////////////////////
// CIconEdit message handlers

LRESULT CIconEdit::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	DefWindowProc(WM_SETFONT, wParam, lParam);
	Prepare();
	return 0;
}

void CIconEdit::OnSize(UINT nType, int cx, int cy)
{
	CEdit::OnSize(nType, cx, cy);
	Prepare();
}

/////////////////////////////////////////////////////////////////////////////
// CIconEdit private helpers

void CIconEdit::CreateIcon()
{
	if (!m_icon.m_hWnd)
	{
		CRect editRect;
		GetRect(&editRect);
		CRect clientRect;
		GetClientRect(&clientRect);
		CRect iconRect(0, 0, editRect.left, clientRect.Height());
		if (m_icon.m_hWnd == NULL)
			m_icon.Create(_T(""), WS_CHILD | WS_VISIBLE, iconRect, this, 1);
	}
	if (!m_bFontCreated)
	{
		m_bFontCreated = TRUE;
		LOGFONT lf = {0};
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
		m_fontUI.CreateFontIndirect(&lf);
		SetFont(&m_fontUI, false);
	}
}
///////////////////////////////////////////////////////////////////////////////
void CIconEdit::OnContextMenu(CWnd* pWnd, CPoint point)
{
	SetFocus();
	CMenu menu;
	CString strMsg;
	int iIndex = 0;
	menu.CreatePopupMenu();
	BOOL bReadOnly = GetStyle() & ES_READONLY;
	DWORD flags = CanUndo() && !bReadOnly ? 0 : MF_GRAYED;
	strMsg.LoadString(IDS_STRING_UNDO);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, EM_UNDO, strMsg);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | MF_SEPARATOR);

	DWORD sel = GetSel();
	flags = LOWORD(sel) == HIWORD(sel) ? MF_GRAYED : 0;
	strMsg.LoadString(IDS_STRING_COPY);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, WM_COPY, strMsg);

	flags = (flags == MF_GRAYED || bReadOnly) ? MF_GRAYED : 0;
	strMsg.LoadString(IDS_STRING_CUT);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, WM_CUT, strMsg);

	strMsg.LoadString(IDS_STRING_DELETE);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, WM_CLEAR, strMsg);

	menu.InsertMenu(iIndex++, MF_BYPOSITION | MF_SEPARATOR);
	flags = IsClipboardFormatAvailable(CF_TEXT) && !bReadOnly ? 0 : MF_GRAYED;
	strMsg.LoadString(IDS_STRING_PASTE);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, WM_PASTE, strMsg);

	strMsg.LoadString(IDS_STRING_PASTE_SEARCH);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, ME_PASTE_SEARCH, strMsg);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | MF_SEPARATOR);

	strMsg.LoadString(IDS_STRING_SEARCH);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, ME_SEARCH, strMsg);
	menu.InsertMenu(iIndex++, MF_BYPOSITION | MF_SEPARATOR);

	int len = GetWindowTextLength();
	flags = (!len || (LOWORD(sel) == 0 && HIWORD(sel) == len)) ? MF_GRAYED : 0;
	strMsg.LoadString(IDS_STRING_SELECTALL);

	menu.InsertMenu(iIndex++, MF_BYPOSITION | flags, ME_SELECTALL, strMsg);

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y, this);
}

BOOL CIconEdit::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case EM_UNDO:
	case WM_CUT:
	case WM_COPY:
	case WM_CLEAR:
	case WM_PASTE:
		return SendMessage(LOWORD(wParam)) ? TRUE : FALSE;
	case ME_SELECTALL:
		return SendMessage(EM_SETSEL, 0, -1) ? TRUE : FALSE;
	case ME_PASTE_SEARCH:
	{
		SendMessage(LOWORD(WM_PASTE));
		if (m_pParentWnd)
		{
			CString strTxt;
			this->GetWindowText(strTxt);
			m_pParentWnd->SendMessage(WM_SEL_SEARCH, 0, (LPARAM)(LPCTSTR)strTxt);
		}
		return TRUE;
	}
	case ME_SEARCH:
	{
		if (m_pParentWnd)
		{
			CString strTxt;
			this->GetWindowText(strTxt);
			m_pParentWnd->SendMessage(WM_SEL_SEARCH, 0, (LPARAM)(LPCTSTR)strTxt);
		}
		return TRUE;
	}
	default:
		return CEdit::OnCommand(wParam, lParam);
	}
}
BOOL CIconEdit::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_RETURN:
		{
			CString strTxt;
			this->GetWindowText(strTxt);
			m_pParentWnd->SendMessage(WM_SEL_SEARCH, 0, (LPARAM)(LPCTSTR)strTxt);
			break;
		}
		}
	}
	return CEdit::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// CIconWnd

CIconWnd::CIconWnd()
{
	m_icon = NULL;
}

CIconWnd::~CIconWnd()
{
}
IMPLEMENT_DYNCREATE(CIconWnd, CStatic)

BEGIN_MESSAGE_MAP(CIconWnd, CStatic)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIconWnd message handlers

void CIconWnd::OnPaint()
{
	CPaintDC dc(this);
	if (m_icon)
	{
		CRect rect;
		GetClientRect(&rect);
		dc.FillSolidRect(rect, GetSysColor(COLOR_WINDOW));
		int width = GetSystemMetrics(SM_CXSMICON);
		int height = GetSystemMetrics(SM_CYSMICON);
		::DrawIconEx(dc.m_hDC, 1, 1, m_icon, width, height, 0, NULL, DI_NORMAL);
	}
}

BOOL CIconWnd::OnEraseBkgnd(CDC*)
{
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CIconWnd implementation

void CIconWnd::SetIcon(HICON icon)
{
	m_icon = icon;
	if (::IsWindow(m_hWnd))
		RedrawWindow();
}
