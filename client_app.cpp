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

	//GetAuthCredentials�������Ȃ��Ȃ����̂�2019-06-13
	//command_line->AppendSwitchWithValue(_T("disable-features"), _T("NetworkService"));
	//2019-07-24 �����悤�ɂȂ����B
	//Chromium Embedded Framework Version 75.1.4+g4210896+chromium-75.0.3770.100

	//2020-09-16
	//https://bitbucket.org/chromiumembedded/cef/issues/2989/m85-print-preview-fails-to-load-pdf-file
	//CEF 85.3.6+gacfac2f+chromium-85.0.4183.102 / Chromium 85.0.4183.102
	//CEF 84�܂ł�OK�������B
	//--enable-print-preview
	command_line->AppendSwitch(_T("enable-print-preview"));

	//--disk-cache-size=536870912 512MB
	command_line->AppendSwitchWithValue(_T("disk-cache-size"), _T("536870912"));  //512MB
	command_line->AppendSwitchWithValue(_T("media-cache-size"), _T("209715200")); //200MB

	//	//enable-net-security-expiration
	//	command_line->AppendSwitch(_T("enable-net-security-expiration"));

	//"enable-media-stream"
	//localhost������OK�A���̑��͋�������@���s��
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

	//�Ǝ���Proxy���[�����Z�b�g
	//Direct
	if (theApp.m_AppSettings.GetProxyType() == CSG_PROXY_NA)
	{
		command_line->AppendSwitch(_T("no-proxy-server"));
	}
	//IE�ˑ�
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
			//pac�𗘗p
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
			// �������ォ��r�b�g�}�b�v�t�@�C����ǂݍ���
			pngImage_src.Load(pIStream);

			//// �m�F�pSave
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
					//�t�@�C�����Ɏg���Ȃ�������u��������B
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
