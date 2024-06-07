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

#pragma warning(push, 0)
#include <codeanalysis/warnings.h>
#pragma warning(disable \
		: ALL_CODE_ANALYSIS_WARNINGS)
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_path_util.h"
#include "include/cef_process_util.h"
#include "include/cef_trace.h"
#include "include/wrapper/cef_helpers.h"
#pragma warning(pop)
#include "client_util.h"
#include "DlgAuth.h"
#include "sbcommon.h"

// Required for selecting client certificates
#pragma comment(lib, "Crypt32")
#pragma comment(lib, "cryptui")

// For backward compatibility, use custom dialog for Windows 10
// Because there is a bug that new tab can't be opened after accessing certstore.
// See https://github.com/ThinBridge/Chronos/pull/141
#include "DlgCertification.h"

#pragma warning(push, 0)
#pragma warning(disable : 26812)

// https://magpcss.org/ceforum/apidocs3/projects/(default)/CefMenuModel.html#GetCommandIdAt(int)
#define CH_MENU_INVALID_OR_SEPARATOR (-1)

ClientHandler::ClientHandler()
{
	m_bDownLoadStartFlg = FALSE;
	m_RendererPID = 0;
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
			CString confirmMsg;
			confirmMsg.LoadString(IDS_STRING_CONFIRM_CANCEL_DOWNLOAD);
			int iRet = theApp.SB_MessageBox(hWindow, confirmMsg, NULL, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2, TRUE);
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

	// The frame window will be the parent of the browser window
	HWND hWindow = GetSafeParentWnd(browser);
	if (SafeWnd(hWindow))
	{
		//CEF93からポインタを直接SendMessageで渡すことができなくなった。
		//関数を直接呼び出す
		CChildView* pChild = NULL;
		pChild = theApp.GetChildViewPtr(hWindow);
		if (pChild)
		{
			if (SafeWnd(pChild->m_hWnd))
			{
				if (pChild->m_hWnd == hWindow)
					((CChildView*)pChild)->SetBrowserPtr(nBrowserId, browser);
			}
		}
	}
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
		LPCWSTR pszURL = NULL;
		pszURL = (LPCWSTR)target_url.c_str();
		switch (target_disposition)
		{
		case cef_window_open_disposition_t::CEF_WOD_NEW_FOREGROUND_TAB:
		{
			::SendMessageTimeout(hWindow, WM_NEW_WINDOW_URL, (WPARAM)target_disposition, (LPARAM)pszURL, SMTO_NORMAL, 1000, NULL);
			return true;
		}
		case cef_window_open_disposition_t::CEF_WOD_NEW_BACKGROUND_TAB:
		{
			::SendMessageTimeout(hWindow, WM_NEW_WINDOW_URL, (WPARAM)target_disposition, (LPARAM)pszURL, SMTO_NORMAL, 1000, NULL);
			return true;
		}
		case cef_window_open_disposition_t::CEF_WOD_NEW_WINDOW:
		{
			::SendMessageTimeout(hWindow, WM_NEW_WINDOW_URL, (WPARAM)target_disposition, (LPARAM)pszURL, SMTO_NORMAL, 1000, NULL);
			return true;
		}
		default:
			break;
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
		case cef_window_open_disposition_t::CEF_WOD_NEW_POPUP:
		{
			lRet = ::SendMessage(hWindow, WM_APP_CEF_NEW_WINDOW, (WPARAM)&popupFeatures, (LPARAM)&windowInfo);
			return false;
		}
		case cef_window_open_disposition_t::CEF_WOD_NEW_FOREGROUND_TAB:
		{
#if CHROME_VERSION_MAJOR >= 110
			if (popupFeatures.isPopup)
			{
				// Since CEF110, toolBarVisible and menuBarVisible was removed.
				// When isPopup is true, browser interface elements is hidden,
				// then create new browser window.
				lRet = ::SendMessage(hWindow, WM_APP_CEF_NEW_WINDOW, (WPARAM)&popupFeatures, (LPARAM)&windowInfo);
				return false;
			}
#else
			if (popupFeatures.toolBarVisible)
			{
				if ( //popupFeatures.locationBarVisible==false
				    popupFeatures.menuBarVisible == false)
				{
					lRet = ::SendMessage(hWindow, WM_APP_CEF_NEW_WINDOW, (WPARAM)&popupFeatures, (LPARAM)&windowInfo);
					return false;
				}
			}
#endif
			lRet = ::SendMessage(hWindow, WM_APP_CEF_NEW_WINDOW, (WPARAM)NULL, (LPARAM)&windowInfo);
			return false;
		}
		case cef_window_open_disposition_t::CEF_WOD_CURRENT_TAB:
		case cef_window_open_disposition_t::CEF_WOD_SINGLETON_TAB:
		case cef_window_open_disposition_t::CEF_WOD_NEW_BACKGROUND_TAB:
		case cef_window_open_disposition_t::CEF_WOD_NEW_WINDOW:
		case cef_window_open_disposition_t::CEF_WOD_SAVE_TO_DISK:
		case cef_window_open_disposition_t::CEF_WOD_OFF_THE_RECORD:
		case cef_window_open_disposition_t::CEF_WOD_IGNORE_ACTION:
		{
			lRet = ::SendMessage(hWindow, WM_APP_CEF_NEW_WINDOW, (WPARAM)NULL, (LPARAM)&windowInfo);
			return false;
			;
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
				CString contextMenuOpenLinkTabLabel;
				contextMenuOpenLinkTabLabel.LoadString(ID_CONTEXT_MENU_OPEN_LINK_TAB);
				CefString cefContextMenuOpenLinkTabLabel(contextMenuOpenLinkTabLabel);
				model->InsertItemAt(0, CEF_MENU_ID_OPEN_LINK_NEW, cefContextMenuOpenLinkTabLabel);

				CString contextMenuOpenLinkTabInactiveLabel;
				contextMenuOpenLinkTabInactiveLabel.LoadString(ID_CONTEXT_MENU_OPEN_LINK_TAB_INACTIVE);
				CefString cefContextMenuOpenLinkTabInactiveLabel(contextMenuOpenLinkTabInactiveLabel);
				model->InsertItemAt(1, CEF_MENU_ID_OPEN_LINK_NEW_NOACTIVE, cefContextMenuOpenLinkTabInactiveLabel);
			}
			else
			{
				CString contextMenuOpenLinkWindowLabel;
				contextMenuOpenLinkWindowLabel.LoadString(ID_CONTEXT_MENU_OPEN_LINK_WINDOW);
				CefString cefContextMenuOpenLinkWindowLabel(contextMenuOpenLinkWindowLabel);
				model->InsertItemAt(0, CEF_MENU_ID_OPEN_LINK_NEW, cefContextMenuOpenLinkWindowLabel);

				CString contextMenuOpenLinkWindowInactiveLabel;
				contextMenuOpenLinkWindowInactiveLabel.LoadString(ID_CONTEXT_MENU_OPEN_LINK_WINDOW_INACTIVE);
				CefString cefContextMenuOpenLinkWindowInactiveLabel(contextMenuOpenLinkWindowInactiveLabel);
				model->InsertItemAt(1, CEF_MENU_ID_OPEN_LINK_NEW_NOACTIVE, cefContextMenuOpenLinkWindowInactiveLabel);
			}
			CString contextMenuCopyLinkLabel;
			contextMenuCopyLinkLabel.LoadString(ID_CONTEXT_MENU_COPY_LINK);
			CefString cefContextMenuCopyLinkLabel(contextMenuCopyLinkLabel);
			model->InsertItemAt(2, CEF_MENU_ID_COPY_LINK, cefContextMenuCopyLinkLabel);

			CString contextMenuSaveLinkLabel;
			contextMenuSaveLinkLabel.LoadString(ID_CONTEXT_MENU_SAVE_LINK);
			CefString cefContextMenuSaveLinkLabel(contextMenuSaveLinkLabel);
			model->InsertItemAt(3, CEF_MENU_ID_SAVE_FILE, cefContextMenuSaveLinkLabel);
		}
		if ((Flg & (CM_TYPEFLAG_MEDIA | CM_MEDIATYPE_IMAGE)) != 0)
		{
			if (!params->GetSourceUrl().empty())
			{
				model->Remove(MENU_ID_BACK);
				model->Remove(MENU_ID_FORWARD);
				if (theApp.m_bTabEnable_Init)
				{
					CString contextMenuOpenImgTabLabel;
					contextMenuOpenImgTabLabel.LoadString(ID_CONTEXT_MENU_OPEN_IMG_TAB);
					CefString cefContextMenuOpenImgTabLabel(contextMenuOpenImgTabLabel);
					model->AddItem(CEF_MENU_ID_OPEN_IMG, cefContextMenuOpenImgTabLabel);

					CString contextMenuOpenImgTabInactiveLabel;
					contextMenuOpenImgTabInactiveLabel.LoadString(ID_CONTEXT_MENU_OPEN_IMG_TAB_INACTIVE);
					CefString cefContextMenuOpenImgTabInactiveLabel(contextMenuOpenImgTabInactiveLabel);
					model->AddItem(CEF_MENU_ID_OPEN_IMG_NOACTIVE, cefContextMenuOpenImgTabInactiveLabel);
				}
				else
				{
					CString contextMenuOpenImgWindowLabel;
					contextMenuOpenImgWindowLabel.LoadString(ID_CONTEXT_MENU_OPEN_IMG_WINDOW);
					CefString cefContextMenuOpenImgWindowLabel(contextMenuOpenImgWindowLabel);
					model->AddItem(CEF_MENU_ID_OPEN_IMG, cefContextMenuOpenImgWindowLabel);

					CString contextMenuOpenImgWindowInactiveLabel;
					contextMenuOpenImgWindowInactiveLabel.LoadString(ID_CONTEXT_MENU_OPEN_IMG_WINDOW_INACTIVE);
					CefString cefContextMenuOpenImgWindowInactiveLabel(contextMenuOpenImgWindowInactiveLabel);
					model->AddItem(CEF_MENU_ID_OPEN_IMG_NOACTIVE, cefContextMenuOpenImgWindowInactiveLabel);
				}

				CString contextMenuSaveImgLabel;
				contextMenuSaveImgLabel.LoadString(ID_CONTEXT_MENU_SAVE_IMG);
				CefString cefContextMenuSaveImgLabel(contextMenuSaveImgLabel);
				model->AddItem(CEF_MENU_ID_SAVE_IMG, cefContextMenuSaveImgLabel);

				if (!theApp.IsSGMode())
				{
					CString contextMenuCopyImgLabel;
					contextMenuCopyImgLabel.LoadString(ID_CONTEXT_MENU_COPY_IMG);
					CefString cefContextMenuCopyImgLabel(contextMenuCopyImgLabel);
					model->AddItem(CEF_MENU_ID_IMG_COPY, cefContextMenuCopyImgLabel);
				}

				CString contextMenuCopyImgLinkLabel;
				contextMenuCopyImgLinkLabel.LoadString(ID_CONTEXT_MENU_COPY_IMG_LINK);
				CefString cefContextMenuCopyImgLinkLabel(contextMenuCopyImgLinkLabel);
				model->AddItem(CEF_MENU_ID_IMG_COPY_LINK, cefContextMenuCopyImgLinkLabel);
			}
		}
		if (Flg & CM_TYPEFLAG_SELECTION)
		{
			CString strSelText;
			CefString strCfSt;
			strCfSt = params->GetSelectionText();
			strSelText = (LPCWSTR)strCfSt.c_str();
			strSelText.TrimLeft();
			strSelText.TrimRight();
			if (!strSelText.IsEmpty())
			{
				SBUtil::GetDivChar(strSelText, 48, strSelText, TRUE);
				CString contextMenuSearchLabel;
				contextMenuSearchLabel.LoadString(ID_CONTEXT_MENU_SEARCH);
				CString strFmt;
				strFmt.Format(contextMenuSearchLabel, strSelText);
				CefString strCFmt(strFmt);
				model->InsertItemAt(0, CEF_MENU_ID_OPEN_SEARCH, strCFmt);
			}
		}
	}

	CString contextMenuReloadLabel;
	contextMenuReloadLabel.LoadString(ID_CONTEXT_MENU_RELOAD);
	CefString cefContextMenuReloadLabel(contextMenuReloadLabel);
	model->AddItem(MENU_ID_RELOAD, cefContextMenuReloadLabel);

	CString contextMenuPrintPDFLabel;
	contextMenuPrintPDFLabel.LoadString(ID_CONTEXT_MENU_PRINT_PDF);
	CefString cefContextMenuPrintPDFLabel(contextMenuPrintPDFLabel);
	model->AddItem(CEF_MENU_ID_PRINT_PDF, cefContextMenuPrintPDFLabel);

	CString contextMenuPrintLabel;
	contextMenuPrintLabel.LoadString(ID_CONTEXT_MENU_PRINT);
	CefString cefContextMenuPrintLabel(contextMenuPrintLabel);
	model->AddItem(MENU_ID_PRINT, cefContextMenuPrintLabel);

	
	// メニュー項目調整後、Separatorが連続することがあるので、連続している場合は削除する。
	size_t count = model->GetCount();
	int beforeCommandId = CH_MENU_INVALID_OR_SEPARATOR;
	int commandId;
	for (size_t i = count - 1; i > 0; i--)
	{
		commandId = model->GetCommandIdAt(i);
		if (commandId == CH_MENU_INVALID_OR_SEPARATOR && beforeCommandId == CH_MENU_INVALID_OR_SEPARATOR)
		{
			model->RemoveAt(i);
		}
		beforeCommandId = commandId;
	}
	// 先頭がSeparatorだった場合、まだ残存している。
	commandId = model->GetCommandIdAt(0);
	if (commandId == CH_MENU_INVALID_OR_SEPARATOR)
	{
		model->RemoveAt(0);
	}

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
			strSelText = (LPCWSTR)strCfSt.c_str();
			strSelText.TrimLeft();
			strSelText.TrimRight();
			::SendMessageTimeout(hWindow, WM_APP_CEF_SEARCH_URL, (WPARAM)(LPCTSTR)strSelText, (LPARAM)TRUE, SMTO_NORMAL, 1000, NULL);
			return true;
		}
		else if (command_id == CEF_MENU_ID_OPEN_LINK_NEW)
		{
			CefString strURLC;
			strURLC = params->GetLinkUrl();
			LPCWSTR pszURL = {0};
			pszURL = (LPCWSTR)strURLC.c_str();
			::SendMessageTimeout(hWindow, WM_NEW_WINDOW_URL, (WPARAM)cef_window_open_disposition_t::CEF_WOD_NEW_WINDOW, (LPARAM)pszURL, SMTO_NORMAL, 1000, NULL);
			return true;
		}
		else if (command_id == CEF_MENU_ID_OPEN_LINK_NEW_NOACTIVE)
		{
			CefString strURLC;
			strURLC = params->GetLinkUrl();
			LPCWSTR pszURL = {0};
			pszURL = (LPCWSTR)strURLC.c_str();
			::SendMessageTimeout(hWindow, WM_NEW_WINDOW_URL, (WPARAM)cef_window_open_disposition_t::CEF_WOD_NEW_BACKGROUND_TAB, (LPARAM)pszURL, SMTO_NORMAL, 1000, NULL);
			return true;
		}
		else if (command_id == CEF_MENU_ID_OPEN_IMG)
		{
			CefString strURLC;
			strURLC = params->GetSourceUrl();
			LPCWSTR pszURL = {0};
			pszURL = (LPCWSTR)strURLC.c_str();
			::SendMessageTimeout(hWindow, WM_NEW_WINDOW_URL, (WPARAM)cef_window_open_disposition_t::CEF_WOD_NEW_WINDOW, (LPARAM)pszURL, SMTO_NORMAL, 1000, NULL);
			return true;
		}
		else if (command_id == CEF_MENU_ID_OPEN_IMG_NOACTIVE)
		{
			CefString strURLC;
			strURLC = params->GetSourceUrl();
			LPCWSTR pszURL = {0};
			pszURL = (LPCWSTR)strURLC.c_str();
			::SendMessageTimeout(hWindow, WM_NEW_WINDOW_URL, (WPARAM)cef_window_open_disposition_t::CEF_WOD_NEW_BACKGROUND_TAB, (LPARAM)pszURL, SMTO_NORMAL, 1000, NULL);
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
			str = (LPCWSTR)strURLC.c_str();
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
			str = (LPCWSTR)strURLC.c_str();
			if (!str.IsEmpty())
			{
				LPCWSTR pszURL = {0};
				pszURL = (LPCWSTR)strURLC.c_str();
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
			str = (LPCWSTR)strURLC.c_str();
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
			LPCWSTR pszURL = (LPCWSTR)url.c_str();
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
		LPCWSTR pszTitle = NULL;
		pszTitle = (LPCWSTR)title.c_str();
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
		LPCWSTR pszFavURL = NULL;
		for (UINT i = 0; i < icon_urls.size(); i++)
		{
			strIconList = icon_urls[i];
			pszFavURL = (LPCWSTR)strIconList.c_str();
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
		LPCWSTR pszStatus = NULL;
		pszStatus = (LPCWSTR)value.c_str();
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
			dwLogData.mHWND.Format(_T("CV_WND:0x%08p"), hWindow);
			dwLogData.mFUNCTION_NAME = _T("ConsoleMessage");
			dwLogData.mMESSAGE1 = (LPCWSTR)message.c_str();
			dwLogData.mMESSAGE2 = strLogLevel;
			dwLogData.mMESSAGE3.Format(_T("Source:%s"), (LPCWSTR)source.c_str());
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
			strWriteLine.Format(_T("Message:%s\nSource:%s\nLine:%d\n"), (LPCWSTR)message.c_str(), (LPCWSTR)source.c_str(), line);
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

#if CHROME_VERSION_MAJOR < 125
void ClientHandler::OnBeforeDownload(CefRefPtr<CefBrowser> browser,
				     CefRefPtr<CefDownloadItem> download_item,
				     const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback)
#define RETURN_ON_BEFORE_DOWNLOAD(value) return
#else
bool ClientHandler::OnBeforeDownload(CefRefPtr<CefBrowser> browser,
				     CefRefPtr<CefDownloadItem> download_item,
				     const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback)
#define RETURN_ON_BEFORE_DOWNLOAD(value) return (value)
#endif
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
			CString alertMsg;
			alertMsg.LoadString(ID_MSG_FILE_DOWNLOAD_RESTRICTED);
			theApp.SB_MessageBox(hWindow, alertMsg, NULL, MB_OK | MB_ICONWARNING, TRUE);
		}
		EmptyWindowClose(browser);
		RETURN_ON_BEFORE_DOWNLOAD(true);
	}

	m_bDownLoadStartFlg = TRUE;
	CString strFileName;
	strFileName = (LPCWSTR)suggested_name.c_str();
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
		strURL = (LPCWSTR)strURLC.c_str();
		if (strURL.IsEmpty())
			::SendMessageTimeout(hWindow, WM_APP_CEF_DOWNLOAD_BLANK_PAGE, (WPARAM)TRUE, NULL, SMTO_NORMAL, 1000, NULL);
		else
			::SendMessageTimeout(hWindow, WM_APP_CEF_DOWNLOAD_BLANK_PAGE, (WPARAM)FALSE, NULL, SMTO_NORMAL, 1000, NULL);

		SendMessageTimeout(hWindow, WM_APP_CEF_WINDOW_ACTIVATE, (WPARAM)NULL, (LPARAM)NULL, SMTO_NORMAL, 1000, NULL);

		//ダウンロード中の場合は、警告を表示する。
		if (theApp.m_DlMgr.IsDlProgress(nBrowserId))
		{
			HWND hWindowFrm = GetParent(hWindow);
			CString inProgressDownloadMessage;
			inProgressDownloadMessage.LoadString(ID_MSG_ANOTHER_DOWNLOAD_IN_PROGRESS);
			int iRet = theApp.SB_MessageBox(hWindowFrm, inProgressDownloadMessage, NULL, MB_OK | MB_ICONWARNING, TRUE);
			RETURN_ON_BEFORE_DOWNLOAD(true);
		}

		CString szFilter;
		szFilter.LoadString(ID_FILE_TYPE_ALL);
		CString strTitle;
		strTitle.LoadString(ID_DOWNLOAD_FILE_CHOOSER_TITLE);
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
			StringCchCopy(szSelPath, MAX_PATH, pFileDlg->GetPathName());

			WCHAR szSelFolderPath[MAX_PATH] = {0};
			StringCchCopy(szSelFolderPath, MAX_PATH, pFileDlg->GetPathName());
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
						strURL = (LPCWSTR)download_item->GetURL().c_str();
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
			theApp.m_DlMgr.Cancel(nBrowserId);
			::SendMessageTimeout(hWindow, WM_APP_CEF_BEFORE_DOWNLOAD, (WPARAM)FALSE, NULL, SMTO_NORMAL, 1000, NULL);
			EmptyWindowClose(browser);
			callback->Continue(_T(""), false);
		}
		if (pFileDlg)
		{
			delete pFileDlg;
			pFileDlg = NULL;
		}
		RETURN_ON_BEFORE_DOWNLOAD(true);
	}
	callback->Continue(_T(""), false);
	RETURN_ON_BEFORE_DOWNLOAD(true);
}
#undef RETURN_ON_BEFORE_DOWNLOAD

