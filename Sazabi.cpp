#include "stdafx.h"
#include "Sazabi.h"

#include "MainFrm.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CSazabi, CWinApp)
	//{{AFX_MSG_MAP(CSazabi)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CSazabi::CSazabi()
{
	bCreateFavDone = FALSE;
	m_iWinOSVersion = 0;
	m_iWinOSBuildVersion = 0;
	::ZeroMemory(m_FrmWndClassName, sizeof(TCHAR) * 512);
	m_dwProcessId = 0;
	::ZeroMemory(&m_NormalWindow_FramePlacementCache, sizeof(WINDOWPLACEMENT));
	m_NormalWindow_FramePlacementCache.length = sizeof(WINDOWPLACEMENT);
	m_pSettingDlg = NULL;
	m_bUseApp = FALSE;
	m_pDebugDlg = NULL;
	m_hEventLog = NULL;
	m_hEventLogScript = NULL;
	m_hEventRecovery = NULL;
	m_bNewInstanceParam = FALSE;
	m_bAbortFlg = FALSE;
	m_pScHost = NULL;
	//SZB
	m_bCEFInitialized = FALSE;
	m_bToBeShutdown = FALSE;

	m_hEventURLFilterAllow = NULL;
	m_hEventURLFilterDeny = NULL;
	m_bTabWndChanging = FALSE;
	::ZeroMemory(&m_ActiveFramePlacement, sizeof(WINDOWPLACEMENT));
	m_ActiveFramePlacement.length = sizeof(WINDOWPLACEMENT);
	m_wndpClose = NULL;
	m_wndpActiveTabLine = NULL;
	m_wndpNewTab = NULL;
	m_VEcache = 9999;
	m_tRDS = 9999;
	m_pAPIHook = NULL;
	m_pLogDisp = NULL;
	m_hwndTaskDlg = NULL;
	m_bTabEnable_Init = FALSE;
	m_bShutdownFlg = FALSE;
	m_ScaleDPI = 0.0;
	m_IsSGMode = TRUE;
	m_bFirstInstance = FALSE;
	m_bEnforceDeleteCache = FALSE;
	m_bMultiThreadedMessageLoop = FALSE;
	m_pMessageLoopWorker = NULL;
}
CSazabi::~CSazabi()
{
	if (m_pAPIHook)
	{
		delete m_pAPIHook;
		m_pAPIHook = NULL;
	}
}
CSazabi theApp;

/*
 * ThinAppの配下のプロセスとしてChronosを起動する。
 */
BOOL CSazabi::InitFunc_ExecOnVOS()
{
	PROC_TIME(InitFunc_ExecOnVOS)
	if (InVirtualEnvironment() == VE_NA && m_strExeFileName.CompareNoCase(_T("ChronosN.exe")) == 0)
	{
		CString strChronosVirtAppPath;
		strChronosVirtAppPath = m_strExeFolderPath;
		strChronosVirtAppPath += _T("Chronos.exe");

		//Chronos.exeを実行する
		if (PathFileExists(strChronosVirtAppPath))
		{
			CString strCommand;
			strCommand.Format(_T("\"%s\""), (LPCTSTR)strChronosVirtAppPath);
			if (!m_strCommandParam.IsEmpty())
			{
				strCommand.Format(_T("%s \"%s\""), (LPCTSTR)strCommand, (LPCTSTR)m_strCommandParam);
			}
			if (!m_strOptionParam.IsEmpty())
			{
				strCommand.Format(_T("%s %s"), (LPCTSTR)strCommand, (LPCTSTR)m_strOptionParam);
			}
			if (!m_strConfigParam.IsEmpty())
			{
				strCommand.Format(_T("%s %s"), (LPCTSTR)strCommand, (LPCTSTR)m_strConfigParam);
			}

			STARTUPINFO siC = {0};
			PROCESS_INFORMATION piC = {0};
			siC.cb = sizeof(siC);
			unsigned long ecodeC = 0;
			if (!CreateProcess(NULL, (LPTSTR)(LPCTSTR)strCommand, NULL, NULL, FALSE, 0, NULL, NULL, &siC, &piC))
			{
				SetLastError(NO_ERROR);
				//Retry
				if (!CreateProcess(strChronosVirtAppPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &siC, &piC))
				{
					SetLastError(NO_ERROR);
					if (::ShellExecute(NULL, _T("open"), strChronosVirtAppPath, NULL, NULL, SW_SHOW) <= HINSTANCE(32))
					{
						::ShellExecute(NULL, NULL, strCommand, NULL, NULL, SW_SHOW);
					}
				}
			}
			if (piC.hThread)
			{
				CloseHandle(piC.hThread);
				piC.hThread = 0;
			}
			if (piC.hProcess)
			{
				CloseHandle(piC.hProcess);
				piC.hProcess = 0;
			}
			return FALSE;
		}
		else
		{
#ifdef _DEBUG
			return TRUE;
#endif
			CString alertMsg;
			alertMsg.LoadString(IDS_STRING_ALERT_MISSING_FILES);
			CString strMsg;
			strMsg.Format(alertMsg, strChronosVirtAppPath);
			::MessageBox(NULL, strMsg, m_strThisAppName, MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
			//Debugで困るのでShitキーを押している場合は、続行。
			if (::GetKeyState(VK_SHIFT) < 0)
				return TRUE;
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * Win32 GUIコンポーネント (MFC・コモンコントロール) を初期化する
 */
BOOL CSazabi::InitFunc_Base()
{
	PROC_TIME(InitFunc_Base)
	INITCOMMONCONTROLSEX InitCtrls = {0};
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	CWinApp::InitInstance();

	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	::OleInitialize(NULL);
	if (!AfxOleInit())
	{
		return FALSE;
	}
	AfxOleGetMessageFilter()->EnableNotRespondingDialog(FALSE);
	AfxOleGetMessageFilter()->EnableBusyDialog(FALSE);
	AfxEnableControlContainer();

	/* Enable Direct2D support */
	this->EnableD2DSupport();
	return TRUE;
}

/*
 * スレッド同期用のイベントオブジェクトを準備する。
 *
 * イベントオブジェクトは排他制御（ミューテックス）用に利用する。
 * クリティカルセッション進入時に WaitForSingleObject() でロックを取得し、
 * 処理が完了した際に SetEvent() で解放する。
 *
 * それぞれのイベントが利用される箇所は Sazabi.h を参照。
 */
BOOL CSazabi::InitFunc_Events()
{
	PROC_TIME(InitFunc_Events)
	DWORD pidCurrent = GetCurrentProcessId();
	m_strEventLogName.Format(_T("CSG_EventLog%08x"), pidCurrent);
	m_strEventLogScriptName.Format(_T("CSG_EventLogScript%08x"), pidCurrent);
	m_strEventRecoveryName.Format(_T("CSG_EventRecovery%08x"), pidCurrent);
	m_strEventURLFilterAllow.Format(_T("CSG_EventURLAllow%08x"), pidCurrent);
	m_strEventURLFilterDeny.Format(_T("CSG_EventURLDeny%08x"), pidCurrent);

	m_hEventLog = CreateEvent(NULL, FALSE, TRUE, m_strEventLogName);
	m_hEventLogScript = CreateEvent(NULL, FALSE, TRUE, m_strEventLogScriptName);
	m_hEventRecovery = CreateEvent(NULL, FALSE, TRUE, m_strEventRecoveryName);
	m_hEventURLFilterAllow = CreateEvent(NULL, FALSE, TRUE, m_strEventURLFilterAllow);
	m_hEventURLFilterDeny = CreateEvent(NULL, FALSE, TRUE, m_strEventURLFilterDeny);

	return TRUE;
}

/*
 * 実行ファイル名からパス変数を初期化する。
 *
 * C:\Program Files\Chronos
 * ├── ChronosN.exe           ... 実行ファイル
 * ├── ChronosDefault.conf    ... 設定ファイル
 * ├── Chronos_trace.log      ... ログファイル
 * ├── ChTaskMgrexe           ... 
 * ├── SPC.exe                ... 起動時のスプラッシュスクリーン (SPlash screen for Chronos)
 * └── DBLC.exe               ... IE/Firefox/Edge/Chromeのブラウザの起動、
 *                                お気入りの追加 (Default Browser Launcher for Chronos)
 */
BOOL CSazabi::InitFunc_Paths()
{
	PROC_TIME(initFunc_Paths)
	TCHAR szPath1[MAX_PATH + 1] = {0};
	TCHAR szDrive1[_MAX_DRIVE + 1] = {0};
	TCHAR szDir1[_MAX_DIR + 1] = {0};
	TCHAR szFileName1[_MAX_FNAME + 1] = {0};
	TCHAR szExt1[_MAX_EXT + 1] = {0};
	GetModuleFileName(AfxGetInstanceHandle(), szPath1, MAX_PATH);
	_tsplitpath_s(szPath1, szDrive1, _MAX_DRIVE, szDir1, _MAX_DIR, szFileName1, _MAX_FNAME, szExt1, _MAX_EXT);

	m_strExeFullPath = szPath1;
	m_strExeFolderPath = szDrive1;
	m_strExeFolderPath += szDir1;

	m_strExeFullPath.Replace(_T("\\.\\"), _T("\\"));
	m_strExeFolderPath.Replace(_T("\\.\\"), _T("\\"));
	SetLastError(NO_ERROR);
	m_strExeFileName = szFileName1;
	m_strExeFileName += szExt1;
	if (m_strExeFileName.IsEmpty())
	{
		m_strExeFileName = m_strThisAppName;
	}

	m_strSettingFileFullPath = m_strExeFolderPath;
	m_strSettingFileFullPath += _T("ChronosDefault.conf");

	// Debug用のLogファイルを準備
	m_strLogFileFullPath = m_strExeFolderPath;
	m_strLogFileFullPath += _T("Chronos_trace.log");

	m_strDBL_EXE_FullPath = m_strExeFolderPath;
	m_strDBL_EXE_FullPath += _T("DBLC.EXE");
	m_strDBL_EXE_Default_FullPath = m_strDBL_EXE_FullPath;

	// パスをカレントに変更しておく。セキュリティ対策。
	::SetCurrentDirectory(m_strExeFolderPath);
	SetLastError(NO_ERROR);

	return TRUE;
}

/*
 * 設定ファイルを読み込んで適用する。
 *
 *  - LoadDefaultData()    ... 標準設定値
 *  - ChronosDefault.conf  ... 設定ファイル
 *  - Chronos.conf         ... 設定ファイル (ThinApp環境のみ)
 */
BOOL CSazabi::InitFunc_Settings()
{
	PROC_TIME(InitFunc_Settings)

	// AppSettingsにSystem Guardフラグをコピーする
	this->m_AppSettings.m_IsSGMode = this->m_IsSGMode;

	// 設定データの初期値を読み込む
	this->m_AppSettings.LoadDefaultData();

	if (InVirtualEnvironment() == VE_NA)
	{
		// ネイティブ環境の場合、設定ファイルパスが指定されていればそちらから、
		// 指定されていなければChronosDefault.confから設定データを読み込む
		CString configPath = m_strSettingFileFullPath;
		if (PathFileExists(m_strSettingFileFullPath))
		{
			this->m_AppSettings.LoadDataFromFile(m_strSettingFileFullPath);
		}
		if (theApp.m_AppSettings.IsEnableUserConfig() && 
		    PathFileExists(m_strUserConfigFilePath))
		{
			theApp.m_AppSettings.LoadDefaultData();
			this->m_AppSettings.LoadDataFromFile(m_strUserConfigFilePath);
		}
	}
	else if (InVirtualEnvironment() == VE_THINAPP)
	{
		//  ThinApp環境では、必ず最初にChronosDefault.confから設定データを読み込む
		if (PathFileExists(m_strSettingFileFullPath))
		{
			this->m_AppSettings.LoadDataFromFile(m_strSettingFileFullPath);
		}
		if (theApp.m_AppSettings.IsEnableUserConfig())
		{
			// 更にChronos.confまたは指定されたファイルから設定を追加で読み込む
			CString additionalConfigPath = m_strUserConfigFilePath.IsEmpty() ? 
				GetThinAppEntryPointFolderPath() + _T("Chronos.conf") : 
				m_strUserConfigFilePath;
			if (PathFileExists(additionalConfigPath))
			{
				this->m_AppSettings.LoadDataFromFile(additionalConfigPath);
			}
		}
	}

	// 起動時のTab設定をGlobalに登録しておく、途中での切り替え不可
	m_bTabEnable_Init = this->m_AppSettings.IsEnableTab();

	return TRUE;
}

BOOL CSazabi::InitFunc_SGMode()
{
	PROC_TIME(InitFunc_SGMode)
	if (!this->IsSGMode())
		return FALSE;

	if (!IsFirstInstance())
		return TRUE;

	// -NEWで起動された場合以外
	if (m_bNewInstanceParam)
		return TRUE;

	// 以下の処理はVOS環境のみ実行する
	UINT InVirtEnv = VE_NA;
	InVirtEnv = InVirtualEnvironment();
	if (InVirtEnv == VE_NA)
		return TRUE;

	// エクスプローラからクイックアクセスを削除する
	HKEY hKey = {0};
	DWORD dwDisposition = 0;
	LONG lResult = 0L;
	CString strRegKey;
	DWORD Val = 0xa0600000;
	strRegKey = _T("CLSID\\{679f85cb-0220-4080-b29b-5540cc05aab6}\\ShellFolder");
	lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT, strRegKey,
				 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition);
	if (lResult == ERROR_SUCCESS)
	{
		RegSetValueEx(hKey, _T("Attributes"), 0, REG_DWORD, (LPBYTE)&Val, sizeof(DWORD));
		RegCloseKey(hKey);
	}
	strRegKey = _T("SOFTWARE\\Classes\\CLSID\\{679f85cb-0220-4080-b29b-5540cc05aab6}\\ShellFolder");
	lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, strRegKey,
				 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition);
	if (lResult == ERROR_SUCCESS)
	{
		RegSetValueEx(hKey, _T("Attributes"), 0, REG_DWORD, (LPBYTE)&Val, sizeof(DWORD));
		RegCloseKey(hKey);
	}

	CString strRegKeyBase;
	strRegKeyBase = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\");
	strRegKey.Format(_T("%s%s"), (LPCTSTR)strRegKeyBase, _T("{B4BFCC3A-DB2C-424C-B029-7FE99A87C641}"));
	SHDeleteKey(HKEY_LOCAL_MACHINE, strRegKey);
	lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, strRegKey,
				 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition);
	if (lResult == ERROR_SUCCESS)
	{
		RegSetValueEx(hKey, _T("Attributes"), 0, REG_DWORD, (LPBYTE)&Val, sizeof(DWORD));
		RegCloseKey(hKey);
	}
	SHDeleteKey(HKEY_LOCAL_MACHINE, strRegKey);

	// SpC.exeを実行する
	CString strSpCAppPath;
	strSpCAppPath = m_strExeFolderPath;
	strSpCAppPath += _T("SpC.exe");

	if (PathFileExists(strSpCAppPath))
	{
		CString strCommandC;
		CString strParam;

		// https://github.com/ThinBridge/Chronos/pull/300
		// 
		// `SpC.exe "B:\" /ChronosConfig="path/to/config"`といったコマンドラインが作成された場合、`\"`がエスケープされ
		// SpC.exeは`B:\" /ChronosConfig="path/to/config`という一つの引数を受け取る。先頭の`"`は文字列開始の`"`として
		// 解釈され、対応する`"`がないので消えている。本来は`B:\`と`/ChronosConfig="path/to/config`という二つの引数となるべきである。
		// これは、`\"`とならなければ回避可能である。そのため、フォルダーの末尾の`\`が存在すれば削除し、`\"`とならないようにしている。
		// （なお、`\\"`としても回避可能ではある）
		strParam.Format(_T("\"%s\" %s"), (LPCTSTR)m_AppSettings.GetRootPath().TrimRight(_T("\\")), (LPCTSTR)m_strConfigParam);
		strCommandC.Format(_T("\"%s\" %s"), (LPCTSTR)strSpCAppPath, (LPCTSTR)strParam);
		STARTUPINFO siC = {0};
		PROCESS_INFORMATION piC = {0};
		siC.cb = sizeof(siC);
		unsigned long ecodeC = 0;
		if (!CreateProcess(NULL, (LPTSTR)(LPCTSTR)strCommandC, NULL, NULL, FALSE, 0, NULL, NULL, &siC, &piC))
		{
			SetLastError(NO_ERROR);
			//Retry
			if (!CreateProcess(strSpCAppPath, (LPTSTR)(LPCTSTR)strParam, NULL, NULL, FALSE, 0, NULL, NULL, &siC, &piC))
			{
				SetLastError(NO_ERROR);
			}
		}
		else
		{
			WaitForSingleObject(piC.hProcess, 1500);
		}

		if (piC.hThread)
		{
			CloseHandle(piC.hThread);
			piC.hThread = 0;
		}
		if (piC.hProcess)
		{
			CloseHandle(piC.hProcess);
			piC.hProcess = 0;
		}

		CString strDiskPath;
		CString strDosDrive;
		TCHAR szTargetPath[4096] = {0};
		CString strDrive;
		TCHAR szPath[MAX_PATH] = {0};
		StringCchCopy(szPath, MAX_PATH, m_AppSettings.GetRootPath());
		if (PathStripToRoot(szPath))
			strDrive = szPath;
		if (strDrive.IsEmpty())
			strDrive = _T("B:\\");
		strDosDrive = strDrive[0];
		strDosDrive += _T(":");
		if (0 != QueryDosDevice(strDosDrive, szTargetPath, 4096))
		{
			strDiskPath = szTargetPath;
			if (strDiskPath.Find(_T("\\??\\")) == 0)
			{
				strDiskPath.Replace(_T("\\??\\"), _T(""));
				strDiskPath.TrimRight('\\');
				strDiskPath += _T("\\");
				if (!IsFolderExists(strDiskPath))
				{
					MakeDirectoryPath(strDiskPath);
				}
			}
		}
	}
	return TRUE;
}

/*
 * MFCアプリケーションのエントリポイント
 */
BOOL CSazabi::InitInstance()
{
	PROC_TIME(InitInstance)
	CString logmsg;
	// 本メソッドのInitMultipleInstanceとExitInstanceのIsExistsAnotherInstanceが
	// 並列実行されると、他のインスタンスの存在が正しく判定されない可能性がある
	// ため、排他制御を行う。
	HANDLE hMutex = {0};
	DWORD dwWaitResult = WAIT_FAILED;
	CHRONOS_ENTER_CRITICAL_SECTION(hMutex, dwWaitResult, 30 * 1000);
	if (!InitFunc_Base())
	{
		CHRONOS_LEAVE_CRITICAL_SECTION(hMutex, dwWaitResult);
		return FALSE;
	}

	m_strThisAppName = gstrThisAppNameR;

	if (!InitFunc_Events())
	{
		CHRONOS_LEAVE_CRITICAL_SECTION(hMutex, dwWaitResult);
		return FALSE;
	}

	if (!InitFunc_Paths())
	{
		CHRONOS_LEAVE_CRITICAL_SECTION(hMutex, dwWaitResult);
		return FALSE;
	}

	this->GetOSVersion();
	this->SetThisAppVersionString();

	// SpC.exeの有り無しでSGモードかどうかを判断する
	CString strSpCAppPath;
	strSpCAppPath = m_strExeFolderPath;
	strSpCAppPath += _T("SpC.exe");

	if (PathFileExists(strSpCAppPath))
	{
		m_IsSGMode = TRUE;
		m_strThisAppName = gstrThisAppNameSG;
	}
	else
	{
		// SpC.exeがない場合は、SGモードではなく通常モード (旧仕様)
		m_IsSGMode = FALSE;
		m_strThisAppName = gstrThisAppNameR;
	}

	// コマンドラインオプションを解析する
	this->InitParseCommandLine();

	// 設定ファイル初期読み込み
	if (!InitFunc_Settings())
	{
		CHRONOS_LEAVE_CRITICAL_SECTION(hMutex, dwWaitResult);
		return FALSE;
	}
	ReflectEnforcedOptionParam();
	InitAtomParam();

	// VOS以外（物理環境で直接このEXEが起動された場合は、VOS環境で再実行）
	if (!InitFunc_ExecOnVOS())
	{
		CHRONOS_LEAVE_CRITICAL_SECTION(hMutex, dwWaitResult);
		return TRUE;
	}

	// 2重起動をチェックする。EXEのパスが違う場合は、起動を許可する。
	if (InitMultipleInstance())
	{
		CHRONOS_LEAVE_CRITICAL_SECTION(hMutex, dwWaitResult);
		return TRUE;
	}

	// Chronos.exeのバージョンとChronosN.exeのバージョンが異なる場合、警告する
	CheckChronosVersionMismatch();

	// 保存 (ChronosDefault.conf)
	this->m_AppSettings.SaveDataToFileEx(this->m_strSettingFileFullPath);

	// API Hook初期化
	if (!m_pAPIHook)
	{
		m_pAPIHook = new APIHookC;
		m_pAPIHook->DoHookComDlgAPI();
	}

	// SZB
	InitializeCef();

	// SystemGuard用の処理
	if (this->IsSGMode())
	{
		InitFunc_SGMode();
	}

	CopyDBLEXEToTempInit();
	SetRecoveryFilePath();

	// 起動時のログを出力
	if (m_AppSettings.IsAdvancedLogMode())
	{
		if (IsFirstInstance())
			this->InitLogWrite();
	}

	if (!hMutex)
	{
		WriteDebugTraceDateTime(_T("InitInstance: Failed to CreateMutex"), DEBUG_LOG_TYPE_GE);
	}
	if (hMutex && dwWaitResult != WAIT_OBJECT_0)
	{
		logmsg.Format(_T("InitInstance: Failed to acquire mutex: %lx"), dwWaitResult);
		WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_GE);
	}

	// 各種confファイルの読込
	this->InitReadConfSetting();

	// Message文言セット
	m_strZoneMessageDBL.LoadString(IDS_STRING_ZONE_MSG_DBL);
	m_strZoneMessageNG.LoadString(IDS_STRING_ZONE_MSG_NG);
	m_strZoneMessageIE.LoadString(IDS_STRING_ZONE_MSG_IE);
	m_strZoneMessageFF.LoadString(IDS_STRING_ZONE_MSG_FF);
	m_strZoneMessageCHR.LoadString(IDS_STRING_ZONE_MSG_CHR);
	m_strZoneMessageEDG.LoadString(IDS_STRING_ZONE_MSG_EDG);
	m_strZoneMessageCustom.LoadString(IDS_STRING_ZONE_MSG_CUSTOM);

	// ???
	bCreateFavDone = FALSE;

	// アイコン画像を読み込む
	CBitmap bitmap1;
	bitmap1.LoadBitmap(IDB_BITMAP4);
	m_imgMenuIcons.Create(16, 16, ILC_COLOR8 | ILC_MASK, 8, 1);
	m_imgMenuIcons.Add(&bitmap1, RGB(255, 0, 255));
	bitmap1.DeleteObject();

	m_imgFavIcons.Create(24, 24, ILC_COLOR32 | ILC_MASK, 8, 10);

	// ウィンドウを一意に識別するIDを生成する (e.g. "_2552")
	WCHAR szAppID[129] = {0};
	PWSTR pszAppID = szAppID;
	DWORD pidCurrent = GetCurrentProcessId();
	GetCurrentProcessExplicitAppUserModelID(&pszAppID);
	m_strAppIDw.Format(L"%s_%d", szAppID, pidCurrent);
	SetCurrentProcessExplicitAppUserModelID(m_strAppIDw);

	m_bUseApp = TRUE;

	// VBShost Create
	m_pScHost = new CScriptHost;

	// Hash Table init
	if (m_AppSettings.IsEnableURLRedirect())
		m_CacheRedirectFilter_None.InitHashTable(1231, true);

	if (m_AppSettings.IsEnableURLFilter())
	{
		m_CacheURLFilter_Allow.InitHashTable(1231, true);
		m_CacheURLFilter_Deny.InitHashTable(1231, true);
	}

	// LogDisp
	if (!m_pLogDisp)
	{
		m_pLogDisp = new CLogDispatcher;
		m_pLogDisp->Init();
	}

	// メインフレームを初期化する
	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	RECT rcMfrm = {0};
	DWORD dwCreateStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE;
	pFrame->Create(NULL, m_strThisAppName,
		       dwCreateStyle, rcMfrm, NULL, NULL);

	//@Log Write
	if (m_AppSettings.IsAdvancedLogMode())
	{
		if (IsFirstInstance())
		{
			logmsg.Format(_T("CommandParam:[%s]"), (LPCTSTR)m_lpCmdLine);
			WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_GE);
			logmsg.Format(_T("AtomParam:[%s]"), (LPCTSTR)m_strAtomParam);
			WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_GE);
		}
	}
	::GetWindowThreadProcessId(pFrame->m_hWnd, &m_dwProcessId);
	m_hProcess.Attach(::OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_dwProcessId));
	InitProcessSetting();
	CHRONOS_LEAVE_CRITICAL_SECTION(hMutex, dwWaitResult);
	return TRUE;
}

