// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "stdafx.h"
#include "Sazabi.h"
#include "BroView.h"

#include "client_handler.h"
#include <stdio.h>
#include <algorithm>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_path_util.h"
#include "include/cef_process_util.h"
#include "include/cef_trace.h"
#include "include/wrapper/cef_helpers.h"
#include "client_util.h"
#include "DlgAuth.h"
#include "sbcommon.h"

ClientHandler::ClientHandler()
{
	m_bDownLoadStartFlg = FALSE;
	m_RendererPID = 0;
	pChildView=NULL;
}

ClientHandler::~ClientHandler()
{
}

bool ClientHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
	PROC_TIME(DoClose)
	// get browser ID
	INT nBrowserId = browser->GetIdentifier();
	// The frame window will be the parent of the browser window
	HWND hWindow = GetSafeParentWnd(browser);
	if (SafeWnd(hWindow))
	{
		//ダウンロード中の場合は、警告を表示する。
		if (theApp.m_DlMgr.IsDlProgress(nBrowserId))
		{
			SendMessageTimeout(hWindow, WM_APP_CEF_WINDOW_ACTIVATE, (WPARAM)NULL, (LPARAM)NULL, SMTO_NORMAL, 1000, NULL);
			hWindow = GetParent(hWindow);
			int iRet = theApp.SB_MessageBox(hWindow, _T("ダウンロード中のウインドウを閉じてもよろしいですか？\n\nウインドウを閉じるとダウンロードがキャンセルされます。\nダウンロードを続行する場合は、「いいえ」をクリックして下さい。"), NULL, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2, TRUE);
			if (iRet != IDYES)
			{
				return true;
			}
			theApp.m_DlMgr.Release_DLDlg(nBrowserId);
		}
	}
	// call parent
	return CefLifeSpanHandler::DoClose(browser);
}

void ClientHandler::CreateBrowser(CefWindowInfo const& info, CefBrowserSettings const& settings, CefString const& url)
{
	CefBrowserHost::CreateBrowser(info, this, url, settings, nullptr, nullptr);
}
void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	REQUIRE_UI_THREAD();
	PROC_TIME(OnAfterCreated)

	// get browser ID
	INT nBrowserId = browser->GetIdentifier();
	
	//CEF93からポインタを直接SendMessageで渡すことができなくなった。
	//関数を直接呼び出す
	if(pChildView)
		((CChildView*)pChildView)->SetBrowserPtr(nBrowserId, browser);

	//// The frame window will be the parent of the browser window
	//HWND hWindow = GetSafeParentWnd(browser);

	//// assign new browser
	////CefBrowser* pBrowser = browser;
	//if (SafeWnd(hWindow))
	//{
	//	::SendMessageTimeout(hWindow, WM_APP_CEF_NEW_BROWSER, (WPARAM)nBrowserId, (LPARAM)pBrowser, SMTO_NORMAL, 1000, NULL);
	//}
	// call parent
	CefLifeSpanHandler::OnAfterCreated(browser);
}

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	REQUIRE_UI_THREAD();
	PROC_TIME(OnBeforeClose)
	// call parent
	CefLifeSpanHandler::OnBeforeClose(browser);
}

bool ClientHandler::OnOpenURLFromTab(CefRefPtr<CefBrowser> browser,
				     CefRefPtr<CefFrame> frame,
				     const CefString& target_url,
				     WindowOpenDisposition target_disposition,
				     bool user_gesture)
{
	PROC_TIME(OnOpenURLFromTab)

	if (browser->GetHost()->IsWindowRenderingDisabled())
	{
		// Cancel popups in off-screen rendering mode.
		return true;
	}

	// The frame window will be the parent of the browser window
	HWND hWindow = GetSafeParentWnd(browser);
	if (SafeWnd(hWindow))
	{
		LPCTSTR pszURL = NULL;
		pszURL = target_url.c_str();
		switch (target_disposition)
		{
			case cef_window_open_disposition_t::WOD_NEW_FOREGROUND_TAB:
			{
				::SendMessageTimeout(hWindow, WM_NEW_WINDOW_URL, (WPARAM)target_disposition, (LPARAM)pszURL, SMTO_NORMAL, 1000, NULL);
				return true;
			}
			case cef_window_open_disposition_t::WOD_NEW_BACKGROUND_TAB:
			{
				::SendMessageTimeout(hWindow, WM_NEW_WINDOW_URL, (WPARAM)target_disposition, (LPARAM)pszURL, SMTO_NORMAL, 1000, NULL);
				return true;
			}
			case cef_window_open_disposition_t::WOD_NEW_WINDOW:
			{
				::SendMessageTimeout(hWindow, WM_NEW_WINDOW_URL, (WPARAM)target_disposition, (LPARAM)pszURL, SMTO_NORMAL, 1000, NULL);
				return true;
			}
			default:break;
		}
	}
	return false;
}

bool ClientHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& target_url,
		const CefString& target_frame_name,
		WindowOpenDisposition target_disposition,
		bool user_gesture,
		const CefPopupFeatures& popupFeatures,
		CefWindowInfo& windowInfo,
		CefRefPtr<CefClient>& client,
		CefBrowserSettings& settings,
		CefRefPtr<CefDictionaryValue>& extra_info,
		bool* no_javascript_access)
{
	PROC_TIME(OnBeforePopup)

	if (browser->GetHost()->IsWindowRenderingDisabled())
	{
		// Cancel popups in off-screen rendering mode.
		return true;
	}

	// set client
	client = this;
	// The frame window will be the parent of the browser window
	HWND hWindow = GetSafeParentWnd(browser);
	if (SafeWnd(hWindow))
	{
		LRESULT lRet = 0;
		switch (target_disposition)
		{
			case cef_window_open_disposition_t::WOD_NEW_POPUP:
			{
				lRet = ::SendMessage(hWindow, WM_APP_CEF_NEW_WINDOW, (WPARAM)&popupFeatures, (LPARAM)&windowInfo);
				return false;
			}
			case cef_window_open_disposition_t::WOD_NEW_FOREGROUND_TAB:
			{
				if (popupFeatures.toolBarVisible)
				{
					if (//popupFeatures.locationBarVisible==false
						popupFeatures.menuBarVisible == false
					)
					{
						lRet = ::SendMessage(hWindow, WM_APP_CEF_NEW_WINDOW, (WPARAM)&popupFeatures, (LPARAM)&windowInfo);
						return false;
					}
				}
				lRet = ::SendMessage(hWindow, WM_APP_CEF_NEW_WINDOW, (WPARAM)NULL, (LPARAM)&windowInfo);
				return false;
			}
			case cef_window_open_disposition_t::WOD_CURRENT_TAB:
			case cef_window_open_disposition_t::WOD_SINGLETON_TAB:
			case cef_window_open_disposition_t::WOD_NEW_BACKGROUND_TAB:
			case cef_window_open_disposition_t::WOD_NEW_WINDOW:
			case cef_window_open_disposition_t::WOD_SAVE_TO_DISK:
			case cef_window_open_disposition_t::WOD_OFF_THE_RECORD:
			case cef_window_open_disposition_t::WOD_IGNORE_ACTION:
			{
				lRet = ::SendMessage(hWindow, WM_APP_CEF_NEW_WINDOW, (WPARAM)NULL, (LPARAM)&windowInfo);
				return false;;
			}
			default:
				break;
		}
		if (lRet == 0)
			return false;
	}
	return CefLifeSpanHandler::OnBeforePopup(browser, frame, target_url, target_frame_name, target_disposition, user_gesture, popupFeatures, windowInfo, client, settings, extra_info, no_javascript_access);
}

void ClientHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
					CefRefPtr<CefFrame> frame,
					CefRefPtr<CefContextMenuParams> params,
					CefRefPtr<CefMenuModel> model)
{
	model->Remove(MENU_ID_VIEW_SOURCE);
	model->Remove(MENU_ID_PRINT);
	cef_context_menu_type_flags_t Flg = CM_TYPEFLAG_NONE;
	Flg = params->GetTypeFlags();
	if ((Flg & (CM_TYPEFLAG_PAGE | CM_TYPEFLAG_FRAME)) != 0)
	{
		if ((Flg & (CM_TYPEFLAG_LINK)) != 0)
		{
			if (theApp.m_bTabEnable_Init)
			{
				model->InsertItemAt(0, CEF_MENU_ID_OPEN_LINK_NEW, _T("新しいタブで開く"));
				model->InsertItemAt(1, CEF_MENU_ID_OPEN_LINK_NEW_NOACTIVE, _T("新しいタブで開く(非アクティブ)"));
			}
			else
			{
				model->InsertItemAt(0, CEF_MENU_ID_OPEN_LINK_NEW, _T("新しいウィンドウで開く"));
				model->InsertItemAt(1, CEF_MENU_ID_OPEN_LINK_NEW_NOACTIVE, _T("新しいウィンドウで開く(非アクティブ)"));
			}
			model->InsertItemAt(2, CEF_MENU_ID_COPY_LINK, _T("リンクのアドレスをコピー"));
			model->InsertItemAt(3, CEF_MENU_ID_SAVE_FILE, _T("名前を付けてリンク先を保存"));
		}
		if ((Flg & (CM_TYPEFLAG_MEDIA | CM_MEDIATYPE_IMAGE)) != 0)
		{
			if (!params->GetSourceUrl().empty())
			{
				model->Remove(MENU_ID_BACK);
				model->Remove(MENU_ID_FORWARD);
				if (theApp.m_bTabEnable_Init)
				{
					model->AddItem(CEF_MENU_ID_OPEN_IMG, _T("新しいタブで画像を開く"));
					model->AddItem(CEF_MENU_ID_OPEN_IMG_NOACTIVE, _T("新しいタブで画像を開く(非アクティブ)"));
				}
				else
				{
					model->AddItem(CEF_MENU_ID_OPEN_IMG, _T("新しいウィンドウで画像を開く"));
					model->AddItem(CEF_MENU_ID_OPEN_IMG_NOACTIVE, _T("新しいウィンドウで画像を開く(非アクティブ)"));
				}

				model->AddItem(CEF_MENU_ID_SAVE_IMG, _T("名前を付けて画像を保存"));
				if (!theApp.IsSGMode())
					model->AddItem(CEF_MENU_ID_IMG_COPY, _T("画像をコピー"));
				model->AddItem(CEF_MENU_ID_IMG_COPY_LINK, _T("画像アドレスをコピー"));
			}
		}
		if (Flg & CM_TYPEFLAG_SELECTION)
		{
			CString strSelText;
			CefString strCfSt;
			strCfSt = params->GetSelectionText();
			strSelText = strCfSt.c_str();
			strSelText.TrimLeft();
			strSelText.TrimRight();
			if (!strSelText.IsEmpty())
			{
				SBUtil::GetDivChar(strSelText, 48, strSelText, TRUE);
				CString strFmt;
				strFmt.Format(_T("Googleで検索：\"%s\""), strSelText);
				CefString strCFmt(strFmt);
				model->InsertItemAt(0, CEF_MENU_ID_OPEN_SEARCH, strCFmt);
			}
		}
	}
	model->AddItem(MENU_ID_RELOAD, _T("再読み込み"));
	model->AddItem(CEF_MENU_ID_PRINT_PDF, _T("印刷(PDF出力)"));
	model->AddItem(MENU_ID_PRINT, _T("印刷"));

	// call parent
	CefContextMenuHandler::OnBeforeContextMenu(browser, frame, params, model);
}

