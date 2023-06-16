#include "stdafx.h"
#include "sbcommon.h"
#include "Sazabi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CMyTaskDlg::OnInit()
{
	HWND hwnd = ::GetActiveWindow();
	if (hwnd)
	{
		TCHAR pszName[64] = {0};
		if (::GetClassName(hwnd, pszName, 64))
		{
			if (::_tcscmp(pszName, _T("#32770")) == 0)
			{
				m_hMyWnd = hwnd;
				CWnd* pWnd = CWnd::FromHandle(hwnd);
				if (pWnd)
				{
					pWnd->CenterWindow(pWnd->GetParent());
				}
				theApp.m_hwndTaskDlg = m_hMyWnd;
			}
		}
	}
	return S_OK;
}
HRESULT CMyTaskDlg::OnDestroy()
{
	m_hMyWnd = NULL;
	theApp.m_hwndTaskDlg = m_hMyWnd;
	return S_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMyObject::Invoke(DISPID dispIdMember,
			       REFIID riid,
			       LCID lcid,
			       WORD wFlags,
			       DISPPARAMS FAR* pDispParams,
			       VARIANT FAR* pVarResult,
			       EXCEPINFO FAR* pExcepInfo,
			       unsigned int FAR* puArgErr)
{
	try
	{
		if (dispIdMember == 1)
		{
			if (wFlags != DISPATCH_METHOD)
				return DISP_E_MEMBERNOTFOUND;

			if (pDispParams->cArgs == 0)
				return DISP_E_MEMBERNOTFOUND;
			else if (pDispParams->cArgs == 1)
			{
				_variant_t str(pDispParams->rgvarg);
				str.ChangeType(VT_BSTR);
				if (str.bstrVal)
				{
					CString strMsg(str.bstrVal);
					CString logmsg;
					DebugWndLogData dwLogData;
					dwLogData.mHWND.Format(_T("APP_WND:0x%08x"), 0);
					dwLogData.mFUNCTION_NAME = _T("CSG_Script");
					dwLogData.mMESSAGE1 = strMsg;
					theApp.AppendDebugViewLog(dwLogData);
					logmsg = dwLogData.GetString();
					theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_URL);
				}
			}
		}
		else if (dispIdMember == 2)
		{
			if (!(wFlags & DISPATCH_METHOD))
				return DISP_E_MEMBERNOTFOUND;

			if (pDispParams->cArgs == 0)
				return DISP_E_MEMBERNOTFOUND;
			else if (pDispParams->cArgs == 1)
				return DISP_E_MEMBERNOTFOUND;
			else if (pDispParams->cArgs == 2)
			{
				if (!(pDispParams->rgvarg[0].vt == VT_BSTR &&
				      pDispParams->rgvarg[1].vt == VT_BSTR))
				{
					_variant_t str0(pDispParams->rgvarg[0]);
					str0.ChangeType(VT_BSTR);
					_variant_t str1(pDispParams->rgvarg[1]);
					str1.ChangeType(VT_BSTR);

					CString cszArg0(str0.bstrVal);
					CString cszArg1(str1.bstrVal);
					CStringA strPet(cszArg0);
					CStringA strStrURL(cszArg1);
					if (pVarResult != NULL)
					{
						BOOL bRet = false;
						if (wildcmp(strPet, strStrURL))
							bRet = true;
						else
							bRet = false;
						VariantInit(pVarResult);
						V_VT(pVarResult) = VT_BOOL;
						if (bRet)
							V_BOOL(pVarResult) = true;
						else
							V_BOOL(pVarResult) = false;
					}
				}
			}
		}
		else
		{
			return DISP_E_MEMBERNOTFOUND;
		}
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return S_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CActiveScriptSite::OnScriptError(IActiveScriptError* pscripterror)
{
	//スクリプトでエラーが起こったときの通知
	HRESULT hRes = {0};

	try
	{
		if (!pscripterror)
			return E_POINTER;

		EXCEPINFO ei = {0};
		hRes = pscripterror->GetExceptionInfo(&ei);
		DWORD dwSrcContext;
		ULONG ulLine;
		LONG ichError;
		CComBSTR bstrLine;
		CString logmsg;

		pscripterror->GetExceptionInfo(&ei);
		pscripterror->GetSourcePosition(&dwSrcContext, &ulLine, &ichError);
		pscripterror->GetSourceLineText(&bstrLine);

		CString desc;
		CString src;

		desc = (LPCWSTR)ei.bstrDescription;
		src = (LPCWSTR)ei.bstrSource;

		DebugWndLogData dwLogData;
		dwLogData.mHWND.Format(_T("APP_WND:0x%08x"), 0);
		dwLogData.mFUNCTION_NAME = _T("CSG_Script");
		dwLogData.mMESSAGE1 = desc;
		dwLogData.mMESSAGE2.Format(_T("Src:%s"), (LPCTSTR)src);
		dwLogData.mMESSAGE3.Format(_T("Line:%d"), ulLine);
		dwLogData.mMESSAGE4.Format(_T("Error:%d"), (int)ei.wCode);
		dwLogData.mMESSAGE5.Format(_T("Scode:%x"), ei.scode);
		theApp.AppendDebugViewLog(dwLogData);
		logmsg = dwLogData.GetString();
		theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_URL);
	}
	catch (...)
	{
		ATLASSERT(0);
	}
	return hRes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////CLogDispatcher///////////////
#define ERR_SERVER_SETTINGS_NOT_FOUND 1
#define ERR_SERVER_SESSION	      2
#define ERR_SERVER_SETTINGS_INVALID   3
#define ERR_SERVER_CONNECT	      4
#define ERR_SERVER_REQUEST	      5
#define ERR_SERVER_SEND_REQUEST	      6
#define ERR_SERVER_RECEIVE_RESPONSE   7
#define ERR_SERVER_QUERY_HEADERS      8
#define ERR_SERVER_HTTP_STATUS	      9
#define ERR_SERVER_RESPONSE_EMPTY     10
#define ERR_SERVER_RESPONSE_INVALID   11
#define ERR_SERVER_RESPONSE_STOP      12

#define ERR_SERVER_ETC 999
#define OK_SERVER      200

CString CLogDispatcher::GetOpStr(int iLogType)
{
	CString strRet;
	if (iLogType == LOG_UPLOAD)
		strRet = _T("upload");
	else if (iLogType == LOG_DOWNLOAD)
		strRet = _T("download");
	else if (iLogType == LOG_BROWSING)
		strRet = _T("browsing");
	else if (iLogType == LOG_ACCESS_ALL)
		strRet = _T("urlaccess");
	else
		strRet = _T("download");
	return strRet;
}
void CLogDispatcher::ChkThread()
{
	try
	{
		if (m_bStop)
			return;

		CWinThread* pThread = NULL;
		CString strKey;

		//終了した物を消す。
		CStringArray strARemoveList;
		POSITION pos = m_MapLogThreadMgr.GetStartPosition();
		while (pos)
		{
			try
			{
				m_MapLogThreadMgr.GetNextAssoc(pos, strKey, (void*&)pThread);
				if (pThread == NULL)
				{
					strARemoveList.Add(strKey);
				}
			}
			catch (...)
			{
				::OutputDebugString(_T("#ChkThread_Exception1"));
			}
		}
		int iDelCnt = 0;
		iDelCnt = (int)strARemoveList.GetCount();
		for (int i = 0; i < iDelCnt; i++)
		{
			try
			{
				strKey = strARemoveList.GetAt(i);
				if (!strKey.IsEmpty())
				{
					if (m_MapLogThreadMgr.Lookup(strKey, (void*&)pThread))
					{
						m_MapLogThreadMgr.RemoveKey(strKey);
					}
				}
			}
			catch (...)
			{
				::OutputDebugString(_T("#ChkThread_Exception2"));
			}
		}
		pos = m_MapLogThreadMgr.GetStartPosition();
		while (pos)
		{
			try
			{
				m_MapLogThreadMgr.GetNextAssoc(pos, strKey, (void*&)pThread);
				if (m_bStop)
					return;

				if (pThread)
				{
					if (m_MapLogThreadMgr.Lookup(strKey, (void*&)pThread))
					{
						if (pThread)
						{
							DWORD dRet = 0;
							if (m_bStop)
								return;

							for (;;)
							{
								try
								{
									dRet = ::WaitForSingleObject(pThread->m_hThread, 30 * 1000);
								}
								catch (...)
								{
									::OutputDebugString(_T("#ChkThread_ExceptionWFSO"));
									try
									{
										m_MapLogThreadMgr.SetAt(strKey, NULL);
									}
									catch (...)
									{
										::OutputDebugString(_T("#ChkThread_ExceptionWFSO2"));
									}
									break;
								}

								// スレッド終了待ち
								if (dRet == WAIT_TIMEOUT)
								{
									m_strForceStopIdx = strKey;
									::Sleep(10 * 1000);
								}
								else if (dRet == WAIT_ABANDONED || dRet == WAIT_FAILED)
								{
									DebugWndLogData dwLogData;
									dwLogData.mHWND.Format(_T("SEND_LOG_WND:0x%08x"), 0);
									dwLogData.mFUNCTION_NAME = _T("ChkThreadWait_ABANDED_FAILED");
									dwLogData.mMESSAGE1 = strKey;
									theApp.AppendDebugViewLog(dwLogData);
									m_logmsg = dwLogData.GetString();
									theApp.WriteDebugTraceDateTime(m_logmsg, DEBUG_LOG_TYPE_URL);

									CloseThreadList(strKey);
									break;
								}
								// スレッド終了待ち
								if (::WaitForSingleObject(pThread->m_hThread, 60 * 1000) == WAIT_TIMEOUT)
								{
									// スレッド強制停止
									// (絶対に停止するなら WaitForSingleObjectで INFINITE も可）
#pragma warning(push, 0)
//警告 C6258 TerminateThread を使用すると、正しくスレッドをクリーンアップすることができません。
// -> 普通に停止できなかった場合に強制停止するために使っている。正しい使い方なので警告を無視。
#pragma warning(disable : 6258)
									::TerminateThread(pThread->m_hThread, 0xffffffff);
#pragma warning(pop)
									::CloseHandle(pThread->m_hThread);
									DebugWndLogData dwLogData;
									dwLogData.mHWND.Format(_T("SEND_LOG_WND:0x%08x"), 0);
									dwLogData.mFUNCTION_NAME = _T("ChkThreadWait_Timeout");
									dwLogData.mMESSAGE1 = strKey;
									theApp.AppendDebugViewLog(dwLogData);
									m_logmsg = dwLogData.GetString();
									theApp.WriteDebugTraceDateTime(m_logmsg, DEBUG_LOG_TYPE_URL);

									CloseThreadList(strKey);
									break;
								}
								break; //for
							}
						}
					}
				}
			}
			catch (...)
			{
				::OutputDebugString(_T("#ChkThread_Exception"));
			}
		}
		size_t iCol = 0;
		try
		{
			iCol = m_MapLogThreadMgr.GetCount();
			m_logmsg.Format(_T("SendLog:ChkThread:(%d)\n"), iCol);
			::OutputDebugString(m_logmsg);
		}
		catch (...)
		{
		}
	}
	catch (...)
	{
		::OutputDebugString(_T("###ChkThread_Exception"));
		m_MapLogThreadMgr.RemoveAll();
	}
	return;
}

UINT MonitorThread(LPVOID ptr)
{
	if (!ptr) return 0;
	CLogDispatcher* ptrDisp = NULL;
	ptrDisp = ((CLogDispatcher*)ptr);
	try
	{
		for (;;)
		{
			if (ptrDisp->m_bStop)
				break;
			Sleep(10 * 1000);
			try
			{
				//Chk
				ptrDisp->ChkThread();
			}
			catch (...)
			{
				::OutputDebugString(_T("###MonitorThread_Exception"));
			}
			if (ptrDisp->m_bStop)
				break;
		}
	}
	catch (...)
	{
		ATLASSERT(0);
		::OutputDebugString(_T("###MonitorThread_Exception_Retry"));
		try
		{
			//Restart
			if (ptrDisp)
			{
				ptrDisp->m_pMonitorThread = NULL;
				ptrDisp->Init();
				::OutputDebugString(_T("###MonitorThread_Exception_Retry_Start"));
				return 0;
			}
		}
		catch (...)
		{
		}
	}
	if (ptrDisp->m_bStop)
	{
		ptrDisp->m_bStopFin = TRUE;
	}
	return 0;
}
void CLogDispatcher::Init()
{
	m_MapLogThreadMgr.RemoveAll();
	m_MapLogThreadMgr.InitHashTable(1231, true);
	if (!m_pMonitorThread)
	{
		m_pMonitorThread = AfxBeginThread(MonitorThread, this);
	}
}

UINT MyThread(LPVOID ptr)
{
	if (!ptr) return 0;

	int iRet = 0;
	CLogDispatcher* ptrDisp = NULL;
	ptrDisp = ((CLogDispatcher*)ptr);
	int m_iLogType = ptrDisp->m_iLogType;
	CString strIndex;
	strIndex = ptrDisp->m_strIndexTMP;

	CString strFileName;
	CString strTargetURL;

	strFileName = ptrDisp->m_strFileName;
	strTargetURL = ptrDisp->m_strTargetURL;
	Sleep(10);

	for (int i = 0; i < 5; i++)
	{
		if (ptrDisp->m_bStop) break;
		iRet = ptrDisp->SendLogThread(m_iLogType, strFileName, strTargetURL);
		//OK
		if (iRet == OK_SERVER)
			break;
		//Server設定が無い。
		else if (iRet == ERR_SERVER_SETTINGS_NOT_FOUND)
			break;
		//Server設定のURL指定が妥当でない。
		else if (iRet == ERR_SERVER_SETTINGS_INVALID)
			break;
		//httpセッションが張れない。リトライしても無駄なケースが多いので終了
		else if (iRet == ERR_SERVER_SESSION)
			break;
		else
			Sleep(1000);
		if (ptrDisp->m_strForceStopIdx == strIndex)
		{
			ptrDisp->m_strForceStopIdx.Empty();
			break;
		}
	}
	ptrDisp->CloseThreadList(strIndex);
	return 0;
}
void CLogDispatcher::SendLog(int iLogType, LPCTSTR lpFileName, LPCTSTR lpTargetURL)
{
	PROC_TIME(LogDispatcher_SendLog)
	HANDLE hEvent = {0};
	try
	{
		hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, m_strEventSendMsgName);
		DWORD waitRes = WaitForSingleObject(hEvent, 200);
		if (waitRes == WAIT_TIMEOUT)
		{
			ATLASSERT(0);
		}
		else
		{
			if (theApp.m_AppSettings.IsEnableLogging())
			{
				CString strServerURL;
				strServerURL = theApp.m_AppSettings.GetLogServerURL();
				if (!strServerURL.IsEmpty())
				{
					if (!m_bStop)
					{
						m_iLogType = iLogType;
						m_strFileName = lpFileName;
						m_strTargetURL = lpTargetURL;
						CString strIndexTmp;
						strIndexTmp = this->GetIndex();
						m_strIndexTMP = strIndexTmp;
						AddThreadMGR(AfxBeginThread(MyThread, this), strIndexTmp);
					}
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

	return;
}

int CLogDispatcher::SendLogThread(int iLogType, LPCTSTR lpFileName, LPCTSTR lpTargetURL)
{
	_wsetlocale(LC_ALL, _T("jpn"));

	PROC_TIME(LogDispatcher_SendLogThread)

	CString strServerURL;
	strServerURL = theApp.m_AppSettings.GetLogServerURL();
	if (strServerURL.IsEmpty())
		return ERR_SERVER_SETTINGS_NOT_FOUND;

	int iLogMethod = 0;
	CString strLogM;
	strLogM = theApp.m_AppSettings.GetLogMethod();
	if (strLogM.CompareNoCase(_T("GET")) == 0)
		iLogMethod = LOG_M_GET;
	else if (strLogM.CompareNoCase(_T("POST")) == 0)
		iLogMethod = LOG_M_POST;
	else
		iLogMethod = LOG_M_GET;

	int iRet = OK_SERVER;

	// セッションオブジェクトの作成
	HINTERNET hConnect = {0};
	HINTERNET hRequest = {0};
	HINTERNET hSession = {0};

	// 変数
	CStringW strServer;
	INTERNET_PORT nPort = 0;
	CStringW strVerb;
	CStringW strObjectName;
	CStringW strHeaders;

	CString logmsg;

	//1回で抜ける gotoの変わり
	while (1)
	{
		if (m_bStop)
		{
			iRet = OK_SERVER;
			break;
		}

		WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig = {0};
		ZeroMemory(&ieProxyConfig, sizeof(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG));
		WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig);
		if (ieProxyConfig.lpszProxy)
		{
			// InternetExplorerのプロキシ設定でセッションオブジェクト作成
			hSession = WinHttpOpen(L"CSGAgent",										// UserAgent名
					       WINHTTP_ACCESS_TYPE_NAMED_PROXY,								// プロキシ設定
					       ieProxyConfig.lpszProxy,									// プロキシ名
					       ieProxyConfig.lpszProxyBypass ? ieProxyConfig.lpszProxyBypass : WINHTTP_NO_PROXY_BYPASS, // プロキシバイパス
					       WINHTTP_FLAG_ASYNC);									// 振る舞いオプション。現状WINHTTP_FLAG_ASYNCのみ指定可能。
		}
		else
		{
			hSession = WinHttpOpen(L"CSGAgent",			  // UserAgent名
					       WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, // プロキシ設定
					       WINHTTP_NO_PROXY_NAME,		  // プロキシ名
					       WINHTTP_NO_PROXY_BYPASS,		  // プロキシバイパス
					       WINHTTP_FLAG_ASYNC);		  // 振る舞いオプション。現状WINHTTP_FLAG_ASYNCのみ指定可能。
		}
		if (!hSession)
		{
			iRet = ERR_SERVER_SESSION;
			break;
		}

		WinHttpSetTimeouts(hSession,
				   10000, //dwResolveTimeout
				   10000, //dwConnectTimeout
				   10000, //dwSendTimeout
				   10000  //dwReceiveTimeout
		);

		// URL解析
		URL_COMPONENTS urlcomponents = {0};
		ZeroMemory(&urlcomponents, sizeof(URL_COMPONENTS));
		urlcomponents.dwStructSize = sizeof(URL_COMPONENTS);

		std::unique_ptr<TCHAR> szHostName(new TCHAR[URLBUFFER_SIZE]());
		std::unique_ptr<TCHAR> szUrlPath(new TCHAR[URLBUFFER_SIZE]());

		urlcomponents.lpszHostName = szHostName.get();
		urlcomponents.lpszUrlPath = szUrlPath.get();
		urlcomponents.dwHostNameLength = URLBUFFER_SIZE;
		urlcomponents.dwUrlPathLength = URLBUFFER_SIZE;

		SendLogDataUtil stUSERID;
		stUSERID.SetData(this->GetUserNameData());

		SendLogDataUtil stComputerName;
		stComputerName.SetData(GetComputerNameData());

		SendLogDataUtil stFileName(lpFileName);
		SendLogDataUtil stTargetURL(lpTargetURL);

		CTime timeDlg = CTime::GetCurrentTime();
		SendLogDataUtil stTime(timeDlg.Format(_T("%Y-%m-%d %H:%M:%S")));

		CString strURL(strServerURL);
		CString strURLParam;

		//GET
		if (iLogMethod == LOG_M_GET)
		{
			strURLParam = _T("?operation=");
			strURLParam += GetOpStr(iLogType);

			strURLParam += _T("&pcname=");
			strURLParam += stComputerName.GetURLEncode();

			strURLParam += _T("&userid=");
			strURLParam += stUSERID.GetURLEncode();

			strURLParam += _T("&name=");
			strURLParam += stFileName.GetURLEncode();

			strURLParam += _T("&url=");
			strURLParam += stTargetURL.GetURLEncode();

			strURLParam += _T("&time=");
			strURLParam += stTime.GetURLEncode();
			strURL += strURLParam;
			int MAX_URL_LENGTH = INTERNET_MAX_URL_LENGTH;
			if (strURL.GetLength() > MAX_URL_LENGTH)
			{
				CString strHeadSize(strServerURL);
				strHeadSize += _T("?operation=");
				strHeadSize += GetOpStr(iLogType);
				strHeadSize += _T("&pcname=");
				strHeadSize += stComputerName.GetURLEncode();
				strHeadSize += _T("&userid=");
				strHeadSize += stUSERID.GetURLEncode();
				strHeadSize += _T("&time=");
				strHeadSize += stTime.GetURLEncode();
				UINT iHeadSize = strHeadSize.GetLength();

				CString strNameSize;
				strNameSize = _T("&name=");
				strNameSize += stFileName.GetURLEncode();
				UINT iNameSize = strNameSize.GetLength();

				CString strUrlSize;
				strUrlSize = _T("&url=");
				strUrlSize += stTargetURL.GetURLEncode();
				UINT iUrlSize = strUrlSize.GetLength();

				int iDiv = 0;
				iDiv = MAX_URL_LENGTH - iHeadSize;

				CString strTempFileName;
				strTempFileName = lpFileName;
				if (strTempFileName.GetLength() > 50)
				{
					SBUtil::GetDivChar(strTempFileName, 50, strTempFileName, FALSE);
				}
				SendLogDataUtil stTempFileName(strTempFileName);
				strNameSize = _T("&name=");
				strNameSize += stTempFileName.GetURLEncode();
				iNameSize = strNameSize.GetLength();
				iDiv -= iNameSize;
				SBUtil::GetDivChar(strUrlSize, iDiv, strUrlSize, FALSE);

				strURL = strServerURL;
				strURLParam = _T("?operation=");
				strURLParam += GetOpStr(iLogType);

				strURLParam += _T("&pcname=");
				strURLParam += stComputerName.GetURLEncode();

				strURLParam += _T("&userid=");
				strURLParam += stUSERID.GetURLEncode();
				strURLParam += strNameSize;
				strURLParam += strUrlSize;
				strURLParam += _T("&time=");
				strURLParam += stTime.GetURLEncode();
				strURL += strURLParam;
				SBUtil::GetDivChar(strURL, MAX_URL_LENGTH, strURL, FALSE);
			}
		}

		if (!InternetCrackUrl(strURL, 0, 0, &urlcomponents))
		{
			// URLの解析に失敗
			iRet = ERR_SERVER_SETTINGS_INVALID;
			break;
		}
		strServer = urlcomponents.lpszHostName;
		strObjectName = urlcomponents.lpszUrlPath;
		nPort = urlcomponents.nPort;

		// HTTPかHTTPSかそれ以外か
		DWORD dwOpenRequestFlag = (/*INTERNET_SCHEME_HTTPS*/ 4 == urlcomponents.nScheme) ? WINHTTP_FLAG_SECURE : 0;
		dwOpenRequestFlag = dwOpenRequestFlag | WINHTTP_FLAG_REFRESH;
		//GET or POST
		strVerb = L"GET";
		strHeaders.Empty();
		CString strRequestHeader;
		strRequestHeader = theApp.m_AppSettings.GetRequestHeader();

		if (!strRequestHeader.IsEmpty())
		{
			strRequestHeader.Replace(_T("\\r"), _T(""));
			strRequestHeader.Replace(_T("\\n"), _T("\r\n"));
			strRequestHeader += _T("\r\n");
			strHeaders = strRequestHeader;
		}

		//POSTの場合はJSON形式
		if (iLogMethod == LOG_M_POST)
		{
			strVerb = L"POST";
			strHeaders += L"Content-Type: application/json; charset=utf-8\r\n";
			//{ "operation": "upload",
			//  "pcname": "pcname",
			//  "userid": "userid",
			//  "filename": "filename",
			//  "url": "url",
			//  "time": "2018-08-20 18:00:00"
			//}
		}

		if (m_bStop)
		{
			iRet = OK_SERVER;
			break;
		}

		// HTTP接続
		hConnect = WinHttpConnect(hSession, strServer, nPort, 0);
		if (NULL == hConnect)
		{
			iRet = ERR_SERVER_CONNECT;
			break;
		}

		// HTTP接続を開く
		hRequest = WinHttpOpenRequest(hConnect,
					      strVerb,
					      strObjectName,
					      NULL,
					      WINHTTP_NO_REFERER,
					      WINHTTP_DEFAULT_ACCEPT_TYPES,
					      dwOpenRequestFlag);
		if (NULL == hRequest)
		{
			iRet = ERR_SERVER_REQUEST;
			break;
		}
		if (m_bStop)
		{
			iRet = OK_SERVER;
			break;
		}

		// リクエストの送信

		//POST
		if (iLogMethod == LOG_M_POST)
		{
			CString strJSONSendFormat;
			strJSONSendFormat = _T("{");
			strJSONSendFormat += _T("\"operation\": \"");
			strJSONSendFormat += GetOpStr(iLogType);
			strJSONSendFormat += _T("\"");

			strJSONSendFormat += _T(",\"pcname\": \"");
			strJSONSendFormat += stComputerName.m_strDataJSON;
			strJSONSendFormat += _T("\"");

			strJSONSendFormat += _T(",\"userid\": \"");
			strJSONSendFormat += stUSERID.m_strDataJSON;
			strJSONSendFormat += _T("\"");

			strJSONSendFormat += _T(",\"name\": \"");
			strJSONSendFormat += stFileName.m_strDataJSON;
			strJSONSendFormat += _T("\"");

			strJSONSendFormat += _T(",\"url\": \"");
			strJSONSendFormat += stTargetURL.m_strDataJSON;
			strJSONSendFormat += _T("\"");

			strJSONSendFormat += _T(",\"time\": \"");
			strJSONSendFormat += stTime.m_strDataJSON;
			strJSONSendFormat += _T("\"");
			strJSONSendFormat += _T("}");

			DebugWndLogData dwLogData;
			dwLogData.mHWND.Format(_T("SEND_LOG_WND:0x%08x"), 0);
			dwLogData.mFUNCTION_NAME = _T("SendLogThread");
			dwLogData.mMESSAGE1 = strURL;
			dwLogData.mMESSAGE2 = strJSONSendFormat;
			theApp.AppendDebugViewLog(dwLogData);
			logmsg = dwLogData.GetString();
			theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_URL);

			SendLogDataUtil SendJSONData(strJSONSendFormat);
			char* ptrStr = NULL;
			if (!WinHttpSendRequest(hRequest,
						strHeaders,
						strHeaders.GetLength(),
						SendJSONData.m_pstrData_UTF8,
						SendJSONData.m_UTF8Len,
						SendJSONData.m_UTF8Len,
						0))
			{
				iRet = ERR_SERVER_SEND_REQUEST;
				break;
			}
		}
		//GET
		else
		{
			if (!WinHttpSendRequest(hRequest,
						strHeaders,
						strHeaders.GetLength(),
						WINHTTP_NO_REQUEST_DATA,
						0,
						0,
						0))
			{
				iRet = ERR_SERVER_SEND_REQUEST;
				break;
			}
		}
		if (m_bStop)
		{
			iRet = OK_SERVER;
			break;
		}

		// レスポンスの到着の待機
		if (!WinHttpReceiveResponse(hRequest, NULL))
		{
			iRet = ERR_SERVER_RECEIVE_RESPONSE;
			break;
		}
		if (m_bStop)
		{
			iRet = OK_SERVER;
			break;
		}

		// ステータスコードの取得
		DWORD dwStatusCode = 0;
		DWORD dwStatusCodeSize = sizeof(DWORD);
		if (!WinHttpQueryHeaders(hRequest,
					 WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, // ステータスコードをDWORD型で取得する。
					 WINHTTP_HEADER_NAME_BY_INDEX,				// ヘッダー名のポインタ
					 &dwStatusCode,						// バッファー
					 &dwStatusCodeSize,					// バッファーサイズ
					 WINHTTP_NO_HEADER_INDEX))				// 最初に発生したヘッダーのみ取り出す
		{
			iRet = ERR_SERVER_QUERY_HEADERS;
			break;
		}
		if (m_bStop)
		{
			iRet = OK_SERVER;
			break;
		}

		if (HTTP_STATUS_OK != dwStatusCode)
		{
			iRet = ERR_SERVER_HTTP_STATUS;
			break;
		}
		break;
	}
	if (hRequest)
		WinHttpCloseHandle(hRequest);
	if (hConnect)
		WinHttpCloseHandle(hConnect);
	if (hSession)
		WinHttpCloseHandle(hSession);

	CString strServerErrMsg;
	if (iRet == OK_SERVER)
		strServerErrMsg = _T("OK_SERVER");
	else if (iRet == ERR_SERVER_SETTINGS_NOT_FOUND)
		strServerErrMsg = _T("ERR_SERVER_SETTINGS_NOT_FOUND");
	else if (iRet == ERR_SERVER_SESSION)
		strServerErrMsg = _T("ERR_SERVER_SESSION");
	else if (iRet == ERR_SERVER_SETTINGS_INVALID)
		strServerErrMsg = _T("ERR_SERVER_SETTINGS_INVALID");
	else if (iRet == ERR_SERVER_CONNECT)
		strServerErrMsg = _T("ERR_SERVER_CONNECT");
	else if (iRet == ERR_SERVER_REQUEST)
		strServerErrMsg = _T("ERR_SERVER_REQUEST");
	else if (iRet == ERR_SERVER_SEND_REQUEST)
		strServerErrMsg = _T("ERR_SERVER_SEND_REQUEST");
	else if (iRet == ERR_SERVER_RECEIVE_RESPONSE)
		strServerErrMsg = _T("ERR_SERVER_RECEIVE_RESPONSE");
	else if (iRet == ERR_SERVER_QUERY_HEADERS)
		strServerErrMsg = _T("ERR_SERVER_QUERY_HEADERS");
	else if (iRet == ERR_SERVER_HTTP_STATUS)
		strServerErrMsg = _T("ERR_SERVER_HTTP_STATUS");
	else if (iRet == ERR_SERVER_RESPONSE_EMPTY)
		strServerErrMsg = _T("ERR_SERVER_RESPONSE_EMPTY");
	else if (iRet == ERR_SERVER_RESPONSE_INVALID)
		strServerErrMsg = _T("ERR_SERVER_RESPONSE_INVALID");
	else if (iRet == ERR_SERVER_RESPONSE_STOP)
		strServerErrMsg = _T("ERR_SERVER_RESPONSE_STOP");
	else if (iRet == ERR_SERVER_ETC)
		strServerErrMsg = _T("ERR_SERVER_ETC");
	else
		strServerErrMsg = _T("UNKNOWN_CODE");

	DebugWndLogData dwLogData;
	dwLogData.mHWND.Format(_T("SEND_LOG_WND:0x%08x"), 0);
	dwLogData.mFUNCTION_NAME = _T("SendLogThread:ResultCode");
	dwLogData.mMESSAGE1.Format(_T("ResultCode:%d"), iRet);
	dwLogData.mMESSAGE2 = strServerErrMsg;
	theApp.AppendDebugViewLog(dwLogData);
	logmsg = dwLogData.GetString();
	theApp.WriteDebugTraceDateTime(logmsg, DEBUG_LOG_TYPE_URL);

	_wsetlocale(LC_ALL, _T(""));

	return iRet;
}

CString CLogDispatcher::GetUserNameData()
{
	TCHAR szBuffer[512] = {0};
	ULONG iSize = 512;
	::GetUserNameEx(NameSamCompatible, szBuffer, &iSize);
	CString strRet;
	strRet = szBuffer;
	strRet.TrimLeft();
	strRet.TrimRight();
	return strRet;
}

CString CLogDispatcher::GetComputerNameData()
{
	TCHAR szBuffer[512] = {0};
	ULONG iSize = 512;
	::GetComputerName(szBuffer, &iSize);
	CString strRet;
	strRet = szBuffer;
	strRet.TrimLeft();
	strRet.TrimRight();
	return strRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
