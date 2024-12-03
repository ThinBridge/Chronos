#include "StdAfx.h"
#include "client_app.h"
#include "client_util.h"
#include "Sazabi.h"

ClientApp::ClientApp(void)
{
}

ClientApp::ClientApp(HWND hWnd)
{
}

ClientApp::~ClientApp(void)
{
}

void ClientApp::OnContextInitialized()
{
	REQUIRE_UI_THREAD()
}

void ClientApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
	PROC_TIME(OnBeforeCommandLineProcessing)

	//GetAuthCredentialsが動かなくなったので2019-06-13
	//command_line->AppendSwitchWithValue(_T("disable-features"), _T("NetworkService"));
	//2019-07-24 動くようになった。
	//Chromium Embedded Framework Version 75.1.4+g4210896+chromium-75.0.3770.100
	
#if CHROME_VERSION_MAJOR < 122
	//CEF119: FirstPartySetsが有効だとYouTubeの検索窓にカーソルを合わせるとクラッシュする問題への対処。
	//https://github.com/chromiumembedded/cef/issues/3643
	//Chromeが3rd party cookiesをサポート外としたのは2024/1からだが、CEF119は2023/11のリリース。
	//そのため、CEF119ではまだthird party cookiesをサポートしているので、first party setsを無効化して問題ない。
	//https://cloud.google.com/looker/docs/best-practices/chrome-third-party-cookie-deprecation?hl=en
	//CEF122以降でこの問題は解消している
	command_line->AppendSwitchWithValue(_T("disable-features"), _T("FirstPartySets"));
#endif

	//2020-09-16
	//https://bitbucket.org/chromiumembedded/cef/issues/2989/m85-print-preview-fails-to-load-pdf-file
	//CEF 85.3.6+gacfac2f+chromium-85.0.4183.102 / Chromium 85.0.4183.102
	//CEF 84まではOKだった。
	//--enable-print-preview
	command_line->AppendSwitch(_T("enable-print-preview"));

	//--disable-popup-blocking
	//CEF 128以降、runtime_styleにCEF_RUNTIME_STYLE_ALLOYを指定すると、デフォルトではポップアップが表示されない。
	//disable-popup-blockingを指定してポップアップブロック機能を無効化する必要がある。
	//https://www.magpcss.org/ceforum/viewtopic.php?f=6&t=19988
	command_line->AppendSwitch(_T("disable-popup-blocking"));

	//--disk-cache-size=536870912 512MB
	command_line->AppendSwitchWithValue(_T("disk-cache-size"), _T("536870912"));  //512MB
	command_line->AppendSwitchWithValue(_T("media-cache-size"), _T("209715200")); //200MB

	//	//enable-net-security-expiration
	//	command_line->AppendSwitch(_T("enable-net-security-expiration"));

	//"enable-media-stream"
	//localhostだけはOK、その他は許可する方法が不明
	//command_line->AppendSwitch(_T("enable-media-stream"));

	//	m_Command_line = CefCommandLine::CreateCommandLine();
	//	m_Command_line->InitFromString(::GetCommandLineW());
	//	//--single-process --ppapi-in-process
	//	command_line->AppendSwitch(_T("disable-extensions"));
	//command_line->AppendSwitchWithValue(_T("proxy-server"),_T("127.0.0.1:8080"));

	//pdf
	if (!theApp.m_AppSettings.IsEnablePDFExtension())
		command_line->AppendSwitch(_T("disable-pdf-extension"));

	////flash
	//2020-12-31 EOL
	//if(theApp.m_AppSettings.IsEnableFlashPlayer())
	//{
	//	command_line->AppendSwitch(_T("enable-system-flash"));
	//}

	//MemCache
	if (theApp.m_AppSettings.IsEnableMemcache())
	{
		command_line->AppendSwitch(_T("disable-gpu-shader-disk-cache"));
	}

	//gpu
	if (!theApp.m_AppSettings.IsGPURendering())
	{
		command_line->AppendSwitch(_T("disable-gpu"));
		command_line->AppendSwitch(_T("disable-gpu-compositing"));
		command_line->AppendSwitch(_T("enable-begin-frame-scheduling"));
	}
	//else
	//{
	//	command_line->AppendSwitchWithValue(_T("enable-gpu-rasterization"),_T("Enabled"));
	//	command_line->AppendSwitchWithValue(_T("num-raster-threads"), _T("4"));
	//	command_line->AppendSwitch(_T("disable-accelerated-2d-canvas"));
	//	command_line->AppendSwitch(_T("disable-gpu-compositing"));
	//	command_line->AppendSwitch(_T("disable-gpu-driver-bug-workarounds"));
	//	command_line->AppendSwitch(_T("disable-gpu-rasterization"));
	//	command_line->AppendSwitch(_T("disable-gpu-sandbox"));
	//	command_line->AppendSwitch(_T("disable-gpu-shader-disk-cache"));
	//	command_line->AppendSwitch(_T("disable-native-gpu-memory-buffers"));
	//	command_line->AppendSwitch(_T("disable-gpu-early-init"));
	//	command_line->AppendSwitch(_T("disable-gpu-memory-buffer-compositor-resources"));
	//}

	//Proxy Settings
	CString strProxyName;

	//独自のProxyルールをセット
	//Direct
	if (theApp.m_AppSettings.GetProxyType() == CSG_PROXY_NA)
	{
		command_line->AppendSwitch(_T("no-proxy-server"));
	}
	//IE依存
	else if (theApp.m_AppSettings.GetProxyType() == CSG_PROXY_IE)
	{
		;
	}
	//Proxy
	else if (theApp.m_AppSettings.GetProxyType() == CSG_PROXY_TF)
	{
		strProxyName = theApp.m_AppSettings.GetProxyAddress();
		if (!strProxyName.IsEmpty())
		{
			//pacを利用
			if (SBUtil::IsURL_HTTP(strProxyName) || SBUtil::IsURL_FILE(strProxyName))
			{
				command_line->AppendSwitchWithValue(_T("proxy-pac-url"), (LPCTSTR)strProxyName);
			}
			else
			{
				command_line->AppendSwitchWithValue(_T("proxy-server"), (LPCTSTR)strProxyName);
				//Proxy Bypass Name
				CString strProxyBypassName = theApp.m_AppSettings.GetProxyBypassAddress();
				strProxyBypassName.TrimLeft();
				strProxyBypassName.TrimRight();
				if (!strProxyBypassName.IsEmpty())
					command_line->AppendSwitchWithValue(_T("proxy-bypass-list"), (LPCTSTR)strProxyBypassName);
			}
		}
	}
}