void CSazabi::SendLoggingMsg(int LOG_TYPE, LPCTSTR lpFileName, HWND hWnd)
{
	CString strFileName;
	strFileName = lpFileName;
	CString strURL;
	CBrowserFrame* pFrame = NULL;
	pFrame = ((CMainFrame*)m_pMainWnd)->GetBrowserFrameFromChildWnd(hWnd);
	if (IsWnd(pFrame))
	{
		if (IsWnd(&pFrame->m_wndView))
		{
			strURL = pFrame->m_wndView.m_strURL;
			if (m_pLogDisp)
			{
				m_pLogDisp->SendLog(LOG_TYPE, strFileName, strURL);
				return;
			}
		}
	}
}
CChildView* CSazabi::GetChildViewPtr(HWND hWnd)
{
	CChildView* pChild = NULL;
	pChild = GetActiveViewPtr();
	if (IsWnd(pChild))
	{
		if (pChild->m_hWnd == hWnd)
			return pChild;
	}

	CBrowserFrame* pFrame = NULL;
	pFrame = ((CMainFrame*)m_pMainWnd)->GetBrowserFrameFromChildWnd(hWnd);
	if (IsWnd(pFrame))
	{
		if (IsWnd(&pFrame->m_wndView))
		{
			return &pFrame->m_wndView;
		}
	}
	return NULL;
}