bool ClientHandler::OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
					 CefRefPtr<CefFrame> frame,
					 CefRefPtr<CefContextMenuParams> params,
					 int command_id, EventFlags event_flags)
{
	// The frame window will be the parent of the browser window
	HWND hWindow = GetSafeParentWnd(browser);
	if (SafeWnd(hWindow))
	{
		if (command_id == CEF_MENU_ID_OPEN_SEARCH)
		{
			CString strSelText;
			CefString strCfSt;
			strCfSt = params->GetSelectionText();
			strSelText = strCfSt.c_str();
			strSelText.TrimLeft();
			strSelText.TrimRight();
			::SendMessageTimeout(hWindow, WM_APP_CEF_SEARCH_URL, (WPARAM)(LPCTSTR)strSelText, (LPARAM)TRUE, SMTO_NORMAL, 1000, NULL);
			return true;
		}
		else if (command_id == CEF_MENU_ID_OPEN_LINK_NEW)
		{
			CefString strURLC;
			strURLC = params->GetLinkUrl();
			LPCTSTR pszURL = {0};
			pszURL = strURLC.c_str();
			::SendMessageTimeout(hWindow, WM_NEW_WINDOW_URL, (WPARAM)cef_window_open_disposition_t::WOD_NEW_WINDOW, (LPARAM)pszURL, SMTO_NORMAL, 1000, NULL);
			return true;
		}
		else if (command_id == CEF_MENU_ID_OPEN_LINK_NEW_NOACTIVE)
		{
			CefString strURLC;
			strURLC = params->GetLinkUrl();
			LPCTSTR pszURL = {0};
			pszURL = strURLC.c_str();
			::SendMessageTimeout(hWindow, WM_NEW_WINDOW_URL, (WPARAM)cef_window_open_disposition_t::WOD_NEW_BACKGROUND_TAB, (LPARAM)pszURL, SMTO_NORMAL, 1000, NULL);
			return true;
		}
		else if (command_id == CEF_MENU_ID_OPEN_IMG)
		{
			CefString strURLC;
			strURLC = params->GetSourceUrl();
			LPCTSTR pszURL = {0};
			pszURL = strURLC.c_str();
			::SendMessageTimeout(hWindow, WM_NEW_WINDOW_URL, (WPARAM)cef_window_open_disposition_t::WOD_NEW_WINDOW, (LPARAM)pszURL, SMTO_NORMAL, 1000, NULL);
			return true;
		}
		else if (command_id == CEF_MENU_ID_OPEN_IMG_NOACTIVE)
		{
			CefString strURLC;
			strURLC = params->GetSourceUrl();
			LPCTSTR pszURL = {0};
			pszURL = strURLC.c_str();
			::SendMessageTimeout(hWindow, WM_NEW_WINDOW_URL, (WPARAM)cef_window_open_disposition_t::WOD_NEW_BACKGROUND_TAB, (LPARAM)pszURL, SMTO_NORMAL, 1000, NULL);
			return true;
		}
		else if (command_id == CEF_MENU_ID_SAVE_IMG)
		{
			browser->GetHost()->StartDownload(params->GetSourceUrl());
			return true;
		}
		else if (command_id == CEF_MENU_ID_SAVE_FILE)
		{
			browser->GetHost()->StartDownload(params->GetLinkUrl());
			return true;
		}
		else if (command_id == CEF_MENU_ID_PRINT_PDF)
		{
			::PostMessage(hWindow, WM_COMMAND, ID_PRINT_PDF, 0);
			return true;
		}
		else if (command_id == CEF_MENU_ID_IMG_COPY_LINK)
		{
			CString str;
			CefString strURLC;
			strURLC = params->GetSourceUrl();
			str = strURLC.c_str();
			if (!str.IsEmpty())
			{
				//data:image/pngの場合があるので、IsURL判定を行わない。
				//if (SBUtil::IsURL_HTTP(str))
				{
					if (::OpenClipboard(NULL))
					{
						int nByte = (str.GetLength() + 1) * sizeof(TCHAR);
						HGLOBAL hText = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, nByte);
						if (hText != NULL)
						{
							BYTE* pText = (BYTE*)::GlobalLock(hText);
							if (pText != NULL)
							{
								::memcpy(pText, (LPCTSTR)str, nByte);
								::GlobalUnlock(hText);
								::EmptyClipboard();
								::SetClipboardData(CF_UNICODETEXT, hText);
							}
						}
						::CloseClipboard();
					}
				}
			}
			return true;
		}
		else if (command_id == CEF_MENU_ID_IMG_COPY)
		{
			CString str;
			CefString strURLC;
			strURLC = params->GetSourceUrl();
			str = strURLC.c_str();
			if (!str.IsEmpty())
			{
				LPCTSTR pszURL = {0};
				pszURL = strURLC.c_str();
				::SendMessageTimeout(hWindow, WM_COPY_IMAGE, (WPARAM)(LPCTSTR)str, NULL, SMTO_NORMAL, 1000, NULL);
				return true;
			}
			return true;
		}
		else if (command_id == CEF_MENU_ID_COPY_LINK)
		{
			CString str;
			CefString strURLC;
			strURLC = params->GetUnfilteredLinkUrl();
			str = strURLC.c_str();
			if (!str.IsEmpty())
			{
				if (::OpenClipboard(NULL))
				{
					int nByte = (str.GetLength() + 1) * sizeof(TCHAR);
					HGLOBAL hText = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, nByte);
					if (hText != NULL)
					{
						BYTE* pText = (BYTE*)::GlobalLock(hText);
						if (pText != NULL)
						{
							::memcpy(pText, (LPCTSTR)str, nByte);
							::GlobalUnlock(hText);
							::EmptyClipboard();
							::SetClipboardData(CF_UNICODETEXT, hText);
						}
					}
					::CloseClipboard();
				}
			}
			return true;
		}
	}
	// call parent
	return CefContextMenuHandler::OnContextMenuCommand(browser, frame, params, command_id, event_flags);
}

void ClientHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
	REQUIRE_UI_THREAD();

	INT nState = 0;
	// set state
	if (isLoading)
		nState |= CEF_BIT_IS_LOADING;
	if (canGoBack)
		nState |= CEF_BIT_CAN_GO_BACK;
	if (canGoForward)
		nState |= CEF_BIT_CAN_GO_FORWARD;

	// The frame window will be the parent of the browser window
	HWND hWindow = GetSafeParentWnd(browser);
	if (SafeWnd(hWindow))
	{
		// send message
		::SendMessageTimeout(hWindow, WM_APP_CEF_STATE_CHANGE, (WPARAM)nState, NULL, SMTO_NORMAL, 1000, NULL);
	}
	// call parent
	CefLoadHandler::OnLoadingStateChange(browser, isLoading, canGoBack, canGoForward);
}

