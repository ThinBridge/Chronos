// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
#pragma once

#include <list>
#include <map>
#include <set>
#include <string>
#pragma warning(push, 0)
#include <codeanalysis/warnings.h>
#pragma warning(disable \
		: ALL_CODE_ANALYSIS_WARNINGS)
#include "include/cef_client.h"
#include "include/cef_version.h"
#include "include/base/cef_lock.h"
#pragma warning(pop)
#include "resource.h"

#include <cryptuiapi.h>

#pragma warning(push, 0)
#pragma warning(disable : 26812)
#if CHROME_VERSION_MAJOR >= 115
#define uint32 uint32_t
#endif

class MyV8Handler : public CefV8Handler
{
public:
	MyV8Handler() {}
	virtual bool Execute(const CefString& name,
			     CefRefPtr<CefV8Value> object,
			     const CefV8ValueList& arguments,
			     CefRefPtr<CefV8Value>& retval,
			     CefString& exception) override;
	// Provide the reference counting implementation for this class.
	IMPLEMENT_REFCOUNTING(MyV8Handler);
};
// ClientHandler implementation.
class ClientHandler : public CefClient,
		      public CefContextMenuHandler,
		      public CefDisplayHandler,
		      public CefDownloadHandler,
		      public CefLifeSpanHandler,
		      public CefLoadHandler,
		      public CefRequestHandler,
		      public CefResourceRequestHandler,
		      public CefJSDialogHandler,
		      public CefDragHandler,
		      public CefPermissionHandler
{
public:
	// Interface implemented to handle off-screen rendering.
	class RenderHandler : public CefRenderHandler
	{
	public:
		virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) = 0;
	};
	void CreateBrowser(CefWindowInfo const& info, CefBrowserSettings const& settings, CefString const& url);

	ClientHandler();
	virtual ~ClientHandler();

	typedef cef_window_open_disposition_t WindowOpenDisposition;
#if CHROME_VERSION_MAJOR <= 98
	// Since CEF99, cef_plugin_policy_t is not available anymore.
	typedef cef_plugin_policy_t PluginPolicy;
#endif

	// CefClient methods
	virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override { return this; }
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
	virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler() override { return this; }
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
	virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }
	virtual CefRefPtr<CefJSDialogHandler> GetJSDialogHandler() override { return this; }
	virtual CefRefPtr<CefDragHandler> GetDragHandler() override { return this; }
	virtual CefRefPtr<CefPermissionHandler> GetPermissionHandler() override { return this; }

	// CefLifeSpanHandler methods
	virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
	virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url, const CefString& target_frame_name, WindowOpenDisposition target_disposition, bool user_gesture, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings, CefRefPtr<CefDictionaryValue>& extra_info, bool* no_javascript_access) override;

	// CefContextMenuHandler methods
	virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) override;
	virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags) override;

	// CefLoadHandler methods
	virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) override;

	// CefDisplayHandler methods
	virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url) override;
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
	virtual void OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value) override;
	virtual void OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls) override;
	virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
				      cef_log_severity_t level,
				      const CefString& message,
				      const CefString& source,
				      int line) override;
	virtual void OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen) override;
	virtual void OnLoadingProgressChange(CefRefPtr<CefBrowser> browser, double progress) override;

	// CefDownloadHandler methods
	virtual void OnBeforeDownload(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback) override;
	virtual void OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback) override;

	// CefLoadHandler methods
	virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type) override;
	virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;
	virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) override;

	// CefRequestHandler methods
	virtual bool GetAuthCredentials(CefRefPtr<CefBrowser> browser, const CefString& origin_url, bool isProxy, const CefString& host, int port, const CefString& realm, const CefString& scheme, CefRefPtr<CefAuthCallback> callback) override;

	virtual CefRefPtr<CefResourceHandler> GetResourceHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request) override;
#if CHROME_VERSION_MAJOR < 109
	virtual bool OnQuotaRequest(CefRefPtr<CefBrowser> browser, const CefString& origin_url, int64 new_size, CefRefPtr<CefCallback> callback) override;
