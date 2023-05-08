// DlgDebugWnd.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Sazabi.h"
#include "DlgDebugWnd.h"
#include "afxdialogex.h"

// CDlgDebugWnd ダイアログ

IMPLEMENT_DYNAMIC(CDlgDebugWnd, CDialogEx)

CDlgDebugWnd::CDlgDebugWnd(CWnd* pParent /*=NULL*/)
    : CDialogEx(CDlgDebugWnd::IDD, pParent)
{
	m_hEventLogDebugWnd = NULL;
	m_bAutoScFlg = TRUE;
}

CDlgDebugWnd::~CDlgDebugWnd()
{
	ClearData();
	if (m_hEventLogDebugWnd)
	{
		SetEvent(m_hEventLogDebugWnd);
		CloseHandle(m_hEventLogDebugWnd);
		m_hEventLogDebugWnd = NULL;
	}
}
void CDlgDebugWnd::ClearData()
{
	HANDLE hEvent = {0};
	try
	{
		hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, m_strEventLogDebugWnd);
		if (!hEvent)
			throw std::runtime_error("Failed to execute OpenEvent");

		DWORD waitRes = WaitForSingleObject(hEvent, 100);
		if (waitRes == WAIT_TIMEOUT)
		{
			;
		}
		else
		{
			OWNERDATA* lpDataDelete = {0};
			INT_PTR iSize = 0;
			iSize = m_pOwnerData.GetCount();

			for (INT_PTR i = 0; i < iSize; i++)
			{
				lpDataDelete = NULL;
				lpDataDelete = m_pOwnerData.GetAt(i);
				if (lpDataDelete)
				{
					delete lpDataDelete;
				}
			}
			m_pOwnerData.RemoveAll();
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
void CDlgDebugWnd::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);
}

#pragma warning(push, 0)
#pragma warning(disable : 26454)
BEGIN_MESSAGE_MAP(CDlgDebugWnd, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgDebugWnd::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgDebugWnd::OnBnClickedCancel)
	ON_WM_ACTIVATE()
	ON_WM_NCACTIVATE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgDebugWnd::OnBnClickedButton1)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST1, &CDlgDebugWnd::OnGetdispinfoList1)
	ON_BN_CLICKED(IDC_CHECK1, &CDlgDebugWnd::OnBnClickedCheck1)
END_MESSAGE_MAP()
#pragma warning(pop)