void ClientHandler::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
	REQUIRE_UI_THREAD();

	// The frame window will be the parent of the browser window
	HWND hWindow = GetSafeParentWnd(browser);
	if (SafeWnd(hWindow))
	{
		if (frame->IsMain())
		{
			LPCTSTR pszURL = NULL;
			pszURL = url.c_str();
			::SendMessageTimeout(hWindow, WM_APP_CEF_ADDRESS_CHANGE, (WPARAM)pszURL, NULL, SMTO_NORMAL, 1000, NULL);
		}
	}
	// call parent
	CefDisplayHandler::OnAddressChange(browser, frame, url);
}
void ClientHandler::OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen)
{
	REQUIRE_UI_THREAD();

	// The frame window will be the parent of the browser window
	HWND hWindow = GetSafeParentWnd(browser);
	if (SafeWnd(hWindow))
	{
		::SendMessageTimeout(hWindow, WM_APP_CEF_FULLSCREEN_MODE_CHANGE, (WPARAM)fullscreen, NULL, SMTO_NORMAL, 1000, NULL);
	}
	// call parent
	CefDisplayHandler::OnFullscreenModeChange(browser, fullscreen);
}
void ClientHandler::OnLoadingProgressChange(CefRefPtr<CefBrowser> browser, double progress)
{
	REQUIRE_UI_THREAD();

	// The frame window will be the parent of the browser window
	HWND hWindow = GetSafeParentWnd(browser);
	if (SafeWnd(hWindow))
	{
		progress = progress * 100;
		ULONG_PTR ulProgress = 0;
		ulProgress = (ULONG_PTR)progress;
		DWORD dwProgress = 0;
		dwProgress = (DWORD)ulProgress;

		::SendMessageTimeout(hWindow, WM_APP_CEF_PROGRESS_CHANGE, (WPARAM)dwProgress, NULL, SMTO_NORMAL, 1000, NULL);
	}
	// call parent
	CefDisplayHandler::OnLoadingProgressChange(browser, progress);
}

void ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
	REQUIRE_UI_THREAD();

	// The frame window will be the parent of the browser window
	HWND hWindow = GetSafeParentWnd(browser);
	if (SafeWnd(hWindow))
	{
		LPCTSTR pszTitle = NULL;
		pszTitle = title.c_str();
		::SendMessageTimeout(hWindow, WM_APP_CEF_TITLE_CHANGE, (WPARAM)pszTitle, NULL, SMTO_NORMAL, 1000, NULL);
	}
	// call parent
	CefDisplayHandler::OnTitleChange(browser, title);
}
void ClientHandler::OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls)
{
	REQUIRE_UI_THREAD();

	// The frame window will be the parent of the browser window
	HWND hWindow = GetSafeParentWnd(browser);
	if (SafeWnd(hWindow))
	{
		CefString strIconList;
		LPCTSTR pszFavURL = NULL;
		for (UINT i = 0; i < icon_urls.size(); i++)
		{
			strIconList = icon_urls[i];
			pszFavURL = strIconList.c_str();
		}
		if (pszFavURL)
		{
			::SendMessageTimeout(hWindow, WM_APP_CEF_FAVICON_MESSAGE, (WPARAM)pszFavURL, NULL, SMTO_NORMAL, 1000, NULL);
		}
	}
	// call parent
	CefDisplayHandler::OnFaviconURLChange(browser, icon_urls);
}

void ClientHandler::OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value)
{
	REQUIRE_UI_THREAD();

	// The frame window will be the parent of the browser window
	HWND hWindow = GetSafeParentWnd(browser);
	if (SafeWnd(hWindow))
	{
		LPCTSTR pszStatus = NULL;
		pszStatus = value.c_str();
		::SendMessageTimeout(hWindow, WM_APP_CEF_STATUS_MESSAGE, (WPARAM)pszStatus, NULL, SMTO_NORMAL, 1000, NULL);
	}
	// call parent
	CefDisplayHandler::OnStatusMessage(browser, value);
}

bool ClientHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
				     cef_log_severity_t level,
				     const CefString& message,
				     const CefString& source,
				     int line)
{

	REQUIRE_UI_THREAD();
	if (level == LOGSEVERITY_DISABLE) return TRUE;

	CString strWriteLine;

	if (theApp.m_pDebugDlg)
	{
		CString strLogLevel;
		switch (level)
		{
			case LOGSEVERITY_DEFAULT:
				strLogLevel = _T("DEFAULT");
				break;
			case LOGSEVERITY_VERBOSE:
				strLogLevel = _T("VERBOSE");
				break;

			case LOGSEVERITY_INFO:
				strLogLevel = _T("INFO");
				break;

			case LOGSEVERITY_WARNING:
				strLogLevel = _T("WARNING");
				break;

			case LOGSEVERITY_ERROR:
				strLogLevel = _T("ERROR");
				break;

			case LOGSEVERITY_FATAL:
				strLogLevel = _T("FATAL");
				break;
			default:
				strLogLevel = _T("N/A");
				break;
		}
		HWND hWindow = GetSafeParentWnd(browser);
		if (SafeWnd(hWindow))
		{
			DebugWndLogData dwLogData;
			dwLogData.mHWND.Format(_T("CV_WND:0x%08x"), hWindow);
			dwLogData.mFUNCTION_NAME = _T("ConsoleMessage");
			dwLogData.mMESSAGE1 = message.c_str();
			dwLogData.mMESSAGE2 = strLogLevel;
			dwLogData.mMESSAGE3.Format(_T("Source:%s"), source.c_str());
			dwLogData.mMESSAGE4.Format(_T("Line:%d"), line);
			theApp.AppendDebugViewLog(dwLogData);
		}
	}

	if (theApp.m_AppSettings.IsAdvancedLogMode())
	{
		if (theApp.m_AppSettings.GetAdvancedLogLevel() == DEBUG_LOG_LEVEL_OUTPUT_ALL)
		{
			CString strLogPath;
			strLogPath = theApp.m_strCEFCachePath;
			strLogPath += _T("\\console.log");
			strWriteLine.Format(_T("Message:%s\nSource:%s\nLine:%d\n"), message.c_str(), source.c_str(), line);
			_wsetlocale(LC_ALL, _T("jpn"));
			CStdioFile stdFile;
			if (stdFile.Open(strLogPath, CFile::modeWrite | CFile::shareDenyNone | CFile::modeCreate | CFile::modeNoTruncate))
			{
				TRY
				{
					stdFile.SeekToEnd();
					stdFile.WriteString(strWriteLine);
				}
				CATCH(CFileException, eP) {}
				END_CATCH
				stdFile.Close();
			}
			_wsetlocale(LC_ALL, _T(""));
			return TRUE;
		}
	}
	return TRUE;
}

void ClientHandler::OnBeforeDownload(CefRefPtr<CefBrowser> browser,
				     CefRefPtr<CefDownloadItem> download_item,
				     const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback)
{
	REQUIRE_UI_THREAD();

	//Download禁止
	if (theApp.m_AppSettings.IsEnableDownloadRestriction())
	{
		HWND hWindow = GetSafeParentWnd(browser);
		if (SafeWnd(hWindow))
		{
			HWND hWindowFrm = GetParent(hWindow);
			if (SafeWnd(hWindowFrm))
				hWindow = hWindowFrm;
			theApp.SB_MessageBox(hWindow, _T("ファイル ダウンロードは、システム管理者により制限されています。"), NULL, MB_OK | MB_ICONWARNING, TRUE);
		}
		EmptyWindowClose(browser);
		return;
	}

	m_bDownLoadStartFlg = TRUE;
	CString strFileName;
	strFileName = suggested_name.c_str();
	strFileName.TrimLeft();
	strFileName.TrimRight();
	//ファイル名に使えない文字を置き換える。
	strFileName = SBUtil::GetValidFileName(strFileName);

	CString strPath;
	if (theApp.IsSGMode())
	{
		strPath = theApp.m_AppSettings.GetRootPath();
		if (strPath.IsEmpty())
			strPath = _T("B:\\");
	}
	else
	{
		strPath = SBUtil::GetDownloadFolderPath();
	}
	strPath = strPath.TrimRight('\\');
	strPath += _T("\\");

	if (!theApp.m_strLastSelectFolderPath.IsEmpty())
	{
		if (theApp.IsFolderExists(theApp.m_strLastSelectFolderPath))
		{
			strPath = theApp.m_strLastSelectFolderPath;
		}
	}
	HWND hWindow = GetSafeParentWnd(browser);
	if (SafeWnd(hWindow))
	{
		UINT nBrowserId = browser->GetIdentifier();
		CWnd* pCWnd = CWnd::FromHandle(hWindow);

		CString strURL;
		CefString strURLC;
		strURLC = browser->GetMainFrame()->GetURL();
		strURL = strURLC.c_str();
		if (strURL.IsEmpty())
			::SendMessageTimeout(hWindow, WM_APP_CEF_DOWNLOAD_BLANK_PAGE, (WPARAM)TRUE, NULL, SMTO_NORMAL, 1000, NULL);
		else
			::SendMessageTimeout(hWindow, WM_APP_CEF_DOWNLOAD_BLANK_PAGE, (WPARAM)FALSE, NULL, SMTO_NORMAL, 1000, NULL);

		SendMessageTimeout(hWindow, WM_APP_CEF_WINDOW_ACTIVATE, (WPARAM)NULL, (LPARAM)NULL, SMTO_NORMAL, 1000, NULL);

		//ダウンロード中の場合は、警告を表示する。
		if (theApp.m_DlMgr.IsDlProgress(nBrowserId))
		{
			HWND hWindowFrm = GetParent(hWindow);
			int iRet = theApp.SB_MessageBox(hWindowFrm, _T("別のダウンロード処理を実行中です。\n終了するまで次のファイルのダウンロードは出来ません。"), NULL, MB_OK | MB_ICONWARNING, TRUE);
			return;
		}

		CString szFilter;
		szFilter = _T("すべてのファイル(*.*)|*.*||");
		CString strTitle;
		strTitle = _T("ダウンロードファイルを保存");
		CStringW strCaption(theApp.m_strThisAppName);
		CStringW strRootDrive(theApp.m_AppSettings.GetRootPath());
		CStringW strMsg;
		INT_PTR bRet = FALSE;

		CFileDialog* pFileDlg = NULL;
		if (theApp.IsSGMode())
		{
			//SGModeの場合は、Classicダイアログを使用
			pFileDlg = new CFileDialog(FALSE,
						   NULL, strFileName, OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, szFilter, pCWnd, 0, FALSE);
		}
		else
		{
			pFileDlg = new CFileDialog(FALSE,
						   NULL, strFileName, OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, szFilter, pCWnd);
		}
		pFileDlg->m_ofn.lpstrTitle = strTitle.GetString();
		pFileDlg->m_ofn.lpstrInitialDir = strPath;

		WCHAR szSelPath[MAX_PATH + 1] = {0};
		bRet = pFileDlg->DoModal();
		if (bRet == IDOK)
		{
			memset(szSelPath, 0x00, sizeof(WCHAR) * MAX_PATH);
			lstrcpynW(szSelPath, pFileDlg->GetPathName(), MAX_PATH);

			WCHAR szSelFolderPath[MAX_PATH + 1] = {0};
			lstrcpynW(szSelFolderPath, pFileDlg->GetPathName(), MAX_PATH);
			PathRemoveFileSpec(szSelFolderPath);
			theApp.m_strLastSelectFolderPath = szSelFolderPath;

			strPath = pFileDlg->GetPathName();
			if (!strPath.IsEmpty())
			{
				CefString strcfPath(strPath);
				callback->Continue(strcfPath, false);

				if (theApp.m_AppSettings.IsEnableLogging() && theApp.m_AppSettings.IsEnableDownloadLogging())
				{
					CString strFileName;
					TCHAR* ptrFile = NULL;
					ptrFile = PathFindFileName(strPath);
					if (ptrFile)
					{
						strFileName = ptrFile;
					}
					if (strURL.IsEmpty())
					{
						strURL = download_item->GetURL().c_str();
					}
					theApp.m_pLogDisp->SendLog(LOG_DOWNLOAD, strFileName, strURL);
				}
				::SendMessageTimeout(hWindow, WM_APP_CEF_BEFORE_DOWNLOAD, (WPARAM)TRUE, NULL, SMTO_NORMAL, 1000, NULL);
				theApp.m_DlMgr.Init_DLDlg(theApp.m_pMainWnd, nBrowserId);
				theApp.m_DlMgr.SetDlProgress(nBrowserId, TRUE);
			}
		}
		else
		{
			theApp.m_DlMgr.SetDlProgress(nBrowserId, FALSE);
			::SendMessageTimeout(hWindow, WM_APP_CEF_BEFORE_DOWNLOAD, (WPARAM)FALSE, NULL, SMTO_NORMAL, 1000, NULL);
			EmptyWindowClose(browser);
			callback->Continue(_T(""), false);
		}
		if (pFileDlg)
		{
			delete pFileDlg;
			pFileDlg = NULL;
		}
		return;
	}
	callback->Continue(_T(""), false);
}

