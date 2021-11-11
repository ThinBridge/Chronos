#pragma once

#include "include/cef_app.h"
#include "client_handler.h"
#include "client_util.h"

class ClientApp : public CefApp,
		  public CefBrowserProcessHandler
{
public:
	ClientApp(void);
	ClientApp(HWND hWnd);
	~ClientApp(void);

	// CefApp methods:
	virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE { return this; }
	virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line);
	virtual void OnContextInitialized() OVERRIDE;

private:
	IMPLEMENT_REFCOUNTING(ClientApp);
};

class CefNavigationEntryVisitorAdapter : public CefNavigationEntryVisitor
{
private:
public:
	CStringArray m_strArrayRet;
	int m_iCurrentIndex;
	CefNavigationEntryVisitorAdapter()
	{
		m_iCurrentIndex = 0;
		m_strArrayRet.SetSize(20);
	}
	~CefNavigationEntryVisitorAdapter()
	{
	}

	bool Visit(CefRefPtr<CefNavigationEntry> entry,
		   bool current,
		   int index,
		   int total) OVERRIDE
	{
		if (!entry->IsValid()) return true;

		INT_PTR iCurrentSize = 0;
		INT_PTR iIndex = index;
		iCurrentSize = m_strArrayRet.GetSize();
		if (iCurrentSize < total)
		{
			m_strArrayRet.SetSize(total);
		}
		CString strTitle;
		CefString strcefTitle;
		strcefTitle = entry->GetTitle();
		strTitle = strcefTitle.c_str();
		strTitle.TrimLeft();
		strTitle.TrimRight();
		if (strTitle.IsEmpty())
		{
			CString strURL;
			CefString strcefURL;
			strcefURL = entry->GetDisplayURL();
			strURL = strcefURL.c_str();
			strTitle = strURL;
		}
		m_strArrayRet.SetAt(iIndex, strTitle);
		if (current)
			m_iCurrentIndex = index;
		return true;
	}

public:
	void AddRef() const OVERRIDE
	{
	}
	bool Release() const OVERRIDE
	{
		return true;
	}
	bool HasOneRef() const OVERRIDE
	{
		return false;
	}
	bool HasAtLeastOneRef() const OVERRIDE
	{
		return false;
	}
};
class CBrowserFrame;
class DownloadFaviconCB : public CefDownloadImageCallback
{
public:
	virtual void OnDownloadImageFinished(const CefString& image_url,
					     int http_status_code,
					     CefRefPtr<CefImage> image);

public:
	CBrowserFrame* m_pwndFrame;
	void SetFramePtr(CBrowserFrame* pwndFrame) { m_pwndFrame = pwndFrame; }
	void AddRef() const OVERRIDE
	{
	}
	bool Release() const OVERRIDE
	{
		return true;
	}
	bool HasOneRef() const OVERRIDE
	{
		return false;
	}
	bool HasAtLeastOneRef() const OVERRIDE
	{
		return false;
	}
};
class DownloadImageCopyClipboard : public CefDownloadImageCallback
{
public:
	virtual void OnDownloadImageFinished(const CefString& image_url,
					     int http_status_code,
					     CefRefPtr<CefImage> image);

public:
	void AddRef() const OVERRIDE
	{
	}
	bool Release() const OVERRIDE
	{
		return true;
	}
	bool HasOneRef() const OVERRIDE
	{
		return false;
	}
	bool HasAtLeastOneRef() const OVERRIDE
	{
		return false;
	}
};