void CSazabi::InitProcessSetting()
{
	//Server OSの場合は、RDSH環境の可能性が高いので、優先度を下げる。
	if (SBUtil::IsWindowsServerRDS())
	{
		::SetPriorityClass(::GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
	}
}

/*
 * 次の設定ファイルを読み込む。
 *
 * RedirectFilterScript.conf  ... 他のブラウザにリダイレクトする。
 * URL_DomainFilter.conf      ... ドメインで閲覧を無効化する。
 * CustomScript.conf          ...
 *
 * 次のファイルをコピーする。
 *
 * logo{Default}.(bmp|png)    ... Chronosのロゴファイル
 */
void CSazabi::InitReadConfSetting()
{
	PROC_TIME(InitReadConfSetting)

	// -------------------------
	// RedirectFilterScript.conf
	// -------------------------
	CString strRedirectFilterScriptFullPath = m_strExeFolderPath;
	strRedirectFilterScriptFullPath += _T("RedirectFilterScriptDefault.conf");
	//一旦コピーする。
	if (InVirtualEnvironment() == VE_THINAPP)
	{
		CString strTS_Path;
		strTS_Path = GetThinAppEntryPointFolderPath();
		strTS_Path += _T("RedirectFilterScript.conf");
		//Fileが存在する場合は、コピーする。
		if (PathFileExists(strTS_Path))
		{
			//Fileから設定値を読み込む許可
			::CopyFile(strTS_Path, strRedirectFilterScriptFullPath, FALSE);
			SetLastError(NO_ERROR);
		}
	}
	m_cScriptSrc.SetFilePathAndSetData(strRedirectFilterScriptFullPath);

	// -------------------------
	// URL_DomainFilter.conf
	// -------------------------
	m_strDomainFilterFileFullPath = m_strExeFolderPath;
	m_strDomainFilterFileFullPath += _T("URL_DomainFilterDefault.conf");
	//一旦コピーする。
	if (InVirtualEnvironment() == VE_THINAPP)
	{
		CString strTS_Path;
		strTS_Path = GetThinAppEntryPointFolderPath();
		strTS_Path += _T("URL_DomainFilter.conf");
		//Fileが存在する場合は、コピーする。
		if (PathFileExists(strTS_Path))
		{
			//Fileから設定値を読み込む許可
			::CopyFile(strTS_Path, m_strDomainFilterFileFullPath, FALSE);
			SetLastError(NO_ERROR);
		}
	}
	m_cDomainFilterList.SetFilePathAndCreateArrayData(m_strDomainFilterFileFullPath);
	m_cDomainFilterList.m_LogMsg.Empty();

	// -------------------------
	// PopupFilter.conf
	// -------------------------
	m_strPopupFilterFileFullPath = m_strExeFolderPath;
	m_strPopupFilterFileFullPath += _T("PopupFilterDefault.conf");
	// 一旦コピーする。
	if (InVirtualEnvironment() == VE_THINAPP)
	{
		CString strTS_Path;
		strTS_Path = GetThinAppEntryPointFolderPath();
		strTS_Path += _T("PopupFilter.conf");
		// Fileが存在する場合は、コピーする。
		if (PathFileExists(strTS_Path))
		{
			// Fileから設定値を読み込む許可
			::CopyFile(strTS_Path, m_strPopupFilterFileFullPath, FALSE);
			SetLastError(NO_ERROR);
		}
	}
	m_cPopupFilterList.SetFilePathAndCreateArrayData(m_strPopupFilterFileFullPath);
	m_cPopupFilterList.m_LogMsg.Empty();

	// -------------------------
	// CustomScript.conf
	// -------------------------
	m_strCustomScriptConfFullPath = m_strExeFolderPath;
	m_strCustomScriptConfFullPath += _T("CustomScriptDefault.conf");
	//一旦コピーする。
	if (InVirtualEnvironment() == VE_THINAPP)
	{
		CString strTS_Path;
		strTS_Path = GetThinAppEntryPointFolderPath();
		strTS_Path += _T("CustomScript.conf");
		//Fileが存在する場合は、コピーする。
		if (PathFileExists(strTS_Path))
		{
			//Fileから設定値を読み込む許可
			::CopyFile(strTS_Path, m_strCustomScriptConfFullPath, FALSE);
			SetLastError(NO_ERROR);
		}
	}
	m_cCustomScriptList.SetFilePathAndCreateArrayData(m_strCustomScriptConfFullPath);
	m_cCustomScriptList.m_LogMsg.Empty();

	// -------------------------
	// logo.png (and others)
	// -------------------------
	CString strLogoFileFullPath = m_strExeFolderPath;
	strLogoFileFullPath += _T("logoDefault.bmp");
	m_strLogoFileFullPath = strLogoFileFullPath;
	//一旦コピーする。
	if (InVirtualEnvironment() == VE_THINAPP)
	{
		CString strTS_Path;
		strTS_Path = GetThinAppEntryPointFolderPath();
		strTS_Path += _T("logo.bmp");
		//Fileが存在する場合は、コピーする。
		if (PathFileExists(strTS_Path))
		{
			::CopyFile(strTS_Path, strLogoFileFullPath, FALSE);
			SetLastError(NO_ERROR);
		}
		//png
		else
		{
			strLogoFileFullPath = m_strExeFolderPath;
			strLogoFileFullPath += _T("logoDefault.png");
			m_strLogoFileFullPath = strLogoFileFullPath;
			CString strTS_PathPNG;
			strTS_PathPNG = GetThinAppEntryPointFolderPath();
			strTS_PathPNG += _T("logo.png");
			//Fileが存在する場合は、コピーする。
			if (PathFileExists(strTS_PathPNG))
			{
				::CopyFile(strTS_PathPNG, strLogoFileFullPath, FALSE);
				SetLastError(NO_ERROR);
			}
		}
	}
}

/*
 * 多重起動判定に該当した場合にTRUEを返却する。
 *
 * - ChronosはモーダルIDに、実行ファイルのパスを識別子として設定する
 *   (例: "C-Program_Files-Chronos-Chronosexe")
 * - 既にモーダルIDが等しいウィンドウがあれば、多重起動とみなす。
 *
 * ただし、次の場合は多重起動を許可する。
 *
 * - `-NEW` オプションをつけて起動した場合
 * - `EnableMultipleInstance` の設定で多重起動を許可している場合
 */
BOOL CSazabi::InitMultipleInstance()
{
	PROC_TIME(InitMultipleInstance)

	CString strTargetPath;
	if (InVirtualEnvironment() == VE_THINAPP)
	{
		strTargetPath = GetThinAppEntryPointPath();
	}
	else
	{
		strTargetPath = m_strExeFolderPath;
		strTargetPath += _T("Chronos.exe");
	}

	if (strTargetPath.IsEmpty())
		strTargetPath = m_strExeFullPath;

	CString strFrmWndClass = strTargetPath;
	strFrmWndClass.TrimLeft();
	strFrmWndClass.TrimRight();
	strFrmWndClass.MakeUpper();
	strFrmWndClass.Replace(_T("\\"), _T("-"));
	strFrmWndClass.Replace(_T(":"), _T(""));
	strFrmWndClass.Replace(_T("."), _T(""));
	strFrmWndClass.Replace(_T("--"), _T("-"));
	strFrmWndClass.Replace(_T(" "), _T("_"));
	StringCchCopy(m_FrmWndClassName, 512, strFrmWndClass);

	SetAppID(m_FrmWndClassName);

	// ウィンドウを探索して多重起動を確認する
	HWND hWndCap = FindWindow(m_FrmWndClassName, NULL);
	if (hWndCap != NULL)
	{
		TCHAR szTitleMultipleInstance[260] = {0};
		::GetWindowText(hWndCap, szTitleMultipleInstance, 259);
		CString strTitleMultiple;
		strTitleMultiple = szTitleMultipleInstance;
		// モーダルIDを確認する
		if (strTitleMultiple == m_FrmWndClassName)
		{
			// 起動している
			m_bFirstInstance = FALSE;
			// NEWインスタンス　オプションがある場合
			if (m_bNewInstanceParam)
			{
				return FALSE;
			}
			// 多重起動防止の場合。
			if (!m_AppSettings.IsMultipleInstance())
			{
				ATOM nAtom = {0};
				if (m_strAtomParam.IsEmpty())
				{
					nAtom = ::GlobalAddAtom(m_strAtomParam);
					LRESULT lr = SendMessageTimeout(hWndCap, WM_NEWINSTANCE, (WPARAM)0, 0, SMTO_BLOCK, 3 * 1000, NULL);
				}
				else
				{
					nAtom = ::GlobalAddAtom(m_strAtomParam);
					LRESULT lr = SendMessageTimeout(hWndCap, WM_NEWINSTANCE, (WPARAM)nAtom, 0, SMTO_BLOCK, 3 * 1000, NULL);
				}
				return TRUE;
			}
		}
	}
	else
		m_bFirstInstance = TRUE;
	return FALSE;
}

/*
 * Chronos.exe [-NEW] [-VIEW] [-NORMAL] [-MAX] [-MIN] URL

 * 
 * -NEW    ... 新しいインスタンスを起動する
 * -VIEW   ... タブを無効化して新しいインスタンスを起動する
 * -MAX    ... 最大化モード (MainFrm.cpp)
 * -MIN    ... 最小化モード (MainFrm.cpp)
 * -NORMAL ... ウィンドウモード
 * -ChronosConfig="path to config file" ... 設定ファイルを指定する
 * 
 * -の代わりに/も使用可能
 */
void CSazabi::ParseSingleParam(CString param) {
	CString trimmedParam(param);
	trimmedParam.Replace(_T("\""), _T(""));
	trimmedParam.TrimLeft();
	trimmedParam.TrimRight();

	if (trimmedParam.IsEmpty())
	{
		return;
	}

	if (trimmedParam.Find(_T("-")) == 0 || trimmedParam.Find(_T("/")) == 0)
	{
		//-または/で始まる値はオプション
		CString paramValue(trimmedParam);
		paramValue.TrimLeft('-');
		paramValue.TrimLeft('/');
		paramValue.MakeLower();
		// CEFのパラメータを解釈しないように、明示的にChronosのパラメータのみ確認する。
		if (paramValue.CompareNoCase(_T("NEW")) == 0)
		{
			m_bNewInstanceParam = TRUE;
		}
		else if (paramValue.CompareNoCase(_T("VIEW")) == 0)
		{
			m_bNewInstanceParam = TRUE;
		}
		else if (paramValue.CompareNoCase(_T("MAX")) == 0)
		{
			m_strOptionParam = trimmedParam;
		}
		else if (paramValue.CompareNoCase(_T("MIN")) == 0)
		{
			m_strOptionParam = trimmedParam;
		}
		else if (paramValue.CompareNoCase(_T("NORMAL")) == 0)
		{
			m_strOptionParam = trimmedParam;
		}
		else if (paramValue.Find(_T("chronosconfig")) == 0)
		{
			int pos = trimmedParam.Find(_T('='));
			if (pos != -1)
			{
				m_strUserConfigFilePath = trimmedParam.Right(trimmedParam.GetLength() - pos - 1).Trim(_T('"'));
				// 引数解析の際に、パラメータで使用している""が削除される。
				// そのため、ここで明示的に""を足す。
				m_strConfigParam.Format(_T("/ChronosConfig=\"%s\""), (LPCTSTR)m_strUserConfigFilePath);
			}
		}
	}
	else if (SBUtil::IsURL(trimmedParam))
	{
		// URL。強制的にCommandParamとする。
		m_strCommandParam = trimmedParam;
	}
	else if (trimmedParam.Find(_T(":")) == 1)
	{
		// ファイルパス。強制的にCommandParamとする。
		m_strCommandParam = trimmedParam;
	}
}

/*
 * Chronos.exe [-NEW] [-View] [-NORMAL] [-MAX] [-MIN] URL
 *
 * -NEW    ... 新しいインスタンスを起動する
 * -VIEW   ... タブを無効化して新しいインスタンスを起動する
 * -MAX    ... 最大化モード (MainFrm.cpp)
 * -MIN    ... 最小化モード (MainFrm.cpp)
 * -NORMAL ... ウィンドウモード
 *
 * -の代わりに/も使用可能
 */
void CSazabi::InitParseCommandLine()
{
	PROC_TIME(InitParseCommandLine)

	m_strCommandParam.Empty();
	m_strOptionParam.Empty();

	for (int i = 1; i < __argc; i++)
	{
		CString param(__wargv[i]);
		ParseSingleParam(param);
	}
}

void CSazabi::InitAtomParam()
{
	if (m_bNewInstanceParam)
	{
		if (m_strOptionParam.IsEmpty())
		{
			m_strOptionParam = _T("/NORMAL");
		}
	}

	// 一旦クリア
	m_strAtomParam.Empty();

	if (!m_strCommandParam.IsEmpty())
	{
		m_strAtomParam = m_strCommandParam;
	}

	if (!m_strOptionParam.IsEmpty())
	{
		if (!m_strAtomParam.IsEmpty())
		{
			m_strAtomParam += _T("|@@|");
		}
		m_strAtomParam += m_strOptionParam;
	}

	if (!m_strConfigParam.IsEmpty())
	{
		if (!m_strAtomParam.IsEmpty())
		{
			m_strAtomParam += _T("|@@|");
		}
		m_strAtomParam += m_strConfigParam;
	}
}

void CSazabi::ReflectEnforcedOptionParam()
{
	if (!m_strOptionParam.IsEmpty())
	{
		// 既にコマンドラインパラメータがある場合は、そちらを優先する
		return;
	}

	// 起動時の強制パラメータを確認
	CString strEnforceInitParam;
	strEnforceInitParam = m_AppSettings.GetEnforceInitParam();

	// 整形
	strEnforceInitParam.TrimLeft();
	strEnforceInitParam.TrimRight();
	strEnforceInitParam.Replace(_T("\""), _T(""));

	if (strEnforceInitParam.IsEmpty())
	{
		return;
	}
	m_strOptionParam = strEnforceInitParam;
}

void CSazabi::ExitKillZombieProcess()
{
	PROC_TIME(ExitKillZombieProcess)

	if (InVirtualEnvironment() == VE_THINAPP)
	{
		//DBLでゾンビを終了させる。VOS上ではTerminateできなかった。
		CString strCommand;
		CString strParam;
		CopyDBLEXEToTempEx();
		strCommand.Format(_T("\"%s\" -ClosePWait"), (LPCTSTR)m_strDBL_EXE_FullPath);
		strParam = _T("-ClosePWait");
		SetLastError(NO_ERROR);
		if (::ShellExecute(NULL, _T("open"), m_strDBL_EXE_FullPath, strParam, NULL, SW_SHOW) <= HINSTANCE(32))
		{
			::ShellExecute(NULL, NULL, strCommand, NULL, NULL, SW_SHOW);
		}
	}
}
void CSazabi::OpenChFiler(CHFILER_INIT_MODE initMode, LPCTSTR lpOpenPath)
{
	PROC_TIME(OpenChFiler)
	try
	{
		if (!theApp.IsSGMode()) return;

		if (initMode == CHFILER_INIT_MODE::EXIT_CHECK)
		{
			CString strExecCommand;
			CString strCommand;
			LPCTSTR strParam = _T("/ExitChk");
			strCommand.Format(_T("\"%sChFiler.exe\""), (LPCTSTR)m_strExeFolderPath);
			strExecCommand.Format(_T("\"%sChFiler.exe\" %s"), (LPCTSTR)m_strExeFolderPath, strParam);

			STARTUPINFO si = {0};
			PROCESS_INFORMATION pi = {0};
			si.cb = sizeof(si);
			unsigned long ecode = 0;
			if (!CreateProcess(NULL, (LPTSTR)(LPCTSTR)strExecCommand, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
			{
				SetLastError(NO_ERROR);
				//Retry
				if (!CreateProcess(strCommand, (LPTSTR)(LPCTSTR)strParam, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
				{
					SetLastError(NO_ERROR);
				}
			}
#ifdef _WIN64
			//起動まで確認する。
			int iCntLimit = 0;
			CString strFndWndChk;
			strFndWndChk = _T("CFiler:");
			strFndWndChk += m_FrmWndClassName;
			while (WaitForSingleObject(pi.hProcess, 1000) == WAIT_TIMEOUT)
			{
				this->PumpMessage(); // 定期的にメッセージキューを消化させます
				//既に起動しているか？
				HWND hWndCap = FindWindow(strFndWndChk, NULL);
				//起動している。
				if (hWndCap != NULL)
				{
					if (pi.hThread)
					{
						CloseHandle(pi.hThread);
						pi.hThread = 0;
					}
					if (pi.hProcess)
					{
						CloseHandle(pi.hProcess);
						pi.hProcess = 0;
					}
					return;
				}
				iCntLimit++;
				if (iCntLimit >= 10)
				{
					if (pi.hThread)
					{
						CloseHandle(pi.hThread);
						pi.hThread = 0;
					}
					if (pi.hProcess)
					{
						CloseHandle(pi.hProcess);
						pi.hProcess = 0;
					}
					return;
				}
			}
#endif
			if (pi.hThread)
			{
				CloseHandle(pi.hThread);
				pi.hThread = 0;
			}
			if (pi.hProcess)
			{
				CloseHandle(pi.hProcess);
				pi.hProcess = 0;
			}
		}
		else
		{
			CString strExecCommand;
			CString strCommand;
			CString strParam;
			CString strOpenPath;
			if (lpOpenPath)
			{
				strOpenPath = lpOpenPath;
				// https://github.com/ThinBridge/Chronos/pull/300
				//
				// `ChFiler.exe "B:\" /ChronosConfig="path/to/config"`といったコマンドラインが作成された場合、`\"`がエスケープされ
				// ChFiler.exeは`B:\" /ChronosConfig="path/to/config`という一つの引数を受け取る。先頭の`"`は文字列開始の`"`として
				// 解釈され、対応する`"`がないので消えている。本来は`B:\`と`/ChronosConfig="path/to/config`という二つの引数となるべきである。
				// これは、`\"`とならなければ回避可能である。そのため、フォルダーの末尾の`\`が存在すれば削除し、`\"`とならないようにしている。
				// （なお、`\\"`としても回避可能ではある）
				strOpenPath.TrimRight(_T("\\"));
				strCommand.Format(_T("\"%sChFiler.exe\""), (LPCTSTR)m_strExeFolderPath);
				LPCTSTR mode = initMode == CHFILER_INIT_MODE::OPEN ? _T("") : _T("/Transfer");
				strParam.Format(_T("%s %s \"%s\""), mode, (LPCTSTR)m_strConfigParam, (LPCTSTR)strOpenPath);
				strExecCommand.Format(_T("\"%sChFiler.exe\" %s"), (LPCTSTR)m_strExeFolderPath, (LPCTSTR)strParam);
			}
			else
			{
				strCommand.Format(_T("\"%sChFiler.exe\""), (LPCTSTR)m_strExeFolderPath);
				strParam = m_strConfigParam;
				strExecCommand.Format(_T("\"%sChFiler.exe\" %s"), (LPCTSTR)m_strExeFolderPath, (LPCTSTR)strParam);
			}
			CString strFrmWnd;
			strFrmWnd = _T("CFiler:");
			strFrmWnd += m_FrmWndClassName;
			//既に起動しているか？
			HWND hWndCap = FindWindow(strFrmWnd, NULL); //APのハンドル取得
								    //起動している。
			if (hWndCap != NULL)
			{
				TCHAR szTitleMultipleInstance[260] = {0};
				::GetWindowText(hWndCap, szTitleMultipleInstance, 259);
				CString strTitleMultiple;
				strTitleMultiple = szTitleMultipleInstance;
				//window名を確認する。
				if (strTitleMultiple == strFrmWnd)
				{
					ATOM nAtom = {0};
					if (strParam.IsEmpty())
					{
						LRESULT lr = SendMessageTimeout(hWndCap, WM_USER + 58, (WPARAM)0, static_cast<LPARAM>(CHFILER_INIT_MODE::OPEN), SMTO_BLOCK, 3 * 1000, NULL);
					}
					else
					{
						nAtom = ::GlobalAddAtom(strOpenPath);
						LRESULT lr = SendMessageTimeout(hWndCap, WM_USER + 58, (WPARAM)nAtom, static_cast<LPARAM>(initMode), SMTO_BLOCK, 3 * 1000, NULL);
					}
					return;
				}
			}
			else
			{
				STARTUPINFO si = {0};
				PROCESS_INFORMATION pi = {0};
				si.cb = sizeof(si);
				unsigned long ecode = 0;
				if (!CreateProcess(NULL, (LPTSTR)(LPCTSTR)strExecCommand, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
				{
					SetLastError(NO_ERROR);
					//Retry
					if (!CreateProcess(strCommand, (LPTSTR)(LPCTSTR)strParam, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
					{
						SetLastError(NO_ERROR);
					}
				}
				if (pi.hThread)
				{
					CloseHandle(pi.hThread);
					pi.hThread = 0;
				}
				if (pi.hProcess)
				{
					CloseHandle(pi.hProcess);
					pi.hProcess = 0;
				}
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}
void CSazabi::OpenChTaskMgr()
{
	PROC_TIME(OpenChTaskMgr)
	DWORD tSG_WM_NEWINSTANCE = WM_USER + 58;

	TCHAR FrmWndClassName[256] = {0};
	CString strFndWndChk;
	strFndWndChk = _T("ChTaskMGR:");
	strFndWndChk += m_FrmWndClassName;
	StringCchCopy(FrmWndClassName, _countof(FrmWndClassName), strFndWndChk);

	HWND hWndCap = ::FindWindow(FrmWndClassName, NULL); //APのハンドル取得
	//起動している。
	if (hWndCap != NULL)
	{
		TCHAR szTitleMultipleInstance[260] = {0};
		::GetWindowText(hWndCap, szTitleMultipleInstance, 255);
		CString strTitleMultiple;
		strTitleMultiple = szTitleMultipleInstance;
		//window名を確認する。
		if (strTitleMultiple == FrmWndClassName)
		{
			//起動している
			ATOM nAtom = {0};
			LRESULT lr = SendMessageTimeout(hWndCap, tSG_WM_NEWINSTANCE, (WPARAM)0, 0, SMTO_NORMAL, 5 * 1000, NULL);
			return;
		}
	}

	CString strtSGPathResult;
	CString strtSGPathFolder;

	strtSGPathFolder = theApp.m_strExeFolderPath;
	strtSGPathFolder.TrimRight('\\');
	strtSGPathFolder += _T("\\");

	CStringArray szTF_ExeName;
	szTF_ExeName.Add(_T("ChTaskMgr.exe"));

	CString strTempSG;
	for (int i = 0; i < szTF_ExeName.GetCount(); i++)
	{
		strTempSG = strtSGPathFolder;
		strTempSG += szTF_ExeName.GetAt(i);
		if (::PathFileExists(strTempSG))
		{
			strtSGPathResult = strTempSG;
			break;
		}
	}
	if (strtSGPathResult.IsEmpty())
	{
		return;
	}
	CString strCommand;
	CString strParam;
	strParam.Format(_T("/RESIDENT %s"), (LPCTSTR)m_strConfigParam);
	strCommand.Format(_T("\"%s\" %s"), (LPCTSTR)strtSGPathResult, (LPCTSTR)strParam);
	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi = {0};
	si.cb = sizeof(si);
	unsigned long ecode = 0;
	if (!CreateProcess(NULL, (LPTSTR)(LPCTSTR)strCommand, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
	{
		SetLastError(NO_ERROR);
		//Retry
		if (!CreateProcess(strtSGPathResult, (LPTSTR)(LPCTSTR)strParam, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
		{
			SetLastError(NO_ERROR);
		}
	}
	if (pi.hThread)
	{
		CloseHandle(pi.hThread); // スレッドのハンドルは使わないのですぐ破棄
		pi.hThread = 0;
	}
	if (pi.hProcess)
	{
		CloseHandle(pi.hProcess); // もうプロセスのハンドルは使わないので破棄
		pi.hProcess = 0;
	}
}
void CSazabi::InitLogWrite()
{
	PROC_TIME(InitLogWrite)

	CString logmsg;
	//Debug用のLogファイルのバックアップ5世代管理
	TraceLogBackup();
	WriteDebugTraceDateTime(_T("===================================================================================================="), DEBUG_LOG_TYPE_GE);
	WriteDebugTraceDateTime(_T("InitInstance"), DEBUG_LOG_TYPE_GE);
	WriteDebugTraceDateTime(m_strThisAppVersionString, DEBUG_LOG_TYPE_GE);
	logmsg.Format(_T("ThisAppName:%s"), (LPCTSTR)m_strThisAppName);
	WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_GE);
	logmsg.Format(_T("ExeName:%s"), (LPCTSTR)m_strExeFileName);
	WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_GE);

#ifndef _WIN64
	WriteDebugTraceDateTime(_T("WIN32 App"), DEBUG_LOG_TYPE_GE);
#else  //WIN64
	WriteDebugTraceDateTime(_T("WIN64 App"), DEBUG_LOG_TYPE_GE);
#endif //WIN64

	logmsg.Format(_T("OS:%s"), (LPCTSTR)GetOSInfo());
	WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_GE);

	logmsg.Format(_T("Kernel:%s"), (LPCTSTR)GetOSKernelVersion());
	WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_GE);

	logmsg.Format(_T("%s"), (LPCTSTR)GetCefVersionStr());
	WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_GE);

	logmsg.Format(_T("%s"), (LPCTSTR)GetChromiumVersionStr());
	WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_GE);

	if (InVirtualEnvironment() == VE_THINAPP)
	{
		logmsg.Format(_T("VOSInfo:%s"), (LPCTSTR)GetVOSInfo());
		WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_GE);
	}
	else if (InVirtualEnvironment() == VE_TURBO)
	{
		logmsg.Format(_T("TurboVMInfo:%s"), (LPCTSTR)GetTurboVMInfo());
		WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_GE);
	}
	//設定値を出力
	WriteDebugTraceDateTime(m_AppSettings.ExportTxt(), DEBUG_LOG_TYPE_DE);
}

UINT CSazabi::GetWindowCount()
{
	UINT iRet = 0;
	if (IsWnd(m_pMainWnd))
	{
		iRet = ((CMainFrame*)m_pMainWnd)->GetWindowCount();
	}
	return iRet;
}

void CSazabi::SetThisAppVersionString()
{
	TCHAR path[MAX_PATH] = {0};
	DWORD handle = 0;
	::GetModuleFileName(AfxGetInstanceHandle(), path, MAX_PATH);
	DWORD size = ::GetFileVersionInfoSize(path, &handle);
	CByteArray buf;
	buf.SetSize(size);
	::GetFileVersionInfo(path, 0, (DWORD)buf.GetSize(), buf.GetData());
	LPVOID item = {0};
	UINT itemsize = 0;
	::VerQueryValue(buf.GetData(), _T("\\VarFileInfo\\Translation"), &item, &itemsize);

	CString verstr;
	verstr.Format(_T("\\StringFileInfo\\%04X%04X\\"), LOWORD(*((LPDWORD)item)), HIWORD(*((LPDWORD)item)));
	::VerQueryValue(buf.GetData(), (LPTSTR)(LPCTSTR)(verstr + _T("FileVersion")), &item, &itemsize);
	m_strThisAppVersionString = (LPTSTR)item;
	m_strThisAppVersionString.Replace(_T(" "), _T(""));
}
void CSazabi::UnInitializeObjects()
{
	PROC_TIME(UnInitializeObjects)
	CMFCVisualManager::DestroyInstance();
	CKeyboardManager::CleanUp();
	if (m_pSettingDlg)
	{
		delete m_pSettingDlg;
		m_pSettingDlg = NULL;
	}

	if (m_pDebugDlg)
	{
		delete m_pDebugDlg;
		m_pDebugDlg = NULL;
	}
	if (m_hEventLog)
	{
		SetEvent(m_hEventLog);
		CloseHandle(m_hEventLog);
		m_hEventLog = NULL;
	}
	if (m_hEventLogScript)
	{
		SetEvent(m_hEventLogScript);
		CloseHandle(m_hEventLogScript);
		m_hEventLogScript = NULL;
	}
	if (m_hEventRecovery)
	{
		SetEvent(m_hEventRecovery);
		CloseHandle(m_hEventRecovery);
		m_hEventRecovery = NULL;
	}
	if (m_hEventURLFilterAllow)
	{
		SetEvent(m_hEventURLFilterAllow);
		CloseHandle(m_hEventURLFilterAllow);
		m_hEventURLFilterAllow = NULL;
	}
	if (m_hEventURLFilterDeny)
	{
		SetEvent(m_hEventURLFilterDeny);
		CloseHandle(m_hEventURLFilterDeny);
		m_hEventURLFilterDeny = NULL;
	}
	if (m_pScHost)
	{
		delete m_pScHost;
		m_pScHost = NULL;
	}
	if (m_wndpClose)
	{
		delete m_wndpClose;
		m_wndpClose = NULL;
	}
	if (m_wndpNewTab)
	{
		delete m_wndpNewTab;
		m_wndpNewTab = NULL;
	}
	if (m_wndpActiveTabLine)
	{
		delete m_wndpActiveTabLine;
		m_wndpActiveTabLine = NULL;
	}
	if (m_pLogDisp)
	{
		m_pLogDisp->m_bStop = TRUE;
	}
}
int CSazabi::ExitInstance()
{
	PROC_TIME(ExitInstance)
	// 以下の理由からここでは排他制御をおこなう
	// * CloseAllで複数のプロセスでこの部分を通る可能性がある
	// * InitInstanceでのInitMultipleInstanceと本メソッドのIsExistsAnotherInstanceが
	//   並列実行されると、他のインスタンスの存在が正しく判定されない可能性がある。
	HANDLE hMutex = {0};
	DWORD dwWaitResult = WAIT_FAILED;
	CHRONOS_ENTER_CRITICAL_SECTION(hMutex, dwWaitResult, 30 * 1000);
	if (!hMutex)
	{
		WriteDebugTraceDateTime(_T("ExitInstance: Failed to CreateMutex"), DEBUG_LOG_TYPE_GE);
	}
	if (hMutex && dwWaitResult != WAIT_OBJECT_0)
	{
		CString logmsg;
		logmsg.Format(_T("ExitInstance: Failed to acquire mutex: %lx"), dwWaitResult);
		WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_GE);
	}
	PROC_TIME_S(ExitInstance_p1)

	WriteDebugTraceDateTime(_T("----------------------------------------------------------------------------------------------------"), DEBUG_LOG_TYPE_GE);

	//Object開放
	UnInitializeObjects();

	//RecoveryFileを削除する。
	if (!m_strRecoveryFileFullPath.IsEmpty())
	{
		if (!m_bAbortFlg)
		{
			DeleteFileFix(m_strRecoveryFileFullPath);
		}
	}

	PROC_TIME_E(ExitInstance_p1)

	PROC_TIME_S(ExitInstance_p2)
	if (m_bUseApp)
	{
		//強制キャッシュクリア
		if (m_bEnforceDeleteCache)
		{
			ProgressDlg DlgMsgP(NULL);
			DlgMsgP.Create(MAKEINTRESOURCE(IDD_DLG_PROGRESS), NULL);
			DlgMsgP.ShowWindow(SW_SHOW);
			CString statusMsg;
			statusMsg.LoadString(IDS_STRING_CLEARING_CACHE);
			CString strDlgMsg;
			strDlgMsg.Format(statusMsg);
			DlgMsgP.SetMsg(strDlgMsg);

			this->UnInitializeCef();
			this->DeleteCEFCache();
			ExecNewInstance(_T(""));
			m_bEnforceDeleteCache = FALSE;
		}
		else
		{
			//他のプロセスがいない、最後の処理
			if (!IsExistsAnotherInstance())
			{
				DeleteDirectoryTempFolder(m_strDBL_EXE_FolderPath);

				if (InVirtualEnvironment() != VE_NA && this->IsSGMode())
				{
					SetLastError(NO_ERROR);

					if (::GetLastError() != ERROR_ALREADY_EXISTS)
					{
						CString confirmMsg;
						confirmMsg.LoadString(IDS_STRING_CONFIRM_CLOSE_APPLICATION);
						CString strMsg;
						strMsg.Format(confirmMsg, m_strThisAppName);
						int iRt = 0;
						//シャットダウン処理中は、メッセージボックスを表示しない。
						if (m_bShutdownFlg)
						{
							iRt = IDYES;
						}
						else
						{
							//settings.external_message_pump = trueを指定した状態では、Exit処理中にはMB_DEFAULT_DESKTOP_ONLYがないとダイアログが表示されない
							iRt = ::MessageBox(NULL, strMsg, m_strThisAppName, MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL | MB_DEFAULT_DESKTOP_ONLY);
						}

						if (iRt == IDCANCEL || iRt == IDNO || iRt == IDTIMEOUT)
						{
							ExecNewInstance(_T(""));
						}
						else
						{
							if (InVirtualEnvironment() == VE_THINAPP)
								CloseVOSProcessOther();

							this->UnInitializeCef();
							this->DeleteCEFCacheAll();
						}
					}
				}
				else
				{
					this->UnInitializeCef();
					this->DeleteCEFCacheAll();
				}
				//ゾンビプロセス化を防ぐ。
				ExitKillZombieProcess();
			}
		}
	}
	PROC_TIME_E(ExitInstance_p2)
	if (m_pLogDisp)
	{
		delete m_pLogDisp;
		m_pLogDisp = NULL;
	}

	BOOL bRet = FALSE;
	PROC_TIME_S(ExitInstance_p3)
	this->UnInitializeCef();
	AfxOleTerm(FALSE);
	bRet = CWinApp::ExitInstance();
	PROC_TIME_E(ExitInstance_p3)
	CHRONOS_LEAVE_CRITICAL_SECTION(hMutex, dwWaitResult);
	return bRet;
}

int CSazabi::GetOSVersion()
{
	PROC_TIME(GetOSVersion)

	//毎回取得するのは、効率が悪いのでココで入れる。
	if (m_iWinOSVersion == 0)
	{
		//ひとまず、Win10を入れておく。
		m_iWinOSVersion = 100;
		DWORD dwDummy = 0;
		DWORD dwSize = 0;
		DWORD dwMajor = 0;
		DWORD dwMinor = 0;
		DWORD dwBuild = 0;
		DWORD dwPrivate = 0;
		dwSize = ::GetFileVersionInfoSize(_T("KERNEL32.DLL"), &dwDummy);
		if (dwSize > 0)
		{
			PBYTE pData = new BYTE[dwSize];
			memset(pData, 0x00, dwSize);
			if (::GetFileVersionInfo(_T("KERNEL32.DLL"), 0, dwSize, pData))
			{
				VS_FIXEDFILEINFO* pFileInfo = {0};
				UINT nLen = 0;
				if (::VerQueryValue(pData, _T("\\"), (PVOID*)&pFileInfo, &nLen))
				{
					dwMajor = HIWORD(pFileInfo->dwFileVersionMS);
					dwMinor = LOWORD(pFileInfo->dwFileVersionMS);
					dwBuild = HIWORD(pFileInfo->dwFileVersionLS);
					dwPrivate = LOWORD(pFileInfo->dwFileVersionLS);
					m_iWinOSBuildVersion = dwBuild;
					m_iWinOSVersion = dwMajor * 10; //メジャーを10倍
					m_iWinOSVersion += dwMinor;
					//想定
					//WinXP 5.1->51
					//WinVista 6.0->60
					//Win7  6.1->61
					//Win8  6.2->62
					//Win8.1 6.3->63
					//Win10 10.0->100
					//WinServer2008 6.0
					//WinServer2008R2 6.1
					//WinServer2012 6.2
					//WinServer2012R2 6.3
					//WinServer2016 10.0
				}
			}
			delete[] pData;
		}
	}
	ASSERT(m_iWinOSVersion);
	return m_iWinOSVersion;
}

CString CSazabi::GetThinAppEntryPointFolderPath()
{
	CString strPath;
	TCHAR szTargetPath[512] = {0};
	if (GetEnvironmentVariable(_T("TS_ORIGIN"), szTargetPath, 512))
	{
		PathCchRemoveFileSpec(szTargetPath, 512);
		PathCchAddBackslash(szTargetPath, 512);
		strPath = szTargetPath;
	}
	return strPath;
}

CString CSazabi::GetThinAppEntryPointPath()
{
	PROC_TIME(GetThinAppEntryPointPath)

	CString strPath;
	CString strSubKey;
	strSubKey = _T("Software\\Thinstall\\StubProcessList");
	HKEY hKey = {0};
	LONG lResult = 0L;
	DWORD dwType = 0;
	DWORD dwCount = 0;
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey,
			       0, KEY_READ, &hKey);
	if (lResult == ERROR_SUCCESS)
	{
		DWORD dwValueNameSize = 256;
		TCHAR szValueName[256] = {0};
		BYTE lpData[1024] = {0};
		DWORD dwDataSize = 1024;
		int iRet = 0;
		for (int i = 0; i < 10; i++)
		{
			dwValueNameSize = sizeof(szValueName) / sizeof(szValueName[0]);
			dwDataSize = sizeof(lpData) / sizeof(lpData[0]);
			iRet = RegEnumValue(hKey, i, szValueName, &dwValueNameSize, NULL, &dwType, lpData, &dwDataSize);
			CString strValue((LPCTSTR)lpData);
			CString strValueName(szValueName);
			if (iRet == ERROR_NO_MORE_ITEMS)
				break;
			if (iRet == ERROR_SUCCESS)
			{
				if (!strValue.IsEmpty())
				{
					strPath = strValue;
					break;
				}
			}
		}
		RegCloseKey(hKey);
	}
	//TS_ORIGINから。
	if (strPath.IsEmpty())
	{
		TCHAR szTargetPath[512] = {0};
		if (GetEnvironmentVariable(_T("TS_ORIGIN"), szTargetPath, 512))
		{
			strPath = szTargetPath;
			if (strPath.Find(_T(".dat")) > 0)
			{
				strPath = _T("");
			}
		}
	}
	return strPath;
}

CString CSazabi::GetVOSProcessString(BOOL bCurrent, DWORD* pdwCnt, BOOL bNeedCmdLine)
{
	PROC_TIME(GetVOSProcessString)

	CString strRet;
	CString strTemp;
	if (InVirtualEnvironment() != VE_THINAPP)
		return strRet;

	CString strSubKey;
	strSubKey = _T("Software\\Thinstall\\ProcessList");
	CString strVal;
	HKEY hKey = {0};
	LONG lResult = 0L;
	DWORD dwType = 0;
	DWORD dwCount = 0;
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey,
			       0, KEY_READ, &hKey);
	DWORD pidCurrent = GetCurrentProcessId();
	if (lResult == ERROR_SUCCESS)
	{
		DWORD dwValueNameSize = 256;
		TCHAR szValueName[256] = {0};
		BYTE lpData[1024] = {0};
		DWORD dwDataSize = 1024;
		int iRet = 0;
		for (int i = 0; i < 1024; i++)
		{
			dwValueNameSize = sizeof(szValueName) / sizeof(szValueName[0]);
			dwDataSize = sizeof(lpData) / sizeof(lpData[0]);
			iRet = RegEnumValue(hKey, i, szValueName, &dwValueNameSize, NULL, &dwType, lpData, &dwDataSize);
			CString strValue((LPCTSTR)lpData);
			CString strValueName(szValueName);
			if (iRet == ERROR_NO_MORE_ITEMS)
				break;
			if (iRet == ERROR_SUCCESS)
			{
				DWORD pid = 0;
				pid = _ttoi(strValueName);
				if (IsProcessExists(pid))
				{
					CString strCommandLine;
					if (bNeedCmdLine)
						strCommandLine = SBUtil::GetCommandLineData(pid);
					else
					{
						//EXEのパス、コマンドラインは非表示、EXE名のみ
						strValue = PathFindFileNameW(strValue);
						strCommandLine = _T("");
					}

					if (pidCurrent == pid)
					{
						//自身のプロセスも含める
						if (bCurrent)
						{
							strTemp.Format(_T("PID[*]:%s \"%s\" %s\r\n"), (LPCTSTR)strValueName, (LPCTSTR)strValue, (LPCTSTR)strCommandLine);
							strRet += strTemp;
						}
					}
					else
					{
						strTemp.Format(_T("PID:%s \"%s\" %s\r\n"), (LPCTSTR)strValueName, (LPCTSTR)strValue, (LPCTSTR)strCommandLine);
						strRet += strTemp;
					}
					dwCount++;
				}
			}
		}
		RegCloseKey(hKey);
	}
	if (pdwCnt)
	{
		*pdwCnt = dwCount;
	}
	return strRet;
}

#define WM_TSG_CLOSE WM_USER + 60
void CSazabi::CloseVOSProcessOther()
{
	PROC_TIME(CloseVOSProcessOther)

	BOOL bThinFilerExecFlg = FALSE;
	CString strRet;
	CString strSubKey;
	strSubKey = _T("Software\\Thinstall\\ProcessList");
	CString strVal;
	HKEY hKey = {0};
	LONG lResult = 0L;
	DWORD dwType = 0;
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_READ, &hKey);
	DWORD pidCurrent = GetCurrentProcessId();
	if (lResult != ERROR_SUCCESS)
		return;
	CString strThinFilerPath;

	strThinFilerPath.Format(_T("%sChFiler.exe"), (LPCTSTR)m_strExeFolderPath);

	DWORD dwValueNameSize = 256;
	TCHAR szValueName[256] = {0};
	BYTE lpData[1024] = {0};
	DWORD dwDataSize = 1024;
	int iRet = 0;
	for (int i = 0; i < 1024; i++)
	{
		dwValueNameSize = sizeof(szValueName) / sizeof(szValueName[0]);
		dwDataSize = sizeof(lpData) / sizeof(lpData[0]);
		iRet = RegEnumValue(hKey, i, szValueName, &dwValueNameSize, NULL, &dwType, lpData, &dwDataSize);
		CString strValue((LPCTSTR)lpData);
		CString strValueName(szValueName);

		if (iRet == ERROR_NO_MORE_ITEMS)
			break;
		if (iRet == ERROR_SUCCESS)
		{
			DWORD pid = 0;
			pid = _ttoi(strValueName);
			//自身のpidの場合はスキップ
			if (pidCurrent == pid)
				continue;
			HWND hWndTop = {0};
			hWndTop = GetTopWindowHandle(pid);
			//filemanagerは特別な対応
			if (strValue.CompareNoCase(strThinFilerPath) == 0)
			{
				CString strFWM;
				strFWM.Format(_T("CFiler:%s"), m_FrmWndClassName);
				HWND hWndCap = FindWindow(strFWM, NULL); //APのハンドル取得
				if (hWndCap)
				{
					//起動している
					//ATOM nAtom = { 0 };
					LRESULT lr = SendMessageTimeout(hWndCap, WM_TSG_CLOSE, (WPARAM)0, 0, SMTO_NORMAL, 5 * 1000, NULL);
					bThinFilerExecFlg = TRUE;
				}
				continue;
			}
			if (hWndTop != NULL)
			{
				//手続きどおりに、閉じるお願いをする。
				::PostMessage(hWndTop, WM_CLOSE, 0, 0);
				::Sleep(100);
				for (int j = 0; j < 5; j++)
				{
					//終了された。
					if (!IsProcessExists(pid))
						break;

					HWND hWndTop2 = {0};
					hWndTop2 = GetTopWindowHandle(pid);
					//終了された。
					if (hWndTop2 == NULL)
						break;

					//もう一度お願いする。
					::PostMessage(hWndTop2, WM_CLOSE, 0, 0);
					::Sleep(300);
				}
			}
			//終了されていない。
			if (IsProcessExists(pid))
			{
				::Sleep(300);
				HANDLE processHandle = {0};
				processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
				//それでも閉じない場合は、強制終了。
				::TerminateProcess(processHandle, 0);
				if (processHandle)
					CloseHandle(processHandle);
			}
		}
	}
	RegCloseKey(hKey);
	if (!bThinFilerExecFlg)
	{
		OpenChFiler(CHFILER_INIT_MODE::EXIT_CHECK, NULL);
	}
}

void CSazabi::CloseVOSProcessReadReg(BOOL bForce)
{
	PROC_TIME(CloseVOSProcessReadReg)

	CString strRet;
	CString strSubKey;
	strSubKey = _T("Software\\Thinstall\\ProcessList");
	CString strVal;
	HKEY hKey = {0};
	LONG lResult = 0L;
	DWORD dwType = 0;
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey,
			       0, KEY_READ, &hKey);
	DWORD pidCurrent = GetCurrentProcessId();
	if (lResult == ERROR_SUCCESS)
	{
		DWORD dwValueNameSize = 256;
		TCHAR szValueName[256] = {0};
		BYTE lpData[1024] = {0};
		DWORD dwDataSize = 1024;
		int iRet = 0;
		for (int i = 0; i < 1024; i++)
		{
			dwValueNameSize = sizeof(szValueName) / sizeof(szValueName[0]);
			dwDataSize = sizeof(lpData) / sizeof(lpData[0]);
			iRet = RegEnumValue(hKey, i, szValueName, &dwValueNameSize, NULL, &dwType, lpData, &dwDataSize);
			CString strValue((LPCTSTR)lpData);
			CString strValueName(szValueName);

			if (iRet == ERROR_NO_MORE_ITEMS)
				break;
			if (iRet == ERROR_SUCCESS)
			{
				DWORD pid = 0;
				pid = _ttoi(strValueName);
				HWND hWndTop = {0};
				hWndTop = GetTopWindowHandle(pid);
				HANDLE processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
				if (hWndTop != NULL)
				{
					if (pidCurrent != pid)
					{
						if (!bForce)
						{
							::PostMessage(hWndTop, WM_CLOSE, 0, 0);
							::Sleep(100);
							for (int j = 0; j < 50; j++)
							{
								HWND hWndTop2 = {0};
								hWndTop2 = GetTopWindowHandle(pid);
								if (hWndTop2 == NULL)
									break;
								::PostMessage(hWndTop2, WM_CLOSE, 0, 0);
								::Sleep(100);
								if (SafeTerminateProcess(processHandle, 0))
									::TerminateProcess(processHandle, 0);
								break;
							}
						}
						else
						{
							if (SafeTerminateProcess(processHandle, 0))
								::TerminateProcess(processHandle, 0);
							::Sleep(100);
						}
					}
				}
				else
				{
					if (pidCurrent != pid)
					{
						if (IsProcessExists(pid))
						{
							if (SafeTerminateProcess(processHandle, 0))
								::TerminateProcess(processHandle, 0);

							::Sleep(100);
						}
					}
				}
				if (processHandle)
					CloseHandle(processHandle);
			}
		}
		RegCloseKey(hKey);
	}
}

BOOL CSazabi::CloseVOSProc()
{
	if (InVirtualEnvironment() != VE_THINAPP) return FALSE;
	CString strString;
	DWORD dwVOSPCount = 0;
	strString = theApp.GetVOSProcessString(FALSE, &dwVOSPCount, FALSE);

	strString.TrimLeft();
	strString.TrimRight();
	strString.Replace(_T("\r\n"), _T("\n"));
	if (!strString.IsEmpty())
	{
		CString confirmMsg;
		confirmMsg.LoadString(IDS_STRING_CONFIRM_CLOSE_VOS_PROCESS);
		CString strMsg;
		strMsg.Format(_T("%s\n%s"), (LPCTSTR)confirmMsg, (LPCTSTR)strString);
		int iRet = AfxMessageBox(strMsg, MB_ICONQUESTION | MB_YESNO);
		if (iRet != IDYES)
			return FALSE;
		CloseVOSProcessReadReg(FALSE);
		strString = theApp.GetVOSProcessString(FALSE, &dwVOSPCount, FALSE);
		strString.TrimLeft();
		strString.TrimRight();
		if (!strString.IsEmpty())
		{
			CloseVOSProcessReadReg(TRUE);
		}
	}
	return TRUE;
}
BOOL CSazabi::IsProcessExists(DWORD dPID)
{
	PROC_TIME(IsProcessExists)

	BOOL bRet = FALSE;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot)
	{
		PROCESSENTRY32 pe32 = {0};
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapshot, &pe32))
		{
			do
			{
				if (pe32.th32ProcessID == dPID)
				{
					bRet = TRUE;
					break;
				}
			} while (Process32Next(hSnapshot, &pe32));
		}
		CloseHandle(hSnapshot);
	}
	return bRet;
}
CString CSazabi::IsProcessExistsName(DWORD dPID)
{
	PROC_TIME(IsProcessExistsName)

	BOOL bRet = FALSE;
	CString strRet;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot)
	{
		PROCESSENTRY32 pe32 = {0};
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapshot, &pe32))
		{
			do
			{
				if (pe32.th32ProcessID == dPID)
				{
					strRet = pe32.szExeFile;
					bRet = TRUE;
					break;
				}
			} while (Process32Next(hSnapshot, &pe32));
		}
		CloseHandle(hSnapshot);
	}
	return strRet;
}