void ClientHandler::OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback)
{
	///https://www.catalog.update.microsoft.com/Search.aspx?q=KB4051963

	REQUIRE_UI_THREAD();
	//Download禁止
	if (theApp.m_AppSettings.IsEnableDownloadRestriction())
	{
		return;
	}

	CEFDownloadItemValues values={0};

	values.bIsValid = download_item->IsValid();
	values.bIsInProgress = download_item->IsInProgress();
	values.bIsComplete = download_item->IsComplete();
	values.bIsCanceled = download_item->IsCanceled();
	values.nProgress = download_item->GetPercentComplete();
	values.nSpeed = download_item->GetCurrentSpeed();
	values.nReceived = download_item->GetReceivedBytes();
	values.nTotal = download_item->GetTotalBytes();

	if (download_item->IsValid())
	{
		CefString cefFulPath = download_item->GetFullPath();
		if(cefFulPath.c_str())
			lstrcpyn(values.szFullPath, cefFulPath.c_str(), 512);
	}
	HWND hWindow = GetSafeParentWnd(browser);
	UINT nBrowserId = browser->GetIdentifier();
	//theApp.m_DlMgr.SetDlProgress(nBrowserId, FALSE);
	if (SafeWnd(hWindow))
	{
		if (values.bIsComplete)
		{
			theApp.m_DlMgr.SetDlProgress(nBrowserId, FALSE);
			::SendMessageTimeout(hWindow, WM_APP_CEF_DOWNLOAD_UPDATE, (WPARAM)FALSE, NULL, SMTO_NORMAL, 1000, NULL);
			theApp.m_DlMgr.DLComp_DLDlg(nBrowserId, values.szFullPath);
			EmptyWindowClose(browser);
			m_bDownLoadStartFlg = FALSE;
			return;
		}

		if (values.bIsInProgress)
		{
			if (values.bIsValid)
			{
				if (!m_bDownLoadStartFlg)
					return;
				CString strFilePath;
				strFilePath = values.szFullPath;

				//if (!strFilePath.IsEmpty() && ::PathFileExists(strFilePath))
				{
					theApp.m_DlMgr.SetDlProgress(nBrowserId, TRUE);
					::SendMessageTimeout(hWindow, WM_APP_CEF_DOWNLOAD_UPDATE, (WPARAM)TRUE, NULL, SMTO_NORMAL, 1000, NULL);

					if (values.nProgress < 98)
					{
						CString szStatus;
						szStatus = FormatTransferInfo(values.nReceived, values.nTotal, values.nProgress);
						theApp.m_DlMgr.Show_DLDlg(TRUE, nBrowserId);
						CString strSpeed;
						// format read
						if (values.nSpeed < 1024)
							strSpeed.Format(_T("%I64u B / 秒"), values.nSpeed);
						else if (values.nSpeed < 1048576)
							strSpeed.Format(_T("%0.1f KB / 秒"), static_cast<LONGLONG>(values.nSpeed) / 1024.0);
						else
							strSpeed.Format(_T("%0.2f MB / 秒"), static_cast<LONGLONG>(values.nSpeed) / 1048576.0);
						theApp.m_DlMgr.Set_DLDlgState(nBrowserId, values.nProgress, values.szFullPath, szStatus, strSpeed);

						if (theApp.m_DlMgr.IsCanceld(nBrowserId))
						{
							theApp.m_DlMgr.SetDlProgress(nBrowserId, FALSE);
							::SendMessageTimeout(hWindow, WM_APP_CEF_DOWNLOAD_UPDATE, (WPARAM)FALSE, NULL, SMTO_NORMAL, 1000, NULL);

							callback->Cancel();
							theApp.m_DlMgr.Release_DLDlg(nBrowserId);
							EmptyWindowClose(browser);
							m_bDownLoadStartFlg = FALSE;
							return;
						}
					}
				}
				if (theApp.m_DlMgr.IsCanceld(nBrowserId))
				{
					theApp.m_DlMgr.SetDlProgress(nBrowserId, FALSE);
					::SendMessageTimeout(hWindow, WM_APP_CEF_DOWNLOAD_UPDATE, (WPARAM)FALSE, NULL, SMTO_NORMAL, 1000, NULL);

					callback->Cancel();
					theApp.m_DlMgr.Release_DLDlg(nBrowserId);
					EmptyWindowClose(browser);
					m_bDownLoadStartFlg = FALSE;
					return;
				}

				//else
				//{
				//	if (theApp.m_DlMgr.IsCanceld(nBrowserId))
				//	{
				//		theApp.m_DlMgr.SetDlProgress(nBrowserId, FALSE);
				//		::SendMessageTimeout(hWindow, WM_APP_CEF_DOWNLOAD_UPDATE, (WPARAM)FALSE, NULL, SMTO_NORMAL, 1000, NULL);

				//		callback->Cancel();
				//		theApp.m_DlMgr.Release_DLDlg(nBrowserId);
				//		EmptyWindowClose(browser);
				//		m_bDownLoadStartFlg = FALSE;
				//		return;
				//	}
				//}
				return;
			}
			if (values.bIsCanceled)
			{
				if (theApp.m_DlMgr.IsCanceld(nBrowserId))
				{
					theApp.m_DlMgr.SetDlProgress(nBrowserId, FALSE);
					::SendMessageTimeout(hWindow, WM_APP_CEF_DOWNLOAD_UPDATE, (WPARAM)FALSE, NULL, SMTO_NORMAL, 1000, NULL);

					callback->Cancel();
					theApp.m_DlMgr.Release_DLDlg(nBrowserId);
					EmptyWindowClose(browser);
					m_bDownLoadStartFlg = FALSE;
					return;
				}
				return;
			}
		}
	}
}
cef_return_value_t ClientHandler::OnBeforeResourceLoad(
	CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefRequest> request,
	CefRefPtr<CefRequestCallback> callback)
{
	PROC_TIME(OnBeforeResourceLoad)

	CefRequest::HeaderMap cefHeaders;
	request->GetHeaderMap(cefHeaders);
	cefHeaders.insert(std::make_pair("DNT", "1"));
	request->SetHeaderMap(cefHeaders);

	CefString cefURL = request->GetURL();
	CString strTranURL(cefURL.c_str());
	CString logmsg;

	HWND hWindow = GetSafeParentWnd(browser);

	if (SafeWnd(hWindow))
	{
		DebugWndLogData dwLogData;
		dwLogData.mHWND.Format(_T("CV_WND:0x%08x"), hWindow);
		dwLogData.mFUNCTION_NAME = _T("OnBeforeResourceLoad");
		dwLogData.mMESSAGE1 = strTranURL;
		dwLogData.mMESSAGE2.Format(_T("FrameName:%s"), frame->GetName().c_str());
		dwLogData.mMESSAGE3.Format(_T("IsMain:%s"), frame->IsMain() ? _T("TRUE") : _T("FALSE"));
		theApp.AppendDebugViewLog(dwLogData);
		logmsg = dwLogData.GetString();
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_URL);
	}

	if (theApp.m_AppSettings.IsEnableLogging() && theApp.m_AppSettings.IsEnableAccessAllLogging())
	{
		CefURLParts cfURLpa;
		CString strHost;
		if (CefParseURL(cefURL, cfURLpa))
		{
			CefString cfHost(&cfURLpa.host);
			strHost = cfHost.c_str();
		}
		if (SBUtil::IsURL_HTTP(strTranURL))
		{
			if (theApp.m_pLogDisp)
			{
				theApp.m_pLogDisp->SendLog(LOG_ACCESS_ALL, strHost, strTranURL);
			}
		}
	}

	if (!theApp.m_AppSettings.IsEnableURLFilter())
		return RV_CONTINUE;

	// get URL requested
	CefString newURL = request->GetURL();
	CefURLParts cfURLparts;
	LPCTSTR pszURL = NULL;
	pszURL = newURL.c_str();
	if (CefParseURL(newURL, cfURLparts))
	{
		CefString cfScheme(&cfURLparts.scheme);
		CefString cfHost(&cfURLparts.host);
		CefString cfPath(&cfURLparts.path);
		//CefString cfQuery(&cfURLparts.query);

		CString strScheme(cfScheme.c_str());
		CString strHost(cfHost.c_str());
		CString strPath(cfPath.c_str());

		if (strScheme.Find(_T("http")) != 0) //http|https
			return RV_CONTINUE;

		if (strHost.IsEmpty())
			return RV_CONTINUE;

		if (strPath.IsEmpty())
			strPath = _T("/");
//2019-05-14 GoogleDrive側が変更されたようなので、正常に動作する。対処コードをコメントアウト
//		if(strHost==_T("drive.google.com"))
//		{
//			request->GetHeaderMap(cefHeaders);
//			cefHeaders.erase("User-Agent");
//			cefHeaders.insert(std::make_pair("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:65.0) Gecko/20100101 Firefox/65.0"));
//			request->SetHeaderMap(cefHeaders);
//		}

		//2021-01-07Googleにログインできない。。。
		//調査結果、FirefoxにすればOK, Edge/87.0.0.0をつけてもOK
		//デフォルトのUAをEdgeに変更する対応にする。
		//if(strHost==_T("accounts.google.com"))
		//{
		//	request->GetHeaderMap(cefHeaders);
		//	cefHeaders.erase("User-Agent");
		//	cefHeaders.insert(std::make_pair("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:84.0) Gecko/20100101 Firefox/84.0"));
		//	cefHeaders.insert(std::make_pair("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.88 Safari/537.36 Edg/87.0.664.66"));
		//	cefHeaders.insert(std::make_pair("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.88 Safari/537.36 Edg/87.0.0.0"));
		//	request->SetHeaderMap(cefHeaders);
		//}

		CString strURLChk; //Queryを除く。無駄な情報を省く。
		//strURLChk.Format(_T("%s://%s%s"), strScheme, strHost, strPath);
		strURLChk.Format(_T("%s://%s"), strScheme, strHost);

		if (theApp.IsURLFilterAllow(strURLChk, strScheme, strHost, strPath))
		{
			return RV_CONTINUE;
		}
		else
		{
			callback->Continue(false);
			return RV_CANCEL;
		}
	}
	return RV_CONTINUE;
}