#endif
	virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool& allow_os_execution) override;
	virtual void OnRenderViewReady(CefRefPtr<CefBrowser> browser) override;
	virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool user_gesture, bool is_redirect) override;

	virtual CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(
	    CefRefPtr<CefBrowser> browser,
	    CefRefPtr<CefFrame> frame,
	    CefRefPtr<CefRequest> request,
	    bool is_navigation,
	    bool is_download,
	    const CefString& request_initiator,
	    bool& disable_default_handling)
	{
		return this;
	}

	virtual bool OnSelectClientCertificate(
	    CefRefPtr<CefBrowser> browser,
	    bool isProxy,
	    const CefString& host,
	    int port,
	    const X509CertificateList& certificates,
	    CefRefPtr<CefSelectClientCertificateCallback> callback) override;

	virtual bool OnCertificateError(CefRefPtr<CefBrowser> browser, ErrorCode cert_error, const CefString& request_url, CefRefPtr<CefSSLInfo> ssl_info, CefRefPtr<CefCallback> callback) override;
	virtual bool OnOpenURLFromTab(CefRefPtr<CefBrowser> browser,
				      CefRefPtr<CefFrame> frame,
				      const CefString& target_url,
				      WindowOpenDisposition target_disposition,
				      bool user_gesture) override;
	virtual ReturnValue OnBeforeResourceLoad(
	    CefRefPtr<CefBrowser> browser,
	    CefRefPtr<CefFrame> frame,
	    CefRefPtr<CefRequest> request,
	    CefRefPtr<CefCallback> callback) override;

	// CefJSDialogHandler methods
	virtual bool OnBeforeUnloadDialog(CefRefPtr<CefBrowser> browser, const CefString& message_text, bool is_reload, CefRefPtr<CefJSDialogCallback> callback) override;
	virtual void OnDialogClosed(CefRefPtr<CefBrowser> browser) override;
	virtual bool OnJSDialog(CefRefPtr<CefBrowser> browser,
				const CefString& origin_url,
				JSDialogType dialog_type,
				const CefString& message_text,
				const CefString& default_prompt_text,
				CefRefPtr<CefJSDialogCallback> callback,
				bool& suppress_message) override;

	//CefDragHandler
	virtual bool OnDragEnter(CefRefPtr<CefBrowser> browser,
				 CefRefPtr<CefDragData> dragData,
				 DragOperationsMask mask) override;

	virtual void OnResetDialogState(CefRefPtr<CefBrowser> browser) override;

	virtual bool OnRequestMediaAccessPermission(CefRefPtr<CefBrowser> browser,
						    CefRefPtr<CefFrame> frame,
						    const CefString& requesting_origin,
						    uint32 requested_permissions,
						    CefRefPtr<CefMediaAccessCallback> callback) override;

	void EmptyWindowClose(CefRefPtr<CefBrowser> browser)
	{
		HWND hWindow = GetSafeParentWnd(browser);
		if (SafeWnd(hWindow))
		{
			CString strURL;
			CefString strURLC;
			strURLC = browser->GetMainFrame()->GetURL();
			strURL = (LPCWSTR)strURLC.c_str();
			if (strURL.IsEmpty())
			{
				HWND hRetNULL = {0};
				hRetNULL = GetParent(hWindow);
				//if(SafeWnd(hRetNULL))
				//	::PostMessage(hRetNULL, WM_COMMAND, ID_W_CLOSE, 0);
				browser->GetHost()->CloseBrowser(true);
				//CEF93から動かない
				//browser->GetMainFrame()->ExecuteJavaScript(
				//    "window.close();", "", 0);
			}
		}
	}
	inline HWND GetSafeParentWnd(CefRefPtr<CefBrowser> browser)
	{
		HWND hRetNULL = {0};
		try
		{
			if (browser)
			{
				if (browser->GetHost())
					hRetNULL = GetParent(browser->GetHost()->GetWindowHandle());
				else
					ASSERT(FALSE);
			}
			else
				ASSERT(FALSE);
		}
		catch (...)
		{
			return hRetNULL;
		}
		return hRetNULL;
	}

	inline HWND SafeWnd(HWND wnd)
	{
		HWND hRetNULL = {0};
		if (wnd == NULL)
			return hRetNULL;
		__try
		{
			if (IsWindow(wnd))
				return wnd;
			else
				return hRetNULL;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return hRetNULL;
		}
		return hRetNULL;
	}
	CString FormatTransferInfo(ULONGLONG dwBytesRead, ULONGLONG dwFileSize, INT dwProgress)
	{
		CString szText;
		CString szRead;
		CString szSize;

		// format read
		if (dwBytesRead < 1)
		{
			CString undeterminedMsg;
			undeterminedMsg.LoadString(ID_TRANSFER_SIZE_UNDETERMINED);
			szSize.Format(undeterminedMsg);
		}
		else if (dwBytesRead < 1024)
			szRead.Format(_T("%I64u B"), dwBytesRead);
		else if (dwBytesRead < 1048576)
			szRead.Format(_T("%0.1f KB"), static_cast<LONGLONG>(dwBytesRead) / 1024.0);
		else
			szRead.Format(_T("%0.2f MB"), static_cast<LONGLONG>(dwBytesRead) / 1048576.0);

		// format size
		if (dwFileSize < 1)
		{
			CString undeterminedMsg;
			undeterminedMsg.LoadString(ID_TRANSFER_SIZE_UNDETERMINED);
			szSize.Format(undeterminedMsg);
		}
		else if (dwFileSize < 1024)
			szSize.Format(_T("%I64u B"), dwFileSize);
		else if (dwFileSize < 1048576)
			szSize.Format(_T("%0.1f KB"), static_cast<LONGLONG>(dwFileSize) / 1024.0);
		else
			szSize.Format(_T("%0.2f MB"), static_cast<LONGLONG>(dwFileSize) / 1048576.0);

		// return text
		CString progressMsg;
		progressMsg.LoadString(ID_TRANSFER_PROGRESS_MSG);
		szText.Format(progressMsg, szRead, szSize, abs(dwProgress));

		return szText;
	}
	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
					      CefRefPtr<CefFrame> frame,
					      CefProcessId source_process,
					      CefRefPtr<CefProcessMessage> message) override;

	CString GetSerialNumberAsHexString(const CefRefPtr<CefX509Certificate> certificate);
	CString GetSerialNumberAsHexString(PCERT_INFO pCertInfo);

