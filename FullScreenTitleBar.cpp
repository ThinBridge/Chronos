#include "stdafx.h"
#include "resource.h"
#include "FullScreenTitleBar.h"

CTitleBar* TitleBarThis = NULL;
CRect monitors[MONITORS_MAX_LIM];
int monitors_max = 0;

BOOL myinfoenumproc(HMONITOR hMon, HDC hdcMon, LPRECT lpMon, LPARAM dwDate)
{
	monitors[monitors_max].bottom = lpMon->bottom;
	monitors[monitors_max].left = lpMon->left;
	monitors[monitors_max].right = lpMon->right;
	monitors[monitors_max].top = lpMon->top;
	if (MONITORS_MAX_LIM - 1 <= monitors_max)
		return FALSE;
	++monitors_max;
	return TRUE;
}

CTitleBar::CTitleBar()
{
	hInstance = NULL;
	Parent = NULL;
	ScrollTimerID = 0;
	AutoScrollTimer = 0;
	this->Init();
	m_tbWidth = ConstWidth;
}

CTitleBar::CTitleBar(HINSTANCE hInst, HWND ParentWindow)
{
	hInstance = hInst;
	Parent = ParentWindow;
	this->Init();
}

CTitleBar::~CTitleBar()
{
	DestroyWindow(m_hWnd);
}

void CTitleBar::Init()
{
	SlideDown = TRUE;
	IntAutoHideCounter = 0;
	HideAfterSlide = FALSE;

	LOGFONT lf = {0};
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
	m_Font.CreateFontIndirect(&lf);
	m_strText = _T("");
	m_hWnd = NULL;
	if (Parent != NULL && hInstance != NULL)
	{
		TitleBarThis = this;
		this->CreateDisplay();
	}
}

void CTitleBar::Create(HINSTANCE hInst, HWND ParentWindow)
{
	hInstance = hInst;
	Parent = ParentWindow;
	this->Init();
}

void CTitleBar::CreateDisplay()
{
	//	デスクトップサイズを得る
	CRect rcDesktop;
	CWnd* pView = NULL;
	HWND hWnd = {0};
	pView = (CWnd*)theApp.GetActiveViewPtr();
	if (theApp.IsWnd(pView))
	{
		hWnd = pView->m_hWnd;
	}
	if (hWnd == NULL)
		hWnd = GetDesktopWindow();
	SBUtil::GetMonitorWorkRect(hWnd, &rcDesktop);
	m_tbWidth = rcDesktop.Width();
	int CenterX = rcDesktop.left;
	int HeightPlacement = -tbHeigth + 1;

	WNDCLASS wndclass = {0};
	wndclass.style = CS_DBLCLKS;
	wndclass.lpfnWndProc = CTitleBar::WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = (const TCHAR*)NULL;
	wndclass.lpszClassName = _T("FSTITLEBAR");

	RegisterClass(&wndclass);

	DWORD winstyle = WS_POPUP | WS_SYSMENU;

	if (tbScrollWindow == FALSE)
		HeightPlacement = 0;

	m_hWnd = CreateWindow(_T("FSTITLEBAR"),
			      NULL,
			      winstyle,
			      CenterX,
			      HeightPlacement,
			      m_tbWidth,
			      tbHeigth,
			      Parent,
			      NULL,
			      hInstance,
			      NULL);
	HRGN Range = {0};
	POINT Points[4] = {0};
	Points[0].x = 0;
	Points[0].y = 0;
	Points[1].x = tbTriangularPoint;
	Points[1].y = tbHeigth;
	Points[2].x = m_tbWidth - tbTriangularPoint;
	Points[2].y = tbHeigth;
	Points[3].x = m_tbWidth;
	Points[3].y = 0;
	Range = ::CreatePolygonRgn(Points, 4, ALTERNATE);

	::SetWindowRgn(m_hWnd, Range, TRUE);

	HWND Close = CreateWindow(_T("STATIC"),
				  _T("Close"),
				  WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_OWNERDRAW,
				  m_tbWidth - tbRightSpace - tbcxPicture, tbTopSpace, tbcxPicture, tbcyPicture, m_hWnd,
				  (HMENU)IDB_CLOSE,
				  hInstance,
				  NULL);

	//Maximize button
	HWND Maximize = CreateWindow(_T("STATIC"),
				     _T("Maximize"),
				     WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_OWNERDRAW,
				     m_tbWidth - tbRightSpace - (tbcxPicture * 2) - (tbButtonSpace * 1), tbTopSpace, tbcxPicture, tbcyPicture, m_hWnd,
				     (HMENU)IDB_MAX,
				     hInstance,
				     NULL);

	//Minimize button
	HWND Minimize = CreateWindow(_T("STATIC"),
				     _T("Minimize"),
				     WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_OWNERDRAW,
				     m_tbWidth - tbRightSpace - (tbcxPicture * 3) - (tbButtonSpace * 2), tbTopSpace, tbcxPicture, tbcyPicture, m_hWnd,
				     (HMENU)IDB_MIN,
				     hInstance,
				     NULL);

	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (INT_PTR)this);
	this->LoadPictures();
	if (tbHideAtStartup == FALSE)
	{
		ShowWindow(m_hWnd, SW_SHOW);
	}
	if (tbScrollWindow)
	{
		ScrollTimerID = SetTimer(m_hWnd, tbScrollTimerID, tbScrollDelay, NULL);
	}
	AutoScrollTimer = SetTimer(m_hWnd, tbAutoScrollTimer, tbAutoScrollDelay, NULL);
}

