#include "stdafx.h"
#include "Sazabi.h"
#include "DlgAuth.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CDlgAuth, CDialogEx)

CDlgAuth::CDlgAuth(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_AUTH, pParent)
{
	m_pWnd = NULL;
	m_pWnd = pParent;
	m_hStopWnd = NULL;
	memset(&m_wndClass, 0x00, sizeof(WNDCLASSEX));
}

CDlgAuth::~CDlgAuth()
{
}

void CDlgAuth::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgAuth, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgAuth::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgAuth::OnBnClickedCancel)
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()

void CDlgAuth::OnBnClickedOk()
{
	GetDlgItemText(IDC_EDIT_ID, m_strID);
	GetDlgItemText(IDC_EDIT_PW, m_strPW);
	if (m_strID.IsEmpty())
	{
		::MessageBox(this->m_hWnd, _T("ユーザー名を入力して下さい。"), theApp.m_strThisAppName, MB_OK | MB_ICONWARNING);
		((CButton*)GetDlgItem(IDC_EDIT_ID))->SetFocus();
	}
	CDialogEx::OnOK();
}

void CDlgAuth::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}

BOOL CDlgAuth::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	if (m_strMsgTxt.IsEmpty())
		m_strMsgTxt = _T("認証が必要です。");
	if (m_strCaption.IsEmpty())
	{
		CString strCaption;
		GetWindowText(strCaption);
		strCaption += _T(" - ");
		strCaption += theApp.m_strThisAppName;
		m_strCaption = strCaption;
	}
	SetWindowText(m_strCaption);
	SetDlgItemText(IDC_STATIC_MSG, m_strMsgTxt);
	SetDlgItemText(IDC_EDIT_ID, m_strID);
	SetDlgItemText(IDC_EDIT_PW, m_strPW);

	try
	{
		RECT rectParent = {0};
		HWND hWnd = {0};
		if (m_pWnd)
			hWnd = m_pWnd->m_hWnd;

		m_wndClass.cbSize = sizeof(WNDCLASSEX);
		m_wndClass.style = 0;
		m_wndClass.lpfnWndProc = ::DefWindowProc;
		m_wndClass.cbClsExtra = 0;
		m_wndClass.cbWndExtra = 0;
		m_wndClass.hInstance = GetModuleHandle(NULL);
		m_wndClass.hIcon = NULL;
		m_wndClass.hCursor = NULL;
		m_wndClass.hbrBackground = GetSysColorBrush(COLOR_DESKTOP);
		m_wndClass.lpszMenuName = NULL;
		m_wndClass.lpszClassName = _T("CSGBlackout");
		m_wndClass.hIconSm = NULL;
		UnregisterClass(m_wndClass.lpszClassName, m_wndClass.hInstance);
		if (RegisterClassEx(&m_wndClass))
		{
			if (hWnd == NULL)
				hWnd = ::GetDesktopWindow();

			::GetWindowRect(hWnd, &rectParent);
			int nWidth = rectParent.right - rectParent.left + 10;
			int nHeight = rectParent.bottom - rectParent.top + 10;

			m_hStopWnd = CreateWindowEx(
			    WS_EX_LAYERED,
			    m_wndClass.lpszClassName,
			    NULL,
			    WS_VISIBLE | WS_POPUP,
			    rectParent.left - 5,
			    rectParent.top - 5,
			    nWidth,
			    nHeight,
			    hWnd,
			    NULL,
			    m_wndClass.hInstance,
			    NULL);

			if (m_hStopWnd)
			{
				::SetLayeredWindowAttributes(m_hStopWnd, 0, 196, ULW_ALPHA);
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	::SetWindowPos(this->m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	return FALSE;
}

int CDlgAuth::RunModalLoop(DWORD dwFlags)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(!(m_nFlags & WF_MODALLOOP));
	BOOL bIdle = TRUE;
	LONG lIdleCount = 0;
	BOOL bShowIdle = (dwFlags & MLF_SHOWONIDLE) && !(GetStyle() & WS_VISIBLE);
	HWND hWndParent = ::GetParent(m_hWnd);
	m_nFlags |= (WF_MODALLOOP | WF_CONTINUEMODAL);
	MSG* pMsg = AfxGetCurrentMessage();

	for (;;)
	{
		//SZB
		if (theApp.m_bCEFInitialized)
			CefRunMessageLoop();

		ASSERT(ContinueModal());

		while (bIdle &&
		       !::PeekMessage(pMsg, NULL, NULL, NULL, PM_NOREMOVE))
		{
			ASSERT(ContinueModal());
			//SZB
			if (theApp.m_bCEFInitialized)
				CefRunMessageLoop();

			if (bShowIdle)
			{
				ShowWindow(SW_SHOWNORMAL);
				UpdateWindow();
				bShowIdle = FALSE;
			}

			if (!(dwFlags & MLF_NOIDLEMSG) && hWndParent != NULL && lIdleCount == 0)
			{
				::SendMessage(hWndParent, WM_ENTERIDLE, MSGF_DIALOGBOX, (LPARAM)m_hWnd);
			}
			if ((dwFlags & MLF_NOKICKIDLE) ||
			    !SendMessage(WM_KICKIDLE, MSGF_DIALOGBOX, lIdleCount++))
			{
				bIdle = FALSE;
			}
		}

		do
		{
			//SZB
			if (theApp.m_bCEFInitialized)
				CefRunMessageLoop();

			ASSERT(ContinueModal());

			if (!AfxPumpMessage())
			{
				AfxPostQuitMessage(0);
				return -1;
			}

			if (bShowIdle &&
			    (pMsg->message == 0x118 || pMsg->message == WM_SYSKEYDOWN))
			{
				ShowWindow(SW_SHOWNORMAL);
				UpdateWindow();
				bShowIdle = FALSE;
			}

			if (!ContinueModal())
				goto ExitModal;

			if (AfxIsIdleMessage(pMsg))
			{
				bIdle = TRUE;
				lIdleCount = 0;
			}

		} while (::PeekMessage(pMsg, NULL, NULL, NULL, PM_NOREMOVE));
	}

ExitModal:
	m_nFlags &= ~(WF_MODALLOOP | WF_CONTINUEMODAL);
	return m_nModalResult;
}

INT_PTR CDlgAuth::DoModal()
{
	INT_PTR iRet = {0};
	iRet = CDialogEx::DoModal();
	try
	{
		if (m_hStopWnd)
		{
			::DestroyWindow(m_hStopWnd);
			m_hStopWnd = NULL;
			UnregisterClass(m_wndClass.lpszClassName, m_wndClass.hInstance);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return iRet;
}

void CDlgAuth::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (nIDCtl == IDC_STATIC)
	{
		HDC hSrcDC = {0};
		BITMAP bmp = {0};
		HBITMAP hBitmap = {0};
		HINSTANCE hInst = theApp.m_hInstance;
		if (hInst == NULL)
			hInst = GetModuleHandle(NULL);

		hBitmap = (HBITMAP)::LoadImage(hInst, MAKEINTRESOURCE(IDB_BMP_KEY), IMAGE_BITMAP, 0, 0, 0);
		int w = 0;
		int h = 0;
		GetObject(hBitmap, sizeof(BITMAP), &bmp);
		w = (int)bmp.bmWidth;
		h = (int)bmp.bmHeight;
		hSrcDC = CreateCompatibleDC(lpDrawItemStruct->hDC);
		SelectObject(hSrcDC, hBitmap);
		StretchBlt(lpDrawItemStruct->hDC, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top,
			   lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top,
			   hSrcDC, 0, 0, w, h, SRCCOPY);
		DeleteDC(hSrcDC);
	}

	CDialogEx::OnDrawItem(nIDCtl, lpDrawItemStruct);
}
/////////////////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CDlgAuthPWC, CDialogEx)

CDlgAuthPWC::CDlgAuthPWC(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_AUTH_PWC, pParent)
{
	m_pWnd = NULL;
	m_pWnd = pParent;
	m_hStopWnd = NULL;
	memset(&m_wndClass, 0x00, sizeof(WNDCLASSEX));
}

CDlgAuthPWC::~CDlgAuthPWC()
{
}

void CDlgAuthPWC::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgAuthPWC, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgAuthPWC::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgAuthPWC::OnBnClickedCancel)
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()

void CDlgAuthPWC::OnBnClickedOk()
{
	GetDlgItemText(IDC_EDIT_PW_CURRENT, m_strPW_Current);
	GetDlgItemText(IDC_EDIT_PW_NEW, m_strPW_NEW);
	GetDlgItemText(IDC_EDIT_PW_NEW2, m_strPW_NEW2);
	if (m_strPW_NEW != m_strPW_NEW2)
	{
		::MessageBox(this->m_hWnd, _T("新しいパスワードとパスワードの確認が一致していません。再度入力して下さい。"), theApp.m_strThisAppName, MB_OK | MB_ICONWARNING);
		SetDlgItemText(IDC_EDIT_PW_NEW2, _T(""));
		((CButton*)GetDlgItem(IDC_EDIT_PW_NEW2))->SetFocus();
		return;
	}
	CDialogEx::OnOK();
}

void CDlgAuthPWC::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}

BOOL CDlgAuthPWC::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	if (m_strMsgTxt.IsEmpty())
		m_strMsgTxt = _T("パスワード変更");
	if (m_strCaption.IsEmpty())
	{
		CString strCaption;
		GetWindowText(strCaption);
		strCaption += _T(" - ");
		strCaption += theApp.m_strThisAppName;
		m_strCaption = strCaption;
	}
	SetWindowText(m_strCaption);
	SetDlgItemText(IDC_STATIC_MSG, m_strMsgTxt);
	SetDlgItemText(IDC_EDIT_ID, m_strID);
	SetDlgItemText(IDC_EDIT_PW_CURRENT, m_strPW_Current);
	SetDlgItemText(IDC_EDIT_PW_NEW, m_strPW_NEW);
	SetDlgItemText(IDC_EDIT_PW_NEW2, m_strPW_NEW2);
	((CButton*)GetDlgItem(IDC_EDIT_PW_CURRENT))->SetFocus();
	try
	{
		RECT rectParent = {0};
		HWND hWnd = {0};
		if (m_pWnd)
			hWnd = m_pWnd->m_hWnd;

		m_wndClass.cbSize = sizeof(WNDCLASSEX);
		m_wndClass.style = 0;
		m_wndClass.lpfnWndProc = ::DefWindowProc;
		m_wndClass.cbClsExtra = 0;
		m_wndClass.cbWndExtra = 0;
		m_wndClass.hInstance = GetModuleHandle(NULL);
		m_wndClass.hIcon = NULL;
		m_wndClass.hCursor = NULL;
		m_wndClass.hbrBackground = GetSysColorBrush(COLOR_DESKTOP);
		m_wndClass.lpszMenuName = NULL;
		m_wndClass.lpszClassName = _T("CSGBlackout");
		m_wndClass.hIconSm = NULL;
		UnregisterClass(m_wndClass.lpszClassName, m_wndClass.hInstance);
		if (RegisterClassEx(&m_wndClass))
		{
			if (hWnd == NULL)
				hWnd = ::GetDesktopWindow();

			::GetWindowRect(hWnd, &rectParent);
			int nWidth = rectParent.right - rectParent.left + 10;
			int nHeight = rectParent.bottom - rectParent.top + 10;

			m_hStopWnd = CreateWindowEx(
			    WS_EX_LAYERED,
			    m_wndClass.lpszClassName,
			    NULL,
			    WS_VISIBLE | WS_POPUP,
			    rectParent.left - 5,
			    rectParent.top - 5,
			    nWidth,
			    nHeight,
			    hWnd,
			    NULL,
			    m_wndClass.hInstance,
			    NULL);

			if (m_hStopWnd)
			{
				::SetLayeredWindowAttributes(m_hStopWnd, 0, 196, ULW_ALPHA);
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	::SetWindowPos(this->m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	return FALSE;
}

INT_PTR CDlgAuthPWC::DoModal()
{
	INT_PTR iRet = {0};
	iRet = CDialogEx::DoModal();
	try
	{
		if (m_hStopWnd)
		{
			::DestroyWindow(m_hStopWnd);
			m_hStopWnd = NULL;
			UnregisterClass(m_wndClass.lpszClassName, m_wndClass.hInstance);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return iRet;
}

void CDlgAuthPWC::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (nIDCtl == IDC_STATIC)
	{
		HDC hSrcDC = {0};
		BITMAP bmp = {0};
		HBITMAP hBitmap = {0};
		HINSTANCE hInst = theApp.m_hInstance;
		if (hInst == NULL)
			hInst = GetModuleHandle(NULL);

		hBitmap = (HBITMAP)::LoadImage(hInst, MAKEINTRESOURCE(IDB_BMP_KEY), IMAGE_BITMAP, 0, 0, 0);
		int w = 0;
		int h = 0;
		GetObject(hBitmap, sizeof(BITMAP), &bmp);
		w = (int)bmp.bmWidth;
		h = (int)bmp.bmHeight;
		hSrcDC = CreateCompatibleDC(lpDrawItemStruct->hDC);
		SelectObject(hSrcDC, hBitmap);
		StretchBlt(lpDrawItemStruct->hDC, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top,
			   lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top,
			   hSrcDC, 0, 0, w, h, SRCCOPY);
		DeleteDC(hSrcDC);
	}

	CDialogEx::OnDrawItem(nIDCtl, lpDrawItemStruct);
}