BOOL CDlgDebugWnd::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_autoResize.AddSnapToRightBottom(GetDlgItem(IDC_LIST1));

	CIconHelper ICoHelper;
	ICoHelper = theApp.LoadIcon(IDR_MAINFRAME);
	SetIcon(ICoHelper, TRUE);  // 大きいアイコンの設定
	SetIcon(ICoHelper, FALSE); // 小さいアイコンの設定

	DWORD pidCurrent = GetCurrentProcessId();
	m_strEventLogDebugWnd.Format(_T("CSG_EventLogDWND%08x"), pidCurrent);
	m_hEventLogDebugWnd = CreateEvent(NULL, FALSE, TRUE, m_strEventLogDebugWnd);
	this->CreateListView();

	ListView_SetItemCountEx(m_List.m_hWnd, 0, LVSICF_NOINVALIDATEALL);
	ListView_SetItemState(m_List.m_hWnd, -1, 0, LVIS_SELECTED);

	((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(m_bAutoScFlg);
	CString strCount;
	strCount.LoadString(ID_DEBUG_ITEMS_COUNT);
	CString strDataLineCnt;
	strDataLineCnt.Format(strCount, m_pOwnerData.GetCount());
	SetDlgItemText(IDC_STATIC_LINE, strDataLineCnt);

	int iRet = 0;
	return FALSE; // return TRUE unless you set the focus to a control
}

void CDlgDebugWnd::OnBnClickedCheck1()
{
	if (((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck() == 1)
		m_bAutoScFlg = TRUE;
	else
		m_bAutoScFlg = FALSE;
}

void CDlgDebugWnd::CreateListView()
{
	// リストビューの拡張スタイルの取得とセットマクロ
	DWORD style = m_List.GetExtendedStyle();
	m_List.SetExtendedStyle(style /*| LVS_EX_INFOTIP*/ | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	// カラムヘッダーをセット
	LVCOLUMN lvc = {0};
	lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvc.cx = 150;

	lvc.fmt = LVCFMT_RIGHT;
	lvc.cx = 60;
	lvc.pszText = _T("#");
	lvc.iSubItem = LIST_INDEX;
	m_List.InsertColumn(lvc.iSubItem, &lvc);

	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 100;
	lvc.pszText = _T("Time");
	lvc.iSubItem = LIST_DATE_TIME;
	m_List.InsertColumn(lvc.iSubItem, &lvc);

	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 170;
	lvc.pszText = _T("WndType");
	lvc.iSubItem = LIST_HWND;
	m_List.InsertColumn(lvc.iSubItem, &lvc);

	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 250;
	lvc.pszText = _T("Function");
	lvc.iSubItem = LIST_FUNCTION_NAME;
	m_List.InsertColumn(lvc.iSubItem, &lvc);

	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 800;
	lvc.pszText = _T("Message1");
	lvc.iSubItem = LIST_MESSAGE1;
	m_List.InsertColumn(lvc.iSubItem, &lvc);

	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 180;
	lvc.pszText = _T("Message2");
	lvc.iSubItem = LIST_MESSAGE2;
	m_List.InsertColumn(lvc.iSubItem, &lvc);

	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 130;
	lvc.pszText = _T("Message3");
	lvc.iSubItem = LIST_MESSAGE3;
	m_List.InsertColumn(lvc.iSubItem, &lvc);

	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 85;
	lvc.pszText = _T("Message4");
	lvc.iSubItem = LIST_MESSAGE4;
	m_List.InsertColumn(lvc.iSubItem, &lvc);

	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 85;
	lvc.pszText = _T("Message5");
	lvc.iSubItem = LIST_MESSAGE5;
	m_List.InsertColumn(lvc.iSubItem, &lvc);

	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 85;
	lvc.pszText = _T("Message6");
	lvc.iSubItem = LIST_MESSAGE6;
	m_List.InsertColumn(lvc.iSubItem, &lvc);
	return;
}

void CDlgDebugWnd::OnBnClickedOk()
{
	ClearData();
	ListView_SetItemCountEx(m_List.m_hWnd, m_pOwnerData.GetCount(), LVSICF_NOINVALIDATEALL);
	CString strCount;
	strCount.LoadString(ID_DEBUG_ITEMS_COUNT);
	CString strDataLineCnt;
	strDataLineCnt.Format(strCount, m_pOwnerData.GetCount());
	SetDlgItemText(IDC_STATIC_LINE, strDataLineCnt);
	m_List.InvalidateRect(NULL, TRUE);
}

void CDlgDebugWnd::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
	if (theApp.m_pDebugDlg == this)
	{
		this->DestroyWindow();
		delete theApp.m_pDebugDlg;
		theApp.m_pDebugDlg = NULL;
	}
}
BOOL CDlgDebugWnd::PreTranslateMessage(MSG* pMsg)
{
	ASSERT(pMsg != NULL);
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);
	CString strLine;

	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
	}
	if (pMsg->hwnd == m_List.m_hWnd)
	{
		if (pMsg->message == WM_KEYDOWN)
		{
			if (pMsg->wParam == 'A')
			{
				if (::GetKeyState(VK_CONTROL) < 0)
				{
					CWaitCursor wc;
					HWND hWnd = m_List.m_hWnd;
					if (hWnd && m_pOwnerData.GetCount() > 0)
					{
						ListView_SetItemState(hWnd, -1, 0, LVIS_SELECTED); // deselect all items
						ListView_SetItemState(hWnd, -1, LVIS_SELECTED, LVIS_SELECTED);
						m_List.InvalidateRect(NULL, FALSE);
						return TRUE;
					}
				}
			}
			else if (pMsg->wParam == 'C')
			{
				if (::GetKeyState(VK_CONTROL) < 0)
				{
					HWND hWnd = m_List.m_hWnd;
					if (hWnd && m_pOwnerData.GetCount() > 0)
					{
						OnBnClickedButton1();
						return TRUE;
					}
				}
			}
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}
BOOL CDlgDebugWnd::OnNcActivate(BOOL bActive)
{
	return CDialogEx::OnNcActivate(bActive);
}

void CDlgDebugWnd::OnPaint()
{
	CPaintDC dc(this);
}

void CDlgDebugWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialogEx::OnActivate(nState, pWndOther, bMinimized);
}

void CDlgDebugWnd::OnBnClickedButton1()
{
	if (::GetKeyState(VK_CONTROL) >= 0)
	{
		ListView_SetItemState(m_List.m_hWnd, -1, 0, LVIS_SELECTED); // deselect all items
		ListView_SetItemState(m_List.m_hWnd, -1, LVIS_SELECTED, LVIS_SELECTED);
	}

	CWaitCursor wc;
	POSITION pos = m_List.GetFirstSelectedItemPosition();
	int nItem = 0;

	if (!pos)
		return;

	CTypedPtrArray<CPtrArray, OWNERDATA*> pptPIDArray;
	OWNERDATA* lpDataLine = {0};

	while (pos)
	{
		nItem = m_List.GetNextSelectedItem(pos);
		lpDataLine = NULL;
		lpDataLine = new OWNERDATA;
		if (lpDataLine)
		{
			//DataCopy
			lpDataLine->iIndex = m_pOwnerData.GetAt(nItem)->iIndex;
			lpDataLine->mDATE_TIME = m_pOwnerData.GetAt(nItem)->mDATE_TIME;
			lpDataLine->mHWND = m_pOwnerData.GetAt(nItem)->mHWND;
			lpDataLine->mFUNCTION_NAME = m_pOwnerData.GetAt(nItem)->mFUNCTION_NAME;
			lpDataLine->mMESSAGE1 = m_pOwnerData.GetAt(nItem)->mMESSAGE1;
			lpDataLine->mMESSAGE2 = m_pOwnerData.GetAt(nItem)->mMESSAGE2;
			lpDataLine->mMESSAGE3 = m_pOwnerData.GetAt(nItem)->mMESSAGE3;
			lpDataLine->mMESSAGE4 = m_pOwnerData.GetAt(nItem)->mMESSAGE4;
			lpDataLine->mMESSAGE5 = m_pOwnerData.GetAt(nItem)->mMESSAGE5;
			lpDataLine->mMESSAGE6 = m_pOwnerData.GetAt(nItem)->mMESSAGE6;
			pptPIDArray.Add(lpDataLine);
		}
	}
	int iSelCnt = (int)pptPIDArray.GetCount();
	CString strHeader;
	CString strCopyData;
	CString strCopyDataALL;
	CString strFmt;
	strHeader.Format(_T("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\r\n"),
			 _T("#"),
			 _T("Time"),
			 _T("WndType"),
			 _T("Function"),
			 _T("Message1"),
			 _T("Message2"),
			 _T("Message3"),
			 _T("Message4"),
			 _T("Message5"),
			 _T("Message6"));
	for (int iii = 0; iii < iSelCnt; iii++)
	{
		if (iii == 0)
			strCopyDataALL = strHeader;

		lpDataLine = NULL;
		lpDataLine = pptPIDArray.GetAt(iii);
		DebugWndLogData LogData;
		CString strTemp;
		if (lpDataLine)
		{
			strTemp.Empty();
			LogData.mDATE_TIME = lpDataLine->mDATE_TIME;
			LogData.mHWND = lpDataLine->mHWND;
			LogData.mFUNCTION_NAME = lpDataLine->mFUNCTION_NAME;
			LogData.mMESSAGE1 = lpDataLine->mMESSAGE1;
			LogData.mMESSAGE2 = lpDataLine->mMESSAGE2;
			LogData.mMESSAGE3 = lpDataLine->mMESSAGE3;
			LogData.mMESSAGE4 = lpDataLine->mMESSAGE4;
			LogData.mMESSAGE5 = lpDataLine->mMESSAGE5;
			LogData.mMESSAGE6 = lpDataLine->mMESSAGE6;
			strTemp.Format(_T("%d\t"), lpDataLine->iIndex);
			strTemp += LogData.GetString(TRUE);
			strTemp += _T("\r\n");
			delete lpDataLine;
		}
		if (!strTemp.IsEmpty())
		{
			strCopyDataALL += strTemp;
		}
	}

	if (strCopyDataALL.IsEmpty())
		return;
	if (!::OpenClipboard(NULL))
		return;

	int nByte = (strCopyDataALL.GetLength() + 1) * sizeof(TCHAR);
	HGLOBAL hText = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, nByte);

	if (hText == NULL)
		return;

	BYTE* pText = (BYTE*)::GlobalLock(hText);

	if (pText == NULL)
		return;

	::memcpy(pText, (LPCTSTR)strCopyDataALL, nByte);
	::GlobalUnlock(hText);
	::OpenClipboard(NULL);
	::EmptyClipboard();
	::SetClipboardData(CF_UNICODETEXT, hText);
	::CloseClipboard();
}

void CDlgDebugWnd::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	m_autoResize.Resize(this);
}