HWND CSazabi::GetTopWindowHandle(const DWORD TargetID)
{
	HWND hWnd = GetTopWindow(NULL);
	do
	{
#ifdef _WIN64
		if (GetWindowLong(hWnd, GWLP_HWNDPARENT) != 0 || !theApp.IsWndVisible(hWnd))
#else
		if (GetWindowLong(hWnd, GWL_HWNDPARENT) != 0 || !theApp.IsWndVisible(hWnd))
#endif /* _WIN64 */
		{
			continue;
		}
		DWORD ProcessID = 0;
		GetWindowThreadProcessId(hWnd, &ProcessID);
		if (TargetID == ProcessID)
			return hWnd;
	} while ((hWnd = GetNextWindow(hWnd, GW_HWNDNEXT)) != NULL);

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////
BOOL CSazabi::SafeTerminateProcess(HANDLE hProcess, INT_PTR uExitCode)
{
	PROC_TIME(SafeTerminateProcess)

	DWORD dwTID = 0;
	DWORD dwCode = 0;
	DWORD dwErr = 0;
	HANDLE hProcessDup = INVALID_HANDLE_VALUE;
	HANDLE hRT = NULL;
	HINSTANCE hKernel = GetModuleHandle(_T("Kernel32"));
	if (!hKernel)
		return FALSE;
	BOOL bSuccess = FALSE;
	BOOL bDup = DuplicateHandle(GetCurrentProcess(), hProcess, GetCurrentProcess(), &hProcessDup, PROCESS_ALL_ACCESS, FALSE, 0);
	if (GetExitCodeProcess((bDup) ? hProcessDup : hProcess, &dwCode) && (dwCode == STILL_ACTIVE))
	{
		FARPROC pfnExitProc = {0};
		pfnExitProc = GetProcAddress(hKernel, "ExitProcess");
		hRT = CreateRemoteThread((bDup) ? hProcessDup : hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pfnExitProc, (PVOID)uExitCode, 0, &dwTID);
		if (hRT == NULL)
			dwErr = GetLastError();
	}
	else
	{
		dwErr = ERROR_PROCESS_ABORTED;
	}

	if (hRT)
	{
		DWORD waitRes = 0;
		waitRes = WaitForSingleObject((bDup) ? hProcessDup : hProcess, 5 * 1000);
		if (waitRes == WAIT_TIMEOUT)
		{
			::TerminateProcess((bDup) ? hProcessDup : hProcess, 0);
		}
		CloseHandle(hRT);
		bSuccess = TRUE;
	}
	if (bDup)
		CloseHandle(hProcessDup);
	if (!bSuccess)
		SetLastError(dwErr);
	return bSuccess;
}

void CSazabi::AppendDebugViewLog(const DebugWndLogData& Data)
{
	try
	{
		if (this->m_pDebugDlg)
		{
			if (this->IsWnd(this->m_pDebugDlg))
				this->m_pDebugDlg->SetLogMsg(Data);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

void CSazabi::WriteDebugTraceDateTime(LPCTSTR msg, int iLogType)
{
	if (msg == NULL) return;

	if (!m_AppSettings.IsAdvancedLogMode()) return;
	if (_tcslen(msg) == 0) return;

	CString strWriteLine;
	SYSTEMTIME time;
	GetLocalTime(&time);
	strWriteLine.Format(_T("%u-%02u-%02u %02u:%02u:%02u.%03u\t%s\t%s\n"),
			    time.wYear, time.wMonth, time.wDay,
			    time.wHour, time.wMinute, time.wSecond,
			    time.wMilliseconds, sDEBUG_LOG_TYPE[iLogType], msg);

	_wsetlocale(LC_ALL, _T("jpn"));
	CStdioFile stdFile;
	OutputDebugString(strWriteLine);
	BOOL bFileWriteFlg = FALSE;
	switch (m_AppSettings.GetAdvancedLogLevel())
	{
	//全てのログを出力
	case DEBUG_LOG_LEVEL_OUTPUT_ALL:
	{
		bFileWriteFlg = TRUE;
		break;
	}
	//ファイル書き込み無し。
	case DEBUG_LOG_LEVEL_OUTPUT_NO_FILE:
	{
		bFileWriteFlg = FALSE;
		break;
	}
	//URL関連のみ
	case DEBUG_LOG_LEVEL_OUTPUT_URL:
	{
		//ログの種類が一般、URLの場合
		if (iLogType == DEBUG_LOG_TYPE_GE || iLogType == DEBUG_LOG_TYPE_URL)
		{
			bFileWriteFlg = TRUE;
		}
		break;
	}
	default:
		break;
	}
	if (bFileWriteFlg)
	{
		if (stdFile.Open(m_strLogFileFullPath, CFile::modeWrite | CFile::shareDenyNone | CFile::modeCreate | CFile::modeNoTruncate))
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
	}
	_wsetlocale(LC_ALL, _T(""));
}

void CSazabi::OpenDefaultBrowser(const CString& strURL, DWORD iType, const CString strPath)
{
	PROC_TIME(OpenDefaultBrowser)

	if (SBUtil::IsURL_HTTP(strURL))
	{
		CString strCommand;
		CString logmsg;
		CString strParam;

		CopyDBLEXEToTempEx();
		if (iType == 1)
		{
			strCommand.Format(_T("\"%s\" \"%s\" -IE"), (LPCTSTR)m_strDBL_EXE_FullPath, (LPCTSTR)strURL);
			strParam.Format(_T("\"%s\" -IE"), (LPCTSTR)strURL);
		}
		else if (iType == 2)
		{
			strCommand.Format(_T("\"%s\" \"%s\" -Firefox"), (LPCTSTR)m_strDBL_EXE_FullPath, (LPCTSTR)strURL);
			strParam.Format(_T("\"%s\" -Firefox"), (LPCTSTR)strURL);
		}
		else if (iType == 3)
		{
			strCommand.Format(_T("\"%s\" \"%s\" -Chrome"), (LPCTSTR)m_strDBL_EXE_FullPath, (LPCTSTR)strURL);
			strParam.Format(_T("\"%s\" -Chrome"), (LPCTSTR)strURL);
		}
		else if (iType == 4)
		{
			strCommand.Format(_T("\"%s\" \"%s\" -Edge"), (LPCTSTR)m_strDBL_EXE_FullPath, (LPCTSTR)strURL);
			strParam.Format(_T("\"%s\" -Edge"), (LPCTSTR)strURL);
		}
		else if (iType == 5)
		{
			CString strPathUpper;
			strPathUpper = _T("\\");
			strPathUpper += strPath;
			strPathUpper.MakeUpper();
			if (strPathUpper.Find(_T("\\CMD.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\ARP.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\AT.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\BCDEDIT.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\CACLS.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\CERTREQ.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\CIPHER.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\CMD.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\CONTROL.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\CSCRIPT.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\DLLHOST.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\DOSX.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\EXPLORER.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\FORMAT.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\FSUTIL.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\FTP.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\IPCONFIG.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\ISCSICLI.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\MSCDEXNT.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\MSHTA.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\NBTSTAT.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\NET.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\NET1.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\NETSH.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\NETSTAT.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\NSLOOKUP.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\PING.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\POWERSHELL.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\QPROCESS.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\QUERY.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\REDIR.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\REG.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\REGEDIT.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\REGEDT32.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\ROUTE.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\RUNDLL32.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\SC.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\SCHTASKS.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\SDBINST.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\SUBST.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\SVCHOST.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\SYSPREP.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\SYSTEMINFO.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\TASKKILL.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\TASKLIST.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\TRACERT.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\VER.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\VSSADMIN.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\WHOAMI.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\WINRS.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\WMIC.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\WSCRIPT.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\WUSA.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\COMMAND.COM")) >= 0 ||
			    strPathUpper.Find(_T("\\EVENTVWR.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\PSEXEC.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\CMDKEY.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\MSIEXEC.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\TASKHOSTW.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\TASKHOST.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\BACKGROUNDTASKHOST.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\ATTRIB.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\ICACLS.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\ROBOCOPY.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\XCOPY.EXE")) >= 0 ||
			    strPathUpper.Find(_T("\\QWINSTA.EXE")) >= 0)
			{
				DebugWndLogData dwLogData;
				dwLogData.mHWND.Format(_T("APP_WND:0x%08x"), 0);
				dwLogData.mFUNCTION_NAME = _T("OpenDefaultBrowser");
				dwLogData.mMESSAGE1 = strPath;
				dwLogData.mMESSAGE2 = _T("Prohibited Execution Error");
				dwLogData.mMESSAGE3 = strURL;
				theApp.AppendDebugViewLog(dwLogData);
				logmsg = dwLogData.GetString();
				WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_URL);
				return;
			}

			strCommand.Format(_T("\"%s\" -Custom /Path:\"%s\" \"%s\""), (LPCTSTR)m_strDBL_EXE_FullPath, (LPCTSTR)strPath, (LPCTSTR)strURL);
			strParam.Format(_T("-Custom /Path:\"%s\" \"%s\""), (LPCTSTR)strPath, (LPCTSTR)strURL);

			TCHAR FrmWndClassName[256] = {0};
			CString strFrmWndClass = strPath;
			strFrmWndClass.TrimLeft();
			strFrmWndClass.TrimRight();
			strFrmWndClass.MakeUpper();
			strFrmWndClass.Replace(_T("\\"), _T("-"));
			strFrmWndClass.Replace(_T(":"), _T(""));
			strFrmWndClass.Replace(_T("."), _T(""));
			strFrmWndClass.Replace(_T("--"), _T("-"));
			strFrmWndClass.Replace(_T(" "), _T("_"));
			StringCchCopy(FrmWndClassName, _countof(FrmWndClassName), strFrmWndClass);

			HWND hWndCap = FindWindow(FrmWndClassName, NULL); //APのハンドル取得
			//起動している。
			if (hWndCap != NULL)
			{
				TCHAR szTitleMultipleInstance[260] = {0};
				::GetWindowText(hWndCap, szTitleMultipleInstance, 259);
				CString strTitleMultiple;
				strTitleMultiple = szTitleMultipleInstance;
				//window名を確認する。
				if (strTitleMultiple == FrmWndClassName)
				{
					//起動している
					ATOM nAtom = {0};
					nAtom = ::GlobalAddAtom(strURL);
					::PostMessage(hWndCap, WM_NEWINSTANCE, (WPARAM)nAtom, 0);
					return;
				}
			}
		}
		else
		{
			strCommand.Format(_T("\"%s\" \"%s\""), (LPCTSTR)m_strDBL_EXE_FullPath, (LPCTSTR)strURL);
			strParam.Format(_T("\"%s\""), (LPCTSTR)strURL);
		}
		DebugWndLogData dwLogData;
		dwLogData.mHWND.Format(_T("APP_WND:0x%08x"), 0);
		dwLogData.mFUNCTION_NAME = _T("OpenDefaultBrowser");
		dwLogData.mMESSAGE1 = strURL;
		dwLogData.mMESSAGE2 = strCommand;
		theApp.AppendDebugViewLog(dwLogData);
		logmsg = dwLogData.GetString();
		WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_URL);

		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi = {0};
		si.cb = sizeof(si);
		unsigned long ecode = 0;
		if (!CreateProcess(NULL, (LPTSTR)(LPCTSTR)strCommand, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			logmsg.Format(_T("OpenDefaultBrowser:CreateProcess Retry1 0x%08x"), ::GetLastError());
			WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);
			SetLastError(NO_ERROR);
			//Retry
			if (!CreateProcess(m_strDBL_EXE_FullPath, (LPTSTR)(LPCTSTR)strParam, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
			{
				logmsg.Format(_T("OpenDefaultBrowser:ShellExecute Retry2 0x%08x"), ::GetLastError());
				WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);
				SetLastError(NO_ERROR);
				if (::ShellExecute(NULL, _T("open"), m_strDBL_EXE_FullPath, strParam, NULL, SW_SHOW) <= HINSTANCE(32))
				{
					logmsg.Format(_T("OpenDefaultBrowser:ShellExecute Retry3 0x%08x"), ::GetLastError());
					WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);
					SetLastError(NO_ERROR);
					::ShellExecute(NULL, NULL, strCommand, NULL, NULL, SW_SHOW);
				}
			}
		}
		if (pi.hThread)
		{
			CloseHandle(pi.hThread);
			pi.hThread = 0;
		}
		if (pi.hProcess)
		{
			CloseHandle(pi.hProcess);
			pi.hProcess = 0;
		}
	}
}

void CSazabi::ExecNewInstance(const CString strURL)
{
	PROC_TIME(ExecNewInstance)

	//メモリを開放
	EmptyWorkingSetAll();

	CString strCommand;
	CString strParam;

	if (!strURL.IsEmpty())
	{
		strCommand.Format(_T("\"%s\" \"%s\" -NEW"), (LPCTSTR)m_strExeFullPath, (LPCTSTR)strURL);
		strParam.Format(_T("\"%s\" -NEW"), (LPCTSTR)strURL);
	}
	else
	{
		strCommand.Format(_T("\"%s\" -NEW"), (LPCTSTR)m_strExeFullPath);
		strParam = _T("-NEW");
	}
	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi = {0};
	si.cb = sizeof(si);
	unsigned long ecode = 0;

	if (!CreateProcess(NULL, (LPTSTR)(LPCTSTR)strCommand, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		SetLastError(NO_ERROR);
		//Retry
		if (!CreateProcess(m_strExeFullPath, (LPTSTR)(LPCTSTR)strParam, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			SetLastError(NO_ERROR);
			if (::ShellExecute(NULL, _T("open"), m_strExeFullPath, strParam, NULL, SW_SHOW) <= HINSTANCE(32))
			{
				::ShellExecute(NULL, NULL, strCommand, NULL, NULL, SW_SHOW);
			}
		}
	}
	if (pi.hThread)
	{
		CloseHandle(pi.hThread);
		pi.hThread = 0;
	}
	if (pi.hProcess)
	{
		CloseHandle(pi.hProcess);
		pi.hProcess = 0;
	}
}
void CSazabi::OpenFileExplorer(const CString& strURL)
{
	PROC_TIME(OpenFileExplorer)

	CString strCommand;
	CString strParam;
	CopyDBLEXEToTempEx();
	strCommand.Format(_T("\"%s\" \"%s\""), (LPCTSTR)m_strDBL_EXE_FullPath, (LPCTSTR)strURL);
	strParam.Format(_T("\"%s\""), (LPCTSTR)strURL);
	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi = {0};
	si.cb = sizeof(si);
	unsigned long ecode = 0;
	if (!CreateProcess(NULL, (LPTSTR)(LPCTSTR)strCommand, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		SetLastError(NO_ERROR);
		//Retry
		if (!CreateProcess(m_strDBL_EXE_FullPath, (LPTSTR)(LPCTSTR)strParam, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			SetLastError(NO_ERROR);
			if (::ShellExecute(NULL, _T("open"), m_strDBL_EXE_FullPath, strParam, NULL, SW_SHOW) <= HINSTANCE(32))
			{
				::ShellExecute(NULL, NULL, strCommand, NULL, NULL, SW_SHOW);
			}
		}
	}
	if (pi.hThread)
	{
		CloseHandle(pi.hThread);
		pi.hThread = 0;
	}
	if (pi.hProcess)
	{
		CloseHandle(pi.hProcess);
		pi.hProcess = 0;
	}
}

void CSazabi::CopyDBLEXEToTempInit()
{
	PROC_TIME(CopyDBLEXEToTempInit)
	TCHAR szTemp[MAX_PATH + 1] = {0};
	::GetTempPath(MAX_PATH, szTemp);
	CString strTempPath;
	CString strTempPathDir;

	strTempPath = szTemp;

	CString strTempUpper;
	strTempUpper = strTempPath;
	strTempUpper.MakeUpper();
	if (strTempUpper.Find(_T("\\CHRONOSSG\\")) >= 0)
	{
		strTempPath.TrimRight('\\');
	}
	else
		strTempPath += _T("ChronosSG");

	MakeDirectoryPath(strTempPath);
	strTempPathDir = strTempPath;
	strTempPath += _T("\\DBLC.exe");

	CString strFileName;
	CString strFindFilePath = strTempPathDir;

	//DeleteDirectoryTempFolder(strFindFilePath);

	if (InVirtualEnvironment() == VE_NA)
	{
		m_strDBL_EXE_FullPath = m_strDBL_EXE_Default_FullPath;
	}
	//仮想で実行された場合はTEMPにコピー
	else
	{
		m_strDBL_EXE_FullPath = strTempPath;
		CopyDBLEXEToTempEx();
	}
	m_strDBL_EXE_FolderPath = strTempPathDir;
}

static void LogErrNo(LPCWSTR prefix, int errNum)
{
	wchar_t errBuf[256] = {0};
	_wcserror_s(errBuf, 256, errNum);
	CString strMessage;
	strMessage.Format(L"%ls: %ls", prefix, errBuf);
	theApp.WriteDebugTraceDateTime(strMessage, DEBUG_LOG_TYPE_GE);
}

// 2023-04-07
// Windows 11 22H2において、CopyFile()を使用してDBLC.exeをThinApp内からホストの
// Tempにコピーするとファイルが破損する事象が起こっているため、CopyFile()を独自
// 実装に置き換える。
// 現時点では以下条件でのみ発生すると考えられ、これを満たすのはDBLC.exeのコピー
// 時のみと考えられるため、同処理にのみ適用する。
// * Windows 22H2
//   * Windows 11 21H2以前では発生していない
// * CopyFile()を使用する
// * ThinAppで仮想化されているファイルをコピーする
void CSazabi::CopyDBLEXEToTempEx()
{
	//ネイティブ版は、DBLCは不要
	if (InVirtualEnvironment() == VE_NA)
		return;

	FILE* src = _wfopen(m_strDBL_EXE_Default_FullPath, L"rb");
	if (!src)
	{
		CString strMessage;
		strMessage.Format(L"CopyDBLEXEToTempEx: Failed to open %ls:", (LPCWSTR)m_strDBL_EXE_Default_FullPath);
		LogErrNo(strMessage, errno);
		return;
	}

	FILE* dest = _wfopen(m_strDBL_EXE_FullPath, L"wb");
	if (!dest)
	{
		CString strMessage;
		strMessage.Format(L"CopyDBLEXEToTempEx: Failed to open %ls:", (LPCWSTR)m_strDBL_EXE_FullPath);
		LogErrNo(strMessage, errno);
		fclose(src);
		return;
	}

	do
	{
		char buf[4096];
		int errNum;
		size_t readBytes = fread(buf, 1, sizeof(buf), src);
		errNum = ferror(src);
		if (errNum)
		{
			CString strMessage;
			strMessage.Format(L"CopyDBLEXEToTempEx: Failed to read %ls:", (LPCWSTR)m_strDBL_EXE_Default_FullPath);
			LogErrNo(strMessage, errNum);
			break;
		}

		size_t writeBytes = fwrite(buf, 1, readBytes, dest);
		errNum = ferror(dest);
		if (errNum)
		{
			CString strMessage;
			strMessage.Format(L"CopyDBLEXEToTempEx: Failed to write %ls:", (LPCWSTR)m_strDBL_EXE_FullPath);
			LogErrNo(strMessage, errNum);
			break;
		}
	} while (!feof(src));

	fclose(dest);
	fclose(src);

	SetLastError(NO_ERROR);
}

/*
 * 表示ウィンドウの状態を保存するパスを生成する
 *
 * (e.g. %AppDataLocal%\Temp\2\ChronosSG\CSG_SaveWnd_chronosn.dat)
 */
void CSazabi::SetRecoveryFilePath()
{
	if (m_AppSettings.IsCrashRecovery())
	{
		m_strRecoveryFileFullPath = m_strDBL_EXE_FolderPath;
		DWORD pidCurrent = GetCurrentProcessId();
		m_strRecoveryFileName.Format(_T("CSG_Recovery%d.dat"), pidCurrent);
		if (!m_strRecoveryFileFullPath.IsEmpty())
		{
			m_strRecoveryFileFullPath += _T("\\");
			m_strRecoveryFileFullPath += m_strRecoveryFileName;
		}
	}
	CString strPrx = m_strExeFileName;
	strPrx.MakeLower();
	strPrx.Replace(_T(".exe"), _T(""));

	m_strRestoreFileFullPath.Format(_T("%s\\CSG_SaveWnd_%s.dat"),
					(LPCTSTR)m_strDBL_EXE_FolderPath, (LPCTSTR)strPrx);
}

BOOL CSazabi::DeleteDirectory(LPCTSTR lpPathName, LPCTSTR lpPat)
{
	CFileFind fnd;
	CString strPathName = lpPathName;
	if (strPathName.IsEmpty()) return FALSE;

	strPathName.TrimRight('\\');
	strPathName += _T("\\");
	strPathName += lpPat;

	if (fnd.FindFile(strPathName, 0))
	{
		int i = 1;
		while (i)
		{
			i = fnd.FindNextFile();

			// ファイル名が"."か".."の場合は次を検索
			if (fnd.IsDots())
				continue;

			// 削除するファイル名取得
			CString strDeleteFile = lpPathName;
			strDeleteFile.TrimRight('\\');
			strDeleteFile += _T("\\") + fnd.GetFileName();

			// フォルダだった場合、再帰呼び出しでそのフォルダを削除
			if (fnd.IsDirectory())
			{
				DeleteDirectory(strDeleteFile, lpPat);

				// フォルダの削除
				DWORD dwAttributes = 0;
				SetFileAttributes(strDeleteFile, dwAttributes);
				CStringW strLogFilePath(L"\\\\?\\");
				strLogFilePath += strDeleteFile;
				strLogFilePath.TrimRight('\\');
				strLogFilePath += L"\\";
				::RemoveDirectory(strDeleteFile);
				::RemoveDirectoryW(strLogFilePath);
			}

			// ファイルの削除
			else
			{
				DWORD dwAttributes = 0;
				SetFileAttributes(strDeleteFile, dwAttributes);
				CStringW strLogFilePath(L"\\\\?\\");
				strLogFilePath += strDeleteFile;
				//::DeleteFile(strDeleteFile);
				::DeleteFileW(strLogFilePath);
				SetLastError(NO_ERROR);
				DeleteFileFix(strDeleteFile);
			}
		}
		fnd.Close();
		return TRUE;
	}
	return FALSE;
}

BOOL CSazabi::DeleteDirectoryTempFolder(LPCTSTR lpPathName)
{
	PROC_TIME(DeleteDirectoryTempFolder)

	TCHAR szPat[] = _T("*.*");
	CFileFind fnd;
	CString strPathName = lpPathName;
	if (strPathName.IsEmpty()) return FALSE;

	strPathName.TrimRight('\\');
	strPathName += _T("\\");
	strPathName += szPat;

	if (fnd.FindFile(strPathName, 0))
	{
		int i = 1;
		while (i)
		{
			i = fnd.FindNextFile();

			// ファイル名が"."か".."の場合は次を検索
			if (fnd.IsDots())
				continue;

			// 削除するファイル名取得
			CString strDeleteFile = lpPathName;
			strDeleteFile.TrimRight('\\');
			strDeleteFile += _T("\\") + fnd.GetFileName();

			// フォルダだった場合、再帰呼び出しでそのフォルダを削除
			if (fnd.IsDirectory())
			{
				DeleteDirectory(strDeleteFile, szPat);
				// フォルダの削除
				DWORD dwAttributes = 0;
				SetFileAttributes(strDeleteFile, dwAttributes);
				CStringW strLogFilePath(L"\\\\?\\");
				strLogFilePath += strDeleteFile;
				strLogFilePath.TrimRight('\\');
				strLogFilePath += L"\\";
				::RemoveDirectory(strDeleteFile);
				::RemoveDirectoryW(strLogFilePath);
			}
			// ファイルの削除
			else
			{
				//除外ファイル
				CString strFileName(fnd.GetFileName());
				CString FileExt;
				FileExt = strFileName.Mid(strFileName.ReverseFind('.'));

				DWORD dwAttributes = 0;
				SetFileAttributes(strDeleteFile, dwAttributes);
				CStringW strLogFilePath(L"\\\\?\\");
				strLogFilePath += strDeleteFile;

				//除外ファイル
				if (strFileName.CompareNoCase(_T("DBLC.exe")) == 0)
				{
					continue;
				}
				else if (FileExt.CompareNoCase(_T(".dat")) == 0)
				{
					strFileName.MakeUpper();
					if (strFileName.Find(_T("CSG_RECOVERY")) == 0)
					{
						continue;
					}
					else if (strFileName.Find(_T("CSG_RECOVERY")) == 0)
					{
						continue;
					}
					else if (strFileName.Find(_T("CSG_SAVEWND")) == 0)
					{
						continue;
					}
					else if (strFileName.Find(_T("CSG_SAVEWND")) == 0)
					{
						continue;
					}
					else
					{
						DeleteFileFix(strDeleteFile);
						::DeleteFileW(strLogFilePath);
					}
				}
				else
				{
					DeleteFileFix(strDeleteFile);
					::DeleteFileW(strLogFilePath);
				}
			}
		}
		fnd.Close();
		return TRUE;
	}
	return FALSE;
}

void CSazabi::TraceLogBackup()
{
	if (!m_AppSettings.IsAdvancedLogMode()) return;
	if (m_AppSettings.GetAdvancedLogLevel() == DEBUG_LOG_LEVEL_OUTPUT_NO_FILE)
		return;
	//file backup  起動時にログファイルをローテーションしログファイルによるディスク圧迫を回避
	::CopyFile(m_strLogFileFullPath + ".bak3", m_strLogFileFullPath + ".bak4", FALSE);
	::CopyFile(m_strLogFileFullPath + ".bak2", m_strLogFileFullPath + ".bak3", FALSE);
	::CopyFile(m_strLogFileFullPath + ".bak1", m_strLogFileFullPath + ".bak2", FALSE);
	::CopyFile(m_strLogFileFullPath + ".bak0", m_strLogFileFullPath + ".bak1", FALSE);
	::CopyFile(m_strLogFileFullPath, m_strLogFileFullPath + ".bak0", FALSE);
	DeleteFileFix(m_strLogFileFullPath);
	SetLastError(NO_ERROR);
}

BOOL CSazabi::IsProcOwner(DWORD dwPID)
{
	if (dwPID == 0) return FALSE;

	BOOL bRet = FALSE;
	HANDLE processHandle = {0};
	processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, dwPID);
	if (processHandle)
	{
		bRet = TRUE;
		CloseHandle(processHandle);
	}
	return bRet;
}
typedef struct _VM_COUNTERS_EX
{
	SIZE_T PeakVirtualSize;
	SIZE_T VirtualSize; // note this !!
	ULONG PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
	SIZE_T PrivateUsage;
} VM_COUNTERS_EX;
typedef struct _VM_COUNTERS_EX2
{
	VM_COUNTERS_EX CountersEx;
	SIZE_T PrivateWorkingSetSize;
	ULONGLONG SharedCommitUsage;
} VM_COUNTERS_EX2, *PVM_COUNTERS_EX2;

#include <psapi.h>
typedef NTSTATUS(NTAPI* NtQueryInformationProcessPtr)(
    IN HANDLE ProcessHandle,
    IN int, //PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL);

typedef ULONG(NTAPI* RtlNtStatusToDosErrorPtr)(NTSTATUS Status);

unsigned long long CSazabi::GetMemoryUsageSizeFromPID(DWORD dwPID)
{
	unsigned long long iMem = 0;
	try
	{
		HANDLE processHandle = {0};
		processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, dwPID);
		if (processHandle)
		{
			PROCESS_MEMORY_COUNTERS_EX pmcs = {sizeof(PROCESS_MEMORY_COUNTERS_EX)};
			VM_COUNTERS_EX2 vm = {0};
			// Locating functions
			HINSTANCE hNtDll = GetModuleHandleW(L"ntdll.dll");
			if (!hNtDll)
				throw std::runtime_error("Failed to execute GetModuleHandleW");
			NtQueryInformationProcessPtr NtQueryInformationProcess = (NtQueryInformationProcessPtr)GetProcAddress(hNtDll, "NtQueryInformationProcess");
			RtlNtStatusToDosErrorPtr RtlNtStatusToDosError = (RtlNtStatusToDosErrorPtr)GetProcAddress(hNtDll, "RtlNtStatusToDosError");
			if (!NtQueryInformationProcess || !RtlNtStatusToDosError)
			{
				::GetProcessMemoryInfo(processHandle, (PROCESS_MEMORY_COUNTERS*)&pmcs, sizeof(PROCESS_MEMORY_COUNTERS_EX));
				iMem = pmcs.WorkingSetSize;
				CloseHandle(processHandle);
				return iMem;
			}
			ULONG len = 0;
			NTSTATUS status = NtQueryInformationProcess(
			    processHandle, 3 /* ProcessVmCounters*/, &vm, sizeof(vm), &len);
			SetLastError(RtlNtStatusToDosError(status));
			if (NT_ERROR(status))
			{
				::GetProcessMemoryInfo(processHandle, (PROCESS_MEMORY_COUNTERS*)&pmcs, sizeof(PROCESS_MEMORY_COUNTERS_EX));
				iMem = pmcs.WorkingSetSize;
				CloseHandle(processHandle);
				return iMem;
			}
			iMem = vm.PrivateWorkingSetSize;
			CloseHandle(processHandle);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return iMem;
}
void CSazabi::EmptyWorkingSetSingle(DWORD dwProcessId)
{
	CHandle hProcess;
	hProcess.Attach(::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SET_QUOTA, FALSE, dwProcessId));
	if (hProcess.m_h)
		EmptyWorkingSet(hProcess);
}
void CSazabi::EmptyWorkingSetAll()
{
	try
	{
		DWORD pidCurrent = GetCurrentProcessId();
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot)
		{
			PROCESSENTRY32 pe32 = {0};
			pe32.dwSize = sizeof(PROCESSENTRY32);
			if (Process32First(hSnapshot, &pe32))
			{
				do
				{
					DWORD PID = pe32.th32ProcessID;
					if (IsProcOwner(PID))
					{
						//自分
						if (PID == pidCurrent)
						{
							EmptyWorkingSetSingle(PID);
						}
						else
						{
							DWORD ParentPID = pe32.th32ParentProcessID;
							CString strStrDataTemp = pe32.szExeFile;
							strStrDataTemp.MakeLower();
							//親が一緒
							if (ParentPID == pidCurrent)
							{
								if (strStrDataTemp.Find(_T("chronos")) >= 0)
									EmptyWorkingSetSingle(PID);
							}
						}
					}
				} while (Process32Next(hSnapshot, &pe32));
			}
			CloseHandle(hSnapshot);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return;
}

unsigned long long CSazabi::GetMemoryUsageSize()
{
	PROC_TIME(GetMemoryUsageSize)

	unsigned long long iMem = 0;
	try
	{
		DWORD pidCurrent = GetCurrentProcessId();
		iMem = GetMemoryUsageSizeFromPID(pidCurrent);
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot)
		{
			PROCESSENTRY32 pe32 = {0};
			pe32.dwSize = sizeof(PROCESSENTRY32);
			if (Process32First(hSnapshot, &pe32))
			{
				do
				{
					DWORD PID = pe32.th32ProcessID;
					if (IsProcOwner(PID))
					{
						//自分
						if (PID == pidCurrent)
						{
							;
						}
						else
						{
							DWORD ParentPID = pe32.th32ParentProcessID;
							CString strStrDataTemp = pe32.szExeFile;
							strStrDataTemp.MakeLower();
							//親が一緒
							if (ParentPID == pidCurrent)
							{
								if (strStrDataTemp.Find(_T("chronos")) >= 0)
								{
									iMem += GetMemoryUsageSizeFromPID(PID);
								}
							}
						}
					}
				} while (Process32Next(hSnapshot, &pe32));
			}
			CloseHandle(hSnapshot);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return iMem;
}
int CSazabi::GetProcessRunningTime()
{
	int iRet = 0;
	try
	{
		FILETIME ftMakeTime = {0}; // プロセスの作成時刻
		FILETIME ftExitTime = {0}; // プロセスの終了時刻
		FILETIME ftKernTime = {0}; // プロセスのカーネル動作時間
		FILETIME ftUserTime = {0}; // プロセスのユーザー動作時間
		GetProcessTimes(m_hProcess, &ftMakeTime, &ftExitTime, &ftKernTime, &ftUserTime);
		COleDateTime timeNow = COleDateTime::GetCurrentTime();
		COleDateTime timeCreation(ftMakeTime);
		COleDateTimeSpan timeDiff = timeNow - timeCreation;
		double dMinute = 0.0;
		dMinute = timeDiff.GetTotalMinutes();
		iRet = (int)dMinute;
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return iRet;
}

DWORD CSazabi::GetKeyCombi()
{
	DWORD dRet = 0;
	dRet = dRet | (::GetKeyState(VK_SHIFT) < 0 ? KEY_COMB_SHIFT : 0);
	dRet = dRet | (::GetKeyState(VK_CONTROL) < 0 ? KEY_COMB_CTRL : 0);
	dRet = dRet | (::GetKeyState(VK_MENU) < 0 ? KEY_COMB_ALT : 0);

	dRet = dRet | (::GetKeyState(VK_LEFT) < 0 ? KEY_COMB_LEFT : 0);
	dRet = dRet | (::GetKeyState(VK_UP) < 0 ? KEY_COMB_UP : 0);
	dRet = dRet | (::GetKeyState(VK_RIGHT) < 0 ? KEY_COMB_RIGHT : 0);
	dRet = dRet | (::GetKeyState(VK_DOWN) < 0 ? KEY_COMB_DOWN : 0);

	return dRet;
}
BOOL CSazabi::bValidKeyCombi()
{
	BOOL bRet = FALSE;
	int iKeyCombination = 0;
	iKeyCombination = m_AppSettings.GetKeyCombination();
	if (iKeyCombination == 0)
		iKeyCombination = 999;
	DWORD dwKC = GetKeyCombi();
	if (iKeyCombination == dwKC)
		bRet = TRUE;
	return bRet;
}

BOOL CSazabi::IsCacheRedirectFilterNone(LPCTSTR pURL)
{
	BOOL bRet = FALSE;
	if (pURL == NULL) return bRet;
	HANDLE hEvent = {0};

	try
	{
		hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, m_strEventLogScriptName);
		if (!hEvent)
			throw std::runtime_error("Failed to execute OpenEvent at " __FUNCTION__ " : " MAKE_STRING(__LINE__));

		DWORD waitRes = WaitForSingleObject(hEvent, 50);
		if (waitRes == WAIT_TIMEOUT)
		{
			return bRet;
		}
		else
		{
			//既に登録されているかチェック
			void* ptr = NULL;
			if (m_CacheRedirectFilter_None.Lookup(pURL, ptr))
			{
				bRet = true;
			}
			SetEvent(hEvent);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	if (hEvent)
		CloseHandle(hEvent);
	return bRet;
}
void CSazabi::AddCacheRedirectFilterNone(LPCTSTR pURL)
{
	if (pURL == NULL) return;
	HANDLE hEvent = {0};

	try
	{
		hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, m_strEventLogScriptName);
		if (!hEvent)
			throw std::runtime_error("Failed to execute OpenEvent at " __FUNCTION__ " : " MAKE_STRING(__LINE__));

		DWORD waitRes = WaitForSingleObject(hEvent, 100);
		if (waitRes == WAIT_TIMEOUT)
		{
			;
		}
		else
		{
			//既に登録されているかチェック
			void* ptr = NULL;
			if (m_CacheRedirectFilter_None.Lookup(pURL, ptr))
			{
				;
			}
			else
			{
				m_CacheRedirectFilter_None.SetAt(pURL, ptr);
				int iMaxData = (int)m_CacheRedirectFilter_None.GetCount();
				if (iMaxData > 1000)
				{
					m_CacheRedirectFilter_None.RemoveAll();
				}
			}
			SetEvent(hEvent);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	if (hEvent)
		CloseHandle(hEvent);
}
////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSazabi::IsCacheURLFilterAllow(LPCTSTR pURL)
{
	BOOL bRet = FALSE;
	if (pURL == NULL) return bRet;
	HANDLE hEvent = {0};
	try
	{
		hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, m_strEventURLFilterAllow);
		if (!hEvent)
			throw std::runtime_error("Failed to execute OpenEvent at " __FUNCTION__ " : " MAKE_STRING(__LINE__));

		DWORD waitRes = WaitForSingleObject(hEvent, 50);
		if (waitRes == WAIT_TIMEOUT)
		{
			return bRet;
		}
		else
		{
			//既に登録されているかチェック
			void* ptr = NULL;
			if (m_CacheURLFilter_Allow.Lookup(pURL, ptr))
			{
				bRet = true;
			}
			SetEvent(hEvent);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	if (hEvent)
		CloseHandle(hEvent);
	return bRet;
}
BOOL CSazabi::IsCacheURLFilterDeny(LPCTSTR pURL)
{
	BOOL bRet = FALSE;
	if (pURL == NULL) return bRet;
	HANDLE hEvent = {0};
	try
	{
		hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, m_strEventURLFilterDeny);
		if (!hEvent)
			throw std::runtime_error("Failed to execute OpenEvent at " __FUNCTION__ " : " MAKE_STRING(__LINE__));

		DWORD waitRes = WaitForSingleObject(hEvent, 50);
		if (waitRes == WAIT_TIMEOUT)
		{
			return bRet;
		}
		else
		{
			//既に登録されているかチェック
			void* ptr = NULL;
			if (m_CacheURLFilter_Deny.Lookup(pURL, ptr))
			{
				bRet = true;
			}
			SetEvent(hEvent);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	if (hEvent)
		CloseHandle(hEvent);
	return bRet;
}

void CSazabi::AddCacheURLFilterAllow(LPCTSTR pURL)
{
	if (pURL == NULL) return;
	HANDLE hEvent = {0};

	try
	{
		hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, m_strEventURLFilterAllow);
		if (!hEvent)
			throw std::runtime_error("Failed to execute OpenEvent at " __FUNCTION__ " : " MAKE_STRING(__LINE__));

		DWORD waitRes = WaitForSingleObject(hEvent, 100);
		if (waitRes == WAIT_TIMEOUT)
		{
			;
		}
		else
		{
			//既に登録されているかチェック
			void* ptr = NULL;
			if (m_CacheURLFilter_Allow.Lookup(pURL, ptr))
			{
				;
			}
			else
			{
				m_CacheURLFilter_Allow.SetAt(pURL, ptr);
				int iMaxData = (int)m_CacheURLFilter_Allow.GetCount();
				if (iMaxData > 1000)
				{
					m_CacheURLFilter_Allow.RemoveAll();
				}
			}
			SetEvent(hEvent);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	if (hEvent)
		CloseHandle(hEvent);
}
void CSazabi::AddCacheURLFilterDeny(LPCTSTR pURL)
{
	if (pURL == NULL) return;
	HANDLE hEvent = {0};

	try
	{
		hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, m_strEventURLFilterDeny);
		if (!hEvent)
			throw std::runtime_error("Failed to execute OpenEvent at " __FUNCTION__ " : " MAKE_STRING(__LINE__));

		DWORD waitRes = WaitForSingleObject(hEvent, 100);
		if (waitRes == WAIT_TIMEOUT)
		{
			;
		}
		else
		{
			//既に登録されているかチェック
			void* ptr = NULL;
			if (m_CacheURLFilter_Deny.Lookup(pURL, ptr))
			{
				;
			}
			else
			{
				m_CacheURLFilter_Deny.SetAt(pURL, ptr);
				int iMaxData = (int)m_CacheURLFilter_Deny.GetCount();
				if (iMaxData > 1000)
				{
					m_CacheURLFilter_Deny.RemoveAll();
				}
			}
			SetEvent(hEvent);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	if (hEvent)
		CloseHandle(hEvent);
}

void CSazabi::ShowSettingDlg(CWnd* pParentWnd)
{
	if (!this->IsWnd(pParentWnd))
		return;
	if (this->m_pSettingDlg)
	{
		this->m_pSettingDlg->SetForegroundWindow();
		return;
	}

	this->m_pSettingDlg = new CSettingsDialog(pParentWnd);
	CString strTitle;
	this->m_pSettingDlg->SetLogoText(m_strThisAppName);

	m_AppSettings.CopyData(m_AppSettingsDlgCurrent);
	if (m_AppSettings.GetKeyCombination() > 0)
	{
		CString strMsg;
		strMsg.LoadString(IDS_STRING_ADMIN_LOCK_FEATURE);
		SB_MessageBox(pParentWnd->m_hWnd, strMsg, NULL, MB_OK | MB_ICONERROR, TRUE);
		//隠し設定画面を表示する。Shiftキーを押している場合のみ表示
		if (!bValidKeyCombi())
		{
			delete m_pSettingDlg;
			this->m_pSettingDlg = NULL;
			return;
		}
	}

	strTitle = m_strThisAppName;
	strTitle += _T(" - Settings");
	this->m_pSettingDlg->SetTitle(strTitle);

	//全般設定
	CString titleGen;
	titleGen.LoadString(IDS_STRING_SETTINGS_DLG_TITLE_GEN);
	this->m_pSettingDlg->AddPage(RUNTIME_CLASS(CDlgSetGen), titleGen, IDD_SETTINGS_DLG_GEN, titleGen);
	//画面表示設定
	CString titleDSP;
	titleDSP.LoadString(IDS_STRING_SETTINGS_DLG_TITLE_DSP);
	this->m_pSettingDlg->AddPage(RUNTIME_CLASS(CDlgSetDSP), titleDSP, IDD_SETTINGS_DLG_DSP, titleDSP);
	//起動関連設定
	CString titleINIT;
	titleINIT.LoadString(IDS_STRING_SETTINGS_DLG_TITLE_INIT);
	this->m_pSettingDlg->AddPage(RUNTIME_CLASS(CDlgSetINIT), titleINIT, IDD_SETTINGS_DLG_INIT, titleINIT);
	//インターネット接続設定
	CString titleConnection;
	titleConnection.LoadString(IDS_STRING_SETTINGS_DLG_TITLE_CONNECTION);
	this->m_pSettingDlg->AddPage(RUNTIME_CLASS(CDlgSetConnectionSetting), titleConnection, IDD_SETTINGS_DLG_CONNECTION, titleConnection);
	//リソース制限設定
	CString titleCAP;
	titleCAP.LoadString(IDS_STRING_SETTINGS_DLG_TITLE_CAP);
	this->m_pSettingDlg->AddPage(RUNTIME_CLASS(CDlgSetCAP), titleCAP, IDD_SETTINGS_DLG_CAP, titleCAP);
	//リダイレクト設定
	CString titleSEC;
	titleSEC.LoadString(IDS_STRING_SETTINGS_DLG_TITLE_SEC);
	this->m_pSettingDlg->AddPage(RUNTIME_CLASS(CDlgSetSEC), titleSEC, IDD_SETTINGS_DLG_SEC, titleSEC);
	//URLフィルター設定
	CString titleURLFilter;
	titleURLFilter.LoadString(IDS_STRING_SETTINGS_DLG_TITLE_URL_FILTER);
	this->m_pSettingDlg->AddPage(RUNTIME_CLASS(CDlgSetDomainFilter), titleURLFilter, IDD_SETTINGS_DLG_URL_FILTER, titleURLFilter);
	//ポップアップフィルター設定
	CString titlePopupFilter;
	titlePopupFilter.LoadString(IDS_STRING_SETTINGS_DLG_TITLE_POPUP_FILTER);
	this->m_pSettingDlg->AddPage(RUNTIME_CLASS(CDlgSetPopupFilter), titlePopupFilter, IDD_SETTINGS_DLG_POPUP_FILTER, titlePopupFilter);
	//CustomScript設定
	CString titleCustomScript;
	titleCustomScript.LoadString(IDS_STRING_SETTINGS_DLG_TITLE_CUSTOM_SCRIPT);
	this->m_pSettingDlg->AddPage(RUNTIME_CLASS(CDlgSetCustomScript), titleCustomScript, IDD_SETTINGS_DLG_CUSTOM_SCRIPT, titleCustomScript);

	//ログ出力設定
	CString titleLog;
	titleLog.LoadString(IDS_STRING_SETTINGS_DLG_TITLE_LOG);
	this->m_pSettingDlg->AddPage(RUNTIME_CLASS(CDlgSetLog), titleLog, IDD_SETTINGS_DLG_LOG, titleLog);

	if (this->IsSGMode())
	{
		//ファイルマネージャ設定
		CString titleFileMgr;
		titleFileMgr.LoadString(IDS_STRING_SETTINGS_DLG_TITLE_FILEMGR);
		this->m_pSettingDlg->AddPage(RUNTIME_CLASS(CDlgSetFileMgr), titleFileMgr, IDD_SETTINGS_DLG_FILEMGR, titleFileMgr);
	}

	this->m_pSettingDlg->DoModal();

	m_AppSettings.CopyData(m_AppSettingsDlgCurrent);

	if (this->m_pSettingDlg)
	{
		delete m_pSettingDlg;
		m_pSettingDlg = NULL;
	}
}

void CSazabi::ShowDebugTraceDlg()
{
	if (this->m_pDebugDlg)
	{
		delete this->m_pDebugDlg;
		this->m_pDebugDlg = NULL;
	}
	this->m_pDebugDlg = new CDlgDebugWnd;
	this->m_pDebugDlg->Create(IDD_DLG_DEBUG_WND, CWnd::GetDesktopWindow());
	this->m_pDebugDlg->ShowWindow(SW_SHOW);
}
BOOL CSazabi::IsShowDevTools()
{
	//2021-07-02 CEF 91ではresources.pakに統合されたので必ず表示する。
	//CString strDevToolPath;
	//strDevToolPath = m_strExeFolderPath;
	//strDevToolPath += _T("devtools_resources.pak");
	//if (PathFileExists(strDevToolPath))
	//{
	//	return TRUE;
	//}
	//return FALSE;
	return TRUE;
}
void CSazabi::ShowDevTools()
{
	if (IsWnd(GetActiveBFramePtr()))
	{
		GetActiveBFramePtr()->PostMessage(WM_COMMAND, ID_SHOW_DEVTOOLS, 0);
	}
}

struct LANGANDCODEPAGE
{
	WORD wLanguage;
	WORD wCodePage;
} * lpTranslate;
CString CSazabi::GetCefVersionStr()
{
	CString strDLLPath;
	strDLLPath = this->m_strExeFolderPath;
	strDLLPath += _T("libcef.dll");
	CString strRet;
	DWORD dwDummy = 0;
	DWORD dwSize = 0;
	DWORD dwMajor = 0;
	DWORD dwMinor = 0;
	DWORD dwBuild = 0;
	DWORD dwPrivate = 0;

	strRet = _T("Chromium Embedded Framework Version N/A");

	dwSize = ::GetFileVersionInfoSize(strDLLPath, &dwDummy);
	if (dwSize > 0)
	{
		PBYTE pData = new BYTE[dwSize];
		memset(pData, 0x00, dwSize);
		UINT TranslateLen = 0;
		if (::GetFileVersionInfo(strDLLPath, 0, dwSize, pData))
		{
			VerQueryValue(pData, _T("\\VarFileInfo\\Translation"),
				      (LPVOID*)&lpTranslate, &TranslateLen);
			void* pvVersion = {0};
			UINT VersionLen = 0;

			for (UINT i = 0; i < TranslateLen / sizeof(*lpTranslate); i++)
			{
				//コードページを指定
				CString name;

				name.Format(_T("\\StringFileInfo\\%04x%04x\\%s"),
					    lpTranslate[i].wLanguage,
					    lpTranslate[i].wCodePage, _T("FileVersion"));
				if (VerQueryValue(pData, name, &pvVersion, &VersionLen))
				{
					CString strVersionStr((LPCTSTR)pvVersion);
					strRet.Format(_T("Chromium Embedded Framework Version %s"), (LPCTSTR)strVersionStr);
					break;
				}
			}
		}
		delete[] pData;
	}
	return strRet;
}

CString CSazabi::GetChromiumVersionStr()
{
	CString strDLLPath;
	strDLLPath = this->m_strExeFolderPath;
	strDLLPath += _T("chrome_elf.dll");
	CString strRet;
	DWORD dwDummy = 0;
	DWORD dwSize = 0;
	DWORD dwMajor = 0;
	DWORD dwMinor = 0;
	DWORD dwBuild = 0;
	DWORD dwPrivate = 0;

	strRet = _T("Chromium(Blink) Version N/A");

	dwSize = ::GetFileVersionInfoSize(strDLLPath, &dwDummy);
	if (dwSize > 0)
	{
		PBYTE pData = new BYTE[dwSize];
		memset(pData, 0x00, dwSize);
		UINT TranslateLen = 0;
		if (::GetFileVersionInfo(strDLLPath, 0, dwSize, pData))
		{
			VerQueryValue(pData, _T("\\VarFileInfo\\Translation"),
				      (LPVOID*)&lpTranslate, &TranslateLen);
			void* pvVersion = {0};
			UINT VersionLen = 0;

			for (UINT i = 0; i < TranslateLen / sizeof(*lpTranslate); i++)
			{
				//コードページを指定
				CString name;

				name.Format(_T("\\StringFileInfo\\%04x%04x\\%s"),
					    lpTranslate[i].wLanguage,
					    lpTranslate[i].wCodePage, _T("FileVersion"));
				if (VerQueryValue(pData, name, &pvVersion, &VersionLen))
				{
					CString strVersionStr((LPCTSTR)pvVersion);
					strRet.Format(_T("Chromium(Blink) Version %s"), (LPCTSTR)strVersionStr);
					break;
				}
			}
		}
		delete[] pData;
	}
	return strRet;
}

std::tuple<WORD, WORD, WORD, WORD> CSazabi::GetFileVersion(CString filePath)
{
	std::tuple<WORD, WORD, WORD, WORD> result = std::make_tuple(0, 0, 0, 0);
	if (!::PathFileExists(filePath))
	{
		return result;
	}
	DWORD handle = 0;
	DWORD versionSize = ::GetFileVersionInfoSize((LPCWSTR)filePath, &handle);
	PBYTE pData = new BYTE[versionSize];
	memset(pData, 0x00, versionSize);
	if (::GetFileVersionInfo((LPCWSTR)filePath, NULL, versionSize, pData))
	{
		VS_FIXEDFILEINFO* fileInfo = nullptr;
		UINT len = 0;
		if (VerQueryValue(pData, _T("\\"), (PVOID*)&fileInfo, &len))
		{
			result = std::make_tuple(HIWORD(fileInfo->dwFileVersionMS),
								     LOWORD(fileInfo->dwFileVersionMS),
								     HIWORD(fileInfo->dwFileVersionLS),
								     LOWORD(fileInfo->dwFileVersionLS));
		}
	}
	delete[] pData;
	return result;
}

void CSazabi::CheckChronosVersionMismatch()
{
	if (!this->IsSGMode())
	{
		return;
	}

	CString chronosExePath = m_strExeFolderPath + _T("Chronos.exe");
	CString chronosNExePath = m_strExeFolderPath + _T("ChronosN.exe");
	std::tuple<WORD, WORD, WORD, WORD> chronosVersion = GetFileVersion(chronosExePath);
	std::tuple<WORD, WORD, WORD, WORD> chronosNVersion = GetFileVersion(chronosNExePath);
	if (chronosVersion != chronosNVersion)
	{
		CString alertMsg;
		alertMsg.LoadString(IDS_EXE_VERSION_MISMATCH);
		alertMsg.Format(alertMsg,
				std::get<0>(chronosVersion),
				std::get<1>(chronosVersion),
				std::get<2>(chronosVersion),
				std::get<3>(chronosVersion),
				std::get<0>(chronosNVersion),
				std::get<1>(chronosNVersion),
				std::get<2>(chronosNVersion),
				std::get<3>(chronosNVersion));
		::MessageBox(NULL, alertMsg, m_strThisAppName, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
	}
}

CString CSazabi::GetVOSVersionFromNT0_DLLStr()
{
	CString strRet;
	DWORD dwDummy = 0;
	DWORD dwSize = 0;
	DWORD dwMajor = 0;
	DWORD dwMinor = 0;
	DWORD dwBuild = 0;
	DWORD dwPrivate = 0;

#ifdef _WIN64
	dwSize = ::GetFileVersionInfoSize(_T("NT0_DLL64.DLL"), &dwDummy);
#else
	dwSize = ::GetFileVersionInfoSize(_T("NT0_DLL.DLL"), &dwDummy);
#endif
	if (dwSize > 0)
	{
		PBYTE pData = new BYTE[dwSize];
		memset(pData, 0x00, dwSize);
#ifdef _WIN64
		if (::GetFileVersionInfo(_T("NT0_DLL64.DLL"), 0, dwSize, pData))
#else
		if (::GetFileVersionInfo(_T("NT0_DLL.DLL"), 0, dwSize, pData))
#endif
		{
			VS_FIXEDFILEINFO* pFileInfo = {0};
			UINT nLen = 0;
			if (::VerQueryValue(pData, _T("\\"), (PVOID*)&pFileInfo, &nLen))
			{
				dwMajor = HIWORD(pFileInfo->dwFileVersionMS);
				dwMinor = LOWORD(pFileInfo->dwFileVersionMS);
				dwBuild = HIWORD(pFileInfo->dwFileVersionLS);
				dwPrivate = LOWORD(pFileInfo->dwFileVersionLS);
				strRet.Format(_T("VOS_NT0_DLL Version %d.%d.%d.%d"), dwMajor, dwMinor, dwBuild, dwPrivate);
			}
		}
		delete[] pData;
	}
	return strRet;
}

CString CSazabi::GetOSKernelVersion()
{
	CString strRet;
	DWORD dwDummy = 0;
	DWORD dwSize = 0;
	DWORD dwMajor = 0;
	DWORD dwMinor = 0;
	DWORD dwBuild = 0;
	DWORD dwPrivate = 0;

	strRet = _T("Kernel Version N/A");

	dwSize = ::GetFileVersionInfoSize(_T("KERNEL32.DLL"), &dwDummy);
	if (dwSize > 0)
	{
		PBYTE pData = new BYTE[dwSize];
		memset(pData, 0x00, dwSize);
		if (::GetFileVersionInfo(_T("KERNEL32.DLL"), 0, dwSize, pData))
		{
			VS_FIXEDFILEINFO* pFileInfo = {0};
			UINT nLen = 0;
			if (::VerQueryValue(pData, _T("\\"), (PVOID*)&pFileInfo, &nLen))
			{
				dwMajor = HIWORD(pFileInfo->dwFileVersionMS);
				dwMinor = LOWORD(pFileInfo->dwFileVersionMS);
				dwBuild = HIWORD(pFileInfo->dwFileVersionLS);
				dwPrivate = LOWORD(pFileInfo->dwFileVersionLS);
				strRet.Format(_T("Kernel Version %d.%d.%d.%d"), dwMajor, dwMinor, dwBuild, dwPrivate);
			}
		}
		delete[] pData;
	}
	return strRet;
}
CString CSazabi::GetTurboVMInfo()
{
	CString strRet;
	DWORD dwDummy = 0;
	DWORD dwSize = 0;
	DWORD dwMajor = 0;
	DWORD dwMinor = 0;
	DWORD dwBuild = 0;
	DWORD dwPrivate = 0;

	dwSize = ::GetFileVersionInfoSize(_T("vmx.dll"), &dwDummy);
	if (dwSize > 0)
	{
		PBYTE pData = new BYTE[dwSize];
		memset(pData, 0x00, dwSize);
		if (::GetFileVersionInfo(_T("vmx.dll"), 0, dwSize, pData))
		{
			VS_FIXEDFILEINFO* pFileInfo = {0};
			UINT nLen = 0;
			if (::VerQueryValue(pData, _T("\\"), (PVOID*)&pFileInfo, &nLen))
			{
				dwMajor = HIWORD(pFileInfo->dwFileVersionMS);
				dwMinor = LOWORD(pFileInfo->dwFileVersionMS);
				dwBuild = HIWORD(pFileInfo->dwFileVersionLS);
				dwPrivate = LOWORD(pFileInfo->dwFileVersionLS);
				strRet.Format(_T("VMX Version %d.%d.%d.%d"), dwMajor, dwMinor, dwBuild, dwPrivate);
			}
		}
		delete[] pData;
	}
	return strRet;
}

CString CSazabi::GetVOSInfo()
{
	TCHAR szTargetPath[512] = {0};
	DWORD dwZero = 0;
	DWORD dwVerInfoSize = 0;
	UCHAR* pBlock = {0};
	//バージョンを取得する為のバッファ
	void* pvVersion = {0};
	UINT VersionLen = 0;
	UINT TranslateLen = 0;

	CString szThinAppVersion;
	CString szThinAppLicense;
	CString szThinAppBuildDateTime;
	CString szYYYY;
	CString szMM;
	CString szDD;
	CString szHH;
	CString sz24M;
	CString szSS;
	CString strDateTime;
	CString strDateTimeTmp;
	CString strRet;
	for (;;)
	{
		if (0 == GetEnvironmentVariable(_T("TS_ORIGIN"), szTargetPath, 512))
		{
			break;
		}
		dwVerInfoSize = GetFileVersionInfoSize(szTargetPath, &dwZero);
		if (dwVerInfoSize == 0)
		{
			break;
		}
		pBlock = new UCHAR[dwVerInfoSize];
		if (pBlock == NULL)
			break;
		memset(pBlock, 0x00, dwVerInfoSize * sizeof(UCHAR));
		GetFileVersionInfo(szTargetPath, 0, dwVerInfoSize, pBlock);

		VerQueryValue(pBlock, _T("\\VarFileInfo\\Translation"),
			      (LPVOID*)&lpTranslate, &TranslateLen);

		for (UINT i = 0; i < TranslateLen / sizeof(*lpTranslate); i++)
		{
			//コードページを指定
			CString name;

			name.Format(_T("\\StringFileInfo\\%04x%04x\\%s"),
				    lpTranslate[i].wLanguage,
				    lpTranslate[i].wCodePage, _T("ThinAppVersion"));
			if (VerQueryValue(pBlock, name, &pvVersion, &VersionLen))
			{
				szThinAppVersion = (LPCTSTR)pvVersion;
			}

			name.Format(_T("\\StringFileInfo\\%04x%04x\\%s"),
				    lpTranslate[i].wLanguage,
				    lpTranslate[i].wCodePage, _T("ThinAppLicense"));
			if (VerQueryValue(pBlock, name, &pvVersion, &VersionLen))
			{
				szThinAppLicense = (LPCTSTR)pvVersion;
			}

			name.Format(_T("\\StringFileInfo\\%04x%04x\\%s"),
				    lpTranslate[i].wLanguage,
				    lpTranslate[i].wCodePage, _T("ThinAppBuildDateTime"));
			if (VerQueryValue(pBlock, name, &pvVersion, &VersionLen))
			{
				szThinAppBuildDateTime = (LPCTSTR)pvVersion;
				if (!szThinAppBuildDateTime.IsEmpty())
				{
					strDateTimeTmp = szThinAppBuildDateTime;
					strDateTimeTmp.TrimLeft();
					strDateTimeTmp.TrimRight();
					if (strDateTimeTmp.GetLength() >= 15)
					{
						szYYYY = strDateTimeTmp.Mid(0, 4);
						szMM = strDateTimeTmp.Mid(4, 2);
						szDD = strDateTimeTmp.Mid(4 + 2, 2);

						szHH = strDateTimeTmp.Mid(4 + 2 + 2 + 1, 2);
						sz24M = strDateTimeTmp.Mid(4 + 2 + 2 + 1 + 2, 2);
						szSS = strDateTimeTmp.Mid(4 + 2 + 2 + 1 + 2 + 2, 2);
						strDateTime.Format(_T("%s-%s-%s %s:%s:%s"), 
							(LPCTSTR)szYYYY, (LPCTSTR)szMM, 
							(LPCTSTR)szDD, (LPCTSTR)szHH, 
							(LPCTSTR)sz24M, (LPCTSTR)szSS);
					}
				}
			}
		}
		strRet.Format(_T("%s\r\n%s %s"),
			      (LPCTSTR)GetVOSVersionFromNT0_DLLStr(),
			      _T("TS_ORIGIN"), (LPCTSTR)szTargetPath);
		CString strTmp;
		if (!szThinAppVersion.IsEmpty())
		{
			strTmp.Format(_T("\r\n%s %s\r\n%s %s\r\n%s %s"),
				      _T("ThinAppVersion"), (LPCTSTR)szThinAppVersion,
				      _T("ThinAppLicense"), (LPCTSTR)szThinAppLicense,
				      _T("ThinAppBuildDateTime"), (LPCTSTR)strDateTime);
			strRet += strTmp;
		}
		break;
	}
	if (pBlock)
		delete[] pBlock;

	return strRet;
}

CString CSazabi::GetOSInfo(void)
{
	CString strBuff = _T("Windows");

	strBuff += IsWindowsServer() ? _T(" Server") : _T(" Client");

	strBuff += SBUtil::Is64BitWindows() ? _T(" x64") : _T(" x86");

	if (GetSystemMetrics(SM_REMOTESESSION))
	{
		strBuff += _T(" (RDS Session)");
	}

	return strBuff;
}
CString CSazabi::GetAllModules()
{
	PROC_TIME(GetAllModules)

	CString strRet;

	try
	{
		HMODULE hMods[1024] = {0};
		HANDLE hProcess = {0};
		DWORD cbNeeded = 0;
		unsigned int i = 0;

		DWORD processID = 0;
		processID = GetCurrentProcessId();
		strRet.Format(_T("\nSA-DAO150915=============================================\nProcess ID: %d\n"), processID);
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
					   PROCESS_VM_READ,
				       FALSE, processID);
		if (NULL == hProcess)
			return strRet;

		TCHAR szModName[MAX_PATH * 2] = {0};
		CString strTemp;
		if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
		{
			for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
			{
				memset(szModName, 0x00, sizeof(szModName));
				if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
							sizeof(szModName) / sizeof(TCHAR)))
				{
					strTemp.Format(_T("%s (0x%08p)\n"), (LPCTSTR)szModName, hMods[i]);
					strRet += strTemp;
				}
			}
			strRet += _T("SA-DAO150915---------------------------------------------");
		}
		CloseHandle(hProcess);
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return strRet;
}

BOOL CSazabi::ProcessMessageFilter(int code, LPMSG lpMsg)
{
	__try
	{
		return CWinApp::ProcessMessageFilter(code, lpMsg);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ASSERT(FALSE);
		WriteDebugTraceDateTime(_T("EXCEPTION_In_ProcessMessageFilter"), DEBUG_LOG_TYPE_EX);
		return TRUE;
	}
}
CBrowserFrame* CSazabi::GetNextGenerationActiveWindow(CBrowserFrame* pTarget)
{
	CBrowserFrame* pBFrame = NULL;
	__try
	{
		if (IsWnd(m_pMainWnd))
		{
			pBFrame = ((CMainFrame*)m_pMainWnd)->GetNextGenerationActiveWindow(pTarget);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return pBFrame;
}
HWND CSazabi::GetActiveBFramePtrHWND()
{
	HWND hRet = {0};
	CBrowserFrame* pFrm = NULL;
	pFrm = GetActiveBFramePtr();
	if (pFrm)
	{
		hRet = pFrm->m_hWnd;
	}
	return hRet;
}
CBrowserFrame* CSazabi::GetActiveBFramePtr()
{
	CBrowserFrame* pFrm = NULL;
	__try
	{
		if (IsWnd(m_pMainWnd))
		{
			//popupを除く
			pFrm = dynamic_cast<CBrowserFrame*>(m_pMainWnd->GetForegroundWindow());
			if (IsWnd(pFrm))
			{
				if (!pFrm->m_bIsPopupWindow)
					return pFrm;
			}

			pFrm = dynamic_cast<CBrowserFrame*>(m_pMainWnd->GetActiveWindow());
			if (IsWnd(pFrm))
			{
				if (!pFrm->m_bIsPopupWindow)
					return pFrm;
			}

			pFrm = dynamic_cast<CBrowserFrame*>(((CMainFrame*)m_pMainWnd)->m_pActiveWindow);
			if (IsWnd(pFrm))
			{
				if (!pFrm->m_bIsPopupWindow)
					return pFrm;
			}

			pFrm = dynamic_cast<CBrowserFrame*>(((CMainFrame*)m_pMainWnd)->m_pPrevActiveWindow);
			if (IsWnd(pFrm))
			{
				if (!pFrm->m_bIsPopupWindow)
					return pFrm;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return pFrm;
}
CChildView* CSazabi::GetActiveViewPtr()
{
	CChildView* pView = NULL;
	__try
	{
		if (IsWnd(m_pMainWnd))
		{
			CBrowserFrame* pFrm = NULL;
			pFrm = dynamic_cast<CBrowserFrame*>(m_pMainWnd->GetForegroundWindow());
			if (!IsWnd(pFrm))
				pFrm = dynamic_cast<CBrowserFrame*>(m_pMainWnd->GetActiveWindow());
			if (IsWnd(pFrm))
			{
				pView = &pFrm->m_wndView;
				if (!IsWnd(pView))
				{
					pView = NULL;
				}
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return pView;
}
WINDOWPLACEMENT CSazabi::GetActiveFrameWindowPlacement()
{
	WINDOWPLACEMENT wp = {0};
	wp.length = sizeof(WINDOWPLACEMENT);
	CBrowserFrame* pFrm = NULL;
	pFrm = GetActiveBFramePtr();
	if (IsWnd(pFrm))
	{
		pFrm->GetWindowPlacement(&wp);
	}
	return wp;
}

CString CSazabi::GetActivePageURL()
{
	CString strURL;
	CChildView* pView = NULL;
	pView = GetActiveViewPtr();
	if (pView)
	{
		strURL = pView->GetLocationURL();
	}
	return strURL;
}

void CSazabi::HideRebar(CWnd* pWnd)
{
	CBrowserFrame* pFrame = (CBrowserFrame*)pWnd;
	if (!IsWnd(pFrame))
		return;

	//ステータスバーを非表示にする。
	if (IsWnd(pFrame->m_pwndStatusBar))
		pFrame->m_pwndStatusBar->ShowWindow(SW_HIDE);

	if (pFrame->m_pwndReBar)
	{
		CReBarCtrl& rbCtrl = pFrame->m_pwndReBar->GetReBarCtrl();
		REBARBANDINFO rbbi = {0};
		REBARBANDINFO rbbiNull = {0};

		UINT cbCompileSize = 0;

		cbCompileSize = sizeof(REBARBANDINFO);
		rbbi.cbSize = cbCompileSize;

		rbbi.fMask = RBBIM_CHILD | RBBIM_STYLE;
		rbbiNull = rbbi;

		rbbi.fMask = RBBIM_STYLE;
		VERIFY(rbCtrl.GetBandInfo(0, &rbbi));
		if (!(rbbi.fStyle & RBBS_HIDDEN))
			VERIFY(rbCtrl.ShowBand(0, FALSE));

		rbbi = rbbiNull;
		rbbi.fMask = RBBIM_STYLE;
		VERIFY(rbCtrl.GetBandInfo(1, &rbbi));
		if (!(rbbi.fStyle & RBBS_HIDDEN))
			VERIFY(rbCtrl.ShowBand(1, FALSE));

		rbbi = rbbiNull;
		rbbi.fMask = RBBIM_STYLE;
		VERIFY(rbCtrl.GetBandInfo(2, &rbbi));
		if (!(rbbi.fStyle & RBBS_HIDDEN))
			VERIFY(rbCtrl.ShowBand(2, FALSE));
	}
	pFrame->ShowWindow(SW_MAXIMIZE);
}
void CSazabi::InitializeCef()
{
	PROC_TIME(InitializeCef)

	m_cefApp = new ClientApp();
#if CHROME_VERSION_MAJOR < 112
	CefEnableHighDPISupport();
#endif

	CefMainArgs mainargs(m_hInstance);
	void* sandbox_info = NULL;

	CefSettings settings;

	// 別スレッドでメッセージループを管理しない
	// (CefDoMessageLoopWork()をメインプログラムから呼び出す)
	m_bMultiThreadedMessageLoop = FALSE;
	settings.multi_threaded_message_loop = m_bMultiThreadedMessageLoop;
	settings.external_message_pump = true;
#if CHROME_VERSION_MAJOR >= 125 && CHROME_VERSION_MAJOR <= 127
	// CEF 124まではchrome_runtime = false（デフォルト値）を使用。
	// CEF 128からは常にChrome runtimeを使用するため、chrome_runtimeオプションは削除された。
	// https://github.com/chromiumembedded/cef/commit/a461a89728054d40eb8ad4c609eaa391cf4ea52e#diff-447d40892c51b4ffc870cf1a28bab2ae2d411752216be4750a17193e6675b26bL244
	settings.chrome_runtime = true;
#endif

	settings.no_sandbox = true;
	if (!m_IsSGMode)
		settings.command_line_args_disabled = true;

	CString strUA = GetUserAgent();
	if (!strUA.IsEmpty())
	{
		CefString(&settings.user_agent) = strUA.GetString();
	}

	CString strLocale;
	strLocale.LoadString(IDS_STRING_CEF_LOCALE);
	CefString strCefLocale;
	strCefLocale = strLocale;
	CefString(&settings.locale) = strCefLocale;

	CString strLAcceptLanguageList;
	strLAcceptLanguageList.LoadString(IDS_STRING_CEF_ACCEPT_LANGUAGE_LIST);
	CefString strCefAcceptLanguageList;
	strCefAcceptLanguageList = strLAcceptLanguageList;
	CefString(&settings.accept_language_list) = strCefAcceptLanguageList;
	DWORD pidCurrent = GetCurrentProcessId();

	// キャッシュフォルダのパスを取得する。
	//
	// * C:\Users\<user>\AppData\Local\Thinstall\ChronosSG\%drive_C%\Chronos\CEFCache\{pid} (SGモード)
	// * C:\Users\<user>\AppData\Local\ChronosCache\{pid} (通常モード)
	//
	if (this->IsSGMode())
	{
		m_strCEFCachePathBase.Format(_T("%s\\CEFCache"), (LPCTSTR)m_strExeFolderPath);
		m_strCEFCachePath.Format(_T("%s\\CEFCache\\%d"), (LPCTSTR)m_strExeFolderPath, pidCurrent);
	}
	else
	{
		CString strLocalAppPath;
		strLocalAppPath = SBUtil::GetLocalAppDataPath();
		if (strLocalAppPath.IsEmpty())
		{
			strLocalAppPath = m_strExeFolderPath;
		}
		strLocalAppPath = strLocalAppPath.TrimRight('\\');
		m_strCEFCachePathBase.Format(_T("%s\\ChronosCache"), (LPCTSTR)strLocalAppPath);
		m_strCEFCachePath.Format(_T("%s\\ChronosCache\\%d"), (LPCTSTR)strLocalAppPath, pidCurrent);
	}

	if (IsFirstInstance())
	{
		DeleteCEFCacheAll();
	}
	settings.persist_session_cookies = true;
	CefString(&settings.root_cache_path) = m_strCEFCachePath;
	if (!this->m_AppSettings.IsEnableMemcache())
	{
		CefString(&settings.cache_path) = m_strCEFCachePath;
	}

#if CHROME_VERSION_MAJOR < 115
	CString strUserDataPath;
	strUserDataPath = m_strCEFCachePath;
	strUserDataPath += _T("\\UserData");
	CefString(&settings.user_data_path) = strUserDataPath;
#endif
#if CHROME_VERSION_MAJOR <= 127
	// CEF128からChrome runtimeが常に有効になったことに合わせて、persist_user_preferencesも常に有効になったため削除された。
	// https://github.com/chromiumembedded/cef/commit/a461a89728054d40eb8ad4c609eaa391cf4ea52e#diff-447d40892c51b4ffc870cf1a28bab2ae2d411752216be4750a17193e6675b26bL347
	// https://github.com/chromiumembedded/cef/issues/3685
	settings.persist_user_preferences = true;
#endif

	// ログを有効化する (ChronosDefault.conf > EnableAdvancedLogMode)
	if (m_AppSettings.IsAdvancedLogMode())
	{
		m_strFaviconCachePath = m_strCEFCachePath;
		m_strFaviconCachePath += _T("\\Favicon\\");
		MakeDirectoryPath(m_strFaviconCachePath);
		if (m_AppSettings.GetAdvancedLogLevel() == DEBUG_LOG_LEVEL_OUTPUT_ALL)
		{
			CString strLogFile(m_strCEFCachePath);
			strLogFile += _T("\\CEFDebug.log");
			CefString(&settings.log_file) = strLogFile;
			if (m_AppSettings.IsAdvancedLogVerboseMode())
			{
				settings.log_severity = cef_log_severity_t::LOGSEVERITY_VERBOSE;
			}
			else
			{
				settings.log_severity = cef_log_severity_t::LOGSEVERITY_DEFAULT;
			}
		}
		else
			settings.log_severity = cef_log_severity_t::LOGSEVERITY_DISABLE;
	}
	else
	{
		CString strEmpty;
		strEmpty.Empty();
		CefString(&settings.log_file) = strEmpty;
		settings.log_severity = cef_log_severity_t::LOGSEVERITY_DISABLE;
	}

	m_bCEFInitialized = CefInitialize(mainargs, settings, m_cefApp.get(), sandbox_info);
	if (!m_bMultiThreadedMessageLoop)
	{
		m_pMessageLoopWorker = new MessageLoopWorker(m_hInstance);
		m_pMessageLoopWorker->Run();
	}
}

/*
 * 環境に応じたUserAgent文字列を生成する。
 * ----
 * Mozilla/5.0 (Windows NT 10.0;) AppleWebKit/537.36 (KHTML, like Gecko;KA-ZUMA)
 * Chrome/91.0.4472.114 Safari/537.36 Edg/91.0.4472.114 Chronos/SystemGuard
 * ----
 */
CString CSazabi::GetUserAgent()
{
	CString strUA = sgSZB_UA_START; /* sbcommon.h:50 */
	CString strUA_Append = m_AppSettings.GetUserAgentAppendStr();

	if (theApp.m_iWinOSVersion >= 100)
		strUA += _T("Windows NT 10.0;");
	else if (theApp.m_iWinOSVersion == 63)
		strUA += _T("Windows NT 6.3;");
	else if (theApp.m_iWinOSVersion == 62)
		strUA += _T("Windows NT 6.2;");
	else if (theApp.m_iWinOSVersion == 61)
		strUA += _T("Windows NT 6.1;");
	else if (theApp.m_iWinOSVersion == 60)
		strUA += _T("Windows NT 6.0;");
	else
		strUA += _T("Windows NT 10.0;");

#ifdef _WIN64
	strUA += _T(" Win64; x64");
#endif //WIN64
	strUA += sgSZB_UA_END;

	if (!strUA_Append.IsEmpty())
	{
		strUA += strUA_Append;
	}
	return strUA;
}

void CSazabi::UnInitializeCef()
{
	PROC_TIME(UnInitializeCef)

	if (m_bCEFInitialized)
	{
		m_bCEFInitialized = FALSE;
		m_cefApp = nullptr;
		if (m_pMessageLoopWorker)
		{
			m_pMessageLoopWorker->Quit();
			delete m_pMessageLoopWorker;
			m_pMessageLoopWorker = NULL;
		}
		// shutdown CEF
		CefShutdown();
	}
}
BOOL CSazabi::IsURLFilterAllow(LPCTSTR sURL,
			       LPCTSTR sSchme,
			       LPCTSTR sHost,
			       LPCTSTR sPath)
{
	PROC_TIME(IsURLFilterAllow)

	BOOL bRet = FALSE;
	try
	{
		if (!m_AppSettings.IsEnableURLFilter())
			return TRUE;

		//DenyにHitした。
		if (IsCacheURLFilterDeny(sHost))
		{
			return FALSE;
		}

		//AllowにHitした
		if (IsCacheURLFilterAllow(sHost))
		{
			return TRUE;
		}

		size_t iAction = TF_ALLOW;
		iAction = m_cDomainFilterList.HitWildCardURL(sHost);

		//Deny
		if (TF_DENY == iAction)
		{
			AddCacheURLFilterDeny(sHost);
			return FALSE;
		}
		//Allow
		else
		{
			AddCacheURLFilterAllow(sHost);
			return TRUE;
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return bRet;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSazabi::IsLimitChkEx()
{
	PROC_TIME(IsLimitChkEx)

	BOOL bRet = FALSE;
	try
	{
		UINT iWindowCnt = GetWindowCount();
		UINT iWindowL = m_AppSettings.GetWindowCountLimit();
		CString logmsg;

		//Window Count Limits
		if (iWindowCnt + 1 > iWindowL)
		{
			CString alertMsg;
			alertMsg.LoadString(IDS_STRING_TOO_MANY_WINDOWS_MSG);
			logmsg.Format(alertMsg, iWindowL, m_strThisAppName);
			WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_EX);
			::MessageBox(m_pMainWnd->m_hWnd, logmsg, m_strThisAppName, MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
			bRet = TRUE;
			return bRet;
		}

		//check max limit
		unsigned long long iMemSize = GetMemoryUsageSize();
		size_t iMemL = m_AppSettings.GetMemoryUsageLimit();
		//GDI/ User /Memory Limits
		if (iMemSize > (unsigned long long)iMemL * 1024 * 1024)
		{
			CString alertMsg;
			alertMsg.LoadString(IDS_STRING_LOW_SYSTEM_RESOURCE_SHUTDOWN);
			logmsg.Format(alertMsg, m_strThisAppName);
			WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_EX);
			m_bAbortFlg = TRUE;
			::MessageBox(m_pMainWnd->m_hWnd, logmsg, m_strThisAppName, MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
			bRet = TRUE;
			ExecNewInstance(_T(""));
			::Sleep(10 * 1000);
			m_pMainWnd->SendMessage(WM_CLOSE);

			return bRet;
		}
		else
		{
			//WARNING
			if (iMemSize > (unsigned long long)(iMemL - 250) * 1024 * 1024)
			{
				//一旦ワーキングセットをクリアして開放してしまう。
				EmptyWorkingSetAll();
			}
			iMemSize = GetMemoryUsageSize();
			if (iMemSize > (unsigned long long)(iMemL - 250) * 1024 * 1024)
			{
				CString alertMsg;
				alertMsg.LoadString(IDS_STRING_LOW_SYSTEM_RESOURCE_SUGGEST);
				logmsg.Format(alertMsg, m_strThisAppName);
				WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_EX);
				::MessageBox(m_pMainWnd->m_hWnd, logmsg, m_strThisAppName, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
				bRet = TRUE;
				return bRet;
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return bRet;
}

void CSazabi::SetFavicon(CImage* img, CBrowserFrame* pwndFrame)
{
	if (img)
	{
		if (NULL == img)
			return;
		if (NULL == pwndFrame)
			return;

		CBitmap bitmap;
		bitmap.Attach(img->Detach());

		ICONINFO ii = {0};
		ii.fIcon = TRUE;
		ii.hbmColor = bitmap;
		ii.hbmMask = bitmap;

		CIconHelper ICoHelper;
		ICoHelper = ::CreateIconIndirect(&ii);
		bitmap.DeleteObject();

		int iIndex = ((CMainFrame*)m_pMainWnd)->Get_TabWindowIndex(pwndFrame);
		if (iIndex > -1)
		{
			m_imgFavIcons.Replace(iIndex, ICoHelper);
		}
		else
			m_imgFavIcons.Add(ICoHelper);
		((CMainFrame*)m_pMainWnd)->TabWindowMsgBSend(TWNT_REFRESH_COMMAND, NULL);
	}
}
void CSazabi::SetDefaultFavicon(CBrowserFrame* pwndFrame)
{
	if (NULL == pwndFrame)
		return;

	CIconHelper ICoHelper;
	ICoHelper = LoadIcon(IDI_ICON2);

	int iIndex = ((CMainFrame*)m_pMainWnd)->Get_TabWindowIndex(pwndFrame);
	if (iIndex > -1)
	{
		m_imgFavIcons.Replace(iIndex, ICoHelper);
	}
	else
		m_imgFavIcons.Add(ICoHelper);
	((CMainFrame*)m_pMainWnd)->TabWindowMsgBSend(TWNT_REFRESH_COMMAND, NULL);
}

void CSazabi::SetWarmFavicon(HWND hwndF)
{
	if (NULL == hwndF)
		return;

	CBrowserFrame* pFrame = NULL;
	pFrame = ((CMainFrame*)m_pMainWnd)->GetBrowserFrameFromChildWnd(hwndF);
	if (IsWnd(pFrame))
	{
		CIconHelper ICoHelper;
		ICoHelper = LoadIcon(IDI_ICON3);
		int iIndex = ((CMainFrame*)m_pMainWnd)->Get_TabWindowIndex(pFrame);
		if (iIndex > -1)
		{
			m_imgFavIcons.Replace(iIndex, ICoHelper);
		}
		else
			m_imgFavIcons.Add(ICoHelper);
		((CMainFrame*)m_pMainWnd)->TabWindowMsgBSend(TWNT_REFRESH_COMMAND, NULL);
	}
}

#pragma warning(push, 0)
#pragma warning(disable : 26812)
void CSazabi::CreateNewWindow(LPCTSTR lpURL, BOOL bActive)
{
	CChildView* pCView = NULL;
	CBrowserFrame* pFrm = NULL;

	pFrm = GetActiveBFramePtr();
	if (IsWnd(pFrm))
	{
		pCView = &pFrm->m_wndView;
		if (IsWnd(pCView))
		{
			if (pCView->m_hWnd)
				::SendMessageTimeout(pCView->m_hWnd, WM_NEW_WINDOW_URL, (WPARAM)bActive ? cef_window_open_disposition_t::CEF_WOD_NEW_WINDOW : cef_window_open_disposition_t::CEF_WOD_NEW_BACKGROUND_TAB, (LPARAM) _T("chrome://version/"), SMTO_NORMAL, 1000, NULL);
		}
	}
}
//#pragma warning(disable : 26812)
#pragma warning(pop)

typedef int(__stdcall* TMessageBoxTimeout)(HWND, LPCTSTR, LPCTSTR, UINT, WORD, DWORD);
int CSazabi::SB_MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, BOOL bBlackOut /*=FALSE*/, int iTimeOut /*=-1*/)
{
	int iRet = 0;
	try
	{
		//RDS環境の場合は、半透明のフルスクリーンが描画パフォーマンスに影響するため無効にする。
		if (InRDSEnvironment() != RDS_NA)
		{
			bBlackOut = FALSE;
		}
		//0は非表示
		if (iTimeOut == 0)
			return MB_OK;

		LPCTSTR lpstrCaption = NULL;
		if (lpCaption)
			lpstrCaption = lpCaption;
		else
			lpstrCaption = m_strThisAppName;

		BOOL bFreeLibrary = FALSE;
		HMODULE hModule = {0};
		hModule = ::GetModuleHandle(_T("user32.dll"));
		if (!hModule)
		{
			hModule = ::LoadLibrary(_T("user32.dll"));
			if (hModule)
				bFreeLibrary = TRUE;
		}

		if (bBlackOut)
		{
			WNDCLASSEX wndClass = {0};
			HWND hStopWnd = {0};
			RECT rectParent;

			// Register the window class
			wndClass.cbSize = sizeof wndClass;
			wndClass.style = 0;
			wndClass.lpfnWndProc = DefWindowProc;
			wndClass.cbClsExtra = 0;
			wndClass.cbWndExtra = 0;
			wndClass.hInstance = GetModuleHandle(NULL);
			wndClass.hIcon = NULL;
			wndClass.hCursor = NULL;
			wndClass.hbrBackground = GetSysColorBrush(COLOR_DESKTOP);
			wndClass.lpszMenuName = NULL;
			wndClass.lpszClassName = _T("CSGBlackout");
			wndClass.hIconSm = NULL;
			UnregisterClass(wndClass.lpszClassName, wndClass.hInstance);
			if (RegisterClassEx(&wndClass))
			{
				if (hWnd == NULL)
					hWnd = GetDesktopWindow();

				GetWindowRect(hWnd, &rectParent);
				int nWidth = rectParent.right - rectParent.left + 10;
				int nHeight = rectParent.bottom - rectParent.top + 10;

				hStopWnd = CreateWindowEx(
				    WS_EX_LAYERED,
				    wndClass.lpszClassName,
				    NULL,
				    WS_VISIBLE | WS_POPUP,
				    rectParent.left - 5,
				    rectParent.top - 5,
				    nWidth,
				    nHeight,
				    hWnd,
				    NULL,
				    wndClass.hInstance,
				    NULL);

				if (hStopWnd)
				{
					SetLayeredWindowAttributes(hStopWnd, 0, 196, ULW_ALPHA);
					if (CTaskDialog::IsSupported())
					{
						iRet = 0;
						CStringW strWindowTitle(lpstrCaption);
						CStringW strMainInstruction(lpText);
						int iFlags = 0; //TDCBF_OK_BUTTON;
						if ((uType & MB_OKCANCEL) == MB_OKCANCEL)
							iFlags |= TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON;
						if ((uType & MB_ABORTRETRYIGNORE) == MB_ABORTRETRYIGNORE)
							iFlags |= TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON | TDCBF_RETRY_BUTTON;
						if ((uType & MB_YESNOCANCEL) == MB_YESNOCANCEL)
							iFlags |= TDCBF_YES_BUTTON | TDCBF_NO_BUTTON | TDCBF_CANCEL_BUTTON;
						if ((uType & MB_YESNO) == MB_YESNO)
							iFlags |= TDCBF_YES_BUTTON | TDCBF_NO_BUTTON;
						if ((uType & MB_RETRYCANCEL) == MB_RETRYCANCEL)
							iFlags |= TDCBF_RETRY_BUTTON | TDCBF_CANCEL_BUTTON;
						if ((uType & MB_CANCELTRYCONTINUE) == MB_CANCELTRYCONTINUE)
							iFlags |= TDCBF_RETRY_BUTTON | TDCBF_CANCEL_BUTTON;
						if (iFlags == 0)
							iFlags = TDCBF_OK_BUTTON;

						PCWSTR pszIcon = TD_INFORMATION_ICON;
						if ((uType & MB_ICONHAND) == MB_ICONHAND)
							pszIcon = TD_ERROR_ICON;
						if ((uType & MB_ICONQUESTION) == MB_ICONQUESTION)
							pszIcon = TD_INFORMATION_ICON;
						if ((uType & MB_ICONEXCLAMATION) == MB_ICONEXCLAMATION)
							pszIcon = TD_WARNING_ICON;
						if ((uType & MB_ICONASTERISK) == MB_ICONASTERISK)
							pszIcon = TD_INFORMATION_ICON;

						CMyTaskDlg TaskDlg(_T(""), strMainInstruction, strWindowTitle, iFlags);
						TaskDlg.SetMainIcon(pszIcon);
						DWORD defButton = TDCBF_OK_BUTTON;
						if ((uType & MB_DEFBUTTON2) == MB_DEFBUTTON2)
						{
							if ((uType & MB_OKCANCEL) == MB_OKCANCEL)
								defButton = TDCBF_CANCEL_BUTTON;
							else if ((uType & MB_YESNO) == MB_YESNO)
								defButton = TDCBF_NO_BUTTON;
						}
						if (iTimeOut > 0)
						{
							TaskDlg.SetTimeout(iTimeOut, defButton);
						}
						iRet = (int)TaskDlg.DoModal();
					}
					else
					{
						//通常のメッセージボックス or ModuleがNULL
						if (iTimeOut == -1 || hModule == NULL)
						{
							iRet = ::MessageBox(hStopWnd, lpText, lpstrCaption, uType);
						}
						else
						{
							//Timeout
							TMessageBoxTimeout MessageBoxTimeout;
							MessageBoxTimeout = (TMessageBoxTimeout)GetProcAddress(hModule, "MessageBoxTimeoutW");
							if (MessageBoxTimeout)
							{
								iRet = MessageBoxTimeout(hStopWnd, lpText,
											 lpstrCaption, uType, LANG_NEUTRAL, iTimeOut);
							}
							else
							{
								iRet = ::MessageBox(hStopWnd, lpText, lpstrCaption, uType);
							}
						}
					}
					DestroyWindow(hStopWnd);
					UnregisterClass(wndClass.lpszClassName, wndClass.hInstance);
				}
			}
			if (bFreeLibrary)
			{
				FreeLibrary(hModule);
				bFreeLibrary = FALSE;
				hModule = NULL;
			}
			return iRet;
		}

		//通常のメッセージボックス or ModuleがNULL
		if (iTimeOut == -1 || hModule == NULL)
		{
			iRet = ::MessageBox(hWnd, lpText, lpstrCaption, uType);
			if (bFreeLibrary)
			{
				FreeLibrary(hModule);
				bFreeLibrary = FALSE;
				hModule = NULL;
			}
			return iRet;
		}

		//Timeout
		TMessageBoxTimeout MessageBoxTimeout;
		MessageBoxTimeout = (TMessageBoxTimeout)GetProcAddress(hModule, "MessageBoxTimeoutW");
		if (MessageBoxTimeout)
		{
			iRet = MessageBoxTimeout(hWnd, lpText,
						 lpstrCaption, uType, LANG_NEUTRAL, iTimeOut);
		}
		else
		{
			iRet = ::MessageBox(hWnd, lpText, lpstrCaption, uType);
		}
		if (bFreeLibrary)
		{
			FreeLibrary(hModule);
			bFreeLibrary = FALSE;
			hModule = NULL;
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return iRet;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
CAboutDlg::CAboutDlg(CWnd* pParent /*=NULL*/) : CDialog(CAboutDlg::IDD)
{
	m_pParentWnd = pParent;
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
	pParentFrm = NULL;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_VOS_CLOSE_PROCESS, OnCloseVOSProc)
	ON_COMMAND(IDC_STATIC_LOGO_ABOUT, &CAboutDlg::OnDetailShow)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_SHOW_DEV_TOOLS, &CAboutDlg::OnBnClickedShowDevTools)
	ON_BN_CLICKED(IDC_CEF_VERSION, &CAboutDlg::OnBnClickedCefVersion)
END_MESSAGE_MAP()

void CAboutDlg::OnCloseVOSProc()
{
	if (theApp.CloseVOSProc())
		CDialog::OnOK();
}
void CAboutDlg::OnDetailShow()
{
	if (theApp.m_AppSettings.GetKeyCombination() != 0)
	{
		CString welcomeMsg;
		welcomeMsg.LoadString(IDS_STRING_WELCOME_MSG);
		CString strMsg;
		strMsg.Format(welcomeMsg, theApp.m_strThisAppName, theApp.m_strThisAppVersionString);
		::MessageBox(this->GetSafeHwnd(), strMsg, theApp.m_strThisAppName, MB_ICONINFORMATION | MB_OK);
		if (theApp.bValidKeyCombi())
		{
			SetDetailString(TRUE);
			if (theApp.IsShowDevTools())
			{
				GetDlgItem(IDC_SHOW_DEV_TOOLS)->EnableWindow(TRUE);
				GetDlgItem(IDC_SHOW_DEV_TOOLS)->ShowWindow(SW_SHOW);
			}
		}
	}
}

void CAboutDlg::SetDetailString(BOOL bEnableDetail)
{
	PROC_TIME(SetDetailString)

	CString strEditValue;
	strEditValue = theApp.m_strThisAppName + _T("\r\n");

	CString verstr;
	verstr.Format(_T("Version %s  "), (LPCTSTR)theApp.m_strThisAppVersionString);

	CString strTitle;
	strTitle.Format(_T("%s %s"), (LPCTSTR)theApp.m_strThisAppName, (LPCTSTR)verstr);
	this->SetWindowText(strTitle);
	strEditValue += verstr;

#ifndef _WIN64
	strEditValue += _T("WIN32(x86)\r\n");
#else  //WIN64
	strEditValue += _T("WIN64(x64)\r\n");
#endif //WIN64

	DWORD dwProcessId = 0;
	::GetWindowThreadProcessId(this->m_hWnd, &dwProcessId);

	CHandle hProcess;
	hProcess.Attach(::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SET_QUOTA, FALSE, dwProcessId));

	TCHAR memorysize[24] = _T("");
	unsigned long long iMem = theApp.GetMemoryUsageSize();
	::StrFormatByteSize(iMem, memorysize, 24);

	verstr.Format(_T("Memory usage:%s\r\n"), memorysize);
	strEditValue += verstr;
	verstr.Format(_T("GDI Objects:%d\r\n"), ::GetGuiResources(hProcess, GR_GDIOBJECTS));
	strEditValue += verstr;
	verstr.Format(_T("USER Objects:%d\r\n"), ::GetGuiResources(hProcess, GR_USEROBJECTS));
	strEditValue += verstr;

	HANDLE hSnapshot;
	INT nModule = 0;
	INT nThread = 0;

	if ((hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, dwProcessId)) != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 me32 = {sizeof(MODULEENTRY32)};
		THREADENTRY32 te32 = {sizeof(THREADENTRY32)};

		// モジュール数
		if (Module32First(hSnapshot, &me32))
		{
			do
			{
				nModule++;
			} while (Module32Next(hSnapshot, &me32));
		}
		// スレッド数
		if (Thread32First(hSnapshot, &te32))
		{
			do
			{
				if (te32.dwSize >= (FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(DWORD)) && te32.th32OwnerProcessID == dwProcessId)
					nThread++;
			} while (Thread32Next(hSnapshot, &te32));
		}
		CloseHandle(hSnapshot);
		verstr.Format(_T("Modules :%d\r\n"), nModule);
		strEditValue += verstr;
		verstr.Format(_T("Threads :%d\r\n"), nThread);
		strEditValue += verstr;
	}

	FILETIME ftMakeTime = {0}; // プロセスの作成時刻
	FILETIME ftExitTime = {0}; // プロセスの終了時刻
	FILETIME ftKernTime = {0}; // プロセスのカーネル動作時間
	FILETIME ftUserTime = {0}; // プロセスのユーザー動作時間
	GetProcessTimes(hProcess, &ftMakeTime, &ftExitTime, &ftKernTime, &ftUserTime);
	CString strTimeFmt;
	getTimeString(&ftMakeTime, TRUE, strTimeFmt);
	verstr.Format(_T("ProcessStart:%s\r\n"), (LPCTSTR)strTimeFmt);
	strEditValue += verstr;

	int iMinute = theApp.GetProcessRunningTime();
	verstr.Format(_T("ProcessRunningMinutes:%d\r\n"), iMinute);
	strEditValue += verstr;

	strEditValue += theApp.GetCefVersionStr();
	strEditValue += _T("\r\n");
	strEditValue += theApp.GetChromiumVersionStr();

	CString strOSVer;
	strOSVer = theApp.GetOSInfo();
	if (!strOSVer.IsEmpty())
		strEditValue += _T("\r\n") + strOSVer;

	strOSVer = theApp.GetOSKernelVersion();
	if (!strOSVer.IsEmpty())
		strEditValue += _T("\r\n") + strOSVer;

	DWORD dwVOSPCount = 0;
	CString strVOSProc = theApp.GetVOSProcessString(TRUE, &dwVOSPCount, bEnableDetail);
	if (!strVOSProc.IsEmpty())
	{
		strEditValue += _T("\r\n\r\n-----VOS Process Info-----\r\n");
		strEditValue += strVOSProc;
		CWnd* ptrW = NULL;
		ptrW = GetDlgItem(IDC_VOS_CLOSE_PROCESS);
		if (ptrW)
		{
			if (dwVOSPCount > 1)
			{
				ptrW->EnableWindow(TRUE);
				ptrW->ShowWindow(SW_SHOW);
			}
			else
			{
				ptrW->EnableWindow(FALSE);
				ptrW->ShowWindow(SW_SHOW);
			}
		}
	}
	else
	{
		CWnd* ptrW = NULL;
		ptrW = GetDlgItem(IDC_VOS_CLOSE_PROCESS);
		if (ptrW)
		{
			ptrW->EnableWindow(FALSE);
			ptrW->ShowWindow(SW_HIDE);
		}
	}

	CStringArray strATitle;
	CPtrArray ptrAWnd;
	CUIntArray ptrAPID;
	((CMainFrame*)theApp.m_pMainWnd)->Get_WindowInfoArrayWithPID(strATitle, ptrAWnd, ptrAPID);
	CString strPIDTitleTemp;
	CString strPIDTitle;
	unsigned long long iMemRen = 0;
	UINT RenPID = 0;
	for (int ii = 0; ii < strATitle.GetCount(); ii++)
	{
		memset(memorysize, 0x00, sizeof(memorysize));
		iMemRen = 0;
		RenPID = ptrAPID.GetAt(ii);
		iMemRen = theApp.GetMemoryUsageSizeFromPID(RenPID);
		::StrFormatByteSize(iMemRen, memorysize, 24);
		strPIDTitleTemp.Format(_T("[PID:%d] [%s] [%s]\r\n"), RenPID, memorysize, (LPCTSTR)strATitle.GetAt(ii));
		strPIDTitle += strPIDTitleTemp;
	}
	if (!strPIDTitle.IsEmpty())
	{
		strEditValue += _T("\r\n\r\n-----Renderer Process Info-----\r\n");
		strEditValue += strPIDTitle;
	}

	if (bEnableDetail)
	{
		strEditValue += _T("\r\n");
		CString strVer2;
		if (theApp.InVirtualEnvironment() == VE_THINAPP)
			strVer2 = theApp.GetVOSInfo();
		else if (theApp.InVirtualEnvironment() == VE_TURBO)
		{
			strVer2 = theApp.GetTurboVMInfo();
		}

		if (!strVer2.IsEmpty())
		{
			strEditValue += _T("\r\n");
			strEditValue += strVer2;
		}

		CString strURL = theApp.GetActivePageURL();
		if (!strURL.IsEmpty())
		{
			strEditValue += _T("\r\n-----ActivePageInfo-----\r\n");
			strEditValue += strURL;
		}

		CString strModules;
		strModules = theApp.GetAllModules();
		strModules.Replace(_T("\n"), _T("\r\n"));
		strEditValue += _T("\r\n") + strModules;

		if (theApp.m_AppSettings.IsEnableURLFilter())
		{
			if (theApp.m_CacheURLFilter_Deny.GetCount() > 0)
			{
				CString strFilterHeader;
				strFilterHeader.Format(_T("\r\n\r\n-----DomainFilter[Deny:%d]-----\r\n"), theApp.m_CacheURLFilter_Deny.GetCount());
				strEditValue += strFilterHeader;
				CString strContentLine;
				CString strContentData;

				POSITION pos1 = {0};
				pos1 = theApp.m_CacheURLFilter_Deny.GetStartPosition();
				while (pos1 != NULL)
				{
					void* ptr = NULL;
					theApp.m_CacheURLFilter_Deny.GetNextAssoc(pos1, strContentLine, ptr);
					if (!strContentLine.IsEmpty())
					{
						strContentData += strContentLine;
						strContentData += _T("\r\n");
					}
				}
				strEditValue += strContentData;
			}
			if (theApp.m_CacheURLFilter_Allow.GetCount() > 0)
			{
				CString strFilterHeader;
				strFilterHeader.Format(_T("\r\n\r\n-----DomainFilter[Allow:%d]-----\r\n"), theApp.m_CacheURLFilter_Allow.GetCount());
				strEditValue += strFilterHeader;
				CString strContentLine;
				CString strContentData;

				POSITION pos1 = {0};
				pos1 = theApp.m_CacheURLFilter_Allow.GetStartPosition();
				while (pos1 != NULL)
				{
					void* ptr = NULL;
					theApp.m_CacheURLFilter_Allow.GetNextAssoc(pos1, strContentLine, ptr);
					if (!strContentLine.IsEmpty())
					{
						strContentData += strContentLine;
						strContentData += _T("\r\n");
					}
				}
				strEditValue += strContentData;
			}
		}

		CString strSettingStr;
		strSettingStr.Format(_T("#----%s----\r\n"), _T("Chronos.conf"));
		strSettingStr += theApp.m_AppSettings.ExportTxt();
		strSettingStr.Replace(_T("\n"), _T("\r\n"));
		strEditValue += _T("\r\n") + strSettingStr;
	}
	CString strCEF_License;
	strCEF_License.LoadString(IDS_STRING_CEF_LIC);
	strCEF_License.Replace(_T("\n"), _T("\r\n"));
	strEditValue += strCEF_License;

#ifdef PROPRIETARY_CODECS
	CString strProprietaryCodecLicenses;
	strProprietaryCodecLicenses.LoadString(IDS_STRING_PROPRIETARY_CODEC_LICENSES);
	strProprietaryCodecLicenses.Replace(_T("\n"), _T("\r\n"));
	strEditValue += strProprietaryCodecLicenses;
#endif

	SetDlgItemText(IDC_EDIT1, strEditValue);

	theApp.WriteDebugTraceDateTime(strEditValue, DEBUG_LOG_TYPE_AC);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	BOOL bEnableDetail = FALSE;
	//詳細表示許可
	if (theApp.m_AppSettings.GetKeyCombination() == 0)
	{
		bEnableDetail = TRUE;
	}
	else
	{
		GetDlgItem(IDC_SHOW_DEV_TOOLS)->EnableWindow(FALSE);
		GetDlgItem(IDC_SHOW_DEV_TOOLS)->ShowWindow(SW_HIDE);
	}
	if (!theApp.IsShowDevTools())
	{
		GetDlgItem(IDC_SHOW_DEV_TOOLS)->EnableWindow(FALSE);
		GetDlgItem(IDC_SHOW_DEV_TOOLS)->ShowWindow(SW_HIDE);
	}

	SetDetailString(bEnableDetail);
	return FALSE;
}
void CAboutDlg::OnBnClickedOk()
{
	theApp.EmptyWorkingSetAll();
	CDialog::OnOK();
}
void CAboutDlg::OnBnClickedShowDevTools()
{
	theApp.ShowDevTools();
	theApp.EmptyWorkingSetAll();
	CDialog::OnOK();
}
void CAboutDlg::OnBnClickedCefVersion()
{
	CDialog::OnOK();
	theApp.CreateNewWindow(_T("chrome://version/"), TRUE);
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

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
