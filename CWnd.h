#pragma once
#include <Windows.h>
class CDPI
{
	void Init()
	{
		if (!bInitialized)
		{
			nDpiX = 96;
			nDpiY = 96;
			bInitialized = false;
			HDC hDC = GetDC(NULL);
			nDpiX = GetDeviceCaps(hDC, LOGPIXELSX);
			nDpiY = GetDeviceCaps(hDC, LOGPIXELSY);
			ReleaseDC(NULL, hDC);
			bInitialized = true;
		}
	}
	int nDpiX;
	int nDpiY;
	bool bInitialized;

public:
	static int ScaleX(int x)
	{
		CDPI cl;
		cl.Init();
		return ::MulDiv(x, cl.nDpiX, 96);
	}
	static int ScaleY(int y)
	{
		CDPI cl;
		cl.Init();
		return ::MulDiv(y, cl.nDpiY, 96);
	}
	static int UnscaleX(int x)
	{
		CDPI cl;
		cl.Init();
		return ::MulDiv(x, 96, cl.nDpiX);
	}
	static int UnscaleY(int y)
	{
		CDPI cl;
		cl.Init();
		return ::MulDiv(y, 96, cl.nDpiY);
	}
	void ScaleRect(LPRECT lprc)
	{
		lprc->left = ScaleX(lprc->left);
		lprc->right = ScaleX(lprc->right);
		lprc->top = ScaleY(lprc->top);
		lprc->bottom = ScaleY(lprc->bottom);
	}
	void UnscaleRect(LPRECT lprc)
	{
		lprc->left = UnscaleX(lprc->left);
		lprc->right = UnscaleX(lprc->right);
		lprc->top = UnscaleY(lprc->top);
		lprc->bottom = UnscaleY(lprc->bottom);
	}
	int PointsToPixels(int pt, int ptMag = 1)
	{
		Init();
		return ::MulDiv(pt, nDpiY, 72 * ptMag);
	}
	int PixelsToPoints(int px, int ptMag = 1)
	{
		Init();
		return ::MulDiv(px * ptMag, 72, nDpiY);
	}
	static inline int DpiScaleX(int x) { return ScaleX(x); }
	static inline int DpiScaleY(int y) { return ScaleY(y); }
	static inline int DpiUnscaleX(int x) { return UnscaleX(x); }
	static inline int DpiUnscaleY(int y) { return UnscaleY(y); }
	static inline void DpiScaleRect(LPRECT lprc)
	{
		CDPI cl;
		cl.ScaleRect(lprc);
	}
	static inline void DpiUnscaleRect(LPRECT lprc)
	{
		CDPI cl;
		cl.UnscaleRect(lprc);
	}
	static inline int DpiPointsToPixels(int pt, int ptMag = 1)
	{
		CDPI cl;
		return cl.PointsToPixels(pt, ptMag);
	}
	static inline int DpiPixelsToPoints(int px, int ptMag = 1)
	{
		CDPI cl;
		return cl.PixelsToPoints(px, ptMag);
	}
};
static void ActivateFrameWindow(HWND);

class CWndSkr
{
protected:
	friend LRESULT CALLBACK CWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	CWndSkr(const TCHAR* pszInheritanceAppend = _T(""));
	virtual ~CWndSkr();
	// ウィンドウクラス登録
	ATOM RegisterWC(
	    HINSTANCE hInstance,
	    HICON hIcon,	  // Handle to the class icon.
	    HICON hIconSm,	  // Handle to a small icon
	    HCURSOR hCursor,	  // Handle to the class cursor.
	    HBRUSH hbrBackground, // Handle to the class background brush.
	    LPCTSTR lpszMenuName, // Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
	    LPCTSTR lpszClassName // Pointer to a null-terminated string or is an atom.
	);

	//ウィンドウ作成
	HWND Create(
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
	);

	virtual LRESULT DispatchEvent(HWND, UINT, WPARAM, LPARAM);

protected:
	virtual LRESULT DispatchEvent_WM_APP(HWND, UINT, WPARAM, LPARAM);
	virtual void PreviCreateWindow(void) { return; }
	virtual void AfterCreateWindow(void) { ::ShowWindow(m_hWnd, SW_SHOW); }

#define DECLH(method) \
	LRESULT method(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) { return CallDefWndProc(hwnd, msg, wp, lp); }
	virtual DECLH(OnCreate);	  // WM_CREATE
	virtual DECLH(OnCommand);	  // WM_COMMAND
	virtual DECLH(OnPaint);		  // WM_PAINT
	virtual DECLH(OnEraseBkgnd);	  // WM_ERASEBKGND
	virtual DECLH(OnLButtonDown);	  // WM_LBUTTONDOWN
	virtual DECLH(OnLButtonUp);	  // WM_LBUTTONUP
	virtual DECLH(OnLButtonDblClk);	  // WM_LBUTTONDBLCLK
	virtual DECLH(OnRButtonDown);	  // WM_RBUTTONDOWN
	virtual DECLH(OnRButtonUp);	  // WM_RBUTTONUP
	virtual DECLH(OnRButtonDblClk);	  // WM_RBUTTONDBLCLK
	virtual DECLH(OnMButtonDown);	  // WM_MBUTTONDOWN
	virtual DECLH(OnMButtonUp);	  // WM_MBUTTONUP
	virtual DECLH(OnMButtonDblClk);	  // WM_MBUTTONDBLCLK
	virtual DECLH(OnMouseMove);	  // WM_MOUSEMOVE
	virtual DECLH(OnMouseWheel);	  // WM_MOUSEWHEEL
	virtual DECLH(OnMouseHWheel);	  // WM_MOUSEHWHEEL
	virtual DECLH(OnTimer);		  // WM_TIMER
	virtual DECLH(OnSize);		  // WM_SIZE
	virtual DECLH(OnMove);		  // WM_MOVE
	virtual DECLH(OnClose);		  // WM_CLOSE
	virtual DECLH(OnDestroy);	  // WM_DESTROY
	virtual DECLH(OnQueryEndSession); // WM_QUERYENDSESSION

	virtual DECLH(OnMeasureItem);	 // WM_MEASUREITEM
	virtual DECLH(OnMenuChar);	 // WM_MENUCHAR
	virtual DECLH(OnNotify);	 // WM_NOTIFY
	virtual DECLH(OnDrawItem);	 // WM_DRAWITEM
	virtual DECLH(OnCaptureChanged); // WM_CAPTURECHANGED
	virtual LRESULT CallDefWndProc(HWND, UINT, WPARAM, LPARAM);

public:
	//インターフェース
	HWND GetHwnd() const { return m_hWnd; }
	HWND GetParentHwnd() const { return m_hwndParent; }
	HINSTANCE GetAppInstance() const { return m_hInstance; }
	void _SetHwnd(HWND hwnd) { m_hWnd = hwnd; }
	void DestroyWindow();

private:
	HINSTANCE m_hInstance;
	HWND m_hwndParent;
	HWND m_hWnd;
};
///////////////////////////////////////////////////////////////////////