void CDlgDebugWnd::SetLogMsg(LPCTSTR pDATE_TIME,
			     LPCTSTR pHWND,
			     LPCTSTR pFUNCTION_NAME,
			     LPCTSTR pMESSAGE1,
			     LPCTSTR pMESSAGE2,
			     LPCTSTR pMESSAGE3,
			     LPCTSTR pMESSAGE4,
			     LPCTSTR pMESSAGE5,
			     LPCTSTR pMESSAGE6)
{
	HANDLE hEvent = {0};
	try
	{
		hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, m_strEventLogDebugWnd);
		if (!hEvent)
			throw std::runtime_error("Failed to execute OpenEvent");

		DWORD waitRes = WaitForSingleObject(hEvent, 100);
		if (waitRes == WAIT_TIMEOUT)
		{
			;
		}
		else
		{
			INT_PTR iCount = 0;
			iCount = m_pOwnerData.GetCount();
			OWNERDATA* pData = NULL;
			pData = new OWNERDATA;
			pData->iIndex = iCount + 1;
			if (pDATE_TIME)
				pData->mDATE_TIME = pDATE_TIME;
			if (pHWND)
				pData->mHWND = pHWND;
			if (pFUNCTION_NAME)
				pData->mFUNCTION_NAME = pFUNCTION_NAME;
			if (pMESSAGE1)
				pData->mMESSAGE1 = pMESSAGE1;
			if (pMESSAGE2)
				pData->mMESSAGE2 = pMESSAGE2;
			if (pMESSAGE3)
				pData->mMESSAGE3 = pMESSAGE3;
			if (pMESSAGE4)
				pData->mMESSAGE4 = pMESSAGE4;
			if (pMESSAGE5)
				pData->mMESSAGE5 = pMESSAGE5;
			if (pMESSAGE6)
				pData->mMESSAGE6 = pMESSAGE6;
			m_pOwnerData.Add(pData);
			::SendMessageTimeout(m_List.m_hWnd, LVM_SETITEMCOUNT, (WPARAM)m_pOwnerData.GetCount(), (LPARAM)LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL, SMTO_NORMAL, 250, NULL);
			CString strCount;
			strCount.LoadString(ID_DEBUG_ITEMS_COUNT);
			CString strDataLineCnt;
			strDataLineCnt.Format(strCount, m_pOwnerData.GetCount());
			SetDlgItemText(IDC_STATIC_LINE, strDataLineCnt);
			if (m_bAutoScFlg)
			{
				::SendMessageTimeout(m_List.m_hWnd,
						     LVM_ENSUREVISIBLE,
						     (int)iCount,
						     MAKELPARAM(0, 0),
						     SMTO_NORMAL,
						     250,
						     NULL);
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

void CDlgDebugWnd::OnGetdispinfoList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	__try
	{
		NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
		LVITEM* item = &(pDispInfo)->item;
		OWNERDATA* pData = NULL;
		pData = m_pOwnerData.GetAt(item->iItem);
		if (!pData)
			return;
		// テキストをセット
		if (item->mask & LVIF_TEXT)
		{
			switch (item->iSubItem)
			{
			case LIST_INDEX:
			{
				this->DWToString(pData->iIndex, m_strTemp);
				lstrcpy(item->pszText, m_strTemp);
				break;
			}
			case LIST_DATE_TIME:
			{
				item->pszText = (LPTSTR)(LPCTSTR)pData->mDATE_TIME;
				break;
			}
			case LIST_HWND:
			{
				item->pszText = (LPTSTR)(LPCTSTR)pData->mHWND;
				break;
			}
			case LIST_FUNCTION_NAME:
			{
				item->pszText = (LPTSTR)(LPCTSTR)pData->mFUNCTION_NAME;
				break;
			}
			case LIST_MESSAGE1:
			{
				item->pszText = (LPTSTR)(LPCTSTR)pData->mMESSAGE1;
				break;
			}
			case LIST_MESSAGE2:
			{
				item->pszText = (LPTSTR)(LPCTSTR)pData->mMESSAGE2;
				break;
			}
			case LIST_MESSAGE3:
			{
				item->pszText = (LPTSTR)(LPCTSTR)pData->mMESSAGE3;
				break;
			}
			case LIST_MESSAGE4:
			{
				item->pszText = (LPTSTR)(LPCTSTR)pData->mMESSAGE4;
				break;
			}
			case LIST_MESSAGE5:
			{
				item->pszText = (LPTSTR)(LPCTSTR)pData->mMESSAGE5;
				break;
			}
			case LIST_MESSAGE6:
			{
				item->pszText = (LPTSTR)(LPCTSTR)pData->mMESSAGE6;
				break;
			}
			default:
				break;
			}
		}
		*pResult = 0;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

IMPLEMENT_DYNAMIC(CDlgSCEditor, CDialogEx)

CDlgSCEditor::CDlgSCEditor(CWnd* pParent /*=NULL*/)
    : CDialogEx(CDlgSCEditor::IDD, pParent)
{
	bFirstFlg = FALSE;
	m_SelPosE = 0;
	m_SelPosS = 0;
}

CDlgSCEditor::~CDlgSCEditor()
{
}

void CDlgSCEditor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_Edit);
}

BEGIN_MESSAGE_MAP(CDlgSCEditor, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &CDlgSCEditor::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_FUNC, &CDlgSCEditor::OnBnClickedButtonFunc)
	ON_BN_CLICKED(IDC_BUTTON_TOP, &CDlgSCEditor::OnBnClickedButtonTop)
	ON_BN_CLICKED(IDC_BUTTON_URL, &CDlgSCEditor::OnBnClickedButtonUrl)
	ON_BN_CLICKED(IDC_BUTTON_TB_TRACE_LOG, &CDlgSCEditor::OnBnClickedButtonTraceLog)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgSCEditor::OnBnClickedButton1)

	ON_BN_CLICKED(IDOK, &CDlgSCEditor::OnBnClickedOk)
	ON_WM_ACTIVATE()
	ON_WM_NCACTIVATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CDlgSCEditor メッセージ ハンドラー
BOOL CDlgSCEditor::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_autoResize.AddSnapToRightBottom(GetDlgItem(IDC_EDIT1));
	bFirstFlg = TRUE;
	m_SelPosS = 0;
	m_SelPosE = 0;
	m_Edit.LimitText(1024 * 1024 * 10);
	this->ReadText();

	this->m_Edit.SetSel(0, 0);
	this->m_Edit.SetFocus();

	CFont* pFont = m_Edit.GetFont();
	TEXTMETRIC tm = {0};

	CDC* pDC = m_Edit.GetDC();

	CFont* pOldFont = pDC->SelectObject(pFont);

	pDC->GetTextMetrics(&tm);
	pDC->SelectObject(pOldFont);

	CRect rect(0, 0, 100, 1);
	::MapDialogRect((HWND)this, rect);

	int iTab = (4 * tm.tmAveCharWidth * 100) / rect.Width();
	iTab = iTab / 2;
	m_Edit.SetTabStops(iTab);
	return FALSE; // return TRUE unless you set the focus to a control
}