void ClientHandler::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type)
{
	REQUIRE_UI_THREAD();

	// The frame window will be the parent of the browser window
	if (frame->IsMain())
	{
		HWND hWindow = GetSafeParentWnd(browser);
		if (SafeWnd(hWindow))
		{

			// send message
			::SendMessageTimeout(hWindow, WM_APP_CEF_LOAD_START, NULL, NULL, SMTO_NORMAL, 1000, NULL);
		}
	}
	// call parent
	CefLoadHandler::OnLoadStart(browser, frame, transition_type);
}

void ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
	REQUIRE_UI_THREAD();

	// The frame window will be the parent of the browser window
	if (frame->IsMain())
	{
		HWND hWindow = GetSafeParentWnd(browser);
		if (SafeWnd(hWindow))
		{
			// send message
			::SendMessageTimeout(hWindow, WM_APP_CEF_LOAD_END, httpStatusCode, NULL, SMTO_NORMAL, 1000, NULL);
			if (httpStatusCode == 200)
			{
				if (theApp.m_AppSettings.IsEnableCustomScript())
				{
					CString strURL;
					CefString strURLC;
					strURLC = browser->GetMainFrame()->GetURL();
					strURL = strURLC.c_str();
					CString strRetFileName;
					CStringArray stArr;
					if (theApp.m_cCustomScriptList.HitWildCardURL(strURL, &stArr))
					{
						CefLoadHandler::OnLoadEnd(browser, frame, httpStatusCode);
						for (INT_PTR i = 0; i < stArr.GetSize(); i++)
						{
							strRetFileName = stArr.GetAt(i);
							if (!strRetFileName.IsEmpty())
							{
								CString strCustomScriptFullPath = theApp.m_strExeFolderPath;
								strCustomScriptFullPath += strRetFileName;
								if (PathFileExists(strCustomScriptFullPath))
								{
									CString logmsg;
									CScriptSrcMgr cScriptSrc;
									cScriptSrc.SetFilePathAndSetData(strCustomScriptFullPath);
									CString strJSsrc;
									DebugWndLogData dwLogData;
									HWND hWindowFrm = GetParent(hWindow);
									if (SafeWnd(hWindowFrm))
									{
										dwLogData.mHWND.Format(_T("BF_WND:0x%08x"), hWindowFrm);
										hWindow = hWindowFrm;
									}
									else
									{
										dwLogData.mHWND.Format(_T("CV_WND:0x%08x"), hWindow);
									}
									dwLogData.mFUNCTION_NAME = _T("ExcuteCustomJS");
									dwLogData.mMESSAGE1 = strURL;
									dwLogData.mMESSAGE2.Format(_T("FileName:%s"), strRetFileName);
									theApp.AppendDebugViewLog(dwLogData);
									logmsg = dwLogData.GetString();
									theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_URL);

									//最初だけ
									if (i == 0)
										strJSsrc.Format(_T("const ChronosExtParentWnd=\"%d\";\r\ntry{ChronosExt_AppActive(ChronosExtParentWnd);console.log('##CSG_Script:ChronosExtParentWnd:'+ChronosExtParentWnd);}catch(e){}\r\n"), hWindow);

									strJSsrc += cScriptSrc.m_strSrc;
									CefString strCefJsStr(strJSsrc);
									frame->ExecuteJavaScript(strCefJsStr, strURLC, 0);
								}
							}
						}
						return;
					}
				}
			}
		}
	}
	// call parent
	CefLoadHandler::OnLoadEnd(browser, frame, httpStatusCode);
}
#define ERR_PROXY_CONNECTION_FAILED -130
void ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
	REQUIRE_UI_THREAD();
	PROC_TIME(OnLoadError)

	// Don't display an error for downloaded files.
	if (errorCode == ERR_ABORTED)
		return;

	if (errorCode == ERR_UNKNOWN_URL_SCHEME)
	{
		return;
	}
	CString strErrorCode;
	strErrorCode.Format(_T("%d"), errorCode);
	CString strErrorHTMLFmt;

	CString strErrorMsg;
	strErrorMsg = _T("一般的なネットワークエラー");
	switch (errorCode)
	{
		case ERR_NONE:
		{
			strErrorMsg = _T("N/A");
			break;
		}
		case ERR_FAILED:
		{
			strErrorMsg = _T("一般的なネットワークエラー");
			break;
		}
		case ERR_ABORTED:
		{
			strErrorMsg = _T("キャンセルされました。");
			 break;
		}
		case ERR_INVALID_ARGUMENT:
		{
			strErrorMsg = _T("引数が不正です。");
			break;
		}
		case ERR_INVALID_HANDLE:
		{
			strErrorMsg = _T("ハンドルが不正です。");
			break;
		}
		case ERR_FILE_NOT_FOUND:
		{
			strErrorMsg = _T("ファイルが見つかりません。");
			break;
		}
		case ERR_TIMED_OUT:
		{
			strErrorMsg = _T("タイムアウトしました。");
			break;
		}
		case ERR_FILE_TOO_BIG:
		{
			strErrorMsg = _T("ファイルサイズが大きすぎます。");
			break;
		}
		case ERR_UNEXPECTED:
		{
			strErrorMsg = _T("予期せぬ例外が発生しました。");
			break;
		}
		case ERR_ACCESS_DENIED:
		{
			strErrorMsg = _T("アクセスが拒否されました。");
			break;
		}
		case ERR_NOT_IMPLEMENTED:
		{
			strErrorMsg = _T("実装されていません。");
			break;
		}
		case ERR_CONNECTION_CLOSED:
		{
			strErrorMsg = _T("接続が閉じられました (TCP FIN)”");
			break;
		}
		case ERR_CONNECTION_RESET:
		{
			strErrorMsg = _T("接続がリセットされました (TCP RST)");
			break;
		}
		case ERR_CONNECTION_REFUSED:
		{
			strErrorMsg = _T("サーバーに接続を拒否されました。");
			break;
		}
		case ERR_CONNECTION_ABORTED:
		{
			strErrorMsg = _T("接続が中断されました (no ACK received)");
			break;
		}
		case ERR_CONNECTION_FAILED:
		{
			strErrorMsg = _T("接続に失敗しました。");
			break;
		}
		case ERR_NAME_NOT_RESOLVED:
		{
			strErrorMsg = _T("ホスト名が解決できません。");
			break;
		}
		case ERR_INTERNET_DISCONNECTED:
		{
			strErrorMsg = _T("インターネット接続が失われました。");
			break;
		}
		case ERR_SSL_PROTOCOL_ERROR:
		{
			strErrorMsg = _T("SSL プロトコルエラー");
			break;
		}
		case ERR_ADDRESS_INVALID:
		{
			strErrorMsg = _T("不正な IP アドレス または ポート番号です。");
			break;
		}
		case ERR_ADDRESS_UNREACHABLE:
		{
			strErrorMsg = _T("到達不能な IP アドレスです。");
			break;
		}
		case ERR_SSL_CLIENT_AUTH_CERT_NEEDED:
		{
			strErrorMsg = _T("サーバーがSSLクライアント認証用の証明書を要求しました。");
			break;
		}
		case ERR_TUNNEL_CONNECTION_FAILED:
		{
			strErrorMsg = _T("トンネル接続に失敗しました。");
			break;
		}
		case ERR_NO_SSL_VERSIONS_ENABLED:
		{
			strErrorMsg = _T("利用可能なSSLプロトコルバージョンがありません。");
			break;
		}
		case ERR_SSL_VERSION_OR_CIPHER_MISMATCH:
		{
			strErrorMsg = _T("クライアントとサーバーは、一般的なSSLプロトコルのバージョンまたは Cipher Suite をサポートしていません。");
			break;
		}
		case ERR_SSL_RENEGOTIATION_REQUESTED:
		{
			strErrorMsg = _T("サーバーが再ネゴシエーションを要求しました。 (再ハンドシェイク)");
			break;
		}
		case ERR_CERT_COMMON_NAME_INVALID:
		{
			strErrorMsg = _T("サーバーは、証明書の共通名とホスト名が一致しない証明書であると返しました。");
			break;
		}
		case ERR_CERT_DATE_INVALID:
		{
			strErrorMsg = _T("サーバーは、証明書の期限が不正であると返しました。");
			break;
		}
		case ERR_CERT_AUTHORITY_INVALID:
		{
			strErrorMsg = _T("サーバーは、信頼されていない機関によって署名された証明書であると返しました。");
			break;
		}
		case ERR_CERT_CONTAINS_ERRORS:
		{
			strErrorMsg = _T("サーバーは、エラーを含む証明書であると返しました。");
			break;
		}
		case ERR_CERT_NO_REVOCATION_MECHANISM:
		{
			strErrorMsg = _T("この証明書には、失効したかを判断する仕組みがありません。");
			break;
		}
		case ERR_CERT_UNABLE_TO_CHECK_REVOCATION:
		{
			strErrorMsg = _T("このサイトのセキュリティ証明書の失効情報は利用できません。");
			break;
		}
		case ERR_CERT_REVOKED:
		{
			strErrorMsg = _T("サーバーは、証明書が失効していると返しました。");
			break;
		}
		case ERR_CERT_INVALID:
		{
			strErrorMsg = _T("サーバーは、証明書が無効であると返しました。");
			break;
		}
		case ERR_CERT_WEAK_SIGNATURE_ALGORITHM:
		{
			strErrorMsg = _T("サーバーは、脆弱な署名アルゴリズムを使用して署名された証明書であると返しました。");
			break;
		}
		case ERR_CERT_NON_UNIQUE_NAME:
		{
			strErrorMsg = _T("証明書に指定されたホスト名はユニークでありません。");
			break;
		}
		case ERR_CERT_WEAK_KEY:
		{
			strErrorMsg = _T("サーバーは、証明書が脆弱なキーを含んでいると返しました。");
			break;
		}
		case ERR_CERT_NAME_CONSTRAINT_VIOLATION:
		{
			strErrorMsg = _T("サーバーは、証明書のDNS名が命名規約に違反していると返しました。");
			break;
		}
		case ERR_CERT_VALIDITY_TOO_LONG:
		{
			strErrorMsg = _T("証明書の有効期間が長すぎます");
			break;
		}
		case ERR_INVALID_URL:
		{
			strErrorMsg = _T("URLが不正です。");
			break;
		}
		case ERR_DISALLOWED_URL_SCHEME:
		{
			strErrorMsg = _T("許可されていないURLスキーマです。");
			break;
		}
		case ERR_UNKNOWN_URL_SCHEME:
		{
			strErrorMsg = _T("未定義のURLスキーマです。");
			break;
		}
		case ERR_TOO_MANY_REDIRECTS:
		{
			strErrorMsg = _T("このウェブページにはリダイレクトループが含まれています。");
			break;
		}
		case ERR_UNSAFE_REDIRECT:
		{
			strErrorMsg = _T("安全でないリダイレクトを要求しました。");
			break;
		}
		case ERR_UNSAFE_PORT:
		{
			strErrorMsg = _T("安全でないポートを要求しました。");
			break;
		}
		case ERR_INVALID_RESPONSE:
		{
			strErrorMsg = _T("レスポンスが不正です。");
			break;
		}
		case ERR_INVALID_CHUNKED_ENCODING:
		{
			strErrorMsg = _T("不正なチャンクエンコードです。");
			break;
		}
		case ERR_METHOD_NOT_SUPPORTED:
		{
			strErrorMsg = _T("メソッドがサポートされていません。");
			break;
		}
		case ERR_UNEXPECTED_PROXY_AUTH:
		{
			strErrorMsg = _T("Proxyサーバー認証エラー");
			break;
		}
		case ERR_EMPTY_RESPONSE:
		{
			strErrorMsg = _T("レスポンスが空白です。");
			break;
		}
		case ERR_RESPONSE_HEADERS_TOO_BIG:
		{
			strErrorMsg = _T("レスポンスヘッダーが大きすぎます。");
			break;
		}
		case ERR_CACHE_MISS:
		{
			strErrorMsg = _T("キャッシュ ミスが発生しました。");
			break;
		}
		case ERR_INSECURE_RESPONSE:
		{
			strErrorMsg = _T("安全でないレスポンスが発生しました。");
			break;
		}
		case ERR_PROXY_CONNECTION_FAILED:
		{
			strErrorMsg = _T("Proxyサーバーに接続できません。");
			break;
		}
		default:
			break;
	}

	CString strFaildUrl(failedUrl.c_str());
	strFaildUrl.Replace(_T("<"), _T(""));
	strFaildUrl.Replace(_T(">"), _T(""));
	strFaildUrl.Replace(_T("&"), _T(""));
	strFaildUrl.Replace(_T("\""), _T(""));
	strFaildUrl.Replace(_T("'"), _T(""));
	strFaildUrl.Replace(_T("`"), _T(""));
	strFaildUrl.Replace(_T("javascript"), _T(""));
	strFaildUrl.Replace(_T("eval("), _T(""));
	strErrorHTMLFmt = _T("<html><head><meta http-equiv='Content-Type' content='text/html; charset=Shift_JIS'/><title>読込エラー</title>");
	strErrorHTMLFmt += _T("<style type='text/css'>");
	strErrorHTMLFmt += _T("*{margin:0;padding:0;font-weight:normal;color:#444;}");
	strErrorHTMLFmt += _T("html{overflow-y:scroll;background-color:#5b85b0;}");
	strErrorHTMLFmt += _T("body{margin:0;line-height:1.4;font-size:75%;text-align:center;}");
	strErrorHTMLFmt += _T("#base{width:100%;height:100%;margin:0 auto;padding:0;text-align:left;background-color:#5b85b0;}");
	strErrorHTMLFmt += _T("#container{margin:0;padding:0;width:100%;height:100%;text-align:center;}");
	strErrorHTMLFmt += _T("h1{padding-top:20px;font-size:24px;font-weight:bold;color:white;}");
	strErrorHTMLFmt += _T("h2{text-align:center;padding:0;padding-top:60px;color:white;font-family:Helvetica,sans-serif;font-weight:bold;font-size:48px;}");
	strErrorHTMLFmt += _T("#cause{padding-top:20px;margin:0 auto;width:90%;text-align:left;}");
	strErrorHTMLFmt += _T("h3{margin-top:80px;font-size:24px;color:white;font-weight:bold;padding:0}");
	strErrorHTMLFmt += _T("#cause h4{color:#666;font-size:24px;font-weight:bold;}");
	strErrorHTMLFmt += _T("#white_box{margin:0 auto;width:90%;height:40%;background-color:white;text-align:left}");
	strErrorHTMLFmt += _T("</style></head><body>");
	strErrorHTMLFmt += _T("<div id='base'><div id='container'><h1>アクセスしようとしたページが表示できませんでした。</h1>");
	strErrorHTMLFmt += _T("<h2>");
	strErrorHTMLFmt += strErrorMsg;
	strErrorHTMLFmt += _T("</h2>");
	strErrorHTMLFmt += _T("<h3>詳細</h3><div id='white_box'>");
	strErrorHTMLFmt += _T("<div id='cause'>");
	strErrorHTMLFmt += _T("<h4>URL：<a href='");
	strErrorHTMLFmt += strFaildUrl;
	strErrorHTMLFmt += _T("'/>");
	strErrorHTMLFmt += strFaildUrl;
	strErrorHTMLFmt += _T("</a></h4>");
	strErrorHTMLFmt += _T("<h4>エラーコード：");
	strErrorHTMLFmt += strErrorCode;
	strErrorHTMLFmt += _T("</h4>");
	strErrorHTMLFmt += _T("<h4>エラー内容：");
	strErrorHTMLFmt += strErrorMsg;
	strErrorHTMLFmt += _T("</h4>");
	strErrorHTMLFmt += _T("<h4>エラー名称：");
	strErrorHTMLFmt += errorText.c_str();
	strErrorHTMLFmt += _T("</h4></div></div></div></div></body></html>");

	CefString strCefErrorHTML(strErrorHTMLFmt);

	CString strJsStr;
	strJsStr = _T("var msgDiv = document.createElement('div');\n");
	strJsStr += _T("msgDiv.innerHTML=\"");

	strJsStr += strErrorHTMLFmt;

	strJsStr += _T("\";\n");
	strJsStr += _T("if (document.getElementsByTagName('body').length){ \n");
	strJsStr += _T("    document.getElementsByTagName('body')[0].appendChild(msgDiv); \n");
	strJsStr += _T("}\n");
	//	strJsStr += _T("    alert('appended');\n}\n");