void ClientHandler::OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback)
{
	///https://www.catalog.update.microsoft.com/Search.aspx?q=KB4051963

	REQUIRE_UI_THREAD();
	//Download禁止
	if (theApp.m_AppSettings.IsEnableDownloadRestriction())
	{
		return;
	}

	CEFDownloadItemValues values = {0};

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
		LPCWSTR fullPath = (LPCWSTR)cefFulPath.c_str();
		if (fullPath)
			StringCchCopy(values.szFullPath, 512, fullPath);
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
		else if (values.bIsInProgress)
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
						CString strSpeedTemplate;
						// format read
						if (values.nSpeed < 1024)
						{
							strSpeedTemplate.LoadString(ID_TRANSFER_SPEED_B);
							strSpeed.Format(strSpeedTemplate, values.nSpeed);
						}
						else if (values.nSpeed < 1048576)
						{
							strSpeedTemplate.LoadString(ID_TRANSFER_SPEED_KB);
							strSpeed.Format(strSpeedTemplate, static_cast<LONGLONG>(values.nSpeed) / 1024.0);
						}
						else
						{
							strSpeedTemplate.LoadString(ID_TRANSFER_SPEED_MB);
							strSpeed.Format(strSpeedTemplate, static_cast<LONGLONG>(values.nSpeed) / 1048576.0);
						}
						theApp.m_DlMgr.Set_DLDlgState(nBrowserId, values.nProgress, values.szFullPath, szStatus, strSpeed);

						if (theApp.m_DlMgr.IsCanceled(nBrowserId))
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
				if (theApp.m_DlMgr.IsCanceled(nBrowserId))
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
				//	if (theApp.m_DlMgr.IsCanceled(nBrowserId))
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
		}
		else
		{
			// values.bIsCanceled または DownloadItem::INTERRUPTE
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
cef_return_value_t ClientHandler::OnBeforeResourceLoad(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    CefRefPtr<CefCallback> callback)
{
	PROC_TIME(OnBeforeResourceLoad)

	CefRequest::HeaderMap cefHeaders;
	request->GetHeaderMap(cefHeaders);
	cefHeaders.insert(std::make_pair("DNT", "1"));
	request->SetHeaderMap(cefHeaders);

	CefString cefURL = request->GetURL();
	CString strTranURL((LPCWSTR)cefURL.c_str());
	CString logmsg;

	HWND hWindow = GetSafeParentWnd(browser);

	if (SafeWnd(hWindow))
	{
		DebugWndLogData dwLogData;
		dwLogData.mHWND.Format(_T("CV_WND:0x%08p"), hWindow);
		dwLogData.mFUNCTION_NAME = _T("OnBeforeResourceLoad");
		dwLogData.mMESSAGE1 = strTranURL;
		dwLogData.mMESSAGE2.Format(_T("FrameName:%s"), (LPCWSTR)frame->GetName().c_str());
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
			strHost = (LPCWSTR)cfHost.c_str();
		}
		if (SBUtil::IsURL_HTTP(strTranURL))
		{
			if (theApp.m_pLogDisp)
			{
				theApp.m_pLogDisp->SendLog(LOG_ACCESS_ALL, strHost, strTranURL);
			}
		}
	}

	// get URL requested
	CefString newURL = request->GetURL();
	CefURLParts cfURLparts;
	LPCWSTR pszURL = NULL;
	pszURL = (LPCWSTR)newURL.c_str();
	if (CefParseURL(newURL, cfURLparts))
	{
		CefString cfScheme(&cfURLparts.scheme);
		CefString cfHost(&cfURLparts.host);
		CefString cfPath(&cfURLparts.path);
		//CefString cfQuery(&cfURLparts.query);

		CString strScheme((LPCWSTR)cfScheme.c_str());
		CString strHost((LPCWSTR)cfHost.c_str());
		CString strPath((LPCWSTR)cfPath.c_str());

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
		//2021-11-30 ↑の対策がNGになっていることに気がついた。UAにEdgeをつけてもNG
		//↓のコード復活
		//accounts.google.comへのアクセス時は、FirefoxのUAにしてしまう。
		if (strHost == _T("accounts.google.com"))
		{
			request->GetHeaderMap(cefHeaders);
			cefHeaders.erase("User-Agent");
			cefHeaders.insert(std::make_pair("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:124.0) Gecko/20100101 Firefox/124.0"));
			//cefHeaders.insert(std::make_pair("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:94.0) Gecko/20100101 Firefox/94.0"));
			//cefHeaders.insert(std::make_pair("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.88 Safari/537.36 Edg/87.0.664.66"));
			//cefHeaders.insert(std::make_pair("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.88 Safari/537.36 Edg/87.0.0.0"));
			request->SetHeaderMap(cefHeaders);
		}

		if (!theApp.m_AppSettings.IsEnableURLFilter())
			return RV_CONTINUE;

		CString strURLChk; //Queryを除く。無駄な情報を省く。
		//strURLChk.Format(_T("%s://%s%s"), strScheme, strHost, strPath);
		strURLChk.Format(_T("%s://%s"), (LPCTSTR)strScheme, (LPCTSTR)strHost);

		if (theApp.IsURLFilterAllow(strURLChk, strScheme, strHost, strPath))
		{
			return RV_CONTINUE;
		}
		else
		{
			callback->Cancel();
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
					strURL = (LPCWSTR)strURLC.c_str();
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
										dwLogData.mHWND.Format(_T("BF_WND:0x%08p"), hWindowFrm);
										hWindow = hWindowFrm;
									}
									else
									{
										dwLogData.mHWND.Format(_T("CV_WND:0x%08p"), hWindow);
									}
									dwLogData.mFUNCTION_NAME = _T("ExcuteCustomJS");
									dwLogData.mMESSAGE1 = strURL;
									dwLogData.mMESSAGE2.Format(_T("FileName:%s"), (LPCTSTR)strRetFileName);
									theApp.AppendDebugViewLog(dwLogData);
									logmsg = dwLogData.GetString();
									theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_URL);

									//最初だけ
									if (i == 0)
										strJSsrc.Format(_T("const ChronosExtParentWnd=\"%ld\";\r\ntry{ChronosExt_AppActive(ChronosExtParentWnd);console.log('##CSG_Script:ChronosExtParentWnd:'+ChronosExtParentWnd);}catch(e){}\r\n"), (long)hWindow);

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
	strErrorMsg.LoadString(ID_ERROR_MSG_FAILED);
	switch (errorCode)
	{
	case ERR_NONE:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_NONE);
		break;
	}
	case ERR_FAILED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_FAILED);
		break;
	}
	case ERR_ABORTED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_ABORTED);
		break;
	}
	case ERR_INVALID_ARGUMENT:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_INVALID_ARGUMENT);
		break;
	}
	case ERR_INVALID_HANDLE:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_INVALID_HANDLE);
		break;
	}
	case ERR_FILE_NOT_FOUND:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_FILE_NOT_FOUND);
		break;
	}
	case ERR_TIMED_OUT:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_TIMED_OUT);
		break;
	}
	case ERR_FILE_TOO_BIG:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_FILE_TOO_BIG);
		break;
	}
	case ERR_UNEXPECTED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_UNEXPECTED);
		break;
	}
	case ERR_ACCESS_DENIED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_ACCESS_DENIED);
		break;
	}
	case ERR_NOT_IMPLEMENTED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_NOT_IMPLEMENTED);
		break;
	}
	case ERR_CONNECTION_CLOSED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CONNECTION_CLOSED);
		break;
	}
	case ERR_CONNECTION_RESET:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CONNECTION_RESET);
		break;
	}
	case ERR_CONNECTION_REFUSED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CONNECTION_REFUSED);
		break;
	}
	case ERR_CONNECTION_ABORTED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CONNECTION_ABORTED);
		break;
	}
	case ERR_CONNECTION_FAILED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CONNECTION_FAILED);
		break;
	}
	case ERR_NAME_NOT_RESOLVED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_NAME_NOT_RESOLVED);
		break;
	}
	case ERR_INTERNET_DISCONNECTED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_INTERNET_DISCONNECTED);
		break;
	}
	case ERR_SSL_PROTOCOL_ERROR:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_SSL_PROTOCOL_ERROR);
		break;
	}
	case ERR_ADDRESS_INVALID:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_ADDRESS_INVALID);
		break;
	}
	case ERR_ADDRESS_UNREACHABLE:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_ADDRESS_UNREACHABLE);
		break;
	}
	case ERR_SSL_CLIENT_AUTH_CERT_NEEDED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_SSL_CLIENT_AUTH_CERT_NEEDED);
		break;
	}
	case ERR_TUNNEL_CONNECTION_FAILED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_TUNNEL_CONNECTION_FAILED);
		break;
	}
	case ERR_NO_SSL_VERSIONS_ENABLED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_NO_SSL_VERSIONS_ENABLED);
		break;
	}
	case ERR_SSL_VERSION_OR_CIPHER_MISMATCH:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_SSL_VERSION_OR_CIPHER_MISMATCH);
		break;
	}
	case ERR_SSL_RENEGOTIATION_REQUESTED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_SSL_RENEGOTIATION_REQUESTED);
		break;
	}
	case ERR_CERT_COMMON_NAME_INVALID:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CERT_COMMON_NAME_INVALID);
		break;
	}
	case ERR_CERT_DATE_INVALID:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CERT_DATE_INVALID);
		break;
	}
	case ERR_CERT_AUTHORITY_INVALID:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CERT_AUTHORITY_INVALID);
		break;
	}
	case ERR_CERT_CONTAINS_ERRORS:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CERT_CONTAINS_ERRORS);
		break;
	}
	case ERR_CERT_NO_REVOCATION_MECHANISM:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CERT_NO_REVOCATION_MECHANISM);
		break;
	}
	case ERR_CERT_UNABLE_TO_CHECK_REVOCATION:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CERT_UNABLE_TO_CHECK_REVOCATION);
		break;
	}
	case ERR_CERT_REVOKED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CERT_REVOKED);
		break;
	}
	case ERR_CERT_INVALID:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CERT_INVALID);
		break;
	}
	case ERR_CERT_WEAK_SIGNATURE_ALGORITHM:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CERT_WEAK_SIGNATURE_ALGORITHM);
		break;
	}
	case ERR_CERT_NON_UNIQUE_NAME:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CERT_NON_UNIQUE_NAME);
		break;
	}
	case ERR_CERT_WEAK_KEY:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CERT_WEAK_KEY);
		break;
	}
	case ERR_CERT_NAME_CONSTRAINT_VIOLATION:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CERT_NAME_CONSTRAINT_VIOLATION);
		break;
	}
	case ERR_CERT_VALIDITY_TOO_LONG:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CERT_VALIDITY_TOO_LONG);
		break;
	}
	case ERR_INVALID_URL:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_INVALID_URL);
		break;
	}
	case ERR_DISALLOWED_URL_SCHEME:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_DISALLOWED_URL_SCHEME);
		break;
	}
	case ERR_UNKNOWN_URL_SCHEME:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_UNKNOWN_URL_SCHEME);
		break;
	}
	case ERR_TOO_MANY_REDIRECTS:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_TOO_MANY_REDIRECTS);
		break;
	}
	case ERR_UNSAFE_REDIRECT:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_UNSAFE_REDIRECT);
		break;
	}
	case ERR_UNSAFE_PORT:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_UNSAFE_PORT);
		break;
	}
	case ERR_INVALID_RESPONSE:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_INVALID_RESPONSE);
		break;
	}
	case ERR_INVALID_CHUNKED_ENCODING:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_INVALID_CHUNKED_ENCODING);
		break;
	}
	case ERR_METHOD_NOT_SUPPORTED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_METHOD_NOT_SUPPORTED);
		break;
	}
	case ERR_UNEXPECTED_PROXY_AUTH:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_UNEXPECTED_PROXY_AUTH);
		break;
	}
	case ERR_EMPTY_RESPONSE:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_EMPTY_RESPONSE);
		break;
	}
	case ERR_RESPONSE_HEADERS_TOO_BIG:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_RESPONSE_HEADERS_TOO_BIG);
		break;
	}
	case ERR_CACHE_MISS:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_CACHE_MISS);
		break;
	}
	case ERR_INSECURE_RESPONSE:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_INSECURE_RESPONSE);
		break;
	}
	case ERR_PROXY_CONNECTION_FAILED:
	{
		strErrorMsg.LoadString(ID_ERROR_MSG_PROXY_CONNECTION_FAILED);
		break;
	}
	default:
		break;
	}

	CString errorPageTitle;
	errorPageTitle.LoadString(ID_ERROR_PAGE_TITLE);
	CString errorPageHeading;
	errorPageHeading.LoadString(ID_ERROR_PAGE_HEADING);
	CString errorPageHeadingDetails;
	errorPageHeadingDetails.LoadString(ID_ERROR_PAGE_HEADING_DETAILS);
	CString errorPageHeadingURL;
	errorPageHeadingURL.LoadString(ID_ERROR_PAGE_HEADING_URL);
	CString errorPageHeadingCode;
	errorPageHeadingCode.LoadString(ID_ERROR_PAGE_HEADING_CODE);
	CString errorPageHeadingMsg;
	errorPageHeadingMsg.LoadString(ID_ERROR_PAGE_HEADING_MSG);
	CString errorPageHeadingName;
	errorPageHeadingName.LoadString(ID_ERROR_PAGE_HEADING_NAME);

	CString strFailedUrl((LPCWSTR)failedUrl.c_str());
	strFailedUrl.Replace(_T("<"), _T(""));
	strFailedUrl.Replace(_T(">"), _T(""));
	strFailedUrl.Replace(_T("&"), _T(""));
	strFailedUrl.Replace(_T("\""), _T(""));
	strFailedUrl.Replace(_T("'"), _T(""));
	strFailedUrl.Replace(_T("`"), _T(""));
	strFailedUrl.Replace(_T("javascript"), _T(""));
	strFailedUrl.Replace(_T("eval("), _T(""));
	strErrorHTMLFmt = _T("<html><head><meta http-equiv='Content-Type' content='text/html; charset=Shift_JIS'/><title>");
	strErrorHTMLFmt += errorPageTitle;
	strErrorHTMLFmt += _T("</title>");
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
	strErrorHTMLFmt += _T("<div id='base'><div id='container'><h1>");
	strErrorHTMLFmt += errorPageHeading;
	strErrorHTMLFmt += _T("</h1>");
	strErrorHTMLFmt += _T("<h2>");
	strErrorHTMLFmt += strErrorMsg;
	strErrorHTMLFmt += _T("</h2>");
	strErrorHTMLFmt += _T("<h3>");
	strErrorHTMLFmt += errorPageHeadingDetails;
	strErrorHTMLFmt += _T("</h3><div id='white_box'>");
	strErrorHTMLFmt += _T("<div id='cause'>");
	strErrorHTMLFmt += _T("<h4>");
	strErrorHTMLFmt += errorPageHeadingURL;
	strErrorHTMLFmt += _T("<a href='");
	strErrorHTMLFmt += strFailedUrl;
	strErrorHTMLFmt += _T("'/>");
	strErrorHTMLFmt += strFailedUrl;
	strErrorHTMLFmt += _T("</a></h4>");
	strErrorHTMLFmt += _T("<h4>");
	strErrorHTMLFmt += errorPageHeadingCode;
	strErrorHTMLFmt += strErrorCode;
	strErrorHTMLFmt += _T("</h4>");
	strErrorHTMLFmt += _T("<h4>");
	strErrorHTMLFmt += errorPageHeadingMsg;
	strErrorHTMLFmt += strErrorMsg;
	strErrorHTMLFmt += _T("</h4>");
	strErrorHTMLFmt += _T("<h4>");
	strErrorHTMLFmt += errorPageHeadingName;
	strErrorHTMLFmt += (LPCWSTR)errorText.c_str();
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
	//strJsStr += _T("    alert('appended');\n}\n");
	//strJsStr += _T("else alert('no body to append!!');");

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
	values.lpszHost = (LPCWSTR)host.c_str();
	values.lpszRealm = (LPCWSTR)realm.c_str();
	_tcscpy_s(values.szUserName, _T(""));
	_tcscpy_s(values.szUserPass, _T(""));
	if (SafeWnd(hWindow))
	{
		SendMessageTimeout(hWindow, WM_APP_CEF_WINDOW_ACTIVATE, (WPARAM)NULL, (LPARAM)NULL, SMTO_NORMAL, 1000, NULL);

		CString strHost(values.lpszHost);
		CDlgAuth Dlg(CWnd::FromHandle(hWindow)->GetParent());
		CString authRequiredMsg;
		if (isProxy)
			authRequiredMsg.LoadString(IDS_STRING_PROXY_AUTH_REQUIRED);
		else
			authRequiredMsg.LoadString(IDS_STRING_HOST_AUTH_REQUIRED);
		Dlg.m_strMsgTxt.Format(authRequiredMsg, strHost);
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

#if CHROME_VERSION_MAJOR < 109
// Since CEF109, OnQuotaRequest is not available anymore.
bool ClientHandler::OnQuotaRequest(CefRefPtr<CefBrowser> browser, const CefString& origin_url, int64 new_size, CefRefPtr<CefCallback> callback)
{
	static const int64 max_size = 1024 * 1024 * 20; // 20mb.
	// Grant the quota request if the size is reasonable.
	//callback->Continue(new_size <= max_size);
	if (new_size > max_size)
		callback->Cancel();
	// call parent
	return CefRequestHandler::OnQuotaRequest(browser, origin_url, new_size, callback);
}
#endif

bool ClientHandler::OnCertificateError(CefRefPtr<CefBrowser> browser,
				       ErrorCode cert_error, const CefString& request_url, CefRefPtr<CefSSLInfo> ssl_info, CefRefPtr<CefCallback> callback)
{
	CString szMessage;

	CString confirmMsg;
	confirmMsg.LoadString(IDS_STRING_CONFIRM_INSECURE_CONNECTION);
	szMessage.Format(confirmMsg, (LPCWSTR)request_url.c_str());
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
	callback->Continue();

	return TRUE;
}

bool ClientHandler::OnSelectClientCertificate(
    CefRefPtr<CefBrowser> browser,
    bool isProxy,
    const CefString& host,
    int port,
    const X509CertificateList& certificates,
    CefRefPtr<CefSelectClientCertificateCallback> callback)
{
	if (certificates.empty())
		return false;

	if (certificates.size() == 1)
	{
		callback->Select(certificates[0]);
		return true;
	}

	// Use default certification select dialog only for Windows 11 or later.
	HWND hWindow = GetSafeParentWnd(browser);
	bool bUseDefaultCertDialog = SBUtil::IsWindows11OrLater();
	if (!bUseDefaultCertDialog)
	{
		SendMessageTimeout(hWindow, WM_APP_CEF_WINDOW_ACTIVATE, (WPARAM)NULL, (LPARAM)NULL, SMTO_NORMAL, 1000, NULL);
		CDlgCertification dlg(host, certificates, CWnd::FromHandle(hWindow));
		INT_PTR iResult = dlg.DoModal();
		if (iResult == IDOK)
		{
			callback->Select(certificates[dlg.SelectedIndex()]);
			return true;
		}
	}
	else
	{
		DebugWndLogData dwLogData;
		dwLogData.mHWND.Format(_T("CV_WND:0x%08p"), hWindow);
		dwLogData.mFUNCTION_NAME = _T("OnSelectClientCertificate");

		HCERTSTORE hCertStore;
		hCertStore = CertOpenSystemStore(0, L"MY");
		if (!hCertStore)
		{
			// Failed to open personal cert store.
			dwLogData.mMESSAGE1.Format(_T("Failed to open personal certificate store."));
			theApp.WriteDebugTraceDateTime(dwLogData.GetString(), DEBUG_LOG_TYPE_DE);
			return false;
		}

#pragma warning(push)
// CryptUIDlgSelectCertificateFromStoreの第7引数は実際にはNULL許容だが、_In_で定義されているので
//C6387: 「_Param_(7)は'0'である可能性があります」の警告が出る。仕方がないのでこの警告は無視する。
#pragma warning(disable : 6387)
		PCCERT_CONTEXT pCertContext;
		pCertContext = CryptUIDlgSelectCertificateFromStore(hCertStore,
								    hWindow,
								    NULL, // default title
								    NULL, // default explanation
								    0,	  // show default columns
								    0,	  // reserved dwFlags
								    NULL);
#pragma warning(pop)

		if (!pCertContext)
		{
			// Failed to select certificate or canceled or closed dialog.
			// In such a case, it seems that the first certificate will be used.
			dwLogData.mMESSAGE1.Format(_T("Failed to open certificate selection dialog."));
			theApp.WriteDebugTraceDateTime(dwLogData.GetString(), DEBUG_LOG_TYPE_DE);
			CertCloseStore(hCertStore, 0);
			// When it returns false here without selecting client certicate file,
			// it works as if the first certificate was selected by default.
			// As client certificate is not selected, above behavior is not suitable
			// and it should be canceled correctly (as same as the other Browser raise error.)
			// This is incompatible change since 14.1.119.0.
			return true;
		}

		// Find mapped index between cef given certificates and win32 API call results.
		CString selectedSerialNumber = GetSerialNumberAsHexString(pCertContext->pCertInfo);
		int selectedIndex = -1;
		for (int index = 0; index < certificates.size(); index++)
		{
			CefRefPtr<CefX509Certificate> certificate = certificates[index];
			CString serialNumber = GetSerialNumberAsHexString(certificate);
			if (selectedSerialNumber == serialNumber)
			{
				selectedIndex = index;
				break;
			}
		}

		CertFreeCertificateContext(pCertContext);
		boolean bClosedStore = CertCloseStore(hCertStore, CERT_CLOSE_STORE_CHECK_FLAG);
		if (!bClosedStore)
		{
			DWORD dwResult = GetLastError();
			dwLogData.mMESSAGE1.Format(_T("Checked with CERT_CLOSE_STORE_CHECK_FLAG. not freed yet."));
			theApp.WriteDebugTraceDateTime(dwLogData.GetString(), DEBUG_LOG_TYPE_DE);
			if (dwResult == CRYPT_E_PENDING_CLOSE)
			{
				dwLogData.mMESSAGE1.Format(_T("Some certificate context is not freed, but closed certificate store."));
				theApp.WriteDebugTraceDateTime(dwLogData.GetString(), DEBUG_LOG_TYPE_DE);
			}
			else
			{
				dwLogData.mMESSAGE1.Format(_T("Certificate context is not freed, but not pending close yet."));
				theApp.WriteDebugTraceDateTime(dwLogData.GetString(), DEBUG_LOG_TYPE_DE);
			}
		}
		else
		{
			dwLogData.mMESSAGE1.Format(_T("All certificate context and store is closed successfully."));
			theApp.WriteDebugTraceDateTime(dwLogData.GetString(), DEBUG_LOG_TYPE_DE);
		}

		if (selectedIndex >= 0)
		{
			callback->Select(certificates[selectedIndex]);
			return true;
		}
	}
	// When it returns false here without selecting client certicate file, 
	// it works as if the first certificate was selected by default.
	// As client certificate is not selected, above behavior is not suitable
	// and it should be canceled correctly (as same as the other Browser raise error.)
	// This is incompatible change since 14.1.119.0.
	return true;
}

void ClientHandler::OnProtocolExecution(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool& allow_os_execution)
{
	// do default
	//CefResourceRequestHandler::OnProtocolExecution(browser,frame, request,allow_os_execution);
	//return;
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
		LPCWSTR pszURL = NULL;
		pszURL = (LPCWSTR)newURL.c_str();

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
			strWindowName = (LPCWSTR)strName.c_str();
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

bool ClientHandler::OnRequestMediaAccessPermission(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const CefString& requesting_origin,
    uint32 requested_permissions,
    CefRefPtr<CefMediaAccessCallback> callback)
{
	if (requested_permissions == CEF_MEDIA_PERMISSION_NONE)
		return false;
	if (!theApp.m_AppSettings.GetMediaAccessPermission())
	{
		// Do not permit media access.
		return false;
	}

	HWND hWindow = GetSafeParentWnd(browser);
	CString requestOrigin = (LPCWSTR)requesting_origin.c_str();
	if (theApp.m_AppSettings.GetMediaAccessPermission() == static_cast<int>(AppSettings::MediaAccessPermission::MANUAL_MEDIA_APPROVAL))
	{
		std::tuple<CefString, uint32> permissionInfo = std::tie(requesting_origin, requested_permissions);
		std::map<std::tuple<CefString, uint32>, bool>::iterator cachedPermissions = m_originAndPermissionsCache.find(permissionInfo);
		if (cachedPermissions != m_originAndPermissionsCache.end())
		{
			bool accepted = cachedPermissions->second;
			if (accepted)
			{
				callback->Continue(requested_permissions);
			}
			return accepted;
		}

		CString confirmMessage;
		CString enableMediaConfirmation;
		enableMediaConfirmation.LoadString(ID_ENABLE_MEDIA_CONFIRMATION);
		confirmMessage.Format(enableMediaConfirmation, (LPCTSTR)requestOrigin);
		confirmMessage += "\n";

#define ADD_PERMISSION_MESSAGE(permission, messageId) \
		if (requested_permissions & permission)      \
		{                                            \
			confirmMessage += "\n";              \
			CString mediaType;                   \
			mediaType.LoadString(messageId);     \
			confirmMessage += mediaType;         \
		}

		ADD_PERMISSION_MESSAGE(CEF_MEDIA_PERMISSION_DEVICE_AUDIO_CAPTURE, ID_ENABLE_MEDIA_MIC);
		ADD_PERMISSION_MESSAGE(CEF_MEDIA_PERMISSION_DEVICE_VIDEO_CAPTURE, ID_ENABLE_MEDIA_VIDEO);
		ADD_PERMISSION_MESSAGE(CEF_MEDIA_PERMISSION_DESKTOP_AUDIO_CAPTURE, ID_ENABLE_MEDIA_DESKTOP_AUDIO);
		ADD_PERMISSION_MESSAGE(CEF_MEDIA_PERMISSION_DESKTOP_VIDEO_CAPTURE, ID_ENABLE_MEDIA_DESKTOP_VIDEO);

#undef ADD_PERMISSION_MESSAGE

		int iRet = theApp.SB_MessageBox(hWindow, confirmMessage, NULL, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2, TRUE);
		if (iRet == IDNO)
		{
			m_originAndPermissionsCache[permissionInfo] = false;
			return false;
		}
		else
		{
			m_originAndPermissionsCache[permissionInfo] = true;
		}
	}

	DebugWndLogData dwLogData;
	dwLogData.mHWND.Format(_T("CV_WND:0x%08p"), hWindow);
	dwLogData.mFUNCTION_NAME = _T("OnRequestMediaAccessPermission");
	dwLogData.mMESSAGE1 = requestOrigin;
	dwLogData.mMESSAGE2.Format(_T("FrameName:%s"), (LPCWSTR)frame->GetName().c_str());
	dwLogData.mMESSAGE3.Format(_T("Approved permission type:%d"), requested_permissions);
	theApp.AppendDebugViewLog(dwLogData);
	theApp.WriteDebugTraceDateTime(dwLogData.GetString(), DEBUG_LOG_TYPE_URL);

	callback->Continue(requested_permissions);
	return true;
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
	pszMessage = (LPCWSTR)message_text.c_str();
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
	pszMessage = (LPCWSTR)message_text.c_str();
	CString strMsg;
	strMsg = pszMessage;
	//if(strMsg==_T("Is it OK to leave/reload this page?"))
	{
		strMsg.LoadString(IDS_STRING_CONFIRM_LEAVE_PAGE);
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
					strFileName = (LPCWSTR)cstrFileName.c_str();
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
				strFileName = (LPCWSTR)cstrFileName.c_str();
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
	strFilterName = (LPCWSTR)strName.c_str();
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
				::SendMessageTimeout(hWindow, WM_APP_CEF_SET_RENDERER_PID, (WPARAM)m_RendererPID, 0, SMTO_NORMAL, 1000, NULL);
			}
		}
		return true;
	}
	return false;
}