BOOL CDlgSCEditor::PreTranslateMessage(MSG* pMsg)
{
	ASSERT(pMsg != NULL);
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);
	CString strLine;

	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
	}
	if (pMsg->hwnd == m_Edit.m_hWnd)
	{
		if (pMsg->message == WM_KEYDOWN)
		{
			if (pMsg->wParam == VK_TAB)
			{
				int nPos = LOWORD(m_Edit.CharFromPos(m_Edit.GetCaretPos()));
				m_Edit.SetSel(nPos, nPos);
				m_Edit.ReplaceSel(_T("\t"), TRUE);
				return TRUE;
			}
			else if (pMsg->wParam == 'S')
			{
				if (::GetKeyState(VK_CONTROL) < 0)
				{
					PostMessage(WM_COMMAND, IDOK);
					return TRUE;
				}
			}
			else if (pMsg->wParam == 'A')
			{
				if (::GetKeyState(VK_CONTROL) < 0)
				{
					m_Edit.PostMessage(EM_SETSEL, (WPARAM)0, (LPARAM)-1);
					return TRUE;
				}
			}
		}
		if (pMsg->message == WM_MOUSEMOVE ||
		    pMsg->message == WM_LBUTTONDOWN ||
		    pMsg->message == WM_KEYDOWN ||
		    pMsg->message == WM_KEYUP ||
		    pMsg->message == WM_LBUTTONDOWN)
		{
			DWORD dwStart = 0, dwEnd = 0;
			DWORD dwLineIndex = 0;
			CPoint pt;
			BOOL lr = {0};
			if (pMsg->message == WM_MOUSEMOVE && dwStart == dwEnd)
				return lr;
			lr = CDialogEx::PreTranslateMessage(pMsg);

			dwLineIndex = (DWORD)m_Edit.SendMessage(EM_LINEFROMCHAR, -1, 0);
			CString strLineCount;
			strLineCount.LoadString(ID_DEBUG_LINES_COUNT);
			strLine.Format(strLineCount, dwLineIndex + 1);
			SetDlgItemText(IDC_STATIC_LINE, strLine);
			return lr;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}
BOOL CDlgSCEditor::OnNcActivate(BOOL bActive)
{
	if (bActive)
	{
		if (bFirstFlg)
		{
			bFirstFlg = FALSE;
		}
		else
			m_Edit.PostMessage(EM_SETSEL, (WPARAM)m_SelPosS, (LPARAM)m_SelPosE);
	}
	else
	{
		m_Edit.GetSel(m_SelPosS, m_SelPosE);
	}
	return CDialogEx::OnNcActivate(bActive);
}

void CDlgSCEditor::OnPaint()
{
	CPaintDC dc(this);
}

void CDlgSCEditor::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialogEx::OnActivate(nState, pWndOther, bMinimized);
}