//	strJsStr += _T("else alert('no body to append!!');");

	CefString strCefJsStr(strJsStr);
	frame->ExecuteJavaScript(strCefJsStr, failedUrl, 0);

	if (frame->IsMain())
	{
		HWND hWindow = GetSafeParentWnd(browser);
		if (SafeWnd(hWindow))
			theApp.SetWarmFavicon(hWindow);
	}
	// call parent
	CefLoadHandler::OnLoadError(browser, frame, errorCode, errorText, failedUrl);
}
bool ClientHandler::GetAuthCredentials(CefRefPtr<CefBrowser> browser,
				       const CefString& origin_url,
				       bool isProxy, const CefString& host, int port, const CefString& realm, const CefString& scheme, CefRefPtr<CefAuthCallback> callback)
{
	// The frame window will be the parent of the browser window
	HWND hWindow = GetSafeParentWnd(browser);

	CEFAuthenticationValues values = {0};
	values.lpszHost = host.c_str();
	values.lpszRealm = realm.c_str();
	_tcscpy_s(values.szUserName, _T(""));
	_tcscpy_s(values.szUserPass, _T(""));
	if (SafeWnd(hWindow))
	{
		SendMessageTimeout(hWindow, WM_APP_CEF_WINDOW_ACTIVATE, (WPARAM)NULL, (LPARAM)NULL, SMTO_NORMAL, 1000, NULL);

		CString strHost(values.lpszHost);
		CDlgAuth Dlg(CWnd::FromHandle(hWindow)->GetParent());
		if (isProxy)
			Dlg.m_strMsgTxt.Format(_T("認証が必要です。\nプロキシ[%s]には、ユーザー名とパスワードを指定する必要があります。"), strHost);
		else
			Dlg.m_strMsgTxt.Format(_T("認証が必要です。\n接続先ホスト[%s]には、ユーザー名とパスワードが必要です。"), strHost);
		if (IDOK == Dlg.DoModal())
		{
			_tcscpy_s(values.szUserName, Dlg.m_strID);
			_tcscpy_s(values.szUserPass, Dlg.m_strPW);
			callback->Continue(values.szUserName, values.szUserPass);
			return TRUE;
		}
	}
	// canceled
	return FALSE;
}

