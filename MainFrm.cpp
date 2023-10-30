#include "stdafx.h"
#include "Sazabi.h"

#include "MainFrm.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)
BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SETTINGCHANGE()
	//}}AFX_MSG_MAP
	ON_COMMAND(WM_VIEW_INIT_OK, View_InitOK)
	ON_MESSAGE(WM_DELETE_WINDOW_LIST, DeleteWindowList)
	ON_MESSAGE(WM_NEWINSTANCE, OnNewInstance)
	ON_MESSAGE(WM_AUTH_DBL, OnDBLAuthChk)
	ON_MESSAGE(WM_ACTIVE_HEAD, OnActiveHead)
	ON_MESSAGE(WM_ACTIVE_TAIL, OnActiveTail)
	ON_MESSAGE(WM_SAVE_WND_M, OnSaveWindowListMSG)
	ON_COMMAND(WM_CLOSE_TIME_LIMIT, OnAppExitTimeLimit)
	ON_COMMAND(WM_APP_EXIT, OnAppExitEx)
	ON_COMMAND(WM_CLOSE_MAX_MEM, OnAppExitMaxMem)
	ON_WM_QUERYENDSESSION()

END_MESSAGE_MAP()

CMainFrame::CMainFrame()
{
	m_iTimerID = 0;
	m_iRecoveryTimerID = 0;
	m_hHookDlg_hwnd = 0;
	m_bAtomOpen = FALSE;
	m_bGlobalMsgFlg = FALSE;
	m_bGlobalMsgFlgMem = FALSE;
	m_bCleanUPFinish = FALSE;
	m_pActiveWindow = NULL;
	m_pPrevActiveWindow = NULL;
	m_iTabTimerID = 0;
	m_bTabTimerProcLock = FALSE;
}

CMainFrame::~CMainFrame()
{
}

