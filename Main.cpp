#include "stdafx.h"
#include "Sazabi.h"

#include "MainFrm.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MY_WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	int nReturnCode = -1;
	CWinThread* pThread = AfxGetThread();
	CWinApp* pApp = AfxGetApp();
	if (!AfxWinInit(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
	{
		AfxWinTerm();
		return -1;
	}

	if (pApp != NULL)
	{
		if (!pApp->InitApplication())
		{
			AfxWinTerm();
			return -1;
		}
	}

	if (!pThread->InitInstance())
	{
		if (pThread->m_pMainWnd)
		{
			pThread->m_pMainWnd->DestroyWindow();
		}
		nReturnCode = pThread->ExitInstance();
		AfxWinTerm();
		return nReturnCode;
	}

	nReturnCode = pThread->Run();
	AfxWinTerm();
	return nReturnCode;
}


int AFXAPI AfxWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{

#ifdef _DEBUG
	::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif
	HMODULE kernel32 = GetModuleHandleA("KERNEL32");
	if (kernel32)
	{
		typedef BOOL(WINAPI * Proc_pfnSetDllDirectoryW)(LPCWSTR);
		Proc_pfnSetDllDirectoryW pfnSetDllDirectoryW = (Proc_pfnSetDllDirectoryW)GetProcAddress(kernel32, "SetDllDirectoryW");
		if (pfnSetDllDirectoryW)
		{
			pfnSetDllDirectoryW(L"");
		}
		typedef BOOL(WINAPI * Proc_pfnSetSearchPathMode)(DWORD);
		Proc_pfnSetSearchPathMode pfnSetSearchPathMode = (Proc_pfnSetSearchPathMode)GetProcAddress(kernel32, "SetSearchPathMode");
		if (pfnSetSearchPathMode)
		{
			const DWORD dwBASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE = 1;
			const DWORD dwBASE_SEARCH_PATH_PERMANENT = 0x8000;
			pfnSetSearchPathMode(dwBASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | dwBASE_SEARCH_PATH_PERMANENT);
		}
	}
	setlocale(LC_ALL, "Japanese");

	int nRet = 0;
	CString strCommandLineData;
	strCommandLineData = ::GetCommandLine();
	if (strCommandLineData.Find(_T("--type=")) > 0)
	{
#if CHROME_VERSION_MAJOR < 112
		CefEnableHighDPISupport();
#endif
		CefMainArgs mainargs(hInstance);
		void* sandbox_info = NULL;
		if (strCommandLineData.Find(_T("--type=renderer")) > 0)
		{
			CefRefPtr<CefApp> app;
			app = new AppRenderer();
			int exitCode = CefExecuteProcess(mainargs, app.get(), sandbox_info);
			if (exitCode >= 0)
			{
				return exitCode;
			}
		}
		int exit_code = CefExecuteProcess(mainargs, nullptr, sandbox_info);
		if (exit_code >= 0)
		{
			return exit_code;
		}
	}
	nRet = MY_WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	return nRet;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