CefRefPtr<CefResourceHandler> ClientHandler::GetResourceHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request)
{
	return nullptr;
}

bool ClientHandler::OnQuotaRequest(CefRefPtr<CefBrowser> browser, const CefString& origin_url, int64 new_size, CefRefPtr<CefRequestCallback> callback)
{
	static const int64 max_size = 1024 * 1024 * 20; // 20mb.

	// Grant the quota request if the size is reasonable.
	callback->Continue(new_size <= max_size);

	// call parent
	return CefRequestHandler::OnQuotaRequest(browser, origin_url, new_size, callback);
}

bool ClientHandler::OnCertificateError(CefRefPtr<CefBrowser> browser,
				       ErrorCode cert_error, const CefString& request_url, CefRefPtr<CefSSLInfo> ssl_info, CefRefPtr<CefRequestCallback> callback)
{
	CString szMessage;

	szMessage.Format(_T("安全な接続ではありません。\n危険性を認識したうえで、接続を続行する場合は、「はい」をクリックして下さい。\n\n \"%s\"の所有者によるウェブサイトの設定が不適切です。\n\nあなたの情報が盗まれることを防ぐため、このウェブサイトへの接続は確立されません。\n"), request_url.c_str());
	szMessage += _T("不正なセキュリティ証明書を使用しています。 \n発行者の証明書が不明であるためこの証明書は信頼されません。 \nサーバーが適正な中間証明書を送信しない可能性があります。 \n\n");
	szMessage += _T("「いいえ」をクリックすると接続をキャンセルします。");
	HWND hWindow = GetSafeParentWnd(browser);
	if (hWindow)
	{
		SendMessageTimeout(hWindow, WM_APP_CEF_WINDOW_ACTIVATE, (WPARAM)NULL, (LPARAM)NULL, SMTO_NORMAL, 1000, NULL);
		hWindow = GetParent(hWindow);
	}
	int iRet = theApp.SB_MessageBox(hWindow, szMessage, NULL, MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2, TRUE);
	if (iRet == IDNO)
	{
		return FALSE;
	}

	// continue
	callback->Continue(true);

	return TRUE;
}

void ClientHandler::OnProtocolExecution(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool& allow_os_execution)
{
	// do default
//	CefResourceRequestHandler::OnProtocolExecution(browser,frame, request,allow_os_execution);
//	return;
	allow_os_execution = true;
	browser->StopLoad();
}
void ClientHandler::OnRenderViewReady(CefRefPtr<CefBrowser> browser)
{
}

bool ClientHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool user_gesture, bool is_redirect)
{
	PROC_TIME(OnBeforeBrowse)

	// The frame window will be the parent of the browser window
	HWND hWindow = GetSafeParentWnd(browser);
	if (SafeWnd(hWindow))
	{
		// get URL requested
		CefString newURL = request->GetURL();
		UINT bTopPage = FALSE;
		LPCTSTR pszURL = NULL;
		pszURL = newURL.c_str();

		if (frame)
		{
			if (frame->IsMain())
				bTopPage = TRUE;
		}
		::SendMessageTimeout(hWindow, WM_APP_CEF_BEFORE_BROWSE, (WPARAM)pszURL, (LPARAM)&bTopPage, SMTO_NORMAL, 1000, NULL);
		//ナビゲーションをキャンセルする。
		if (bTopPage == 2)
		{
			return TRUE;
		}
		if (request->GetResourceType() == RT_MAIN_FRAME && frame->IsMain() && is_redirect == FALSE)
		{
			CefString strName;
			strName = frame->GetName();
			CString strWindowName;
			strWindowName = strName.c_str();
			if (!strWindowName.IsEmpty())
			{
				HWND hWindowFrame = {0};
				hWindowFrame = GetParent(hWindow);
				if (SafeWnd(hWindowFrame))
				{
					theApp.ShowWnd(hWindowFrame);
					::SetForegroundWindow(hWindowFrame);
					::BringWindowToTop(hWindowFrame);
				}
			}
		}
	}
	// call parent
	return CefRequestHandler::OnBeforeBrowse(browser, frame, request, user_gesture, is_redirect);
}

void ClientHandler::OnDialogClosed(CefRefPtr<CefBrowser> browser)
{
}

void ClientHandler::OnResetDialogState(CefRefPtr<CefBrowser> browser)
{
}

bool ClientHandler::OnJSDialog(CefRefPtr<CefBrowser> browser,
			       const CefString& origin_url,
			       JSDialogType dialog_type,
			       const CefString& message_text,
			       const CefString& default_prompt_text,
			       CefRefPtr<CefJSDialogCallback> callback,
			       bool& suppress_message)
{
	CefString str;
	HWND hWindow = GetSafeParentWnd(browser);
	if (hWindow)
	{
		SendMessageTimeout(hWindow, WM_APP_CEF_WINDOW_ACTIVATE, (WPARAM)NULL, (LPARAM)NULL, SMTO_NORMAL, 1000, NULL);
		hWindow = GetParent(hWindow);
	}
	LPCTSTR pszMessage = NULL;
	pszMessage = message_text.c_str();
	if (dialog_type == JSDIALOGTYPE_ALERT)
	{
		int iRet = theApp.SB_MessageBox(hWindow, pszMessage, NULL, MB_ICONWARNING | MB_OK | MB_TASKMODAL, TRUE);
		return TRUE;
	}
	else if (dialog_type == JSDIALOGTYPE_CONFIRM)
	{
		int iRet = theApp.SB_MessageBox(hWindow, pszMessage, NULL, MB_ICONWARNING | MB_OKCANCEL | MB_TASKMODAL, TRUE);
		callback->Continue(iRet == IDOK ? true : false, str);
		return TRUE;
	}
	// canceled
	return FALSE;
}

bool ClientHandler::OnBeforeUnloadDialog(CefRefPtr<CefBrowser> browser,
					 const CefString& message_text,
					 bool is_reload,
					 CefRefPtr<CefJSDialogCallback> callback)
{
	CefString str;
	HWND hWindow = GetSafeParentWnd(browser);
	if (hWindow)
	{
		SendMessageTimeout(hWindow, WM_APP_CEF_WINDOW_ACTIVATE, (WPARAM)NULL, (LPARAM)NULL, SMTO_NORMAL, 1000, NULL);
		hWindow = GetParent(hWindow);
	}
	LPCTSTR pszMessage = NULL;
	pszMessage = message_text.c_str();
	CString strMsg;
	strMsg = pszMessage;
	//if(strMsg==_T("Is it OK to leave/reload this page?"))
	{
		strMsg = _T("このサイトを離れますか？\n行った変更が保存されない可能性があります。");
	}
	int iRet = theApp.SB_MessageBox(hWindow, strMsg, NULL, MB_ICONWARNING | MB_YESNO | MB_TASKMODAL, TRUE);
	callback->Continue(iRet == IDYES ? true : false, str);
	return TRUE;
}