LRESULT CMainFrame::OnDBLAuthChk(WPARAM wParam, LPARAM lParam)
{
	try
	{
		if (lParam)
		{
			INT_PTR dVal = lParam;
			if (dVal == 6340)
			{
				return 1;
			}
			return 0;
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return 0;
}

LRESULT CMainFrame::OnNewInstance(WPARAM nAtom, LPARAM lParam)
{
	try
	{
		m_bAtomOpen = TRUE;
		if (theApp.m_AppSettings.IsAdvancedLogMode())
		{
			CString logmsg;
			logmsg.Format(_T("MAIN_WND:0x%08p OnNewInstance"), theApp.SafeWnd(this));
			theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_GE);
		}
		CString strCommandLine;
		TCHAR szBuff[4096] = {0};
		if (nAtom)
		{
			if (::GlobalGetAtomName((ATOM)nAtom, szBuff, 4096) != 0)
			{
				strCommandLine = szBuff;
				ParseCommandLineAndNewWnd(strCommandLine);
				::GlobalDeleteAtom((ATOM)nAtom);
			}
		}
		else
		{
			ParseCommandLineAndNewWnd(strCommandLine);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return 0;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	((CMFCVisualManagerWindows*)CMFCVisualManager::GetInstance())->SetOfficeStyleMenus(FALSE);
	LOGFONT lf = {0};
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
	afxGlobalData.SetMenuFont(&lf, true);
	this->View_InitOK();
	return 0;
}

void CMainFrame::View_InitOK()
{
	PROC_TIME(View_InitOK)
	try
	{
		CClientDC dc(this);
		double dScaleDPI = 0.0;
		dScaleDPI = dc.GetDeviceCaps(LOGPIXELSX) / 96.0;
		theApp.m_ScaleDPI = dScaleDPI;

		ProgressDlg DlgMsgP(this);
		DlgMsgP.Create(MAKEINTRESOURCE(IDD_DIALOG1), this);
		DlgMsgP.ShowWindow(SW_SHOW);
		CString startingMsg;
		startingMsg.LoadString(IDS_STRING_STARTING_BROWSER);
		CString strDlgMsg;
		strDlgMsg.Format(startingMsg, theApp.m_strThisAppName);
		DlgMsgP.SetMsg(strDlgMsg);

		BOOL bTraceLog = FALSE;
		if (theApp.m_AppSettings.IsAdvancedLogMode())
		{
			bTraceLog = TRUE;
		}

		CString logmsg;
		this->SetWindowText(theApp.m_FrmWndClassName);
		::SetFocus(NULL);

		if (m_hAccelTable)
		{
			::DestroyAcceleratorTable(m_hAccelTable);
			m_hAccelTable = NULL;
		}

		this->ShowWindow(SW_HIDE);
		this->EnableWindow(FALSE);

		this->ParseCommandLineAndNewWnd(theApp.m_strAtomParam);

		if (!m_iTimerID)
		{
			m_iTimerID = (INT_PTR)this;
			this->SetTimer(m_iTimerID, 5 * 1000, 0);
		}
		if (!m_iRecoveryTimerID)
		{
			m_iRecoveryTimerID = (INT_PTR)this + 40;
			this->SetTimer(m_iRecoveryTimerID, 15 * 1000, 0);
		}

		if (theApp.m_bTabEnable_Init)
		{
			m_bTabTimerProcLock = FALSE;
			if (!m_iTabTimerID)
			{
				m_iTabTimerID = (INT_PTR)this + 48;
				this->SetTimer(m_iTabTimerID, 5 * 1000, 0);
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}
BOOL CMainFrame::CheckRecovery()
{
	if (theApp.m_strRecoveryFileFullPath.IsEmpty())
		return FALSE;
	BOOL bTraceLog = FALSE;
	if (theApp.m_AppSettings.IsAdvancedLogMode())
	{
		bTraceLog = TRUE;
	}

	//RecoveryFile Enum
	_wsetlocale(LC_ALL, _T("jpn"));
	CFileFind fnd;
	CString strPathName = theApp.m_strDBL_EXE_FolderPath;
	if (strPathName.IsEmpty()) return FALSE;

	if (!theApp.m_AppSettings.IsCrashRecovery())
		return FALSE;

	try
	{
		CStringArray strArrayURL;
		CString logmsg;
		if (bTraceLog)
		{
			logmsg.Format(_T("MAIN_WND:0x%08p CheckRecovery %s"), theApp.SafeWnd(this), (LPCTSTR)strPathName);
			theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_CL);
		}

		CString strChkFile;
		CString strDateTime;
		strPathName.TrimRight('\\');
		strPathName += _T("\\CSG_Recovery*.dat");

		BOOL bExecFlg = FALSE;
		if (fnd.FindFile(strPathName, 0))
		{
			int i = 1;
			while (i)
			{
				i = fnd.FindNextFile();
				strChkFile.Empty();
				// ファイル名が"."か".."の場合は次を検索
				if (fnd.IsDots())
					continue;

				// ファイル名取得
				CString strChkFile = theApp.m_strDBL_EXE_FolderPath;
				strChkFile.TrimRight('\\');
				strChkFile += _T("\\") + fnd.GetFileName();
				// フォルダだった場合、再帰呼び出しでそのフォルダを削除
				if (fnd.IsDirectory())
				{
					;
				}
				//ファイル
				else
				{
					//除外ファイル
					CString strFileName(fnd.GetFileName());
					CString FileExt;
					FileExt = strFileName.Mid(strFileName.ReverseFind('.'));
					//除外ファイル
					if (strFileName.CompareNoCase(_T("DBLC.exe")) == 0)
					{
						continue;
					}
					else if (strFileName.CompareNoCase(theApp.m_strRecoveryFileName) == 0)
					{
						continue;
					}
					else if (FileExt.CompareNoCase(_T(".dat")) == 0)
					{
						strFileName.MakeUpper();
						if (strFileName.Find(_T("CSG_RECOVERY")) == 0)
						{
							//check pid
							DWORD dPID = 0;
							strFileName.Replace(_T("CSG_RECOVERY"), _T(""));
							strFileName.Replace(_T(".DAT"), _T(""));
							if (!strFileName.IsEmpty())
							{
								dPID = _ttoi(strFileName);
								CString strProcName;
								strProcName = theApp.IsProcessExistsName(dPID);
								//存在しないプロセス
								if (strProcName.IsEmpty())
								{
									if (bTraceLog)
									{
										logmsg.Format(_T("MAIN_WND:0x%08p CheckRecovery_FIND %s"), theApp.SafeWnd(this), (LPCTSTR)strChkFile);
										theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_CL);
									}

									CStdioFile in;
									if (in.Open(strChkFile, CFile::modeRead | CFile::shareDenyNone))
									{
										CString strTemp;
										int iCntLine = 0;
										while (in.ReadString(strTemp))
										{
											iCntLine++;
											strTemp.TrimLeft();
											strTemp.TrimRight();
											if (iCntLine == 1)
												strDateTime = strTemp;
											else
											{
												CString strURL;
												CString strTitle;
												CStringArray strArr;
												SBUtil::Split(&strArr, strTemp, _T("\t"));
												if (strArr.GetCount() > 1)
												{
													strTitle = strArr.GetAt(0);
													strTitle.TrimLeft();
													strTitle.TrimRight();

													strURL = strArr.GetAt(1);
													strURL.TrimLeft();
													strURL.TrimRight();
												}
												else
												{
													strURL = strTemp;
													strURL.TrimLeft();
													strURL.TrimRight();
												}
												if (SBUtil::IsURL_HTTP(strURL))
												{
													strArrayURL.Add(strURL);
													bExecFlg = TRUE;
												}
											}
										}
										in.Close();
										//delete file
										//::DeleteFile(strChkFile);
										theApp.DeleteFileFix(strChkFile);
									}
									if (bExecFlg)
										break;
								}
							}
						}
					}
					else
						;
				}
			}
			fnd.Close();
		}

		if (!theApp.m_AppSettings.IsCrashRecovery())
			return FALSE;

		//リカバリー可能な場合
		if (bExecFlg)
		{
			//msg
			int iRet = 0;
			size_t iWindowCnt = 0;
			iWindowCnt = strArrayURL.GetSize();
			CString strMsg;
			CString strMsgFmt;
			strMsgFmt.LoadString(IDS_STRING_AUTO_RECOVERY);
			strMsg.Format(strMsgFmt, strDateTime, iWindowCnt);
			iRet = theApp.SB_MessageBox(NULL, strMsg, NULL, MB_ICONQUESTION | MB_YESNO | MB_TASKMODAL | MB_TOPMOST, TRUE);
			if (iRet == IDYES)
			{
				if (bTraceLog)
				{
					//restore
					logmsg.Format(_T("MAIN_WND:0x%08p CheckRecovery_RESTORE %s"), theApp.SafeWnd(this), (LPCTSTR)strChkFile);
					theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_CL);
				}
				CString strTemp;
				for (UINT i = 0; i < iWindowCnt; i++)
				{
					strTemp = strArrayURL.GetAt(i);
					strTemp.TrimLeft();
					strTemp.TrimRight();
					//新規ブラウザWindowの作成
					CreateNewWindow(strTemp);
				}
				return TRUE;
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return FALSE;
}
void CMainFrame::RestoreWnd(LPCTSTR lpFile)
{
	if (!lpFile) return;
	try
	{
		_wsetlocale(LC_ALL, _T("jpn"));
		CStdioFile in;
		if (in.Open(lpFile, CFile::modeRead | CFile::shareDenyNone))
		{
			CString strTemp;
			int iCntLine = 0;
			while (in.ReadString(strTemp))
			{
				iCntLine++;
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if (iCntLine == 1)
					; //strDateTime=strTemp;
				else
				{
					CString strURL;
					CString strTitle;
					CStringArray strArr;
					SBUtil::Split(&strArr, strTemp, _T("\t"));
					if (strArr.GetCount() > 1)
					{
						strTitle = strArr.GetAt(0);
						strTitle.TrimLeft();
						strTitle.TrimRight();

						strURL = strArr.GetAt(1);
						strURL.TrimLeft();
						strURL.TrimRight();
					}
					else
					{
						strURL = strTemp;
						strURL.TrimLeft();
						strURL.TrimRight();
					}
					if (SBUtil::IsURL_HTTP(strURL))
					{
						if (theApp.m_bTabEnable_Init)
						{
							//新規ブラウザWindowの作成(非アクティブ)
							CreateNewWindow(strURL, FALSE);
						}
						else
							CreateNewWindow(strURL, TRUE);
					}
				}
			}
			in.Close();
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}
BOOL CMainFrame::CreateNewWindow(LPCTSTR pURL, BOOL bActive)
{
	PROC_TIME(CreateNewWindow)

	BOOL bRet = FALSE;
	if (!pURL) return FALSE;
	CString strURL;
	strURL = pURL;

	if (!SBUtil::IsURL_HTTP(strURL))
	{
		return FALSE;
	}
	try
	{
		DWORD dwFlags = 0;
		if (bActive)
			dwFlags = 0;
		else
			dwFlags = NWMF_FORCETAB;
		CChildView* pCreateView = NULL;
		if (::IsWindow(theApp.m_pMainWnd->m_hWnd))
		{
			pCreateView = this->NewBrowserWindow(dwFlags);
			if (pCreateView)
			{
				bRet = TRUE;
				pCreateView->Navigate(strURL);

				//Tabなし
				if (!theApp.m_bTabEnable_Init)
				{
					//強制オプションの状態を適用
					CString OptionParam;
					OptionParam = theApp.m_AppSettings.GetEnforceInitParam();
					if (!OptionParam.IsEmpty())
					{
						CBrowserFrame* pFrame = NULL;
						if (theApp.IsWnd(pCreateView->m_pwndFrame))
						{
							pFrame = (CBrowserFrame*)pCreateView->m_pwndFrame;
							if (OptionParam.CompareNoCase(_T("/MIN")) == 0)
							{
								pFrame->ShowWindow(SW_MINIMIZE);
							}
							else if (OptionParam.CompareNoCase(_T("/MAX")) == 0)
							{
								pFrame->ShowWindow(SW_MAXIMIZE);
							}
							else if (OptionParam.CompareNoCase(_T("/NORMAL")) == 0)
							{
								pFrame->ShowWindow(SW_NORMAL);
							}
							else
							{
								;
							}
						}
					}
				}
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return bRet;
}

BOOL CMainFrame::ParseCommandLineAndNewWnd(CString strCommandLine)
{
	PROC_TIME(ParseCommandLineAndNewWnd)

	CString logmsg;
	BOOL bTraceLog = FALSE;
	if (theApp.m_AppSettings.IsAdvancedLogMode())
	{
		bTraceLog = TRUE;
	}

	if (bTraceLog)
	{
		logmsg.Format(_T("MAIN_WND:0x%08p ParseCommandLineAndNewWnd [%s]"), theApp.SafeWnd(this), (LPCTSTR)strCommandLine);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_GE);
	}

	CStringArray strA;
	SBUtil::Split(&strA, strCommandLine, _T("|@@|"));

	CString str1 = strCommandLine;
	CString str2;

	CString CommandParam;
	CString OptionParam;

	DWORD dwFlags = 0;
	CChildView* pCreateView = NULL;

	if (strA.GetCount() > 1)
	{
		str1 = strA.GetAt(0);
		str2 = strA.GetAt(1);
	}

	if (!str1.IsEmpty())
	{
		//URLかFilePathの場合は、強制的にCommandParamとする。
		if (SBUtil::IsURL(str1))
		{
			CommandParam = str1;
		}
		//filepath
		else if (str1.Find(_T(":")) == 1)
		{
			CommandParam = str1;
		}
		else if (str1.Find(_T("-")) == 0)
		{
			OptionParam = str1;
		}
		else if (str1.Find(_T("/")) == 0)
		{
			OptionParam = str1;
		}
	}

	if (!str2.IsEmpty())
	{
		//URLかFilePathの場合は、強制的にCommandParamとする。
		if (SBUtil::IsURL(str2))
		{
			CommandParam = str2;
		}
		//filepath
		else if (str2.Find(_T(":")) == 1)
		{
			CommandParam = str2;
		}
		else if (str2.Find(_T("-")) == 0)
		{
			OptionParam = str2;
		}
		else if (str2.Find(_T("/")) == 0)
		{
			OptionParam = str2;
		}
	}
	if (bTraceLog)
	{
		logmsg.Format(_T("MAIN_WND:0x%08p ParseCommandLineAndNewWnd CommandParam[%s] OptionParam[%s]"), theApp.SafeWnd(this), (LPCTSTR)CommandParam, (LPCTSTR)OptionParam);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_GE);
	}

	//クラッシュリカバリー
	if (!theApp.m_bNewInstanceParam)
	{
		if (this->CheckRecovery())
		{
			return TRUE;
		}
	}

	if (theApp.m_bTabEnable_Init)
	{
		if (OptionParam.CompareNoCase(_T("/View")) == 0)
		{
			dwFlags = NWMF_SUGGESTWINDOW;
		}
	}
	//新規ブラウザWindowの作成
	pCreateView = NewBrowserWindow(dwFlags);
	if (!pCreateView)
	{
		return FALSE;
	}

	CBrowserFrame* pFrame = NULL;
	if (theApp.IsWnd(pCreateView->m_pwndFrame))
	{
		pFrame = (CBrowserFrame*)pCreateView->m_pwndFrame;
	}
	else
	{
		return FALSE;
	}

	//空の場合は、ホームを開く。
	if (CommandParam.IsEmpty())
	{
		pCreateView->GoHomeFirstCall();
	}
	else
	{
		//URLファイルの判定
		CString strFileNameTemp;
		CString FileExt;
		CString strURL;
		strFileNameTemp = CommandParam;
		FileExt = strFileNameTemp.Mid(strFileNameTemp.ReverseFind('.'));
		if (FileExt.CompareNoCase(_T(".url")) == 0 || FileExt.CompareNoCase(_T(".website")) == 0)
		{
			SBUtil::GetInternetShortcutUrl(CommandParam, strURL);
			if (!strURL.IsEmpty())
				pCreateView->Navigate(strURL);
		}
		else
		{
			if (OptionParam.CompareNoCase(_T("/View")) == 0)
			{
				pFrame->m_bOLEViewer = TRUE;
			}
			pCreateView->Navigate(CommandParam);
		}
	}

	//ThinDirectやコマンドラインからの起動の場合は、開いたWindowを必ずアクティブ(前面)に表示する。
	if (m_bAtomOpen)
	{
		SBUtil::SetAbsoluteForegroundWindow(pFrame->m_hWnd, TRUE);
		m_bAtomOpen = FALSE;
	}

	BOOL minMode = FALSE;
	if (OptionParam.CompareNoCase(_T("/MIN")) == 0)
	{
		minMode = TRUE;
		pFrame->ShowWindow(SW_MINIMIZE);
	}
	else if (OptionParam.CompareNoCase(_T("/MAX")) == 0)
	{
		pFrame->ShowWindow(SW_MAXIMIZE);
	}
	else if (OptionParam.CompareNoCase(_T("/NORMAL")) == 0)
	{
		pFrame->ShowWindow(SW_NORMAL);
	}
	else if (OptionParam.CompareNoCase(_T("/View")) == 0)
	{
		theApp.HideRebar(pFrame);
		pFrame->ShowWindow(SW_NORMAL);
		pFrame->m_bOLEViewer = TRUE;
	}
	else
	{
		;
	}
	//Newアクティブ(前面)に表示する。
	if (theApp.m_bNewInstanceParam && !minMode)
		SBUtil::SetAbsoluteForegroundWindow(pFrame->m_hWnd, TRUE);

	return TRUE;
}

BOOL CMainFrame::OnQueryEndSession()
{
	CleanUP();
	theApp.m_bShutdownFlg = TRUE;
	if (!CFrameWnd::OnQueryEndSession())
		return FALSE;
	return TRUE;
}
void CMainFrame::OnEndSession(BOOL bEnable)
{
	CleanUP();
	theApp.m_bShutdownFlg = TRUE;
	CFrameWnd::OnEndSession(bEnable);
}

void CMainFrame::CleanUP()
{
	if (m_bCleanUPFinish) return;
	m_bCleanUPFinish = TRUE;

	try
	{
		if (m_iTimerID)
		{
			this->KillTimer(m_iTimerID);
			m_iTimerID = 0;
		}
		if (m_iRecoveryTimerID)
		{
			this->KillTimer(m_iRecoveryTimerID);
			m_iRecoveryTimerID = 0;
		}

		if (m_iTabTimerID)
		{
			this->KillTimer(m_iTabTimerID);
			m_iTabTimerID = 0;
			m_bTabTimerProcLock = FALSE;
		}
		//2重起動を許可する。終了中のため。
		this->SetWindowText(theApp.m_strThisAppName);

		if (m_colBrowserWindows.GetCount() > 0)
		{
			POSITION pos1 = {0};
			CBrowserFrame* ptd = NULL;
			for (pos1 = m_colBrowserWindows.GetHeadPosition(); pos1 != NULL;)
			{
				ptd = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
				if (ptd != NULL)
				{
					if (::IsWindow(ptd->m_hWnd))
					{
						ptd->PostWM_CLOSE();
					}
				}
				ptd = NULL;
				m_colBrowserWindows.GetNext(pos1);
			}
		}
		//RecoveryFileを削除する。
		if (!theApp.m_strRecoveryFileFullPath.IsEmpty())
		{
			if (!theApp.m_bAbortFlg)
			{
				//::DeleteFile(theApp.m_strRecoveryFileFullPath);
				theApp.DeleteFileFix(theApp.m_strRecoveryFileFullPath);
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

void CMainFrame::OnDestroy()
{
	CleanUP();
	if (theApp.m_AppSettings.IsAdvancedLogMode())
	{
		theApp.WriteDebugTraceDateTime(_T("MainFrame::OnDestroy"), DEBUG_LOG_TYPE_CL);
	}
	CFrameWnd::OnDestroy();
	theApp.m_bToBeShutdown = TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
	WNDCLASS wc = {0};

	wc.lpfnWndProc = AfxWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = theApp.m_hInstance;

	CIconHelper ICoHelper;
	ICoHelper = theApp.LoadIcon(IDR_MAINFRAME);
	wc.hIcon = ICoHelper;

	CCursorHelper ICurHelper;
	ICurHelper = theApp.LoadStandardCursor(IDC_ARROW);
	wc.hCursor = ICurHelper;
	wc.style = CS_DBLCLKS;
	wc.hbrBackground = 0;
	wc.lpszMenuName = NULL;

	wc.lpszClassName = theApp.m_FrmWndClassName;
	AfxRegisterClass(&wc);

	cs.lpszClass = theApp.m_FrmWndClassName;
	if (theApp.m_AppSettings.IsAdvancedLogMode())
	{
		CString logmsg;
		logmsg.Format(_T("MainFrame::PreCreateWindow szClassName[%s]"), theApp.m_FrmWndClassName);
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_DE);
	}
	return TRUE;
}

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG
typedef int(__stdcall* TMessageBoxTimeout)(HWND, LPCTSTR, LPCTSTR, UINT, WORD, DWORD);
void CMainFrame::ShowTimeoutMessageBox(HWND hWnd, LPCTSTR strMsg, int iType, int iTimeOut)
{
	BOOL bFreeLibrary = FALSE;
	HMODULE hModule = {0};
	hModule = ::GetModuleHandle(_T("user32.dll"));
	if (!hModule)
	{
		hModule = ::LoadLibrary(_T("user32.dll"));
		if (hModule)
			bFreeLibrary = TRUE;
	}

	if (hModule)
	{
		TMessageBoxTimeout MessageBoxTimeout;
		MessageBoxTimeout = (TMessageBoxTimeout)GetProcAddress(hModule, "MessageBoxTimeoutW");
		if (MessageBoxTimeout)
		{
			MessageBoxTimeout(hWnd, strMsg,
					  theApp.m_strThisAppName, iType, LANG_NEUTRAL, iTimeOut);
		}
		else
		{
			::MessageBox(hWnd, strMsg, theApp.m_strThisAppName, iType);
		}
		if (bFreeLibrary)
		{
			FreeLibrary(hModule);
			bFreeLibrary = FALSE;
			hModule = NULL;
		}
	}
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	if (m_iTimerID == nIDEvent)
	{
		CheckBrowserWnd();

		//check max limit
		unsigned long long iMemSize = theApp.GetMemoryUsageSize();
		size_t iMemL = theApp.m_AppSettings.GetMemoryUsageLimit();
		CString logmsg;

		if (iMemSize > (unsigned long long)iMemL * 1024 * 1024)
		{
			//一旦ワーキングセットをクリアして開放してしまう。
			theApp.EmptyWorkingSetAll();
			return;
		}
		//起動時間制限
		if (theApp.m_AppSettings.IsEnableRunningTime())
		{
			int iLimitTimeBase = 0;
			iLimitTimeBase = theApp.m_AppSettings.GetRunningLimitTime();
			int iLimitTimeCurrent = 0;
			iLimitTimeCurrent = theApp.GetProcessRunningTime();
			if (iLimitTimeCurrent >= iLimitTimeBase)
			{
				PostMessage(WM_COMMAND, WM_CLOSE_TIME_LIMIT, 0);
			}
		}
	}
	else if (m_iRecoveryTimerID == nIDEvent)
	{
		SaveWindowList(theApp.m_strRecoveryFileFullPath);
		//theApp.DeleteDirectoryTempFolder(theApp.m_strDBL_EXE_FolderPath);
	}
	else if (m_iTabTimerID == nIDEvent)
	{
		if (m_bTabTimerProcLock == FALSE)
		{
			m_bTabTimerProcLock = TRUE;
			TabWindowChk();
			m_bTabTimerProcLock = FALSE;
		}
	}
	CFrameWnd::OnTimer(nIDEvent);
}

UINT CMainFrame::GetWindowCount()
{
	UINT iRet = 0;
	try
	{
		POSITION pos1 = {0};
		CBrowserFrame* ptd = NULL;
		for (pos1 = m_colBrowserWindows.GetHeadPosition(); pos1 != NULL;)
		{
			ptd = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
			if (theApp.IsWnd(ptd))
			{
				iRet++;
			}
			ptd = NULL;
			m_colBrowserWindows.GetNext(pos1);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return iRet;
}
CBrowserFrame* CMainFrame::GetBrowserFrameFromChildWnd(HWND hWnd)
{
	if (hWnd == NULL)
		return NULL;
	if (!IsWindow(hWnd))
		return NULL;
	POSITION pos1 = {0};
	CBrowserFrame* pFrm = NULL;
	CChildView* pView = NULL;
	HWND pParent = NULL;
	pParent = ::GetParent(hWnd);
	CWnd* pCWnd = CWnd::FromHandle(hWnd);
	if (IsWindow(pParent))
	{
		for (;;)
		{
			pFrm = NULL;
			for (pos1 = m_colBrowserWindows.GetHeadPosition(); pos1 != NULL;)
			{
				pFrm = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
				if (theApp.IsWnd(pFrm))
				{
					if (pParent == pFrm->m_hWnd || pFrm->IsChild(pCWnd))
					{
						return pFrm;
					}
				}
				pFrm = NULL;
				m_colBrowserWindows.GetNext(pos1);
			}
			break;
		}
	}
	return NULL;
}

void CMainFrame::CheckBrowserWnd()
{
	try
	{
		POSITION pos1 = {0};
		CBrowserFrame* ptd = NULL;
		int iHideTabWndCnt = 0;
		for (pos1 = m_colBrowserWindows.GetHeadPosition(); pos1 != NULL;)
		{
			ptd = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
			if (!theApp.IsWnd(ptd))
			{
				m_colBrowserWindows.RemoveAt(pos1);
				break;
			}
			ptd = NULL;
			m_colBrowserWindows.GetNext(pos1);
		}

		if (m_colBrowserWindows.GetCount() == 0)
		{
			//2重起動を許可する。終了中のため。
			this->SetWindowText(theApp.m_strThisAppName);
			PostMessage(WM_CLOSE);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}
LRESULT CMainFrame::DeleteWindowList(WPARAM wParam, LPARAM lParam)
{
	try
	{
		CBrowserFrame* Target = (CBrowserFrame*)lParam;
		if (Target)
		{
			POSITION pos1 = {0};
			CBrowserFrame* ptd = NULL;
			for (pos1 = m_colBrowserWindows.GetTailPosition(); pos1 != NULL;)
			{
				ptd = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
				if (ptd == Target)
				{
					m_colBrowserWindows.RemoveAt(pos1);
					break;
				}
				ptd = NULL;
				m_colBrowserWindows.GetPrev(pos1);
			}
			if (m_colBrowserWindows.GetCount() == 0)
			{
				if (theApp.m_AppSettings.IsAdvancedLogMode())
				{
					CString logmsg;
					logmsg.Format(_T("DeleteWindowList:%s"), _T("WM_CLOSE"));
					theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_CL);
				}
				//2重起動を許可する。終了中のため。
				this->SetWindowText(theApp.m_strThisAppName);
				PostMessage(WM_CLOSE);
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMainFrame::SetWindowPlacementFrm(WINDOWPLACEMENT tFramePlacementMFrm, CBrowserFrame* Target)
{
	try
	{
		//Tabなしは、この後の処理を行う必要なし。
		if (!theApp.m_bTabEnable_Init)
			return;
		//tab専用の処理
		CRect rcResult(0, 0, 0, 0);
		if (tFramePlacementMFrm.showCmd == SW_MAXIMIZE)
		{
			CRect rcNormal(tFramePlacementMFrm.rcNormalPosition);
			CRect rcDesktop;
			HWND hWnd = {0};
			hWnd = Target->m_hWnd;
			if (hWnd == NULL)
				hWnd = ::GetDesktopWindow();
			SBUtil::GetMonitorWorkRect(hWnd, &rcDesktop);
			//Win10の7px調整
			if (theApp.m_iWinOSVersion >= 100)
			{
				CRect rectAdj;
				rcDesktop.top -= 9;
				rcDesktop.left -= 9;
				rcDesktop.right += 9;
				rcDesktop.bottom += 9;
			}
			rcResult = rcDesktop;
		}
		else if (tFramePlacementMFrm.showCmd == SW_NORMAL)
		{
			rcResult = tFramePlacementMFrm.rcNormalPosition;
		}
		if (rcResult.Width() > 100)
		{
			POSITION pos1 = {0};
			CBrowserFrame* ptd = NULL;
			for (pos1 = m_colBrowserTabList.GetHeadPosition(); pos1 != NULL;)
			{
				ptd = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
				if (ptd)
				{
					if (Target != ptd)
					{
						if (!theApp.IsWndVisible(ptd->m_hWnd))
						{
							::SetWindowPos(ptd->GetSafeHwnd(), 0, rcResult.left, rcResult.top,
								       rcResult.Width(), rcResult.Height(),
								       SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
						}
					}
				}
				ptd = NULL;
				m_colBrowserTabList.GetNext(pos1);
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}
void CMainFrame::Get_TabWindowInfoArray_hWnd(CPtrArray& ptrAWnd)
{
	try
	{
		ptrAWnd.RemoveAll();
		POSITION pos1 = {0};
		CBrowserFrame* ptd = NULL;
		for (pos1 = m_colBrowserTabList.GetHeadPosition(); pos1 != NULL;)
		{
			ptd = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (ptd)
			{
				ptrAWnd.Add(ptd->m_hWnd);
			}
			ptd = NULL;
			m_colBrowserTabList.GetNext(pos1);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}
void CMainFrame::HideOtherTabWindows(HWND hwndExclude)
{
	try
	{
		POSITION pos1 = {0};
		CBrowserFrame* ptd = NULL;
		LONG dwExStyle = 0;
		for (pos1 = m_colBrowserTabList.GetHeadPosition(); pos1 != NULL;)
		{
			ptd = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (ptd)
			{
				if (ptd->m_hWnd != hwndExclude && theApp.IsWndVisible(ptd->m_hWnd))
				{
					theApp.HideWnd(ptd->m_hWnd);
				}
			}
			ptd = NULL;
			m_colBrowserTabList.GetNext(pos1);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

void CMainFrame::Get_TabWindowInfoArray(CStringArray& strATitle, CPtrArray& ptrAWnd)
{
	try
	{
		strATitle.RemoveAll();
		ptrAWnd.RemoveAll();
		POSITION pos1 = {0};
		CBrowserFrame* ptd = NULL;
		CString strTitle;
		for (pos1 = m_colBrowserTabList.GetHeadPosition(); pos1 != NULL;)
		{
			strTitle.Empty();
			ptd = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (ptd)
			{
				strTitle = ptd->m_wndView.m_strTitle;
				strTitle.TrimLeft();
				strTitle.TrimRight();
				if (strTitle.IsEmpty())
				{
					if (ptd->m_bDownloadBlankPage)
						strTitle.LoadString(ID_DEFAULT_FILE_DOWNLOAD_WINDOW_TITLE);
					else
						strTitle = _T("New Tab");
				}
				strATitle.Add(strTitle);
				ptrAWnd.Add(ptd->m_hWnd);
			}
			ptd = NULL;
			m_colBrowserTabList.GetNext(pos1);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}
CString CMainFrame::Get_TabWindowInfo_Title(HWND hWnd)
{
	CString strRet;
	if (!hWnd) return strRet;
	try
	{
		POSITION pos1 = {0};
		CBrowserFrame* ptd = NULL;
		for (pos1 = m_colBrowserTabList.GetHeadPosition(); pos1 != NULL;)
		{
			ptd = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (ptd)
			{
				if (ptd->m_hWnd == hWnd)
				{
					strRet = ptd->m_wndView.m_strTitle;
					strRet.TrimLeft();
					strRet.TrimRight();
					if (strRet.IsEmpty())
					{
						if (ptd->m_bDownloadBlankPage)
							strRet.LoadString(ID_DEFAULT_FILE_DOWNLOAD_WINDOW_TITLE);
						else
							strRet = _T("New Tab");
					}
					break;
				}
			}
			ptd = NULL;
			m_colBrowserTabList.GetNext(pos1);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return strRet;
}

INT CMainFrame::Get_TabWindowIndex(CBrowserFrame* Target)
{
	if (!Target) return 0;
	UINT iRet = 0;
	try
	{
		POSITION pos1 = {0};
		CBrowserFrame* ptd = NULL;
		for (pos1 = m_colBrowserTabList.GetHeadPosition(); pos1 != NULL;)
		{
			ptd = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (ptd == Target)
			{
				return iRet;
			}
			else
			{
				iRet++;
			}
			ptd = NULL;
			m_colBrowserTabList.GetNext(pos1);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return -1;
}

UINT CMainFrame::Get_TabWindowCount()
{
	UINT iRet = 0;
	try
	{
		POSITION pos1 = {0};
		CBrowserFrame* ptd = NULL;
		for (pos1 = m_colBrowserTabList.GetHeadPosition(); pos1 != NULL;)
		{
			ptd = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (theApp.IsWnd(ptd))
			{
				iRet++;
			}
			ptd = NULL;
			m_colBrowserTabList.GetNext(pos1);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return iRet;
}

void CMainFrame::Add_TabWindow(CBrowserFrame* Target)
{
	try
	{
		if (Target)
		{
			if (Get_TabWindowIndex(Target) < 0)
			{
				m_colBrowserTabList.AddTail(Target);
				//Favicon
				CIconHelper ICoHelper;
				ICoHelper = theApp.LoadIcon(IDI_ICON2);
				theApp.m_imgFavIcons.Add(ICoHelper);

				TabWindowMsgBSend(TWNT_ORDER, Target->GetSafeHwnd());
			}
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

void CMainFrame::Tab_HScrollSync(HWND hWnd, DWORD dw1, DWORD dw2, DWORD dw3)
{
	try
	{
		POSITION pos1 = {0};
		CBrowserFrame* ptdActive = NULL;
		ptdActive = theApp.GetActiveBFramePtr();
		CBrowserFrame* ptd = NULL;
		for (pos1 = m_colBrowserTabList.GetHeadPosition(); pos1 != NULL;)
		{
			ptd = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (theApp.IsWnd(ptd))
			{
				if (hWnd != ptd->GetSafeHwnd())
				{
					if (ptd->m_cTabWnd)
					{
						if (ptd->m_cTabWnd->GetHwnd() != hWnd)
						{
							if (theApp.SafeWnd(ptd->m_cTabWnd->m_hwndTab))
							{
								HWND hwndUpDown = {0};
								DWORD nScrollPos = {0};
								hwndUpDown = ::FindWindowEx(ptd->m_cTabWnd->m_hwndTab, NULL, UPDOWN_CLASS, 0); // タブ内の Up-Down コントロール
								if (hwndUpDown != NULL)
								{
									DWORD nScrollPos = {0};
									nScrollPos = LOWORD(dw2);
									::SendMessage(ptd->m_cTabWnd->m_hwndTab, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, nScrollPos), 0);
								}
							}
						}
					}
				}
			}
			ptd = NULL;
			m_colBrowserTabList.GetNext(pos1);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}
void CMainFrame::TabWindowMsgBSend(int iCommand, HWND hWnd)
{
	try
	{
		POSITION pos1 = {0};
		CBrowserFrame* ptdActive = NULL;
		ptdActive = theApp.GetActiveBFramePtr();
		CBrowserFrame* ptd = NULL;
		for (pos1 = m_colBrowserTabList.GetHeadPosition(); pos1 != NULL;)
		{
			ptd = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (theApp.IsWnd(ptd))
			{
				if (ptd->IsWindowEnabled())
				{
					DWORD_PTR dwResult = {0};
					if (iCommand == TWNT_REFRESH)
					{
						::SendMessageTimeout(ptd->m_hWnd, MYWM_TAB_WINDOW_NOTIFY, iCommand,
								     (LPARAM)(ptdActive == ptd) ? TRUE : NULL, SMTO_NORMAL, 5000, &dwResult);
					}
					else if (iCommand == TWNT_WNDPL_ADJUST)
					{
						if (hWnd == ptd->GetSafeHwnd())
						{
							::SendMessageTimeout(ptd->m_hWnd, MYWM_TAB_WINDOW_NOTIFY, iCommand,
									     (LPARAM)NULL, SMTO_NORMAL, 5000, &dwResult);
							break;
						}
					}
					else if (iCommand == TWNT_DEL)
					{
						if (hWnd == ptd->GetSafeHwnd())
						{
							::SendMessageTimeout(ptd->m_hWnd, MYWM_TAB_WINDOW_NOTIFY, iCommand,
									     (LPARAM)hWnd, SMTO_NORMAL, 5000, &dwResult);
							break;
						}
					}
					else if (iCommand == TWNT_ORDER)
					{
						if (hWnd == ptd->GetSafeHwnd())
						{
							::SendMessageTimeout(ptd->m_hWnd, MYWM_TAB_WINDOW_NOTIFY, iCommand,
									     (LPARAM)ptd->GetSafeHwnd(), SMTO_NORMAL, 5000, &dwResult);
							break;
						}
					}
					else
					{
						::SendMessageTimeout(ptd->m_hWnd, MYWM_TAB_WINDOW_NOTIFY, iCommand,
								     (LPARAM)NULL, SMTO_NORMAL, 5000, &dwResult);
					}
				}
			}
			ptd = NULL;
			m_colBrowserTabList.GetNext(pos1);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

void CMainFrame::Delete_TabWindow(CBrowserFrame* Target)
{
	try
	{
		size_t iDelIConIndex = 0;
		iDelIConIndex = m_colBrowserTabList.GetCount();
		POSITION pos1 = {0};
		CBrowserFrame* ptd = NULL;
		for (pos1 = m_colBrowserTabList.GetTailPosition(); pos1 != NULL;)
		{
			iDelIConIndex--;
			ptd = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (ptd == Target)
			{
				m_colBrowserTabList.RemoveAt(pos1);
				theApp.m_imgFavIcons.Remove((int)iDelIConIndex);
				break;
			}
			ptd = NULL;
			m_colBrowserTabList.GetPrev(pos1);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return;
}
BOOL CMainFrame::SetActiveFramePtr(CBrowserFrame* pTarget)
{
	try
	{
		if (!theApp.IsWnd(m_pActiveWindow))
		{
			m_pActiveWindow = pTarget;
			return TRUE;
		}
		if (m_pActiveWindow != pTarget)
		{
			if (m_pActiveWindow && m_pActiveWindow->m_cTabWnd)
				m_pPrevActiveWindow = m_pActiveWindow;

			UINT iPID_Prev = 0;
			UINT iPID_New = 0;
			if (m_pActiveWindow)
			{
				iPID_Prev = m_pActiveWindow->m_RendererPID;
			}
			if (pTarget)
			{
				iPID_New = pTarget->m_RendererPID;
			}
			if (iPID_Prev > 0 && iPID_New > 0)
			{
				if (iPID_Prev != iPID_New)
				{
					//低
					if (!theApp.SetPriority(iPID_Prev, IDLE_PRIORITY_CLASS))
					{
						m_pActiveWindow->m_RendererPID = 0;
						if (m_pActiveWindow)
						{
							m_pActiveWindow->m_wndView.ReSetRendererPID();
						}
					}
					//中
					if (!theApp.SetPriority(iPID_New, NORMAL_PRIORITY_CLASS))
					{
						pTarget->m_RendererPID = 0;
						if (pTarget)
						{
							pTarget->m_wndView.ReSetRendererPID();
						}
					}
				}
			}
			m_pActiveWindow = pTarget;
			return TRUE;
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return FALSE;
}
CBrowserFrame* CMainFrame::GetNextGenerationActiveWindow(CBrowserFrame* pTarget)
{
	CBrowserFrame* pBFrame = NULL;
	__try
	{
		CBrowserFrame* pFrmActiveNow = NULL;
		pFrmActiveNow = theApp.GetActiveBFramePtr();
		BOOL bCloseWinActiveWin = FALSE;
		if (pFrmActiveNow != NULL && pFrmActiveNow == pTarget)
			bCloseWinActiveWin = TRUE;

		//閉じる画面とアクティブ画面が一致しない場合は、アクティブウインドウを変更しない。
		if (!bCloseWinActiveWin)
		{
			if (theApp.IsWnd(pFrmActiveNow))
				return pFrmActiveNow;
		}

		if (theApp.IsWnd(m_pPrevActiveWindow) &&
		    !theApp.IsWndVisible(m_pPrevActiveWindow->m_hWnd) &&
		    !m_pPrevActiveWindow->m_bDownloadBlankPage &&
		    m_pPrevActiveWindow != pTarget)
		{
			return m_pPrevActiveWindow;
		}

		//閉じるタブの右側(次)
		CBrowserFrame* pRet = NULL;
		POSITION pos1 = {0};
		CBrowserFrame* pFrm = NULL;
		CBrowserFrame* pFrmHead = NULL;
		CBrowserFrame* pFrmTail = NULL;
		CBrowserFrame* pFrmTaPrev = NULL;
		CBrowserFrame* pFrmTaNext = NULL;

		//先頭を入れておく
		pos1 = m_colBrowserTabList.GetHeadPosition();
		if (pos1)
		{
			pFrm = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (theApp.IsWnd(pFrm))
			{
				pFrmHead = pFrm;
			}
		}

		//最後尾を入れておく
		pos1 = m_colBrowserTabList.GetTailPosition();
		if (pos1)
		{
			pFrm = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (theApp.IsWnd(pFrm))
			{
				pFrmTail = pFrm;
			}
		}
		BOOL bFind = FALSE;
		pFrm = NULL;
		for (pos1 = m_colBrowserTabList.GetHeadPosition(); pos1 != NULL;)
		{
			pFrm = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (theApp.IsWnd(pFrm))
			{
				//一致した。
				if (pTarget == pFrm)
				{
					bFind = TRUE;
				}
				else
				{
					if (bFind == TRUE)
					{
						pFrmTaNext = pFrm;
						break;
					}
					else
						pFrmTaPrev = pFrm;
				}
			}
			pFrm = NULL;
			m_colBrowserTabList.GetNext(pos1);
		}
		//ターゲットの次が無い。
		if (pFrmTaNext == NULL)
		{
			//ターゲットの前が無い
			if (pFrmTaPrev == NULL)
			{
				pRet = pFrmTail;
			}
			else
			{
				pRet = pFrmTaPrev;
			}
		}
		else
			pRet = pFrmTaNext;

		if (pRet == NULL || pRet == pTarget)
			pRet = pFrmHead;
		return pRet;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return pBFrame;
}

void CMainFrame::CloseTabWindowLeft(CBrowserFrame* pTarget)
{
	try
	{
		POSITION pos1 = {0};
		CBrowserFrame* pFrm = NULL;
		CPtrArray ptrAWnd;
		for (pos1 = m_colBrowserTabList.GetHeadPosition(); pos1 != NULL;)
		{
			pFrm = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (theApp.IsWnd(pFrm))
			{
				//一致した。
				if (pTarget == pFrm)
				{
					break;
				}
				else
				{
					ptrAWnd.Add(pFrm);
				}
			}
			pFrm = NULL;
			m_colBrowserTabList.GetNext(pos1);
		}
		if (ptrAWnd.GetCount() > 0)
		{
			for (int i = 0; i < ptrAWnd.GetCount(); i++)
			{
				CBrowserFrame* pFrmPtr = NULL;
				pFrmPtr = (CBrowserFrame*)ptrAWnd.GetAt(i);
				if (theApp.IsWnd(pFrmPtr))
				{
					pFrmPtr->OnWClose();
				}
			}
		}
		return;
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return;
}
void CMainFrame::CloseTabWindowRight(CBrowserFrame* pTarget)
{
	try
	{
		POSITION pos1 = {0};
		CBrowserFrame* pFrm = NULL;
		CPtrArray ptrAWnd;
		for (pos1 = m_colBrowserTabList.GetTailPosition(); pos1 != NULL;)
		{
			pFrm = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (theApp.IsWnd(pFrm))
			{
				//一致した。
				if (pTarget == pFrm)
				{
					break;
				}
				else
				{
					ptrAWnd.Add(pFrm);
				}
			}
			pFrm = NULL;
			m_colBrowserTabList.GetPrev(pos1);
		}
		if (ptrAWnd.GetCount() > 0)
		{
			for (int i = 0; i < ptrAWnd.GetCount(); i++)
			{
				CBrowserFrame* pFrmPtr = NULL;
				pFrmPtr = (CBrowserFrame*)ptrAWnd.GetAt(i);
				if (theApp.IsWnd(pFrmPtr))
				{
					pFrmPtr->OnWClose();
				}
			}
		}
		return;
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return;
}
void CMainFrame::TabWindowChk()
{
	//全てHideになっているとマズイのでチェックする。救済処理
	try
	{
		CBrowserFrame* pFrmActiveNow = NULL;
		pFrmActiveNow = theApp.GetActiveBFramePtr();

		//ActiveWndが表示されている。即抜ける
		if (theApp.IsWnd(pFrmActiveNow) &&
		    theApp.IsWndVisible(pFrmActiveNow->m_hWnd) &&
		    pFrmActiveNow->m_cTabWnd)
		{
			return;
		}

		//ActiveWndが表示されている。即抜ける
		if (theApp.IsWnd(m_pActiveWindow) &&
		    theApp.IsWndVisible(m_pActiveWindow->m_hWnd) &&
		    m_pActiveWindow->m_cTabWnd)
		{
			return;
		}

		CString strAllCloseFlg;
		this->GetWindowText(strAllCloseFlg);
		if (strAllCloseFlg == theApp.m_strThisAppName)
		{
			return;
		}

		CBrowserFrame* pFrmVisible = NULL;
		CBrowserFrame* pFrmHead = NULL;
		POSITION pos1 = {0};
		CBrowserFrame* pFrm = NULL;
		BOOL bAllHide = TRUE;
		UINT iHideCnt = 0;
		for (pos1 = m_colBrowserTabList.GetHeadPosition(); pos1 != NULL;)
		{
			pFrm = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (pFrm->m_cTabWnd)
			{
				if (theApp.IsWnd(pFrm))
				{
					if (theApp.IsWndVisible(pFrm->m_hWnd))
					{
						bAllHide = FALSE;
						break;
					}
					else
					{
						if (!pFrmHead)
							pFrmHead = pFrm;
						iHideCnt++;
					}
				}
			}
			pFrm = NULL;
			m_colBrowserTabList.GetNext(pos1);
		}
		if (m_colBrowserTabList.GetCount() > 0 && bAllHide)
		{
			for (;;)
			{
				if (theApp.IsWnd(m_pActiveWindow) &&
				    !theApp.IsWndVisible(m_pActiveWindow->m_hWnd) &&
				    m_pActiveWindow->m_cTabWnd &&
				    !m_pActiveWindow->m_bDownloadBlankPage)
				{
					pFrmVisible = m_pActiveWindow;
					break;
				}

				if (theApp.IsWnd(m_pPrevActiveWindow) &&
				    !theApp.IsWndVisible(m_pPrevActiveWindow->m_hWnd) &&
				    m_pPrevActiveWindow->m_cTabWnd &&
				    !m_pPrevActiveWindow->m_bDownloadBlankPage)
				{
					pFrmVisible = m_pPrevActiveWindow;
					break;
				}
				if (pFrmHead &&
				    theApp.IsWnd(pFrmHead) &&
				    !theApp.IsWndVisible(pFrmHead->m_hWnd) &&
				    pFrmHead->m_cTabWnd &&
				    !pFrmHead->m_bDownloadBlankPage)
				{
					pFrmVisible = pFrmHead;
					break;
				}
				break;
			}
			if (pFrmVisible)
			{
				pFrmVisible->m_cTabWnd->ShowTabWindow((HWND)pFrmVisible->GetSafeHwnd());
			}
		}
		return;
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return;
}
BOOL CMainFrame::ReorderTab(int iOrg, int iAft)
{
	if (0 > iOrg || 0 > iAft || iOrg == iAft)
	{
		return FALSE;
	}

	if (iOrg < iAft)
	{
		POSITION posOrg = m_colBrowserTabList.FindIndex(iOrg);
		POSITION posAft = m_colBrowserTabList.FindIndex(iAft);
		if (posOrg != NULL && posAft != NULL)
		{
			CBrowserFrame* ptd = (CBrowserFrame*)m_colBrowserTabList.GetAt(posOrg);
			m_colBrowserTabList.InsertAfter(posAft, ptd);
			m_colBrowserTabList.RemoveAt(posOrg);
			theApp.m_imgFavIcons.Copy(iOrg, iAft, ILCF_SWAP);
		}
	}
	else
	{
		POSITION posOrg = m_colBrowserTabList.FindIndex(iOrg);
		POSITION posAft = m_colBrowserTabList.FindIndex(iAft);
		if (posOrg != NULL && posAft != NULL)
		{
			CBrowserFrame* ptd = (CBrowserFrame*)m_colBrowserTabList.GetAt(posOrg);
			m_colBrowserTabList.InsertBefore(posAft, ptd);
			m_colBrowserTabList.RemoveAt(posOrg);
			theApp.m_imgFavIcons.Copy(iOrg, iAft, ILCF_SWAP);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::InitFunc(CWnd* pFrame)
{
	PROC_TIME(InitFunc)
	try
	{
		_wsetlocale(LC_ALL, _T("jpn"));
		CString logmsg;
		CBrowserFrame* pFrm = (CBrowserFrame*)pFrame;
		if (!theApp.IsWnd(pFrm))
		{
			pFrm = NULL;
		}

		BOOL bTraceLog = FALSE;
		if (theApp.m_AppSettings.IsAdvancedLogMode())
		{
			bTraceLog = TRUE;
		}

		//起動時のカスタムメッセージを表示する。
		if (theApp.IsFirstInstance())
		{
			//-NEWで起動された場合は非表示
			if (!theApp.m_bNewInstanceParam)
			{
				CString strCustomMessage;
				strCustomMessage = theApp.m_AppSettings.GetInitMessage();
				if (!strCustomMessage.IsEmpty())
				{
					strCustomMessage.Replace(_T("\\n"), _T("\n"));
					pFrame->ShowWindow(SW_MAXIMIZE);
					int iRt = theApp.SB_MessageBox(pFrm == NULL ? NULL : pFrm->m_hWnd, strCustomMessage, NULL, MB_YESNO | MB_ICONINFORMATION /*|MB_DEFBUTTON2*/ | MB_SYSTEMMODAL, TRUE, 30 * 1000);
					if (iRt == IDCANCEL || iRt == IDNO || iRt == IDTIMEOUT)
					{
						HANDLE hMutex = {0};
						hMutex = ::CreateMutex(NULL, FALSE, _T("tfgszb_close"));
						PostMessage(WM_CLOSE);
						return;
					}
				}
			}
		}

		//ロードされているモジュール一覧出力
		if (bTraceLog)
			theApp.WriteDebugTraceDateTime(theApp.GetAllModules(), DEBUG_LOG_TYPE_DE);
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}
CChildView* CMainFrame::NewBrowserWindow(DWORD dwFlags)
{
	CSystemAnimationHelper anih(TRUE);

	CChildView* wndView = NULL;
	PROC_TIME(OnNewBrowserWindow)
	try
	{
		//制限値のチェック
		if (theApp.IsLimitChkEx())
			return NULL;

		CBrowserFrame* pFrame = NULL;
		pFrame = new CBrowserFrame;
		if (pFrame == NULL)
			return NULL;

		if ((dwFlags & NWMF_SUGGESTWINDOW) == NWMF_SUGGESTWINDOW)
		{
			pFrame->m_bIsPopupWindow = TRUE;
		}
		else
		{
			pFrame->m_bIsPopupWindow = FALSE;
		}

		CBrowserFrame* pFrame2 = NULL;
		BOOL bFirstCall = TRUE;
		WINDOWPLACEMENT zFramePlacement = {0};
		zFramePlacement.length = sizeof(WINDOWPLACEMENT);
		POSITION pos1 = {0};

		if (m_colBrowserWindows.GetCount() > 0)
		{
			bFirstCall = FALSE;
			POSITION pos1 = {0};
			CBrowserFrame* pFrm = NULL;
			CChildView* pView = NULL;
			pFrm = NULL;
			pView = NULL;
			BOOL bGetFlg = FALSE;

			//Tabなし
			if (!theApp.m_bTabEnable_Init)
			{
				for (pos1 = m_colBrowserWindows.GetTailPosition(); pos1 != NULL;)
				{
					pFrm = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
					if (theApp.IsWnd(pFrm))
					{
						if (pFrm->IsWindowEnabled())
						{
							pView = &(pFrm->m_wndView);
							if (theApp.IsWnd(pView))
							{
								if (!pView->IsPopupWindow())
								{
									bGetFlg = TRUE;
									pFrm->GetWindowPlacement(&zFramePlacement);
									//ポップアップでは無いWindowのサイズをキャッシュ
									theApp.m_NomalWindow_FramePlacementCache = zFramePlacement;
									break;
								}
							}
						}
					}
					pFrm = NULL;
					pView = NULL;
					m_colBrowserWindows.GetPrev(pos1);
				}
				//通常のウィンドウのサイズを取得できていない。ポップアップしかない。
				if (bGetFlg == FALSE)
				{
					//キャッシュしたサイズをセットする。
					zFramePlacement = theApp.m_NomalWindow_FramePlacementCache;
					//Normalsizeを指定
					zFramePlacement.showCmd = SW_NORMAL;
				}
			}
		}

		//通常のWindow作成(非タブ)
		if (!theApp.m_bTabEnable_Init)
		{
			pFrame->LoadFrame(IDR_MAINFRAME,
					  WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
					  NULL);
		}
		else
		{
			//Tabモードで初回は、通常のWindow作成処理
			if (bFirstCall)
			{
				pFrame->LoadFrame(IDR_MAINFRAME,
						  WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
						  NULL);
			}
			//Tabモードの場合に作成するWindowのサイズ位置は、既存の物に合わせておく(チラツキを抑える為)
			else
			{
				CRect cRect = {10, 10, 800, 600};
				WINDOWPLACEMENT zFramePlacement = {0};
				zFramePlacement.length = sizeof(WINDOWPLACEMENT);
				zFramePlacement = theApp.GetActiveFrameWindowPlacement(); //m_ActiveFramePlacement;
				DWORD dwCreateStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE;
				if (zFramePlacement.rcNormalPosition.bottom > 0)
				{
					CRect rcNormal(zFramePlacement.rcNormalPosition);
					if (zFramePlacement.showCmd == SW_MAXIMIZE)
					{
						CRect rcDesktop;
						CWnd* pView = NULL;
						HWND hWnd = {0};
						pView = (CWnd*)theApp.GetActiveViewPtr();
						if (theApp.IsWnd(pView))
						{
							hWnd = pView->m_hWnd;
						}
						if (hWnd == NULL)
							hWnd = ::GetDesktopWindow();
						SBUtil::GetMonitorWorkRect(hWnd, &rcDesktop);
						//Win10 7px調整
						if (theApp.m_iWinOSVersion >= 100)
						{
							CRect rectAdj;
							rcDesktop.left -= 7;
							rcDesktop.right += 7;
							rcDesktop.bottom += 7;
						}
						cRect = rcDesktop;
					}
					else if (zFramePlacement.showCmd == SW_NORMAL)
					{
						cRect = rcNormal;
					}
				}
				pFrame->Create(NULL, theApp.m_strThisAppName,
					       dwCreateStyle, cRect, NULL, MAKEINTRESOURCE(IDR_MAINFRAME));
				// load accelerator resource
				pFrame->LoadAccelTable(MAKEINTRESOURCE(IDR_MAINFRAME));
			}
		}

		//Window作成に失敗している。
		if (!theApp.IsWnd(pFrame))
		{
			if (pFrame)
				delete pFrame;
			::Sleep(500);
			return NULL;
		}

		//管理リストに追加する
		m_colBrowserWindows.AddTail(pFrame);

		CBrowserFrame* ptd = NULL;

		//起動から初めてのWindow作成の場合
		if (bFirstCall)
		{
			if (theApp.IsFirstInstance())
			{
				this->InitFunc(pFrame);
			}
			pFrame->GetWindowPlacement(&theApp.m_NomalWindow_FramePlacementCache);
			SetWindowPlacementFrm(theApp.m_NomalWindow_FramePlacementCache, pFrame);
			if (!theApp.m_bNewInstanceParam)
			{
				pFrame->ShowWindow(SW_NORMAL);
			}
		}
		else
		{
			CRect rcNormalSize2;
			rcNormalSize2 = zFramePlacement.rcNormalPosition;
			rcNormalSize2.OffsetRect(30, 30);
			zFramePlacement.rcNormalPosition = rcNormalSize2;

			//Tabあり
			if (theApp.m_bTabEnable_Init)
			{
				zFramePlacement = theApp.GetActiveFrameWindowPlacement(); //m_ActiveFramePlacement;
				BOOL bTabNoActivate = FALSE;
				bTabNoActivate = (dwFlags & NWMF_FORCETAB) == NWMF_FORCETAB ? TRUE : FALSE;
				if (bTabNoActivate)
				{
					if (zFramePlacement.showCmd == SW_MAXIMIZE)
					{
						CRect rcDesktop;
						CWnd* pView = NULL;
						HWND hWnd = {0};
						pView = (CWnd*)theApp.GetActiveViewPtr();
						if (theApp.IsWnd(pView))
						{
							hWnd = pView->m_hWnd;
						}
						if (hWnd == NULL)
							hWnd = ::GetDesktopWindow();
						SBUtil::GetMonitorWorkRect(hWnd, &rcDesktop);
						//Win10 7px調整
						if (theApp.m_iWinOSVersion >= 100)
						{
							CRect rectAdj;
							rcDesktop.left -= 7;
							rcDesktop.right += 7;
							rcDesktop.bottom += 7;
						}

						zFramePlacement.rcNormalPosition = rcDesktop;
					}
					zFramePlacement.showCmd = SW_SHOWNA;
					pFrame->SetWindowPlacement(&zFramePlacement);
					pFrame2 = theApp.GetActiveBFramePtr();
					HWND hwndInsertAfter = {0};
					if (theApp.IsWnd(pFrame2))
					{
						pFrame->ShowWindow(SW_HIDE);
						pFrame->m_wndView.EnableWinEx(FALSE);
						hwndInsertAfter = pFrame2->m_hWnd;
						::SetWindowPos(pFrame->m_hWnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					}
				}
				else
				{
					pFrame->SetWindowPlacement(&zFramePlacement);
				}
			}
			//Tab無し
			else
			{
				BOOL bTabNoActivate = FALSE;
				bTabNoActivate = (dwFlags & NWMF_FORCETAB) == NWMF_FORCETAB ? TRUE : FALSE;
				if (bTabNoActivate)
				{
					zFramePlacement.showCmd = SW_SHOWNOACTIVATE;
					pFrame->SetWindowPlacement(&zFramePlacement);
					pFrame2 = (CBrowserFrame*)this->GetActiveWindow();
					HWND hwndInsertAfter = {0};
					if (theApp.IsWnd(pFrame2))
					{
						pFrame->m_wndView.EnableWinEx(FALSE);
						hwndInsertAfter = pFrame2->m_hWnd;
						::SetWindowPos(pFrame->m_hWnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					}
				}
				else
				{
					pFrame->SetWindowPlacement(&zFramePlacement);
				}
			}
		}
		wndView = &pFrame->m_wndView;
		SaveWindowList(theApp.m_strRecoveryFileFullPath);
		return wndView;
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return NULL;
}
void CMainFrame::AppExitExBT(CWnd* ptrW)
{
	PROC_TIME(AppExitExBT)
	try
	{
		//Tabモードの場合
		if (theApp.m_bTabEnable_Init)
		{
			this->SetWindowText(theApp.m_strThisAppName);
		}

		POSITION pos1 = {0};
		CBrowserFrame* ptd = NULL;
		for (pos1 = m_colBrowserWindows.GetHeadPosition(); pos1 != NULL;)
		{
			ptd = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
			if (ptd != NULL)
			{
				if (::IsWindow(ptd->m_hWnd))
				{
					if (ptd != ptrW)
						ptd->OnCloseDelay();
				}
			}
			ptd = NULL;
			m_colBrowserWindows.GetNext(pos1);
		}
		//Tabモードの場合は、他のインスタンスは終了しない。
		if (theApp.m_bTabEnable_Init)
		{
			this->SetWindowText(theApp.m_FrmWndClassName);
			return;
		}
		//同じプロセスを落とす。
		CloseMultiProcess(::GetDesktopWindow());
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}
void CMainFrame::OnAppExitEx()
{
	PROC_TIME(OnAppExitEx)
	POSITION pos1 = {0};
	CBrowserFrame* ptd = NULL;

	//2重起動を許可する。終了中のため。
	this->SetWindowText(theApp.m_strThisAppName);
	for (pos1 = m_colBrowserWindows.GetHeadPosition(); pos1 != NULL;)
	{
		ptd = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
		if (ptd != NULL)
		{
			if (::IsWindow(ptd->m_hWnd))
			{
				ptd->OnCloseDelay();
			}
		}
		ptd = NULL;
		m_colBrowserWindows.GetNext(pos1);
	}
	//Tabモードの場合は、他のインスタンスは終了しない。
	if (theApp.m_bTabEnable_Init)
		return;
	//同じプロセスを落とす。
	CloseMultiProcess(::GetDesktopWindow());
}
void CMainFrame::CloseMultiProcess(HWND hWndParent)
{
	PROC_TIME(CloseMultiProcess)

	HWND hWndChild = ::FindWindowEx(hWndParent, NULL, theApp.m_FrmWndClassName, NULL);
	if (hWndChild == NULL)
	{
		return;
	}
	do
	{
		TCHAR szTitleMultipleInstance[260] = {0};
		::GetWindowText(hWndChild, szTitleMultipleInstance, 259);
		CString strTitleMultiple;
		strTitleMultiple = szTitleMultipleInstance;
		//window名を確認する。
		if (strTitleMultiple == theApp.m_FrmWndClassName)
		{
			if (hWndChild != this->m_hWnd)
				::PostMessage(hWndChild, WM_CLOSE, 0, 0);
		}
		CloseMultiProcess(hWndChild);
	} while ((hWndChild = ::FindWindowEx(hWndParent, hWndChild, theApp.m_FrmWndClassName, NULL)) != NULL);
}

CBrowserFrame* CMainFrame::GetPrevFrmWnd(CBrowserFrame* ptdCurrent)
{
	CBrowserFrame* pRet = NULL;
	if (!theApp.IsWnd(ptdCurrent))
		return NULL;

	POSITION pos1 = {0};
	CBrowserFrame* pFrm = NULL;
	CBrowserFrame* pFrmTail = NULL;
	if (theApp.m_bTabEnable_Init)
	{
		BOOL bFind = FALSE;
		pFrm = NULL;
		for (pos1 = m_colBrowserTabList.GetTailPosition(); pos1 != NULL;)
		{
			pFrm = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (theApp.IsWnd(pFrm))
			{
				if (pFrmTail == NULL)
					pFrmTail = pFrm;

				//一致した。
				if (ptdCurrent == pFrm)
				{
					bFind = TRUE;
				}
				else
				{
					if (bFind == TRUE)
					{
						pRet = pFrm;
						break;
					}
				}
			}
			pFrm = NULL;
			m_colBrowserTabList.GetPrev(pos1);
		}
		if (bFind && pRet == NULL)
		{
			pRet = pFrmTail;
		}
		return pRet;
	}
	for (;;)
	{
		BOOL bFind = FALSE;
		pFrm = NULL;
		for (pos1 = m_colBrowserWindows.GetTailPosition(); pos1 != NULL;)
		{
			pFrm = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
			if (theApp.IsWnd(pFrm))
			{
				if (pFrmTail == NULL)
					pFrmTail = pFrm;

				//一致した。
				if (ptdCurrent == pFrm)
				{
					bFind = TRUE;
				}
				else
				{
					if (bFind == TRUE)
					{
						pRet = pFrm;
						break;
					}
				}
			}
			pFrm = NULL;
			m_colBrowserWindows.GetPrev(pos1);
		}
		if (bFind && pRet == NULL)
		{
			m_MultiProcessHWND.RemoveAll();
			GetMultiProcessWnd(::GetDesktopWindow());
			HWND hWndChild = {0};
			HWND hWndChildPrev = {0};
			size_t iProcCnt = m_MultiProcessHWND.GetCount();
			if (iProcCnt > 1)
			{
				BOOL bFind = FALSE;
				for (UINT iCol = 0; iCol < iProcCnt; iCol++)
				{
					hWndChild = (HWND)m_MultiProcessHWND.GetAt(iCol);
					TCHAR szTitleMultipleInstance[260] = {0};
					::GetWindowText(hWndChild, szTitleMultipleInstance, 259);
					CString strTitleMultiple;
					strTitleMultiple = szTitleMultipleInstance;
					//window名を確認する。
					if (strTitleMultiple == theApp.m_FrmWndClassName)
					{
						if (bFind)
						{
							hWndChildPrev = hWndChild;
							break;
						}
						if (hWndChild == this->m_hWnd)
						{
							bFind = TRUE;
						}
					}
				}
				if (hWndChildPrev)
				{
					::PostMessage(hWndChildPrev, WM_ACTIVE_TAIL, 0, 0);
					return NULL;
				}
				else
				{
					hWndChildPrev = (HWND)m_MultiProcessHWND.GetAt(0);
					if (hWndChildPrev != this->m_hWnd)
					{
						::PostMessage(hWndChildPrev, WM_ACTIVE_TAIL, 0, 0);
						return NULL;
					}
				}
			}
			pRet = pFrmTail;
		}
		break;
	}
	return pRet;
}

CBrowserFrame* CMainFrame::GetNextFrmWnd(CBrowserFrame* ptdCurrent)
{
	CBrowserFrame* pRet = NULL;
	if (!theApp.IsWnd(ptdCurrent))
		return NULL;

	POSITION pos1 = {0};
	CBrowserFrame* pFrm = NULL;
	CBrowserFrame* pFrmHead = NULL;
	CBrowserFrame* pFrmTail = NULL;
	if (theApp.m_bTabEnable_Init)
	{
		BOOL bFind = FALSE;
		pFrm = NULL;
		for (pos1 = m_colBrowserTabList.GetHeadPosition(); pos1 != NULL;)
		{
			pFrm = (CBrowserFrame*)m_colBrowserTabList.GetAt(pos1);
			if (theApp.IsWnd(pFrm))
			{
				if (pFrmHead == NULL)
					pFrmHead = pFrm;
				//一致した。
				if (ptdCurrent == pFrm)
				{
					bFind = TRUE;
				}
				else
				{
					if (bFind == TRUE)
					{
						pRet = pFrm;
						break;
					}
				}
			}
			pFrm = NULL;
			m_colBrowserTabList.GetNext(pos1);
		}
		if (bFind && pRet == NULL)
		{
			pRet = pFrmHead;
		}
		return pRet;
	}

	for (;;)
	{
		BOOL bFind = FALSE;
		pFrm = NULL;
		for (pos1 = m_colBrowserWindows.GetHeadPosition(); pos1 != NULL;)
		{
			pFrm = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
			if (theApp.IsWnd(pFrm))
			{
				if (pFrmHead == NULL)
					pFrmHead = pFrm;
				//一致した。
				if (ptdCurrent == pFrm)
				{
					bFind = TRUE;
				}
				else
				{
					if (bFind == TRUE)
					{
						pRet = pFrm;
						break;
					}
				}
			}
			pFrm = NULL;
			m_colBrowserWindows.GetNext(pos1);
		}
		if (bFind && pRet == NULL)
		{
			m_MultiProcessHWND.RemoveAll();
			GetMultiProcessWnd(::GetDesktopWindow());
			HWND hWndChild = {0};
			HWND hWndChildPrev = {0};
			size_t iProcCnt = m_MultiProcessHWND.GetCount();
			if (iProcCnt > 1)
			{
				for (UINT iCol = 0; iCol < iProcCnt; iCol++)
				{
					hWndChild = (HWND)m_MultiProcessHWND.GetAt(iCol);
					TCHAR szTitleMultipleInstance[260] = {0};
					::GetWindowText(hWndChild, szTitleMultipleInstance, 259);
					CString strTitleMultiple;
					strTitleMultiple = szTitleMultipleInstance;
					//window名を確認する。
					if (strTitleMultiple == theApp.m_FrmWndClassName)
					{
						if (hWndChild == this->m_hWnd)
						{
							break;
						}
						else
							hWndChildPrev = hWndChild;
					}
				}
				if (hWndChildPrev)
				{
					::PostMessage(hWndChildPrev, WM_ACTIVE_HEAD, 0, 0);
					return NULL;
				}
				else
				{
					hWndChildPrev = (HWND)m_MultiProcessHWND.GetAt(iProcCnt - 1);
					if (hWndChildPrev != this->m_hWnd)
					{
						::PostMessage(hWndChildPrev, WM_ACTIVE_HEAD, 0, 0);
						return NULL;
					}
				}
			}
			pRet = pFrmHead;
		}
		break;
	}
	return pRet;
}

LRESULT CMainFrame::OnSaveWindowListMSG(WPARAM wParam, LPARAM lParam)
{
	CString strFile(theApp.m_strRestoreFileFullPath);
	this->SaveWindowList(strFile, TRUE);
	return 0;
}
LRESULT CMainFrame::OnActiveHead(WPARAM wParam, LPARAM lParam)
{
	CBrowserFrame* pFrm = NULL;
	pFrm = NULL;
	pFrm = (CBrowserFrame*)m_colBrowserWindows.GetHead();
	if (theApp.IsWnd(pFrm))
	{
		if (::IsIconic(pFrm->m_hWnd))
		{
			::ShowWindow(pFrm->m_hWnd, SW_HIDE);
			::ShowWindow(pFrm->m_hWnd, SW_NORMAL);
		}
		SBUtil::SetAbsoluteForegroundWindow(pFrm->m_hWnd, FALSE);
		::SetWindowPos(pFrm->m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
	return 0;
}
LRESULT CMainFrame::OnActiveTail(WPARAM wParam, LPARAM lParam)
{
	CBrowserFrame* pFrm = NULL;
	pFrm = NULL;
	pFrm = (CBrowserFrame*)m_colBrowserWindows.GetTail();
	if (theApp.IsWnd(pFrm))
	{
		if (::IsIconic(pFrm->m_hWnd))
		{
			::ShowWindow(pFrm->m_hWnd, SW_HIDE);
			::ShowWindow(pFrm->m_hWnd, SW_NORMAL);
		}
		SBUtil::SetAbsoluteForegroundWindow(pFrm->m_hWnd, FALSE);
		::SetWindowPos(pFrm->m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
	return 0;
}

void CMainFrame::GetMultiProcessWnd(HWND hWndParent)
{
	PROC_TIME(GetMultiProcessWnd)

	HWND hWndChild = ::FindWindowEx(hWndParent, NULL, theApp.m_FrmWndClassName, NULL);
	if (hWndChild == NULL)
	{
		return;
	}
	do
	{
		TCHAR szTitleMultipleInstance[260] = {0};
		::GetWindowText(hWndChild, szTitleMultipleInstance, 259);
		CString strTitleMultiple;
		strTitleMultiple = szTitleMultipleInstance;
		//window名を確認する。
		if (strTitleMultiple == theApp.m_FrmWndClassName)
		{
			m_MultiProcessHWND.Add(hWndChild);
		}
		GetMultiProcessWnd(hWndChild);
	} while ((hWndChild = ::FindWindowEx(hWndParent, hWndChild, theApp.m_FrmWndClassName, NULL)) != NULL);
}

void CMainFrame::OnAppExitTimeLimit()
{
	if (m_bGlobalMsgFlg) return;
	int iRet = 0;
	CString strMsg;
	CString logmsg;
	int iLimitTimeBase = 0;
	iLimitTimeBase = theApp.m_AppSettings.GetRunningLimitTime();

	POSITION pos1 = {0};
	CBrowserFrame* ptd = NULL;

	//2重起動を許可する。終了中のため。
	this->SetWindowText(theApp.m_strThisAppName);
	CBrowserFrame* ptdFirst = NULL;

	int iExit = 0;
	for (pos1 = m_colBrowserWindows.GetHeadPosition(); pos1 != NULL;)
	{
		iExit++;
		if (iExit == 1)
		{
			ptdFirst = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
		}
		else
		{
			ptd = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
			if (ptd != NULL)
			{
				if (::IsWindow(ptd->m_hWnd))
				{
					ptd->OnCloseDelay();
				}
			}
		}
		ptd = NULL;
		m_colBrowserWindows.GetNext(pos1);
	}
	CString alertMsg;
	alertMsg.LoadString(IDS_STRING_ALERT_OVER_RUNNING_TIME_LIMIT);
	logmsg.Format(alertMsg, iLimitTimeBase);
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_EX);
	if (ptdFirst && ::IsWindow(ptdFirst->m_hWnd))
	{
		m_bGlobalMsgFlg = TRUE;
		theApp.SB_MessageBox(ptdFirst->m_hWnd, logmsg, NULL, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL, TRUE);
		SaveWindowList(theApp.m_strRestoreFileFullPath);
		PostMessage(WM_CLOSE);
	}
	else
	{
		m_bGlobalMsgFlg = TRUE;
		theApp.SB_MessageBox(this->m_hWnd, logmsg, NULL, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL, TRUE);
		SaveWindowList(theApp.m_strRestoreFileFullPath);
		PostMessage(WM_CLOSE);
	}
	m_bGlobalMsgFlg = FALSE;
	return;
}

void CMainFrame::OnAppExitMaxMem()
{
	if (m_bGlobalMsgFlgMem) return;
	int iRet = 0;
	CString strMsg;
	CString logmsg;

	POSITION pos1 = {0};
	CBrowserFrame* ptd = NULL;

	//2重起動を許可する。終了中のため。
	this->SetWindowText(theApp.m_strThisAppName);
	CBrowserFrame* ptdFirst = NULL;

	int iExit = 0;
	for (pos1 = m_colBrowserWindows.GetHeadPosition(); pos1 != NULL;)
	{
		iExit++;
		if (iExit == 1)
		{
			ptdFirst = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
		}
		else
		{
			ptd = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
			if (ptd != NULL)
			{
				if (::IsWindow(ptd->m_hWnd))
				{
					ptd->OnCloseDelay();
				}
			}
		}
		ptd = NULL;
		m_colBrowserWindows.GetNext(pos1);
	}
	CString alertMsg;
	alertMsg.LoadString(IDS_STRING_LOW_SYSTEM_RESOURCE_EXIT);
	logmsg.Format(alertMsg, theApp.m_strThisAppName);
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_EX);

	if (ptdFirst && ::IsWindow(ptdFirst->m_hWnd))
	{
		m_bGlobalMsgFlgMem = TRUE;
		theApp.m_bAbortFlg = TRUE;
		theApp.SB_MessageBox(ptdFirst->m_hWnd, logmsg, NULL, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL, TRUE);
		theApp.ExecNewInstance(_T(""));
		::Sleep(2 * 1000);
		PostMessage(WM_CLOSE);
	}
	else
	{
		m_bGlobalMsgFlgMem = TRUE;
		theApp.m_bAbortFlg = TRUE;
		theApp.SB_MessageBox(this->m_hWnd, logmsg, NULL, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL, TRUE);
		theApp.ExecNewInstance(_T(""));
		::Sleep(2 * 1000);
		PostMessage(WM_CLOSE);
	}
	m_bGlobalMsgFlg = FALSE;
	return;
}

void CMainFrame::SaveWindowList(LPCTSTR strPath, BOOL bAppendMode /*=FALSE*/)
{
	if (!strPath) return;
	HANDLE hEvent = {0};
	try
	{
		hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, theApp.m_strEventRecoveryName);
		DWORD waitRes = WaitForSingleObject(hEvent, 100);
		if (waitRes == WAIT_TIMEOUT)
		{
			CloseHandle(hEvent);
			return;
		}

		_wsetlocale(LC_ALL, _T("jpn"));
		CStdioFile out;
		UINT uiFlg = CFile::modeWrite | CFile::modeCreate | CFile::shareDenyNone;
		if (bAppendMode)
			uiFlg |= CFile::modeNoTruncate;

		if (out.Open(strPath, uiFlg))
		{
			CString strURL;
			CString strData;

			POSITION pos1 = {0};
			CBrowserFrame* pFrm = NULL;
			CChildView* pView = NULL;

			CString strWriteTime;
			CTime time = CTime::GetCurrentTime();
			strWriteTime.Format(_T("%s\n"), (LPCTSTR)time.Format(_T("%Y-%m-%d %H:%M:%S")));
			int iCnt = 0;
			if (bAppendMode)
			{
				out.SeekToEnd();
				iCnt = 1;
			}
			for (pos1 = m_colBrowserWindows.GetHeadPosition(); pos1 != NULL; m_colBrowserWindows.GetNext(pos1))
			{
				pFrm = NULL;
				pView = NULL;
				strURL.Empty();

				pFrm = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
				if (!theApp.IsWnd(pFrm))
					continue;
				if (!pFrm->IsWindowEnabled())
					continue;
				pView = &(pFrm->m_wndView);
				if (!theApp.IsWnd(pView))
					continue;
				//Popupは除く
				if (pView->IsPopupWindow())
					continue;
				strURL = pView->GetLocationURL();
				strURL.TrimLeft();
				strURL.TrimRight();
				if (SBUtil::IsURL_HTTP(strURL))
				{
					if (iCnt == 0)
					{
						//日時を初めの行に出力する。
						out.WriteString(strWriteTime);
					}

					//CStdioFileはMBCSで書き込もうとする一方、入力値はUnicodeなので、
					//タイトルに特殊文字が含まれていると文字変換に失敗して上手く書き込めない場合がある。
					//そのため、タイトルは書きこまずURLのみとする
					strData.Format(_T("%s\n"), (LPCTSTR)strURL);

					out.WriteString(strData);
					iCnt++;
				}
			}
			out.Close();
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	if (hEvent)
	{
		SetEvent(hEvent);
		CloseHandle(hEvent);
	}
}
void CMainFrame::Get_WindowInfoArrayWithPID(CStringArray& strATitle, CPtrArray& ptrAWnd, CUIntArray& ptrAPID)
{
	try
	{
		strATitle.RemoveAll();
		ptrAWnd.RemoveAll();
		ptrAPID.RemoveAll();
		POSITION pos1 = {0};
		CBrowserFrame* ptd = NULL;
		CString strTitle;
		for (pos1 = m_colBrowserWindows.GetHeadPosition(); pos1 != NULL;)
		{
			strTitle.Empty();
			ptd = (CBrowserFrame*)m_colBrowserWindows.GetAt(pos1);
			if (ptd)
			{
				strTitle = ptd->m_wndView.m_strTitle;
				strTitle.TrimLeft();
				strTitle.TrimRight();
				if (strTitle.IsEmpty())
				{
					if (ptd->m_bDownloadBlankPage)
						strTitle.LoadString(ID_DEFAULT_FILE_DOWNLOAD_WINDOW_TITLE);
					else
						strTitle = _T("New Tab");
				}
				strATitle.Add(strTitle);
				ptrAWnd.Add(ptd->m_hWnd);
				ptrAPID.Add(ptd->m_RendererPID);
			}
			ptd = NULL;
			m_colBrowserWindows.GetNext(pos1);
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
}

void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	LOGFONT lf = {0};
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
	afxGlobalData.SetMenuFont(&lf, true);
}
