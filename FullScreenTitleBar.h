#pragma once
#include "Sazabi.h"
#define MONITORS_MAX_LIM 16 //	サポートする最大モニター数
#include "FullScreenTitleBarConst.h"
class CTitleBar
{
public:
	UINT_PTR ScrollTimerID;
	UINT_PTR AutoScrollTimer;
	CTitleBar();
	CTitleBar(HINSTANCE hInst, HWND ParentWindow);
	virtual ~CTitleBar();
	void Create(HINSTANCE hInst, HWND ParentWindow);
	void SetText(LPTSTR TextOut);
	void DisplayWindow(BOOL Show, BOOL SetHideFlag = FALSE);
	HWND GetSafeHwnd()
	{
		return m_hWnd;
	};
	CString m_strText;

private:
	void Init();
	void CreateDisplay();
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void Draw();
	HINSTANCE hInstance;
	HWND Parent;
	HWND m_hWnd;
	BOOL SlideDown;
	BOOL HideAfterSlide;
	int IntAutoHideCounter;
	void LoadPictures();
	void FreePictures();
	HBITMAP hClose;
	HBITMAP hMinimize;
	HBITMAP hMaximize;
	CFont m_Font;
	int m_tbWidth;
};
