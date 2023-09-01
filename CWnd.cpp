#include "StdAfx.h"
#include "CWnd.h"

LRESULT CALLBACK CWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CWndSkr* pCWnd = (CWndSkr*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (pCWnd)
	{
		return pCWnd->DispatchEvent(hwnd, uMsg, wParam, lParam);
	}
	else
	{
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

namespace CWindowCreationHook
{
	int g_nCnt = 0; //参照カウンタ
	HHOOK g_hHook = NULL;

	static LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode == HCBT_CREATEWND)
		{
			HWND hwnd = (HWND)wParam;
			CBT_CREATEWND* pCreateWnd = (CBT_CREATEWND*)lParam;
			CWndSkr* pcWnd = static_cast<CWndSkr*>(pCreateWnd->lpcs->lpCreateParams);

			WNDPROC wndproc = (WNDPROC)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
			if (wndproc != CWndProc) goto next;

			::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pcWnd);

			pcWnd->_SetHwnd(hwnd);
		}
	next:
		return ::CallNextHookEx(g_hHook, nCode, wParam, lParam);
	}

	void Use()
	{
		if (++g_nCnt >= 1 && g_hHook == NULL)
		{
			g_hHook = ::SetWindowsHookEx(WH_CBT, CBTProc, NULL, GetCurrentThreadId());
		}
	}

	void Unuse()
	{
		if (--g_nCnt <= 0 && g_hHook != NULL)
		{
			::UnhookWindowsHookEx(g_hHook);
			g_hHook = NULL;
		}
	}
} // namespace CWindowCreationHook

CWndSkr::CWndSkr(const TCHAR* pszInheritanceAppend)
{
	m_hInstance = NULL;
	m_hwndParent = NULL;
	m_hWnd = NULL;
}

CWndSkr::~CWndSkr()
{
	if (::IsWindow(m_hWnd))
	{
		::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)NULL);
		::DestroyWindow(m_hWnd);
	}
	m_hWnd = NULL;
	return;
}

ATOM CWndSkr::RegisterWC(
    HINSTANCE hInstance,
    HICON hIcon,	  // Handle to the class icon.
    HICON hIconSm,	  // Handle to a small icon
    HCURSOR hCursor,	  // Handle to the class cursor.
    HBRUSH hbrBackground, // Handle to the class background brush.
    LPCTSTR lpszMenuName, // Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
    LPCTSTR lpszClassName // Pointer to a null-terminated string or is an atom.
)
{
	m_hInstance = hInstance;

	WNDCLASSEX wc = {0};
	wc.cbSize = sizeof(wc);
	wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = CWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 32;
	wc.hInstance = m_hInstance;
	wc.hIcon = hIcon;
	wc.hCursor = hCursor;
	wc.hbrBackground = hbrBackground;
	wc.lpszMenuName = lpszMenuName;
	wc.lpszClassName = lpszClassName;
	wc.hIconSm = hIconSm;
	return ::RegisterClassEx(&wc);
}

HWND CWndSkr::Create(
    HWND hwndParent,
    DWORD dwExStyle,	   // extended window style
    LPCTSTR lpszClassName, // Pointer to a null-terminated string or is an atom.
    LPCTSTR lpWindowName,  // pointer to window name
    DWORD dwStyle,	   // window style
    int x,		   // horizontal position of window
    int y,		   // vertical position of window
    int nWidth,		   // window width
    int nHeight,	   // window height
    HMENU hMenu		   // handle to menu, or child-window identifier
)
{
	m_hwndParent = hwndParent;

	PreviCreateWindow();
	CWindowCreationHook::Use();

	m_hWnd = ::CreateWindowEx(
	    dwExStyle,	   // extended window style
	    lpszClassName, // pointer to registered class name
	    lpWindowName,  // pointer to window name
	    dwStyle,	   // window style
	    x,		   // horizontal position of window
	    y,		   // vertical position of window
	    nWidth,	   // window width
	    nHeight,	   // window height
	    m_hwndParent,  // handle to parent or owner window
	    hMenu,	   // handle to menu, or child-window identifier
	    m_hInstance,   // handle to application instance
	    (LPVOID)this   // pointer to window-creation data
	);

	CWindowCreationHook::Unuse();
	AfterCreateWindow();
	return m_hWnd;
}

LRESULT CWndSkr::DispatchEvent(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
#define CALLH(message, method) \
	case message:          \
		return method(hwnd, msg, wp, lp)
	switch (msg)
	{
		CALLH(WM_CREATE, OnCreate);
		CALLH(WM_CLOSE, OnClose);
		CALLH(WM_DESTROY, OnDestroy);
		CALLH(WM_SIZE, OnSize);
		CALLH(WM_MOVE, OnMove);
		CALLH(WM_COMMAND, OnCommand);
		CALLH(WM_LBUTTONDOWN, OnLButtonDown);
		CALLH(WM_LBUTTONUP, OnLButtonUp);
		CALLH(WM_LBUTTONDBLCLK, OnLButtonDblClk);
		CALLH(WM_RBUTTONDOWN, OnRButtonDown);
		CALLH(WM_RBUTTONUP, OnRButtonUp);
		CALLH(WM_RBUTTONDBLCLK, OnRButtonDblClk);
		CALLH(WM_MBUTTONDOWN, OnMButtonDown);
		CALLH(WM_MBUTTONUP, OnMButtonUp);
		CALLH(WM_MBUTTONDBLCLK, OnMButtonDblClk);
		CALLH(WM_MOUSEMOVE, OnMouseMove);
		CALLH(WM_MOUSEWHEEL, OnMouseWheel);
		CALLH(WM_MOUSEHWHEEL, OnMouseHWheel);
		CALLH(WM_ERASEBKGND, OnEraseBkgnd);
		CALLH(WM_TIMER, OnTimer);
		CALLH(WM_QUERYENDSESSION, OnQueryEndSession);

		CALLH(WM_MEASUREITEM, OnMeasureItem);
		CALLH(WM_MENUCHAR, OnMenuChar);
		CALLH(WM_NOTIFY, OnNotify);
		CALLH(WM_DRAWITEM, OnDrawItem);
		CALLH(WM_CAPTURECHANGED, OnCaptureChanged);

	default:
		if (WM_APP <= msg && msg <= 0xBFFF)
		{
			return DispatchEvent_WM_APP(hwnd, msg, wp, lp);
		}
		break;
	}
	return CallDefWndProc(hwnd, msg, wp, lp);
}

LRESULT CWndSkr::DispatchEvent_WM_APP(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	return CallDefWndProc(hwnd, msg, wp, lp);
}

LRESULT CWndSkr::CallDefWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	return ::DefWindowProc(hwnd, msg, wp, lp);
}

void CWndSkr::DestroyWindow()
{
	if (m_hWnd)
	{
		::DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}
}