protected:
	BOOL m_bDownLoadStartFlg;
	UINT m_RendererPID;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(ClientHandler);

	// Include the default locking implementation.
	//IMPLEMENT_LOCKING(ClientHandler);
private:
	std::map<std::tuple<CefString, uint32>, bool> m_originAndPermissionsCache;
};
class AppRenderer : public CefApp, public CefRenderProcessHandler
{
public:
	AppRenderer() {}
	virtual ~AppRenderer() {}

	virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override
	{
		return this;
	}
	virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDictionaryValue> extra_info) override
	{
		if (browser)
		{
			CefRefPtr<CefFrame> frame;
			frame = browser->GetMainFrame();
			if (frame)
			{
				UINT iPID = 0;
				iPID = ::GetCurrentProcessId();
				CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("SET_PID");
				CefRefPtr<CefListValue> args = msg->GetArgumentList();
				args->SetInt(0, iPID);
				frame->SendProcessMessage(PID_BROWSER, msg);
			}
		}
	}
	virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override
	{
		CefRefPtr<CefV8Value> object = context->GetGlobal();
		CefRefPtr<CefV8Handler> MyHandler = new MyV8Handler();
		CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction("ChronosExt_DlgLogin", MyHandler);
		object->SetValue("ChronosExt_DlgLogin", func, V8_PROPERTY_ATTRIBUTE_NONE);
		CefRefPtr<CefV8Value> func2 = CefV8Value::CreateFunction("ChronosExt_DlgPWChange", MyHandler);
		object->SetValue("ChronosExt_DlgPWChange", func2, V8_PROPERTY_ATTRIBUTE_NONE);
		CefRefPtr<CefV8Value> func3 = CefV8Value::CreateFunction("ChronosExt_AppActive", MyHandler);
		object->SetValue("ChronosExt_AppActive", func3, V8_PROPERTY_ATTRIBUTE_NONE);
	}

	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
					      CefRefPtr<CefFrame> frame,
					      CefProcessId source_process,
					      CefRefPtr<CefProcessMessage> message) override
	{
		CefString strName = message->GetName();
		CString strFilterName;
		strFilterName = (LPCWSTR)strName.c_str();
		strFilterName.TrimLeft();
		strFilterName.TrimRight();
		if (strFilterName == _T("GET_PID"))
		{
			if (frame)
			{
				UINT iPID = 0;
				iPID = ::GetCurrentProcessId();
				UINT iNowPID = 0;
				CefRefPtr<CefListValue> args = message->GetArgumentList();
				if (args->GetType(0) == VTYPE_INT)
				{
					iNowPID = args->GetInt(0);
				}
				if (iNowPID != iPID)
				{
					CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("SET_PID");
					CefRefPtr<CefListValue> args = msg->GetArgumentList();
					args->SetInt(0, iPID);
					frame->SendProcessMessage(PID_BROWSER, msg);
				}
			}
			return true;
		}
		return false;
	}

private:
	IMPLEMENT_REFCOUNTING(AppRenderer);
	DISALLOW_COPY_AND_ASSIGN(AppRenderer);
};
//#pragma warning(disable : 26812)
#pragma warning(pop)