void ClientApp::OnScheduleMessagePumpWork(int64_t delayMs)
{
	MessageLoopWorker* messageLoopWorker = theApp.m_pMessageLoopWorker;
	if (!messageLoopWorker)
		return;
	messageLoopWorker->PostScheduleMessage(delayMs);
}

#if CHROME_VERSION_MAJOR >= 120
// CEF120以降で、複数のCEFのインスタンスが重複した`CefSettings.root_cache_path`を設定していた場合に呼び出されるハンドラー。 
// https://cef-builds.spotifycdn.com/docs/121.0/classCefBrowserProcessHandler.html#a052a91639483467c0b546d57a05c2f06
// このハンドラーでfalseを返すか、未実装の場合、Chromeスタイルの新しいウィンドウが起動する。それを避けるため、ここでは
// trueを返している。
// 
// インスタンスごとに固有の`CefSettings.root_cache_path value`を使用するようになっているため、このハンドラーが呼ばれる
// ことはないので、念のための処理である。
bool ClientApp::OnAlreadyRunningAppRelaunch(CefRefPtr<CefCommandLine> command_line, const CefString& current_directory)
{
	return true;
}
#endif

void DownloadFaviconCB::OnDownloadImageFinished(const CefString& image_url,
						int http_status_code,
						CefRefPtr<CefImage> image)
{
	REQUIRE_UI_THREAD();
	if (image == NULL || image->IsEmpty())
	{
		theApp.SetDefaultFavicon(m_pwndFrame);
	}
	else
	{
		CefRefPtr<CefBinaryValue> value;
		float scale_factor = 1.0;
		int pixel_width = 0;
		int pixel_height = 0;
		value = image->GetAsPNG(scale_factor, true, pixel_width, pixel_height);
		size_t iSize = 0;
		iSize = value->GetSize();
		std::vector<unsigned char> data(iSize);
		value->GetData(&data[0], iSize, 0);

		HRESULT hr = {0};
		IStream* pIStream = NULL;
		hr = ::CreateStreamOnHGlobal(NULL, TRUE, &pIStream);
		if (SUCCEEDED(hr) && pIStream)
		{
			hr = pIStream->Write(&data[0], (ULONG)iSize, NULL);
			CImage pngImage_src;
			// メモリ上からビットマップファイルを読み込む
			pngImage_src.Load(pIStream);

			//// 確認用Save
			if (theApp.m_AppSettings.IsAdvancedLogMode())
			{
				CefURLParts cfURLparts;
				if (CefParseURL(image_url, cfURLparts))
				{
					//CefString cfScheme(&cfURLparts.scheme);
					CefString cfHost(&cfURLparts.host);
					CefString cfPath(&cfURLparts.path);
					//CString strScheme((LPCWSTR)cfScheme.c_str());
					CString strHost((LPCWSTR)cfHost.c_str());
					CString strPath((LPCWSTR)cfPath.c_str());
					CString strFileName;
					strFileName += strHost;
					//strFileName += _T("");
					strFileName += strPath;
					strFileName += _T(".png");
					//ファイル名に使えない文字を置き換える。
					strFileName = SBUtil::GetValidFileName(strFileName);
					CString strFullPath;
					strFullPath = theApp.m_strFaviconCachePath;
					strFullPath += strFileName;
					pngImage_src.Save(strFullPath, Gdiplus::ImageFormatPNG);
				}
			}
			if (!pngImage_src.IsNull())
			{
				theApp.SetFavicon(&pngImage_src, m_pwndFrame);
			}
			pIStream->Release();
		}
	}
}
void DownloadImageCopyClipboard::OnDownloadImageFinished(const CefString& image_url,
							 int http_status_code,
							 CefRefPtr<CefImage> image)
{
	REQUIRE_UI_THREAD();
	if (image == NULL || image->IsEmpty())
	{
		return;
	}
	else
	{
		CefRefPtr<CefBinaryValue> value;
		float scale_factor = 1.0;
		int pixel_width = 0;
		int pixel_height = 0;
		value = image->GetAsPNG(scale_factor, true, pixel_width, pixel_height);
		size_t iSize = 0;
		iSize = value->GetSize();
		std::vector<unsigned char> data(iSize);
		value->GetData(&data[0], iSize, 0);

		HRESULT hr = {0};
		try
		{
			if (::OpenClipboard(NULL))
			{
				HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, iSize);
				if (hGlobal != NULL)
				{
					BYTE* pData = (BYTE*)::GlobalLock(hGlobal);
					if (pData != NULL)
					{
						::memcpy(pData, (LPCTSTR)&data[0], iSize);
						::GlobalUnlock(hGlobal);
						::EmptyClipboard();
						if (!::SetClipboardData(RegisterClipboardFormat(L"PNG"), hGlobal))
							::GlobalFree(hGlobal);
					}
				}
				::CloseClipboard();
			}
		}
		catch (...)
		{
			return;
		}
	}
}