void CDlgSCEditor::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	m_autoResize.Resize(this);
}

void CDlgSCEditor::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CDialogEx::OnCancel();
}

void CDlgSCEditor::OnBnClickedButtonFunc()
{
	int nPos = LOWORD(m_Edit.CharFromPos(m_Edit.GetCaretPos()));
	m_Edit.SetSel(nPos, nPos);
	CString strText;
	strText = _T("Function OnRedirect()\r\n");
	strText += _T("  OnRedirect=\"Default\"\r\n");
	strText += _T("  Exit Function\r\n\r\n");
	strText += _T("  OnRedirect=\"IE\"\r\n");
	strText += _T("  Exit Function\r\n\r\n");
	strText += _T("  OnRedirect=\"Firefox\"\r\n");
	strText += _T("  Exit Function\r\n\r\n");
	strText += _T("  OnRedirect=\"Chrome\"\r\n");
	strText += _T("  Exit Function\r\n\r\n");
	strText += _T("  OnRedirect=\"Edge\"\r\n");
	strText += _T("  Exit Function\r\n\r\n");
	strText += _T("  OnRedirect=\"Custom1\"\r\n");
	strText += _T("  Exit Function\r\n\r\n");
	strText += _T("  OnRedirect=\"Custom2\"\r\n");
	strText += _T("  Exit Function\r\n\r\n");
	strText += _T("  OnRedirect=\"Custom3\"\r\n");
	strText += _T("  Exit Function\r\n\r\n");
	strText += _T("  OnRedirect=\"Custom4\"\r\n");
	strText += _T("  Exit Function\r\n\r\n");
	strText += _T("  OnRedirect=\"Custom5\"\r\n");
	strText += _T("  Exit Function\r\n\r\n");
	strText += _T("  OnRedirect=\"Block\"\r\n");
	strText += _T("  Exit Function\r\n\r\n");
	strText += _T("End Function\r\n");

	m_Edit.ReplaceSel(strText, TRUE);

	this->m_Edit.SetFocus();
}