LRESULT CALLBACK CTitleBar::WndProc(HWND hwnd, UINT iMsg,
				    WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_CREATE:
		return 0;
	case WM_PAINT:
		TitleBarThis->Draw();
		return 0;
	case WM_CLOSE:
	{
		HWND Window = TitleBarThis->GetSafeHwnd();
		TitleBarThis->FreePictures();
		DestroyWindow(Window);
		return 0;
	}
	case WM_DESTROY:
		TitleBarThis->FreePictures();
		return 0;
	case WM_DRAWITEM:
	{
		HDC hdcMem = {0};
		LPDRAWITEMSTRUCT lpdis = {0};

		lpdis = (LPDRAWITEMSTRUCT)lParam;
		hdcMem = CreateCompatibleDC(lpdis->hDC);

		if (lpdis->CtlID == IDB_CLOSE)
			SelectObject(hdcMem, TitleBarThis->hClose);
		if (lpdis->CtlID == IDB_MAX)
			SelectObject(hdcMem, TitleBarThis->hMaximize);
		if (lpdis->CtlID == IDB_MIN)
			SelectObject(hdcMem, TitleBarThis->hMinimize);

		BitBlt(lpdis->hDC,
		       lpdis->rcItem.left,
		       lpdis->rcItem.top,
		       lpdis->rcItem.right - lpdis->rcItem.left,
		       lpdis->rcItem.bottom - lpdis->rcItem.top,
		       hdcMem,
		       0,
		       0,
		       SRCCOPY);

		DeleteDC(hdcMem);
		return TRUE;
	}

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			if (LOWORD(wParam) == IDB_CLOSE)
			{
				::SendMessage(TitleBarThis->Parent, WM_CLOSE, NULL, NULL);
			}
			if (LOWORD(wParam) == IDB_MAX)
			{
				::PostMessage(TitleBarThis->Parent, WM_COMMAND, ID_FULL_SCREEN, NULL);
			}
			if (LOWORD(wParam) == IDB_MIN)
			{
				::PostMessage(TitleBarThis->Parent, WM_SYSCOMMAND, SC_MINIMIZE, NULL);
			}
		}
		break;
	case WM_MOUSEMOVE:
		if (TitleBarThis->HideAfterSlide == FALSE)
		{
			TitleBarThis->SlideDown = TRUE;
			TitleBarThis->ScrollTimerID = ::SetTimer(TitleBarThis->m_hWnd, tbScrollTimerID, 20, NULL);
		}
		break;

	case WM_LBUTTONDBLCLK:
		if (tbLastIsStandard)
			::SendMessage(TitleBarThis->m_hWnd, WM_COMMAND, MAKEWPARAM(IDB_MAX, BN_CLICKED), NULL);
		break;

	case WM_TIMER:
	{
		UINT TimerID = (UINT)wParam;

		if (TimerID == TitleBarThis->ScrollTimerID)
		{
			RECT lpRect = {0};
			::GetWindowRect(TitleBarThis->m_hWnd, &lpRect);

			if (((lpRect.top == 0) && (TitleBarThis->SlideDown)) ||
			    ((lpRect.top == -tbHeigth + 1) && (TitleBarThis->SlideDown == FALSE)))
			{
				KillTimer(TitleBarThis->m_hWnd, TitleBarThis->ScrollTimerID);

				if (TitleBarThis->HideAfterSlide)
				{
					TitleBarThis->HideAfterSlide = FALSE;
					ShowWindow(TitleBarThis->GetSafeHwnd(), SW_HIDE);
				}
				return 0;
			}

			if (TitleBarThis->SlideDown)
			{
				lpRect.top++;
				lpRect.bottom++;
			}
			else
			{
				lpRect.top--;
				lpRect.bottom--;
			}

			::MoveWindow(TitleBarThis->m_hWnd, lpRect.left, lpRect.top, lpRect.right - lpRect.left, lpRect.bottom - lpRect.top, TRUE);
		}

		if (TimerID == TitleBarThis->AutoScrollTimer)
		{
			RECT lpRect = {0};
			POINT pt = {0};
			::GetWindowRect(TitleBarThis->m_hWnd, &lpRect);
			::GetCursorPos(&pt);

			if (PtInRect(&lpRect, pt) == FALSE)
			{
				TitleBarThis->IntAutoHideCounter++;

				if (TitleBarThis->IntAutoHideCounter == tbAutoScrollTime)
				{
					TitleBarThis->SlideDown = FALSE;
					TitleBarThis->ScrollTimerID = ::SetTimer(TitleBarThis->m_hWnd, tbScrollTimerID, tbScrollDelay, NULL);
				}
			}
			else
			{
				TitleBarThis->IntAutoHideCounter = 0;
			}
		}

		break;
	}
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void CTitleBar::LoadPictures()
{
	hClose = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_CLOSE));
	hMaximize = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MAX));
	hMinimize = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MIN));
}