CString ClientHandler::GetSerialNumberAsHexString(const CefRefPtr<CefX509Certificate> certificate)
{
	auto serialNumber = certificate->GetSerialNumber();
	auto size = serialNumber->GetSize();

	CString serialNumberAsHexString;
	for (size_t i = 0; i < size; i++)
	{
		short buf = 0;
		int gotSize = serialNumber->GetData(&buf, 1, i);
		if (gotSize == 0)
			break;

		CString hex;
		hex.Format(_T("%02X"), buf);
		serialNumberAsHexString += hex;
	}
	return serialNumberAsHexString;
}

CString ClientHandler::GetSerialNumberAsHexString(PCERT_INFO pCertInfo)
{
	BYTE* pbBuf = pCertInfo->SerialNumber.pbData;
	CString serialNumber;
	// Must retrieve in reverse order for SerialNumber field correctly
	for (INT i = pCertInfo->SerialNumber.cbData - 1; i >= 0; i--)
	{
		CString hex;
		hex.Format(L"%02X", pbBuf[i]);
		serialNumber += hex;
	}
	return serialNumber;
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
			CString strText((LPCWSTR)arguments[0]->GetStringValue().c_str());
			CString strCaption((LPCWSTR)arguments[1]->GetStringValue().c_str());
			CString strChronosExtParentWnd((LPCWSTR)arguments[2]->GetStringValue().c_str());
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

				strFmt.Format(_T("{\"username\": \"%s\",\"password\": \"%s\"}"), (LPCTSTR)strID, (LPCTSTR)strPW);
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
			CString exceptionMsg;
			exceptionMsg.LoadString(ID_ERROR_MISMATCHED_PARAMETER);
			exception = CefString(exceptionMsg);
			return false;
		}
		return false;
	}
	else if (name == "ChronosExt_DlgPWChange")
	{
		if (iArgSize == 4 && arguments[0]->IsString() && arguments[1]->IsString() && arguments[2]->IsString() && arguments[3]->IsString())
		{
			CString strID((LPCWSTR)arguments[0]->GetStringValue().c_str());
			CString strText((LPCWSTR)arguments[1]->GetStringValue().c_str());
			CString strCaption((LPCWSTR)arguments[2]->GetStringValue().c_str());
			CString strChronosExtParentWnd((LPCWSTR)arguments[3]->GetStringValue().c_str());
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

				strFmt.Format(_T("{\"username\": \"%s\",\"current_password\": \"%s\",\"new_password\": \"%s\"}"), (LPCTSTR)strID, (LPCTSTR)strPW_Current, (LPCTSTR)strPW_New);
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
			CString exceptionMsg;
			exceptionMsg.LoadString(ID_ERROR_MISMATCHED_PARAMETER);
			exception = CefString(exceptionMsg);
			return false;
		}
		return false;
	}
	else if (name == "ChronosExt_AppActive")
	{
		if (iArgSize == 1 && arguments[0]->IsString())
		{
			CString strChronosExtParentWnd((LPCWSTR)arguments[0]->GetStringValue().c_str());
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
//#pragma warning(disable : 26812)
#pragma warning(pop)