void CDlgSCEditor::OnBnClickedButton1()
{
	int nPos = LOWORD(m_Edit.CharFromPos(m_Edit.GetCaretPos()));
	m_Edit.SetSel(nPos, nPos);
	CString strHelpString;
	strHelpString.LoadString(ID_URL_REDIRECT_SCRIPT_HELP);
	m_Edit.ReplaceSel(strHelpString, TRUE);
	this->m_Edit.SetFocus();
}

void CDlgSCEditor::OnBnClickedButtonTop()
{
	int nPos = LOWORD(m_Edit.CharFromPos(m_Edit.GetCaretPos()));
	m_Edit.SetSel(nPos, nPos);
	m_Edit.ReplaceSel(_T("TB_Global_TOP_PAGE\r\n"), TRUE);
	this->m_Edit.SetFocus();
}

void CDlgSCEditor::OnBnClickedButtonTraceLog()
{
	int nPos = LOWORD(m_Edit.CharFromPos(m_Edit.GetCaretPos()));
	m_Edit.SetSel(nPos, nPos);
	CString defaultLog;
	defaultLog.LoadString(ID_URL_REDIRECT_SCRIPT_TRACE_LOG_DEFAULT);
	CString templateStr;
	templateStr = _T("TB_TRACE_LOG(\"");
	templateStr += defaultLog;
	templateStr += _T("\")\r\n");
	defaultLog.LoadString(ID_URL_REDIRECT_SCRIPT_TRACE_LOG_DEFAULT);
	m_Edit.ReplaceSel(templateStr, TRUE);
	this->m_Edit.SetFocus();
}
void CDlgSCEditor::OnBnClickedButtonUrl()
{
	int nPos = LOWORD(m_Edit.CharFromPos(m_Edit.GetCaretPos()));
	m_Edit.SetSel(nPos, nPos);
	m_Edit.ReplaceSel(_T("TB_Global_URL\r\n"), TRUE);
	this->m_Edit.SetFocus();
}

