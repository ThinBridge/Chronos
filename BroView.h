#pragma once

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Sazabi.h"
#define ViewBaseClass CWnd
#define FRM	      ((CBrowserFrame*)m_pwndFrame)
class CChildView : public ViewBaseClass
{
public:
	CChildView();
	CWnd* m_pwndFrame;

	CString m_UpDateAddressBarURL_Cache;
	CString m_NavigateCompleteURL_Cache;

	CString m_strTitle;
	void ResizeWindowPopup();
	void ResizeWindowPopupInpl();
	void ResizeFrmWindow(RECT& rectClient);
	BOOL ZoomTo(double lFactor);
	double GetZoomSizeEx();
	void SetWheelZoom(int iDel);
	double m_dbZoomSize;
	double m_dbZoomSizeDefault;
	CString m_strURL;

protected:
	//SZB
	CefRefPtr<CefBrowser> m_cefBrowser;
	INT m_nBrowserID;
	CefRefPtr<ClientHandler> m_clientHandler;
	CefPopupFeatures* m_popupFeatures;
	BOOL m_bDevToolsWnd;
	afx_msg LRESULT OnBeforeBrowse(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDownloadUpdate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDownloadBlankPage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBeforeDownload(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLoadStart(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBeforeResourceLoad(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnLoadEnd(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTitleChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFaviconChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFullScreenModeChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnProgressChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStateChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAddressChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStatusMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSearchURL(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFindDialogMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCloseBrowser(WPARAM wParam, LPARAM lParam);
	//afx_msg LRESULT OnNewBrowser(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewWindow(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAuthenticate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBadCertificate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWindowActivate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetRendererPID(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCopyImage(WPARAM wParam, LPARAM lParam);

public:
	void SetBrowserPtr(INT nBrowserId, CefRefPtr<CefBrowser> browser);
	void ReSetRendererPID();
	CString GetLocationURL()
	{
		CString strRet;
		if (IsBrowserNull())
			return strRet;
		if (m_cefBrowser->GetMainFrame())
		{
			CefString strURL;
			strURL = m_cefBrowser->GetMainFrame()->GetURL();
			strRet = strURL.ToWString().c_str();
		}
		return strRet;
	}
	INT GetBrowserId()
	{
		if (IsBrowserNull())
			return -1;
		INT nBrowserId = m_cefBrowser->GetIdentifier();
		return nBrowserId;
	}

protected:
	static const UINT m_pFindDialogMessage;
	CFindReplaceDialog* m_pFindDialog;
	bool m_bFindNext;

	BOOL m_bWndCloseFlg;

public:
	void Navigate(LPCTSTR pszURL);

	void SafeWindowCloseFunction();

	//{{AFX_VIRTUAL(CChildView)
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

public:
	virtual void OnDraw(CDC* pDC){}; // overridden to draw this view

	virtual ~CChildView();

	void UpDateAddressBar();

	void GetTravelLog(CStringArray& strArrayRet, BOOL bBack = true)
	{
		try
		{
			if (IsBrowserNull())
				return;
			strArrayRet.RemoveAll();
			CefNavigationEntryVisitorAdapter CefEntryVisitor;
			m_cefBrowser->GetHost()->GetNavigationEntries(&CefEntryVisitor, false);
			int iMax = (int)CefEntryVisitor.m_strArrayRet.GetSize();
			int iCurrentIndex = 0;
			iCurrentIndex = CefEntryVisitor.m_iCurrentIndex;
			if (bBack)
			{
				for (int i = iCurrentIndex - 1; i >= 0; i--)
				{
					CString strData;
					strData = CefEntryVisitor.m_strArrayRet.GetAt(i);
					strData.TrimLeft();
					strData.TrimRight();
					if (strData.IsEmpty())
						continue;
					strArrayRet.Add(strData);
				}
			}
			else
			{
				for (int i = iCurrentIndex + 1; i < iMax; i++)
				{
					CString strData;
					strData = CefEntryVisitor.m_strArrayRet.GetAt(i);
					strData.TrimLeft();
					strData.TrimRight();
					if (strData.IsEmpty())
						continue;
					strArrayRet.Add(strData);
				}
			}
		}
		catch (...)
		{
			ATLASSERT(0);
		}
	}

	inline void GoBack()
	{
		try
		{
			if (IsBrowserNull())
				return;
			m_cefBrowser->GoBack();
		}
		catch (...)
		{
			ATLASSERT(0);
		}
	}

	inline void GoForward()
	{
		try
		{
			if (IsBrowserNull())
				return;
			m_cefBrowser->GoForward();
		}
		catch (...)
		{
			ATLASSERT(0);
		}
	}

	BOOL m_bFirstCallDontClose;
	inline void GoHomeFirstCall()
	{
		try
		{
			//初めのインスタンスの場合
			if (theApp.IsFirstInstance())
			{
				m_bFirstCallDontClose = TRUE;
			}
			else
			{
				m_bFirstCallDontClose = FALSE;
			}
			this->GoHome();
		}
		catch (...)
		{
			ATLASSERT(0);
		}
	}

	inline void GoHome()
	{
		try
		{
			CString strHomeURL;
			strHomeURL = theApp.m_AppSettings.GetStartURL();
			this->Navigate(strHomeURL);
		}
		catch (...)
		{
			ATLASSERT(0);
		}
	}

	inline void Refresh()
	{
		try
		{
			if (IsBrowserNull())
				return;
			m_cefBrowser->Reload();
		}
		catch (...)
		{
			ATLASSERT(0);
		}
	}

	inline void Refresh2(int nLevel)
	{
		try
		{
			if (IsBrowserNull())
				return;
			m_cefBrowser->ReloadIgnoreCache();
		}
		catch (...)
		{
			ATLASSERT(0);
		}
	}

	inline void Stop()
	{
		try
		{
			if (IsBrowserNull())
				return;
			m_cefBrowser->StopLoad();
		}
		catch (...)
		{
			ATLASSERT(0);
		}
	}

	inline BOOL IsPopupWindow()
	{
		BOOL bSuggestWindow = FALSE;
		if (m_popupFeatures)
			bSuggestWindow = TRUE;
		return bSuggestWindow;
	}

	BOOL GetToolBar() const;
	BOOL GetMenuBar() const;
	BOOL GetStatusBar() const;
	BOOL GetAddressBar() const;

	void OnFullScreen(BOOL bFlg);
	void OnAddressBar(BOOL bFlg);
	void OnToolBar(BOOL bFlg);
	void OnMenuBar(BOOL bFlg);
	void OnStatusBar(BOOL bFlg);
	void OnVisible(BOOL bFlg);
	void OnWindowSetResizable(BOOL bFlg);
	void ShowTimeoutMessageBox(LPCTSTR strMsg, int iType, int iTimeOut);

	void IsRedirectWndAutoCloseChk();
	BOOL IsRedirectURLChk(const CString& strURL, BOOL bTop);
	BOOL IsRedirectScriptEx(LPCTSTR sURL, LPCTSTR sChkURLNoQuery, BOOL bTop);
	BOOL IsFileURINavigation(const CString& strURL);

	void CreateNewBrowserWindow(LPCTSTR lpszUrl, BOOL bActive = true);
	LRESULT OnCreateNewBrowserWindow(WPARAM wParam, LPARAM lParam);

	inline BOOL IsBrowserNull()
	{
		return (m_cefBrowser == NULL);
	}

	HWND m_pFocusWnd;
	virtual void EnableWinEx(BOOL bShow = TRUE)
	{
		if (bShow)
		{
			if (!this->IsWindowEnabled())
			{
				if (theApp.IsWnd(m_pwndFrame))
				{
					if (!m_pwndFrame->IsWindowEnabled())
						return;
				}
				EnableWindow(TRUE);
			}
		}
		else
		{
			if (this->IsWindowEnabled())
			{
				EnableWindow(FALSE);
			}
		}
	}
	BOOL bSetCefBrowserFocus();
	BOOL SetFocusApp()
	{
		BOOL bRet = FALSE;
		try
		{
			EnableWinEx(TRUE);
			if (bSetCefBrowserFocus())
			{
				bRet = TRUE;
			}
		}
		catch (...)
		{
			ATLASSERT(0);
		}
		return bRet;
	}

protected:
	//{{AFX_MSG(CChildView)
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNew();
	afx_msg void OnNewBlank();
	afx_msg void OnReopenCloseTab();
	afx_msg void OnNewSession();
	afx_msg void OnPrint();
	afx_msg void ShowDevTools();
	afx_msg void OnPrintPDF();
	afx_msg void OnGoBack();
	afx_msg void OnUpdateGoBack(CCmdUI* pCmdUI);
	afx_msg void OnGoForward();
	afx_msg void OnUpdateGoForward(CCmdUI* pCmdUI);
	afx_msg void OnGoStartPage();
	afx_msg void OnUpdateGoStartPage(CCmdUI* pCmdUI);
	afx_msg void OnViewRefresh();
	afx_msg void OnUpdateViewRefresh(CCmdUI* pCmdUI);
	afx_msg void OnViewStop();
	afx_msg void OnUpdateViewStop(CCmdUI* pCmdUI);
	afx_msg void OnOpen();
	afx_msg void OnCut();
	afx_msg void OnCopy();
	afx_msg void OnPaste();
	afx_msg void OnSelAll();
	afx_msg void OnFindPage();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnZoomUp();
	afx_msg void OnZoomDown();
	afx_msg void OnZoomReset();
	//}}AFX_MSG
	virtual afx_msg void OnBroBack(UINT nID);
	virtual afx_msg void OnBroForward(UINT nID);
	virtual afx_msg void OnZoom(UINT nID);
	virtual void OnTitleChange(LPCWSTR lpwszText);

	virtual void OnQuit();
	virtual void OnStatusTextChange(LPCTSTR lpszText);

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnUpdateCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePaste(CCmdUI* pCmdUI);
	afx_msg void OnSettings();
	afx_msg void OnAppAbout();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
};