bool ClientHandler::OnDragEnter(CefRefPtr<CefBrowser> browser,
				CefRefPtr<CefDragData> dragData,
				DragOperationsMask mask)
{
	CEF_REQUIRE_UI_THREAD();
	if (theApp.IsSGMode())
	{
		//D&Dを基本無効化する。
		return true;
	}

	//Download禁止
	if (theApp.m_AppSettings.IsEnableDownloadRestriction())
	{
		if (dragData->IsLink())
		{
			return true;
		}
	}

	//Upload禁止
	if (theApp.m_AppSettings.IsEnableUploadRestriction())
	{
		if (dragData->IsFile())
		{
			return true;
		}
	}

	if (theApp.m_AppSettings.IsEnableLogging())
	{
		HWND hWindow = GetSafeParentWnd(browser);
		if (dragData->IsFile())
		{
			if (theApp.m_AppSettings.IsEnableUploadLogging())
			{
				CefString cstrFileName;
				CString strFileName;
				std::vector<CefString> cefstrFiles;
				dragData->GetFileNames(cefstrFiles);
				for (UINT i = 0; i < cefstrFiles.size(); i++)
				{
					cstrFileName = cefstrFiles[i];
					strFileName = cstrFileName.c_str();
					{
						theApp.SendLoggingMsg(LOG_UPLOAD, strFileName, hWindow);
					}
				}
			}
		}
		else if (dragData->IsLink())
		{
			if (theApp.m_AppSettings.IsEnableDownloadLogging())
			{
				CefString cstrFileName;
				CString strFileName;
				cstrFileName = dragData->GetLinkURL();
				strFileName = cstrFileName.c_str();
				theApp.m_pLogDisp->SendLog(LOG_DOWNLOAD, strFileName, strFileName);
			}
		}
	}
	//// Forbid dragging of link URLs.
	//if (mask & DRAG_OPERATION_LINK)
	//{
	//	if(dragData->HasImage())
	//		return false;
	//	return true;
	//}
	return false;
}
bool ClientHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
					     CefRefPtr<CefFrame> frame,
					     CefProcessId source_process,
					     CefRefPtr<CefProcessMessage> message)
{
	CefString strName = message->GetName();
	CString strFilterName;
	strFilterName = strName.c_str();
	strFilterName.TrimLeft();
	strFilterName.TrimRight();
	if (strFilterName == _T("SET_PID"))
	{
		CefRefPtr<CefListValue> args = message->GetArgumentList();
		if (args->GetType(0) == VTYPE_INT)
		{
			m_RendererPID = args->GetInt(0);
			CefRefPtr<CefBrowser> pBrowser = browser;
			HWND hWindow = GetSafeParentWnd(browser);
			if (SafeWnd(hWindow))
			{
				::SendMessageTimeout(hWindow, WM_APP_CEF_SET_RENDERER_PID, (WPARAM)m_RendererPID,0, SMTO_NORMAL, 1000, NULL);
			}
		}
		return true;
	}
	return false;
}

bool MyV8Handler::Execute(const CefString& name,
			  CefRefPtr<CefV8Value> object,
			  const CefV8ValueList& arguments,
			  CefRefPtr<CefV8Value>& retval,
			  CefString& exception)
{
	size_t iArgSize = 0;
	iArgSize = arguments.size();
	CString strFmt;
	if (name == "ChronosExt_DlgLogin")
	{
		if (iArgSize == 3 && arguments[0]->IsString() && arguments[1]->IsString() && arguments[2]->IsString())
		{
			CString strText(arguments[0]->GetStringValue().c_str());
			CString strCaption(arguments[1]->GetStringValue().c_str());
			CString strChronosExtParentWnd(arguments[2]->GetStringValue().c_str());
			CWnd* pWnd = NULL;
			HWND hW = 0;
			if (!strChronosExtParentWnd.IsEmpty())
			{
				hW = (HWND)_ttoll(strChronosExtParentWnd);
				pWnd = CWnd::FromHandle(hW);
			}
			if (!theApp.IsWnd(pWnd))
				pWnd = CWnd::GetForegroundWindow();
			if (!theApp.IsWnd(pWnd))
				pWnd = CWnd::GetActiveWindow();

			CDlgAuth Dlg(pWnd);
			CString strID;
			CString strPW;
			Dlg.m_strMsgTxt = strText;
			Dlg.m_strCaption = strCaption;
			INT_PTR iResult = Dlg.DoModal();
			if (iResult == IDOK)
			{
				strID = Dlg.m_strID;
				strPW = Dlg.m_strPW;
				strID.Replace(_T("\\"), _T("\\\\"));
				strID.Replace(_T("\""), _T("\\\""));
				strID.Replace(_T("/"), _T("\\/"));

				strPW.Replace(_T("\\"), _T("\\\\"));
				strPW.Replace(_T("\""), _T("\\\""));
				strPW.Replace(_T("/"), _T("\\/"));

				strFmt.Format(_T("{\"username\": \"%s\",\"password\": \"%s\"}"), strID, strPW);
				retval = CefV8Value::CreateString(strFmt.GetString());
				if (pWnd)
				{
					pWnd->PostMessage(WM_COMMAND, WM_ACTIVE_FRM, 0);
				}
				return true;
			}
			else
				return false;
		}
		else
		{
			exception = CefString(_T("パラメータが不一致です。"));
			return false;
		}
		return false;
	}
	else if (name == "ChronosExt_DlgPWChange")
	{
		if (iArgSize == 4 && arguments[0]->IsString() && arguments[1]->IsString() && arguments[2]->IsString() && arguments[3]->IsString())
		{
			CString strID(arguments[0]->GetStringValue().c_str());
			CString strText(arguments[1]->GetStringValue().c_str());
			CString strCaption(arguments[2]->GetStringValue().c_str());
			CString strChronosExtParentWnd(arguments[3]->GetStringValue().c_str());
			CWnd* pWnd = NULL;
			HWND hW = 0;
			if (!strChronosExtParentWnd.IsEmpty())
			{
				hW = (HWND)_ttoll(strChronosExtParentWnd);
				pWnd = CWnd::FromHandle(hW);
			}
			if (!theApp.IsWnd(pWnd))
				pWnd = CWnd::GetForegroundWindow();
			if (!theApp.IsWnd(pWnd))
				pWnd = CWnd::GetActiveWindow();

			CDlgAuthPWC Dlg(pWnd);
			CString strPW_Current;
			CString strPW_New;
			//CString strPW_New2;

			Dlg.m_strID = strID;
			Dlg.m_strMsgTxt = strText;
			Dlg.m_strCaption = strCaption;
			INT_PTR iResult = Dlg.DoModal();
			if (iResult == IDOK)
			{
				strPW_Current = Dlg.m_strPW_Current;
				strPW_New = Dlg.m_strPW_NEW;
				//strPW_New2 = Dlg.m_strPW_NEW2;

				strID.Replace(_T("\\"), _T("\\\\"));
				strID.Replace(_T("\""), _T("\\\""));
				strID.Replace(_T("/"), _T("\\/"));

				strPW_Current.Replace(_T("\\"), _T("\\\\"));
				strPW_Current.Replace(_T("\""), _T("\\\""));
				strPW_Current.Replace(_T("/"), _T("\\/"));

				strPW_New.Replace(_T("\\"), _T("\\\\"));
				strPW_New.Replace(_T("\""), _T("\\\""));
				strPW_New.Replace(_T("/"), _T("\\/"));

				//strPW_New2.Replace(_T("\\"), _T("\\\\"));
				//strPW_New2.Replace(_T("\""), _T("\\\""));
				//strPW_New2.Replace(_T("/"), _T("\\/"));

				strFmt.Format(_T("{\"username\": \"%s\",\"current_password\": \"%s\",\"new_password\": \"%s\"}"), strID, strPW_Current, strPW_New);
				retval = CefV8Value::CreateString(strFmt.GetString());
				if (pWnd)
				{
					pWnd->PostMessage(WM_COMMAND, WM_ACTIVE_FRM, 0);
				}
				return true;
			}
			else
				return false;
		}
		else
		{
			exception = CefString(_T("パラメータが不一致です。"));
			return false;
		}
		return false;
	}
	else if (name == "ChronosExt_AppActive")
	{
		if (iArgSize == 1 && arguments[0]->IsString())
		{
			CString strChronosExtParentWnd(arguments[0]->GetStringValue().c_str());
			CWnd* pWnd = NULL;
			HWND hW = 0;
			if (!strChronosExtParentWnd.IsEmpty())
			{
				hW = (HWND)_ttoll(strChronosExtParentWnd);
				pWnd = CWnd::FromHandle(hW);
			}
			if (!theApp.IsWnd(pWnd))
				pWnd = CWnd::GetForegroundWindow();
			if (!theApp.IsWnd(pWnd))
				pWnd = CWnd::GetActiveWindow();
			if (pWnd)
			{
				pWnd->PostMessage(WM_COMMAND, WM_ACTIVE_FRM, 0);
			}
			return true;
		}
		return false;
	}
	// Function does not exist.
	return false;
}