void CDlgSCEditor::OnBnClickedOk()
{
	int iRet = 0;
	CString confirmMsg;
	confirmMsg.LoadString(ID_DEBUG_CONFIRM_SAVE_RESULT);
	iRet = ::MessageBox(this->m_hWnd, confirmMsg, theApp.m_strThisAppName, MB_ICONQUESTION | MB_YESNO);
	if (iRet != IDYES)
		return;
	WriteText();
}

#pragma warning(disable : 4244)
void CDlgSCEditor::ReadText()
{
	_wsetlocale(LC_ALL, _T("jpn"));
	CStdioFile file;
	CString strText;
	CString strData;
	CString strLine;
	if (file.Open(m_strFilePath, CFile::modeRead | CFile::shareDenyNone))
	{
		int iLineNum = 0;
		int iCommentStartLine = 0;
		BOOL bFindComment = FALSE;
		while (file.ReadString(strLine))
		{
			strData += strLine;
			strData += _T("\r\n");
		}
		strText = strData;
		file.Close();
	}
	m_Edit.SetWindowText(strText);
}

void CDlgSCEditor::WriteText()
{
	_wsetlocale(LC_ALL, _T("jpn"));
	CFile file;
	CString strText;
	m_Edit.GetWindowText(strText);

	CString strComment;
	CString strData;
	strData = strComment;
	strData += strText;

	CStringA strDataA;
	strDataA = strData;
	if (file.Open(m_strFilePath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite))
	{
		file.Write((LPCSTR)strDataA, strDataA.GetLength());
		file.Close();
		theApp.m_cScriptSrc.RefreshData();
		theApp.m_CacheRedirectFilter_None.RemoveAll();
		if (theApp.InVirtualEnvironment() == VE_THINAPP)
		{
			CString strSBFilePath;
			strSBFilePath = theApp.GetSandboxFilePath(m_strFilePath);
			if (!strSBFilePath.IsEmpty())
			{
				CString strFilePathCnf;
				strFilePathCnf = m_strFilePath;
				strFilePathCnf.Replace(_T("Default.conf"), _T(".conf"));
				theApp.Exec_SB2PYS_COPY(strSBFilePath, strFilePathCnf);
			}
		}
	}
	else
	{
		CString alertMsg;
		alertMsg.LoadString(ID_DEBUG_ALERT_SAVE_FAILED);
		CString strSaveMsg;
		strSaveMsg.Format(alertMsg, m_strFilePath);
		::MessageBox(this->m_hWnd, strSaveMsg, theApp.m_strThisAppName, MB_OK | MB_ICONERROR);
	}
}