void CTitleBar::FreePictures()
{
	DeleteObject(hClose);
	DeleteObject(hMaximize);
	DeleteObject(hMinimize);
}

void CTitleBar::Draw()
{
	PAINTSTRUCT ps = {0};
	HDC hdc = BeginPaint(m_hWnd, &ps);

	int r1 = GetRValue(tbStartColor);
	int g1 = GetGValue(tbStartColor);
	int b1 = GetBValue(tbStartColor);
	int r2 = GetRValue(tbEndColor);
	int g2 = GetGValue(tbEndColor);
	int b2 = GetBValue(tbEndColor);

	if (tbGradientWay)
	{
		for (int x = 0; x < m_tbWidth; x++)
		{
			RECT Rect = {0};
			Rect.left = x;
			Rect.top = 0;
			Rect.right = x + 1;
			Rect.bottom = tbHeigth;
			HBRUSH Brush = CreateSolidBrush(RGB(r1 * (m_tbWidth - x) / m_tbWidth + r2 * x / m_tbWidth,
							    g1 * (m_tbWidth - x) / m_tbWidth + g2 * x / m_tbWidth, b1 * (m_tbWidth - x) / m_tbWidth + b2 * x / m_tbWidth));

			::FillRect(hdc, &Rect, Brush);
			DeleteObject(Brush);
		}
	}
	else
	{
		for (int y = 0; y < tbHeigth; y++)
		{
			RECT Rect = {0};
			Rect.left = 0;
			Rect.top = y;
			Rect.right = m_tbWidth;
			Rect.bottom = y + 1;

			HBRUSH Brush = CreateSolidBrush(RGB(r1 * (tbHeigth - y) / tbHeigth + r2 * y / tbHeigth,
							    g1 * (tbHeigth - y) / tbHeigth + g2 * y / tbHeigth, b1 * (tbHeigth - y) / tbHeigth + b2 * y / tbHeigth));

			::FillRect(hdc, &Rect, Brush);
			DeleteObject(Brush);
		}
	}
	HPEN Border = ::CreatePen(PS_SOLID, tbBorderWidth, tbBorderPenColor);
	::SelectObject(hdc, Border);
	::MoveToEx(hdc, 0, 0, NULL);
	::LineTo(hdc, tbTriangularPoint, tbHeigth);
	::LineTo(hdc, m_tbWidth - tbTriangularPoint, tbHeigth);
	::LineTo(hdc, m_tbWidth, 0);
	::LineTo(hdc, 0, 0);
	DeleteObject(Border);
	Border = ::CreatePen(PS_SOLID, tbBorderWidth, tbBorderPenShadow);
	::SelectObject(hdc, Border);
	::MoveToEx(hdc, tbTriangularPoint + 1, tbHeigth - 1, NULL);
	::LineTo(hdc, m_tbWidth - tbTriangularPoint - 1, tbHeigth - 1);
	RECT lpRect = {0};
	lpRect.left = tbLeftSpace + tbcxPicture + tbButtonSpace;
	lpRect.top = tbBorderWidth;
	lpRect.right = m_tbWidth - tbRightSpace - (tbcxPicture * 3) - (tbButtonSpace * 3);
	lpRect.bottom = tbHeigth - tbBorderWidth;
	::SelectObject(hdc, m_Font);
	::SetBkMode(hdc, TRANSPARENT);
	::SetTextColor(hdc, tbTextColor);
	::DrawText(hdc, m_strText, -1, &lpRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	EndPaint(m_hWnd, &ps);
	DeleteObject(Border);
}

void CTitleBar::SetText(LPTSTR TextOut)
{
	m_strText = TextOut;
}

void CTitleBar::DisplayWindow(BOOL Show, BOOL SetHideFlag)
{
	IntAutoHideCounter = 0;

	if (Show)
	{
		if (tbScrollWindow)
		{
			if (SetHideFlag)
			{
				HideAfterSlide = FALSE;
				SlideDown = TRUE;
			}
			ShowWindow(m_hWnd, SW_SHOW);
			ScrollTimerID = SetTimer(m_hWnd, tbScrollTimerID, tbScrollDelay, NULL);
		}
		else
		{
			ShowWindow(m_hWnd, SW_SHOW);
		}

		AutoScrollTimer = SetTimer(m_hWnd, tbAutoScrollTimer, tbAutoScrollDelay, NULL);
	}
	else
	{
		if (tbScrollWindow)
		{
			if (SetHideFlag)
			{
				HideAfterSlide = TRUE;
				SlideDown = FALSE;
			}
			ScrollTimerID = SetTimer(m_hWnd, tbScrollTimerID, tbScrollDelay, NULL);
		}
		else
		{
			ShowWindow(m_hWnd, SW_HIDE);
		}

		AutoScrollTimer = SetTimer(m_hWnd, tbAutoScrollTimer, tbAutoScrollDelay, NULL);
	}
}
