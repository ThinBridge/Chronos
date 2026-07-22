// Entry point used when Chronos is built as ChronosN.dll and launched through
// the CEF bootstrap executable (bootstrap.exe renamed to ChronosN.exe).
//
// CEF M138 removed the cef_sandbox.lib static linking route, so this layout is
// the only way to enable the Chromium sandbox for sub-processes. See
// doc/SANDBOX.md and https://chromiumembedded.github.io/cef/sandbox_setup
//
// The standalone executable entry point (AfxWinMain) lives in Sazabi.cpp and is
// selected when CHRONOS_USE_BOOTSTRAP is not defined.

#include "stdafx.h"
#include "Sazabi.h"
#pragma warning(push, 0)
#include <codeanalysis/warnings.h>
#pragma warning(disable \
		: ALL_CODE_ANALYSIS_WARNINGS)
#include "include/cef_sandbox_win.h"
#pragma warning(pop)

#if defined(CHRONOS_USE_BOOTSTRAP)

// The MFC resources (Sazabi.rc) live in this DLL, so remember its module handle
// and hand it to AfxWinInit() rather than the bootstrap executable's handle.
static HINSTANCE s_hModuleInstance = NULL;

// Defining DllMain here keeps MFC's own DllMain (dllmodul.obj), which would run
// CSazabi::InitInstance() at load time, out of the link.
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		s_hModuleInstance = hInstance;
	}
	return TRUE;
}

// Exported as declared by include/cef_sandbox_win.h. bootstrap.exe resolves it
// by name after loading ChronosN.dll, and runs it for every process type
// (browser, renderer, gpu, utility, ...).
int RunWinMain(HINSTANCE hInstance,
    LPTSTR lpCmdLine,
    int nCmdShow,
    void* sandbox_info,
    cef_version_info_t* version_info)
{
	// |version_info| describes the libcef build that bootstrap.exe was made for.
	// It is only needed by clients that load libcef.dll through
	// CefScopedLibraryLoader; Chronos resolves libcef.dll via its import library
	// (delay loaded), so there is nothing to check here.
	UNREFERENCED_PARAMETER(version_info);

	// Tell MFC that this module is the application rather than a DLL hosted by
	// one. MFC normally does this from the _afxInitAppState initializer in
	// appmodul.cpp, but that object is only linked into executables, so
	// AFX_MODULE_STATE::m_bDLL would otherwise stay TRUE and MFC would take its
	// "hosted in a DLL" paths everywhere: AfxWinInit() skips AfxInitThread(),
	// AfxOleInit() skips creating the COleMessageFilter that
	// CSazabi::InitFunc_Base() immediately dereferences, CWinApp::ExitInstance()
	// skips SaveStdProfileSettings(), and so on.
	//
	// This must run before AfxWinInit(). Unlike the executable build it does not
	// register _AfxTermAppState() with atexit(), so the MFC thread local data is
	// released by process teardown instead.
	AfxInitialize(FALSE, _MFC_VER);

	g_hChronosModuleInstance = s_hModuleInstance;
	g_hChronosProcessInstance = hInstance;
	g_pChronosSandboxInfo = sandbox_info;

	return ChronosRunMain(lpCmdLine, nCmdShow);
}

#endif // defined(CHRONOS_USE_BOOTSTRAP)
