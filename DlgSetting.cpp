#include "stdafx.h"
#include "Sazabi.h"
#include "DlgSetting.h"
#include "afxdialogex.h"
IMPLEMENT_DYNAMIC(CPrefsStatic, CStatic)
BEGIN_MESSAGE_MAP(CPrefsStatic, CStatic)
	//{{AFX_MSG_MAP(CPrefsStatic)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
void CPrefsStatic::OnPaint()
{
	CPaintDC dc(this);
	CFont* pOldFont = NULL;
	if (m_csFontName == "")
	{
		CWnd* pWndParent = GetParent();
		if (pWndParent)
		{
			dc.SelectObject(pWndParent->GetFont());
		}
	}
	else
	{
		if (m_captionFont.GetSafeHandle() == NULL)
		{
			m_captionFont.CreateFont(m_fontSize,
						 0, 0, 0,
						 m_fontWeight,
						 0, 0, 0, SHIFTJIS_CHARSET,
						 OUT_DEFAULT_PRECIS,
						 CLIP_DEFAULT_PRECIS,
						 DEFAULT_QUALITY,
						 FF_MODERN,
						 m_csFontName);
		}

		if (m_captionFont.GetSafeHandle() != NULL)
			pOldFont = dc.SelectObject(&m_captionFont);
	}

	CString strText;
	GetWindowText(strText);

	dc.SetTextColor(m_textClr);
	dc.SetBkMode(TRANSPARENT);

	CRect cr;
	GetClientRect(cr);

	cr.left += 5;
	dc.DrawText(strText, cr, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

	if (pOldFont)
		dc.SelectObject(pOldFont);
}

BOOL CPrefsStatic::OnEraseBkgnd(CDC* pDC)
{
	if (!m_bm.GetSafeHandle())
	{
		MakeCaptionBitmap();
	}

	if (m_bm.GetSafeHandle())
	{
		CRect cr;
		GetClientRect(cr);
		CDC memDC;
		memDC.CreateCompatibleDC(pDC);
		CBitmap* pOB = memDC.SelectObject(&m_bm);
		pDC->BitBlt(0, 0, cr.Width(), cr.Height(), &memDC, 0, 0, SRCCOPY);
		memDC.SelectObject(pOB);
	}
	return TRUE;
}

LRESULT CPrefsStatic::OnSetText(WPARAM wParam, LPARAM lParam)
{
	DefWindowProc(WM_SETTEXT, wParam, lParam);
	Invalidate(TRUE);
	return (1);
}

static void PaintRect(CDC& dc, int x, int y, int w, int h, COLORREF color)
{
	CBrush brush(color);
	CBrush* pOldBrush = dc.SelectObject(&brush);
	dc.PatBlt(x, y, w, h, PATCOPY);
	dc.SelectObject(pOldBrush);
}

void CPrefsStatic::MakeCaptionBitmap()
{
	if (m_bm.m_hObject)
		return;
	CRect cr;
	GetClientRect(cr);
	int w = cr.Width();
	int h = cr.Height();

	CWindowDC dcWin(this);
	CDC dc;
	dc.CreateCompatibleDC(&dcWin);
	m_bm.DeleteObject();
	m_bm.CreateCompatibleBitmap(&dcWin, w, h);
	CBitmap* pOldBitmap = dc.SelectObject(&m_bm);

	COLORREF clrBG = ::GetSysColor(COLOR_3DFACE);
	int r = GetRValue(clrBG);
	int g = GetGValue(clrBG);
	int b = GetBValue(clrBG);
	int x = 8 * cr.right / 8;
	int w1 = x - cr.left;
	int NCOLORSHADES = 128;
	int xDelta = max(w / NCOLORSHADES, 1);
	PaintRect(dc, x, 0, cr.right - x, h, clrBG);
	while (x > xDelta)
	{
		x -= xDelta;
		int wmx2 = (w1 - x) * (w1 - x);
		int w2 = w1 * w1;
		PaintRect(dc, x, 0, xDelta, h,
			  RGB(r - (r * wmx2) / w2, g - (g * wmx2) / w2, b - (b * wmx2) / w2));
	}
	PaintRect(dc, 0, 0, x, h, RGB(0, 0, 0));
	if (m_nameFont.GetSafeHandle() == NULL)
	{
		m_nameFont.CreateFont(18, 0, 0, 0, FW_BOLD,
				      0, 0, 0, ANSI_CHARSET,
				      OUT_DEFAULT_PRECIS,
				      CLIP_DEFAULT_PRECIS,
				      DEFAULT_QUALITY,
				      FF_MODERN,
				      m_csFontName);
	}
	CFont* OldFont = dc.SelectObject(&m_nameFont);
	cr.right -= 5;
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(::GetSysColor(COLOR_3DHILIGHT));
	dc.DrawText(m_csConstantText, cr + CPoint(1, 1), DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
	dc.SetTextColor(::GetSysColor(COLOR_3DSHADOW));
	dc.DrawText(m_csConstantText, cr, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
	dc.SelectObject(OldFont);
	dc.SelectObject(pOldBitmap);
}

/////////////////////////////////////////////////////////////////////////////
// CSettingsDialog dialog
CSettingsDialog::CSettingsDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CSettingsDialog::IDD, pParent)
{
	m_pInfo.RemoveAll();
	m_csTitle = _T("");
	m_pParent = pParent;
}

/////////////////////////////////////////////////////////////////////////////
//
CSettingsDialog::~CSettingsDialog()
{
	for (int i = 0; i < m_pInfo.GetSize(); i++)
	{
		PAGE_INFO* pInfo = (PAGE_INFO*)m_pInfo.GetAt(i);
		if (pInfo)
		{
			delete pInfo;
		}
	}
	m_pInfo.RemoveAll();
}

void CSettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsDialog)
	DDX_Control(pDX, IDC_PAGE_FRAME, m_PageFrame);
	DDX_Control(pDX, IDC_TREE_CTRL, m_TreeCtrl);
	DDX_Control(pDX, IDC_CAPTION_BAR, m_CaptionBarCtrl);
	//}}AFX_DATA_MAP
}

#pragma warning(push, 0)
//警告 C26454 演算のオーバーフロー : '-' の操作では、コンパイル時に負の符号なしの結果が生成されます(io .5)。
#pragma warning(disable : 26454)
BEGIN_MESSAGE_MAP(CSettingsDialog, CDialog)
	//{{AFX_MSG_MAP(CSettingsDialog)
	ON_NOTIFY(TVN_GETDISPINFO, IDC_TREE_CTRL, OnGetDispInfoTreeCtrl)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_CTRL, OnTreeSelChanged)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#pragma warning(pop)

/////////////////////////////////////////////////////////////////////////////
// CSettingsDialog message handlers
//
BOOL CSettingsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	CIconHelper ICoHelper;
	ICoHelper = theApp.LoadIcon(IDR_MAINFRAME);
	SetIcon(ICoHelper, TRUE);
	SetIcon(ICoHelper, FALSE);

	SetWindowText(m_csTitle);

	m_CaptionBarCtrl.m_textClr = ::GetSysColor(COLOR_3DFACE);
	m_CaptionBarCtrl.m_fontWeight = FW_BOLD;
	m_CaptionBarCtrl.m_fontSize = 14;
	m_CaptionBarCtrl.m_csFontName = _T("Verdana");
	m_CaptionBarCtrl.SetConstantText(m_csLogoText);

	m_PageFrame.GetWindowRect(m_FrameRect);
	ScreenToClient(m_FrameRect);
	m_FrameRect.DeflateRect(2, 2);
	InitTreeCtrl();
	ExpandTree();
	if (m_pInfo.GetAt(0)) ShowPage(m_pInfo.GetAt(0));
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//
void CSettingsDialog::InitTreeCtrl()
{
	for (int i = 0; i < m_pInfo.GetSize(); i++)
	{
		PAGE_INFO* pInfo = (PAGE_INFO*)m_pInfo.GetAt(i);
		if (!pInfo) continue;
		TV_INSERTSTRUCT tvi = {0};
		if (pInfo->pWndParent)
			tvi.hParent = FindItem(pInfo->pWndParent);
		else
			tvi.hParent = FindItem(pInfo->csParentCaption);
		tvi.hInsertAfter = TVI_LAST;
		tvi.item.cchTextMax = 0;
		tvi.item.pszText = LPSTR_TEXTCALLBACK;
		tvi.item.lParam = (INT_PTR)pInfo;
		tvi.item.mask = TVIF_PARAM | TVIF_TEXT;
		HTREEITEM hTree = m_TreeCtrl.InsertItem(&tvi);
		if (hTree && pInfo->pWnd)
		{
			INT_PTR dwTree = (INT_PTR)hTree;
			m_wndMap.SetAt(pInfo->pWnd, dwTree);
		}
	}
}

CWnd* CSettingsDialog::AddPage(CRuntimeClass* pWndClass,
			       LPCTSTR pCaption, UINT nID,
			       CWnd* pWndParent)
{
	CWnd* pWnd = NULL;
	if (m_hWnd)
	{
		ASSERT(0);
		return pWnd;
	}
	if (pWndClass) pWnd = (CWnd*)pWndClass->CreateObject();

	PAGE_INFO* pInfo = new PAGE_INFO;
	pInfo->bViewClass = TRUE;
	if (pWnd)
	{
		if (pWnd->IsKindOf(RUNTIME_CLASS(CDialog))) pInfo->bViewClass = FALSE;
	}

	pInfo->nID = nID;
	pInfo->pWnd = pWnd;
	pInfo->csCaption = pCaption;
	pInfo->pWndParent = pWndParent;
	pInfo->csParentCaption.Empty();
	m_pInfo.Add(pInfo);
	return pWnd;
}

CWnd* CSettingsDialog::AddPage(CRuntimeClass* pWndClass, LPCTSTR pCaption,
			       UINT nID, LPCTSTR pParentCaption)
{
	CWnd* pWnd = NULL;
	if (m_hWnd)
	{
		ASSERT(0);
		return pWnd;
	}
	if (pWndClass) pWnd = (CWnd*)pWndClass->CreateObject();
	PAGE_INFO* pInfo = new PAGE_INFO;
	pInfo->bViewClass = TRUE;
	if (pWnd)
	{
		if (pWnd->IsKindOf(RUNTIME_CLASS(CDialog))) pInfo->bViewClass = FALSE;
	}
	pInfo->nID = nID;
	pInfo->pWnd = pWnd;
	pInfo->csCaption = pCaption;
	pInfo->pWndParent = NULL;
	pInfo->csParentCaption = pParentCaption;
	m_pInfo.Add(pInfo);
	return pWnd;
}

HTREEITEM CSettingsDialog::FindItem(CWnd* pWnd)
{
	if (pWnd == NULL)
		return TVI_ROOT;
	else
	{
		INT_PTR dwHTree;
		if (m_wndMap.Lookup(pWnd, dwHTree))
			return (HTREEITEM)dwHTree;
		else
		{
			ASSERT(FALSE);
			return (TVI_ROOT);
		}
	}
}

HTREEITEM CSettingsDialog::FindItem(const CString& csCaption)
{
	if (m_TreeCtrl.GetCount() == 0) return TVI_ROOT;
	if (csCaption.IsEmpty()) return TVI_ROOT;

	HTREEITEM hCurrent = m_TreeCtrl.GetRootItem();
	while (hCurrent)
	{
		CString strText = m_TreeCtrl.GetItemText(hCurrent);
		if (!strText.CompareNoCase(csCaption)) return hCurrent;
		hCurrent = GetNextItemCOrS(hCurrent);
	}
	return TVI_ROOT;
}

HTREEITEM CSettingsDialog::GetNextItemCOrS(HTREEITEM hItem)
{
	HTREEITEM hti = {0};
	if (m_TreeCtrl.ItemHasChildren(hItem))
		return m_TreeCtrl.GetChildItem(hItem);
	else
	{
		while ((hti = m_TreeCtrl.GetNextSiblingItem(hItem)) == NULL)
		{
			if ((hItem = m_TreeCtrl.GetParentItem(hItem)) == NULL)
				return NULL;
		}
	}
	return hti;
}

void CSettingsDialog::ShowPage(const PAGE_INFO* pInfo, UINT nShow)
{
	if (!pInfo) return;
	m_CaptionBarCtrl.SetWindowText(_T(""));

	if (pInfo->pWnd)
	{
		if (!::IsWindow(pInfo->pWnd->m_hWnd))
		{
			CreatePage(pInfo);
			pInfo->pWnd->SetWindowPos(&m_TreeCtrl, 0, 0, 0, 0, 0);
			pInfo->pWnd->MoveWindow(m_FrameRect.left, m_FrameRect.top, m_FrameRect.Width(), m_FrameRect.Height());
			if (pInfo->pWnd->IsKindOf(RUNTIME_CLASS(CView)))
				((CView*)pInfo->pWnd)->OnInitialUpdate();
		}

		pInfo->pWnd->ShowWindow(nShow);
		if (nShow == SW_SHOW)
		{
			pInfo->pWnd->SetFocus();
			HTREEITEM hItem = FindItem(pInfo->pWnd);
			if (hItem) m_TreeCtrl.SelectItem(hItem);
		}
	}
	if (nShow == SW_SHOW)
		m_CaptionBarCtrl.SetWindowText(pInfo->csCaption);
}

BOOL CSettingsDialog::CreatePage(const PAGE_INFO* pInfo)
{
	BOOL bCode = FALSE;

	if (!pInfo || !pInfo->pWnd) return (FALSE);
	if (!::IsWindow(pInfo->pWnd->m_hWnd))
	{
		if (pInfo->pWnd->IsKindOf(RUNTIME_CLASS(CDialog)))
		{
			CDialog* pDlg = (CDialog*)pInfo->pWnd;
			bCode = pDlg->Create(pInfo->nID, this);
			pDlg->ModifyStyle(WS_CAPTION, 0);
		}
		else if (pInfo->pWnd->IsKindOf(RUNTIME_CLASS(CWnd)))
		{
			CWnd* pWnd = (CWnd*)pInfo->pWnd;
			bCode = CreateWnd(pInfo->pWnd);
			pWnd->ModifyStyle(WS_BORDER | WS_THICKFRAME, 0);
		}
	}
	return (bCode);
}

BOOL CSettingsDialog::CreateWnd(CWnd* pWnd, CCreateContext* pContext)
{
	CCreateContext context;
	if (pContext == NULL)
	{
		context.m_pCurrentFrame = (CFrameWnd*)this;
		context.m_pCurrentDoc = NULL;
		context.m_pNewViewClass = NULL;
		pContext = &context;
	}

	ASSERT_KINDOF(CWnd, pWnd);
	ASSERT(pWnd->m_hWnd == NULL);

	DWORD dwStyle = AFX_WS_DEFAULT_VIEW;
	CRect rect;
	if (!pWnd->Create(NULL, NULL, dwStyle, rect, this, 0, pContext))
	{
		return FALSE;
	}
	return (TRUE);
}

void CSettingsDialog::OnGetDispInfoTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	if (pTVDispInfo->item.lParam)
	{
		if (pTVDispInfo->item.mask & TVIF_TEXT)
		{
			PAGE_INFO* pInfo = (PAGE_INFO*)pTVDispInfo->item.lParam;
			pTVDispInfo->item.pszText = pInfo->csCaption.GetBufferSetLength(pInfo->csCaption.GetLength());
			pInfo->csCaption.ReleaseBuffer();
		}
	}

	*pResult = 0;
}

void CSettingsDialog::OnTreeSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	if (pNMTreeView->itemNew.lParam)
	{
		PAGE_INFO* pInfo = (PAGE_INFO*)pNMTreeView->itemOld.lParam;
		ShowPage(pInfo, SW_HIDE);
		pInfo = (PAGE_INFO*)pNMTreeView->itemNew.lParam;
		ShowPage(pInfo, SW_SHOW);
	}
	*pResult = 0;
}

void CSettingsDialog::OnCancel()
{
	DestroyPages();
	CDialog::OnCancel();
}

void CSettingsDialog::OnApply()
{
}

void CSettingsDialog::OnOK()
{
	RefreshData();
	for (int i = 0; i < m_pInfo.GetSize(); i++)
	{
		PAGE_INFO* pInfo = (PAGE_INFO*)m_pInfo.GetAt(i);
		if (pInfo && pInfo->pWnd)
		{
			if (theApp.IsWnd(pInfo->pWnd))
			{
				if (pInfo->pWnd->SendMessage(ID_SETTING_OK, 0, 0) != 0)
					return;
			}
		}
	}

	//情報を保存
	theApp.m_AppSettingsDlgCurrent.CopyData(theApp.m_AppSettings);
	if (!theApp.m_AppSettings.SaveDataToFileEx(theApp.m_strSettingFileFullPath))
	{
		CString alertMsg;
		alertMsg.LoadString(ID_ALERT_CANNOT_SAVE_CONFIGS);
		CString strErrMsg;
		strErrMsg.Format(alertMsg);
		::MessageBox(this->m_hWnd, strErrMsg, theApp.m_strThisAppName, MB_OK | MB_ICONERROR);
	}
	else
	{
		if (theApp.InVirtualEnvironment() == VE_THINAPP)
		{
			CString strSBFilePath;
			strSBFilePath = theApp.GetSandboxFilePath(theApp.m_strSettingFileFullPath);
			if (!strSBFilePath.IsEmpty())
			{
				CString strFilePathCnf;
				strFilePathCnf = theApp.m_strSettingFileFullPath;
				strFilePathCnf.Replace(_T("Default.conf"), _T(".conf"));
				theApp.Exec_SB2PYS_COPY(strSBFilePath, strFilePathCnf);
			}
		}
	}

	//キャッシュ系を初期化
	theApp.m_CacheRedirectFilter_None.RemoveAll();
	theApp.m_CacheURLFilter_Deny.RemoveAll();
	theApp.m_CacheURLFilter_Allow.RemoveAll();

	theApp.m_cDomainFilterList.RefreshData();
	theApp.m_cPopupFilterList.RefreshData();
	theApp.m_cCustomScriptList.RefreshData();

	//Scriptを読み込み直し
	theApp.m_cScriptSrc.RefreshData();
	DestroyPages();
	CDialog::OnOK();
}

BOOL CSettingsDialog::RefreshData()
{
	PAGE_INFO *pInfoErr = NULL, *pInfo;
	for (int i = 0; i < m_pInfo.GetSize(); i++)
	{
		pInfo = (PAGE_INFO*)m_pInfo.GetAt(i);
		if (pInfo && pInfo->pWnd)
		{
			if (::IsWindow(pInfo->pWnd->m_hWnd))
				pInfo->pWnd->UpdateData(TRUE);
		}
	}
	return (TRUE);
}

BOOL CSettingsDialog::DestroyPages()
{
	for (int i = 0; i < m_pInfo.GetSize(); i++)
	{
		PAGE_INFO* pInfo = (PAGE_INFO*)m_pInfo.GetAt(i);
		if (pInfo && pInfo->pWnd)
		{
			pInfo->pWnd->DestroyWindow();
			if (!(pInfo->bViewClass))
				delete (pInfo->pWnd);
		}
	}
	return (true);
}

void CSettingsDialog::OnPreferenceHelp()
{
}

/////////////////////////////////////////////////////////////////////////////
//
BOOL CSettingsDialog::PreTranslateMessage(MSG* pMsg)
{
	ASSERT(pMsg != NULL);
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);

	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			OnCancel();
			return TRUE;
		}
	}
	if (CWnd::PreTranslateMessage(pMsg)) return TRUE;

	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	if (pFrameWnd != NULL && pFrameWnd->m_bHelpMode) return FALSE;

	pFrameWnd = GetParentFrame();
	while (pFrameWnd != NULL)
	{
		if (pFrameWnd->PreTranslateMessage(pMsg)) return TRUE;
		pFrameWnd = pFrameWnd->GetParentFrame();
	}

	return PreTranslateInput(pMsg);
}

void CSettingsDialog::SetTitle(CString sTitle)
{
	m_csTitle = sTitle;
}

void CSettingsDialog::SetLogoText(CString sText)
{
	m_csLogoText = sText;
}

/////////////////////////////////////////////////////////////////////////////
//
void CSettingsDialog::ExpandTree()
{
	HTREEITEM hti = m_TreeCtrl.GetRootItem();
	while (hti)
	{
		ExpandBranch(hti);
		hti = m_TreeCtrl.GetNextSiblingItem(hti);
	}
}

/////////////////////////////////////////////////////////////////////////////
//
void CSettingsDialog::ExpandBranch(HTREEITEM hti)
{
	if (m_TreeCtrl.ItemHasChildren(hti))
	{
		m_TreeCtrl.Expand(hti, TVE_EXPAND);
		hti = m_TreeCtrl.GetChildItem(hti);
		do
		{
			ExpandBranch(hti);
		} while ((hti = m_TreeCtrl.GetNextSiblingItem(hti)) != NULL);
	}
	m_TreeCtrl.EnsureVisible(m_TreeCtrl.GetSelectedItem());
}

/////////////////////////////////////////////////////////////////////////////
//
BOOL CSettingsDialog::Create()
{
	return CDialog::Create(CSettingsDialog::IDD);
}

void CSettingsDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgSetLog ダイアログ

IMPLEMENT_DYNCREATE(CDlgSetLog, CPropertyPage)
CDlgSetLog::CDlgSetLog() : CPropertyPage(CDlgSetLog::IDD)
{
	//{{AFX_DATA_INIT(CDlgSetLog)
	//}}AFX_DATA_INIT
}

void CDlgSetLog::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_UPLOAD_LOGGING_URL_TYPE_COMBO, m_ComboUploadLoggingType);
	//{{AFX_DATA_MAP(CDlgSetLog)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgSetLog, CPropertyPage)
	//{{AFX_MSG_MAP(CDlgSetLog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_ENABLE_LOG, OnEnableLog)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_CHECK_EnableLogging, OnEnableLogging)
	//}}AFX_MSG_MAP
	ON_MESSAGE(ID_SETTING_OK, Set_OK)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSetLog メッセージ ハンドラ
void CDlgSetLog::OnEnableLog()
{
	BOOL bChk = FALSE;
	bChk = ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_LOG))->GetCheck();

	if (bChk)
	{
		GetDlgItem(IDC_RADIO_LOG_ALL)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_LOG_NOFILE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_LOG_GE_URL)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_RADIO_LOG_ALL)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_LOG_NOFILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_LOG_GE_URL)->EnableWindow(FALSE);
	}
	return;
}
void CDlgSetLog::OnEnableLogging()
{
	BOOL bChk = FALSE;
	bChk = ((CButton*)GetDlgItem(IDC_CHECK_EnableLogging))->GetCheck();

	if (bChk)
	{
		GetDlgItem(IDC_EDIT_LogServerURL)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_RequestHeader)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_LOG_GET)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_LOG_POST)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_ENABLE_DOWNLOAD_LOG)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_ENABLE_UPLOAD_LOG)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_EnableBrowsingLogging)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_EnableAccessAllLogging)->EnableWindow(TRUE);
		m_ComboUploadLoggingType.EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_EDIT_LogServerURL)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_RequestHeader)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_LOG_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_LOG_POST)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_ENABLE_DOWNLOAD_LOG)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_ENABLE_UPLOAD_LOG)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_EnableBrowsingLogging)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_EnableAccessAllLogging)->EnableWindow(FALSE);
		m_ComboUploadLoggingType.EnableWindow(FALSE);
	}
	return;
}
BOOL CDlgSetLog::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	//DebugLog出力を有効にする
	if (theApp.m_AppSettingsDlgCurrent.IsAdvancedLogMode())
		((CButton*)GetDlgItem(IDC_CHECK_ENABLE_LOG))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_ENABLE_LOG))->SetCheck(0);

	//ログ出力種別
	int iLogLevel = theApp.m_AppSettingsDlgCurrent.GetAdvancedLogLevel();
	if (iLogLevel == DEBUG_LOG_LEVEL_OUTPUT_ALL)
		((CButton*)GetDlgItem(IDC_RADIO_LOG_ALL))->SetCheck(1);
	else if (iLogLevel == DEBUG_LOG_LEVEL_OUTPUT_NO_FILE)
		((CButton*)GetDlgItem(IDC_RADIO_LOG_NOFILE))->SetCheck(1);
	else if (iLogLevel == DEBUG_LOG_LEVEL_OUTPUT_URL)
		((CButton*)GetDlgItem(IDC_RADIO_LOG_GE_URL))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_RADIO_LOG_ALL))->SetCheck(1);

	if (theApp.m_AppSettingsDlgCurrent.IsEnableLogging())
		((CButton*)GetDlgItem(IDC_CHECK_EnableLogging))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_EnableLogging))->SetCheck(0);

	CString strLogServerURL;
	strLogServerURL = theApp.m_AppSettingsDlgCurrent.GetLogServerURL();
	SetDlgItemText(IDC_EDIT_LogServerURL, strLogServerURL);

	CString strRequestHeader;
	strRequestHeader = theApp.m_AppSettingsDlgCurrent.GetRequestHeader();
	SetDlgItemText(IDC_EDIT_RequestHeader, strRequestHeader);

	CString strLogM;
	int iLogMethod = 0;
	strLogM = theApp.m_AppSettings.GetLogMethod();
	if (strLogM.CompareNoCase(_T("GET")) == 0)
		iLogMethod = LOG_M_GET;
	else if (strLogM.CompareNoCase(_T("POST")) == 0)
		iLogMethod = LOG_M_POST;
	else
		iLogMethod = LOG_M_GET;

	if (iLogMethod == LOG_M_GET)
		((CButton*)GetDlgItem(IDC_RADIO_LOG_GET))->SetCheck(1);
	else if (iLogMethod == LOG_M_POST)
		((CButton*)GetDlgItem(IDC_RADIO_LOG_POST))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_RADIO_LOG_GET))->SetCheck(1);

	if (theApp.m_AppSettingsDlgCurrent.IsEnableDownloadLogging())
		((CButton*)GetDlgItem(IDC_CHECK_ENABLE_DOWNLOAD_LOG))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_ENABLE_DOWNLOAD_LOG))->SetCheck(0);

	if (theApp.m_AppSettingsDlgCurrent.IsEnableUploadLogging())
		((CButton*)GetDlgItem(IDC_CHECK_ENABLE_UPLOAD_LOG))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_ENABLE_UPLOAD_LOG))->SetCheck(0);

	if (theApp.m_AppSettingsDlgCurrent.IsEnableBrowsingLogging())
		((CButton*)GetDlgItem(IDC_CHECK_EnableBrowsingLogging))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_EnableBrowsingLogging))->SetCheck(0);

	if (theApp.m_AppSettingsDlgCurrent.IsEnableAccessAllLogging())
		((CButton*)GetDlgItem(IDC_CHECK_EnableAccessAllLogging))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_EnableAccessAllLogging))->SetCheck(0);

	size_t labelIdArraySize = sizeof(kUploadLoggingTypeLabelIdArray) / sizeof(kUploadLoggingTypeLabelIdArray[0]);
	for (size_t i = 0; i < labelIdArraySize; i++)
	{
		CString label;
		label.LoadString(kUploadLoggingTypeLabelIdArray[i]);
		m_ComboUploadLoggingType.AddString(label);
	}
	m_ComboUploadLoggingType.SetCurSel(theApp.m_AppSettingsDlgCurrent.GetUploadLoggingURLType());
	OnEnableLog();
	OnEnableLogging();
	return FALSE;
}
void CDlgSetLog::OnBnClickedButton1()
{
	theApp.ShowDebugTraceDlg();
}

void CDlgSetLog::OnDestroy()
{
	CPropertyPage::OnDestroy();
}
LRESULT CDlgSetLog::Set_OK(WPARAM wParam, LPARAM lParam)
{
	//DebugLog出力を有効にする
	if (((CButton*)GetDlgItem(IDC_CHECK_ENABLE_LOG))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetAdvancedLogMode(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetAdvancedLogMode(0);

	//ログ出力種別
	int iLogLevel = GetCheckedRadioButton(IDC_RADIO_LOG_ALL, IDC_RADIO_LOG_GE_URL);
	if (iLogLevel == IDC_RADIO_LOG_ALL)
		theApp.m_AppSettingsDlgCurrent.SetAdvancedLogLevel(DEBUG_LOG_LEVEL_OUTPUT_ALL);
	else if (iLogLevel == IDC_RADIO_LOG_NOFILE)
		theApp.m_AppSettingsDlgCurrent.SetAdvancedLogLevel(DEBUG_LOG_LEVEL_OUTPUT_NO_FILE);
	else if (iLogLevel == IDC_RADIO_LOG_GE_URL)
		theApp.m_AppSettingsDlgCurrent.SetAdvancedLogLevel(DEBUG_LOG_LEVEL_OUTPUT_URL);
	else
		theApp.m_AppSettingsDlgCurrent.SetAdvancedLogLevel(DEBUG_LOG_LEVEL_OUTPUT_ALL);

	CString strLogServerURL;
	GetDlgItemText(IDC_EDIT_LogServerURL, strLogServerURL);
	theApp.m_AppSettingsDlgCurrent.SetLogServerURL(strLogServerURL);

	CString strRequestHeader;
	GetDlgItemText(IDC_EDIT_RequestHeader, strRequestHeader);
	theApp.m_AppSettingsDlgCurrent.SetRequestHeader(strRequestHeader);

	int iLogMethod = GetCheckedRadioButton(IDC_RADIO_LOG_GET, IDC_RADIO_LOG_POST);
	if (iLogMethod == IDC_RADIO_LOG_GET)
		theApp.m_AppSettingsDlgCurrent.SetLogMethod(_T("GET"));
	else if (iLogMethod == IDC_RADIO_LOG_POST)
		theApp.m_AppSettingsDlgCurrent.SetLogMethod(_T("POST"));
	else
		theApp.m_AppSettingsDlgCurrent.SetLogMethod(_T("GET"));

	if (((CButton*)GetDlgItem(IDC_CHECK_ENABLE_DOWNLOAD_LOG))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetEnableDownloadLogging(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetEnableDownloadLogging(0);

	if (((CButton*)GetDlgItem(IDC_CHECK_ENABLE_UPLOAD_LOG))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetEnableUploadLogging(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetEnableUploadLogging(0);

	if (((CButton*)GetDlgItem(IDC_CHECK_EnableBrowsingLogging))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetEnableBrowsingLogging(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetEnableBrowsingLogging(0);

	if (((CButton*)GetDlgItem(IDC_CHECK_EnableAccessAllLogging))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetEnableAccessAllLogging(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetEnableAccessAllLogging(0);

	if (((CButton*)GetDlgItem(IDC_CHECK_EnableLogging))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetEnableLogging(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetEnableLogging(0);

	theApp.m_AppSettingsDlgCurrent.SetUploadLoggingURLType(m_ComboUploadLoggingType.GetCurSel());
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgSetDSP ダイアログ

IMPLEMENT_DYNCREATE(CDlgSetDSP, CPropertyPage)
CDlgSetDSP::CDlgSetDSP() : CPropertyPage(CDlgSetDSP::IDD)
{
}

void CDlgSetDSP::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgSetDSP, CPropertyPage)
	ON_WM_DESTROY()
	ON_MESSAGE(ID_SETTING_OK, Set_OK)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgSetDSP::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_CHECK_DISABLE_REBAR, &CDlgSetDSP::OnBnClickedCheckDisableRebar)
END_MESSAGE_MAP()

BOOL CDlgSetDSP::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	//TAB
	if (theApp.m_AppSettingsDlgCurrent.IsEnableTab())
		((CButton*)GetDlgItem(IDC_CHECK_TAB))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_TAB))->SetCheck(0);

	//Logo
	if (theApp.m_AppSettingsDlgCurrent.IsShowLogo())
		((CButton*)GetDlgItem(IDC_CHECK_LOGO))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_LOGO))->SetCheck(0);

	//GPUレンダリング
	if (theApp.m_AppSettingsDlgCurrent.IsGPURendering())
		((CButton*)GetDlgItem(IDC_CHECK_GPU))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_GPU))->SetCheck(0);

	//PDF
	if (theApp.m_AppSettingsDlgCurrent.IsEnablePDFExtension())
		((CButton*)GetDlgItem(IDC_CHECK_PDF))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_PDF))->SetCheck(0);

	//Rebar
	if (theApp.m_AppSettingsDlgCurrent.IsRebar())
	{
		((CButton*)GetDlgItem(IDC_CHECK_DISABLE_REBAR))->SetCheck(1);
		GetDlgItem(IDC_CHECK_LOGO)->EnableWindow(TRUE);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_CHECK_DISABLE_REBAR))->SetCheck(0);
		GetDlgItem(IDC_CHECK_LOGO)->EnableWindow(FALSE);
	}

	//ステータスバー
	if (theApp.m_AppSettingsDlgCurrent.IsStatusbar())
		((CButton*)GetDlgItem(IDC_CHECK_DISABLE_STATUSBAR))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_DISABLE_STATUSBAR))->SetCheck(0);

	//幅マージン
	int iW = 25;
	iW = theApp.m_AppSettingsDlgCurrent.GetWideMargin();
	SetDlgItemInt(IDC_EDIT_W, iW);

	//高さマージン
	int iH = 48;
	iH = theApp.m_AppSettingsDlgCurrent.GetHeightMargin();
	SetDlgItemInt(IDC_EDIT_H, iH);

	//拡大・縮小
	int iZoom = 100;
	iZoom = theApp.m_AppSettingsDlgCurrent.GetZoomSize();
	if (!(25 <= iZoom && iZoom <= 500))
		iZoom = 100;
	SetDlgItemInt(IDC_EDIT_ZOOM, iZoom);

	return FALSE;
}
void CDlgSetDSP::OnDestroy()
{
	CPropertyPage::OnDestroy();
}
LRESULT CDlgSetDSP::Set_OK(WPARAM wParam, LPARAM lParam)
{
	//TAB
	if (((CButton*)GetDlgItem(IDC_CHECK_TAB))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetEnableTab(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetEnableTab(0);

	//Logo
	if (((CButton*)GetDlgItem(IDC_CHECK_LOGO))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetShowLogo(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetShowLogo(0);

	//GPUレンダリング
	if (((CButton*)GetDlgItem(IDC_CHECK_GPU))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetGPURendering(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetGPURendering(0);

	//PDF
	if (((CButton*)GetDlgItem(IDC_CHECK_PDF))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetEnablePDFExtension(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetEnablePDFExtension(0);

	//Rebar
	if (((CButton*)GetDlgItem(IDC_CHECK_DISABLE_REBAR))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetRebar(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetRebar(0);

	//ステータスバー
	if (((CButton*)GetDlgItem(IDC_CHECK_DISABLE_STATUSBAR))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetStatusbar(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetStatusbar(0);

	//幅マージン
	int iW = 25;
	iW = GetDlgItemInt(IDC_EDIT_W);
	if (iW < -50 || iW > 150)
		iW = 25;
	theApp.m_AppSettingsDlgCurrent.SetWideMargin(iW);

	//高さマージン
	int iH = 48;
	iH = GetDlgItemInt(IDC_EDIT_H);
	if (iH < -50 || iH > 150)
		iH = 48;
	theApp.m_AppSettingsDlgCurrent.SetHeightMargin(iH);

	//拡大・縮小
	int iZoom = 100;
	iZoom = GetDlgItemInt(IDC_EDIT_ZOOM);
	if (!(25 <= iZoom && iZoom <= 500))
		iZoom = 100;
	theApp.m_AppSettingsDlgCurrent.SetZoomSize(iZoom);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgSetGen ダイアログ
IMPLEMENT_DYNCREATE(CDlgSetGen, CPropertyPage)
CDlgSetGen::CDlgSetGen() : CPropertyPage(CDlgSetGen::IDD)
{
}

void CDlgSetGen::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgSetGen, CPropertyPage)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON1, OnKeyCombiTest)
	ON_MESSAGE(ID_SETTING_OK, Set_OK)
END_MESSAGE_MAP()
void CDlgSetGen::OnKeyCombiTest()
{
	DWORD dSetting = 0;
	CString strMsg;
	CString strSettingKey;
	if (((CButton*)GetDlgItem(IDC_CHK_SHIFT))->GetCheck() == 1)
	{
		dSetting = dSetting | KEY_COMB_SHIFT;
		strSettingKey += _T("Shift ");
	}
	if (((CButton*)GetDlgItem(IDC_CHK_CTRL))->GetCheck() == 1)
	{
		dSetting = dSetting | KEY_COMB_CTRL;
		strSettingKey += _T("Ctrl ");
	}
	if (((CButton*)GetDlgItem(IDC_CHK_ALT))->GetCheck() == 1)
	{
		dSetting = dSetting | KEY_COMB_ALT;
		strSettingKey += _T("Alt ");
	}

	if (((CButton*)GetDlgItem(IDC_CHK_LEFT))->GetCheck() == 1)
	{
		dSetting = dSetting | KEY_COMB_LEFT;
		strSettingKey += _T("← ");
	}
	if (((CButton*)GetDlgItem(IDC_CHK_UP))->GetCheck() == 1)
	{
		dSetting = dSetting | KEY_COMB_UP;
		strSettingKey += _T("↑ ");
	}
	if (((CButton*)GetDlgItem(IDC_CHK_RIGHT))->GetCheck() == 1)
	{
		dSetting = dSetting | KEY_COMB_RIGHT;
		strSettingKey += _T("→ ");
	}
	if (((CButton*)GetDlgItem(IDC_CHK_DOWN))->GetCheck() == 1)
	{
		dSetting = dSetting | KEY_COMB_DOWN;
		strSettingKey += _T("↓");
	}
	if (strSettingKey.IsEmpty())
	{
		CString emptyKeyCombinationMsg;
		emptyKeyCombinationMsg.LoadString(ID_KEY_COMBINATION_EMPTY);
		::MessageBox(this->m_hWnd, emptyKeyCombinationMsg, theApp.m_strThisAppName, MB_OK | MB_ICONERROR);
		return;
	}
	CString confirmKeyCombinationMsg;
	confirmKeyCombinationMsg.LoadString(ID_KEY_COMBINATION_CONFIRM);
	strMsg.Format(confirmKeyCombinationMsg, strSettingKey);
	::MessageBox(this->m_hWnd, strMsg, theApp.m_strThisAppName, MB_OK | MB_ICONINFORMATION);
	{
		DWORD dwKC = theApp.GetKeyCombi();

		CString strResult;
		if ((dwKC & KEY_COMB_SHIFT) == KEY_COMB_SHIFT)
			strResult += _T("Shift ");
		if ((dwKC & KEY_COMB_CTRL) == KEY_COMB_CTRL)
			strResult += _T("Ctrl ");
		if ((dwKC & KEY_COMB_ALT) == KEY_COMB_ALT)
			strResult += _T("Alt ");
		if ((dwKC & KEY_COMB_LEFT) == KEY_COMB_LEFT)
			strResult += _T("← ");
		if ((dwKC & KEY_COMB_UP) == KEY_COMB_UP)
			strResult += _T("↑ ");
		if ((dwKC & KEY_COMB_RIGHT) == KEY_COMB_RIGHT)
			strResult += _T("→ ");
		if ((dwKC & KEY_COMB_DOWN) == KEY_COMB_DOWN)
			strResult += _T("↓");
		if (strResult.IsEmpty())
		{
			return;
		}

		CString resultKeyCombinationMsg;
		if (dSetting == dwKC)
		{
			resultKeyCombinationMsg.LoadString(ID_KEY_COMBINATION_SUCCEEDED);
			strMsg.Format(resultKeyCombinationMsg, strSettingKey, strResult);
			::MessageBox(this->m_hWnd, strMsg, theApp.m_strThisAppName, MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			resultKeyCombinationMsg.LoadString(ID_KEY_COMBINATION_FAILED);
			strMsg.Format(resultKeyCombinationMsg, strSettingKey, strResult);
			::MessageBox(this->m_hWnd, strMsg, theApp.m_strThisAppName, MB_OK | MB_ICONERROR);
		}
	}
}
BOOL CDlgSetGen::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	//DisableMultipleInstance
	if (theApp.m_AppSettingsDlgCurrent.IsMultipleInstance())
		((CButton*)GetDlgItem(IDC_CHECK_DISABLE_MULTIPLE_INSTANCE))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_DISABLE_MULTIPLE_INSTANCE))->SetCheck(0);

	//IsEnableMemcache
	if (theApp.m_AppSettingsDlgCurrent.IsEnableMemcache())
		((CButton*)GetDlgItem(IDC_ENABLE_MEMCACHE))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_ENABLE_MEMCACHE))->SetCheck(0);

	//EnableCrashRecovery
	if (theApp.m_AppSettingsDlgCurrent.IsCrashRecovery())
		((CButton*)GetDlgItem(IDC_CHECK_ENABLECRASHRECOVERY))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_ENABLECRASHRECOVERY))->SetCheck(0);

	int iKeyCombination = 0;
	iKeyCombination = theApp.m_AppSettingsDlgCurrent.GetKeyCombination();

	((CButton*)GetDlgItem(IDC_CHK_SHIFT))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_CHK_CTRL))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_CHK_ALT))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_CHK_LEFT))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_CHK_UP))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_CHK_RIGHT))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_CHK_DOWN))->SetCheck(0);

	if ((iKeyCombination & KEY_COMB_SHIFT) == KEY_COMB_SHIFT)
		((CButton*)GetDlgItem(IDC_CHK_SHIFT))->SetCheck(1);
	if ((iKeyCombination & KEY_COMB_CTRL) == KEY_COMB_CTRL)
		((CButton*)GetDlgItem(IDC_CHK_CTRL))->SetCheck(1);
	if ((iKeyCombination & KEY_COMB_ALT) == KEY_COMB_ALT)
		((CButton*)GetDlgItem(IDC_CHK_ALT))->SetCheck(1);
	if ((iKeyCombination & KEY_COMB_LEFT) == KEY_COMB_LEFT)
		((CButton*)GetDlgItem(IDC_CHK_LEFT))->SetCheck(1);
	if ((iKeyCombination & KEY_COMB_UP) == KEY_COMB_UP)
		((CButton*)GetDlgItem(IDC_CHK_UP))->SetCheck(1);
	if ((iKeyCombination & KEY_COMB_RIGHT) == KEY_COMB_RIGHT)
		((CButton*)GetDlgItem(IDC_CHK_RIGHT))->SetCheck(1);
	if ((iKeyCombination & KEY_COMB_DOWN) == KEY_COMB_DOWN)
		((CButton*)GetDlgItem(IDC_CHK_DOWN))->SetCheck(1);

	return FALSE;
}
void CDlgSetGen::OnDestroy()
{
	CPropertyPage::OnDestroy();
}
LRESULT CDlgSetGen::Set_OK(WPARAM wParam, LPARAM lParam)
{
	//DisableMultipleInstance
	if (((CButton*)GetDlgItem(IDC_CHECK_DISABLE_MULTIPLE_INSTANCE))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetMultipleInstance(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetMultipleInstance(0);

	//SetEnableMemcache
	if (((CButton*)GetDlgItem(IDC_ENABLE_MEMCACHE))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetEnableMemcache(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetEnableMemcache(0);

	//EnableCrashRecovery
	if (((CButton*)GetDlgItem(IDC_CHECK_ENABLECRASHRECOVERY))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetCrashRecovery(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetCrashRecovery(0);

	//////////////////////////////////////////////////////////////////////////////
	DWORD dSetting = 0;
	if (((CButton*)GetDlgItem(IDC_CHK_SHIFT))->GetCheck() == 1)
		dSetting = dSetting | KEY_COMB_SHIFT;
	if (((CButton*)GetDlgItem(IDC_CHK_CTRL))->GetCheck() == 1)
		dSetting = dSetting | KEY_COMB_CTRL;
	if (((CButton*)GetDlgItem(IDC_CHK_ALT))->GetCheck() == 1)
		dSetting = dSetting | KEY_COMB_ALT;
	if (((CButton*)GetDlgItem(IDC_CHK_LEFT))->GetCheck() == 1)
		dSetting = dSetting | KEY_COMB_LEFT;
	if (((CButton*)GetDlgItem(IDC_CHK_UP))->GetCheck() == 1)
		dSetting = dSetting | KEY_COMB_UP;
	if (((CButton*)GetDlgItem(IDC_CHK_RIGHT))->GetCheck() == 1)
		dSetting = dSetting | KEY_COMB_RIGHT;
	if (((CButton*)GetDlgItem(IDC_CHK_DOWN))->GetCheck() == 1)
		dSetting = dSetting | KEY_COMB_DOWN;
	theApp.m_AppSettingsDlgCurrent.SetKeyCombination(dSetting);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgSetSEC ダイアログ

IMPLEMENT_DYNCREATE(CDlgSetSEC, CPropertyPage)
CDlgSetSEC::CDlgSetSEC() : CPropertyPage(CDlgSetSEC::IDD)
{
}

void CDlgSetSEC::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_ComboEmu);
}

BEGIN_MESSAGE_MAP(CDlgSetSEC, CPropertyPage)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_BROWSE1, &CDlgSetSEC::OnBnClickedBrowse1)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE2, &CDlgSetSEC::OnBnClickedBrowse2)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE3, &CDlgSetSEC::OnBnClickedBrowse3)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE4, &CDlgSetSEC::OnBnClickedBrowse4)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE5, &CDlgSetSEC::OnBnClickedBrowse5)
	ON_BN_CLICKED(IDC_CHECK_EnableZoneFilterScript, &CDlgSetSEC::OnBnClickedScript)
	ON_BN_CLICKED(IDC_BUTTON_SCRIPT_EDIT, &CDlgSetSEC::OnBnClickedScriptEdit)
	ON_MESSAGE(ID_SETTING_OK, Set_OK)
END_MESSAGE_MAP()
void CDlgSetSEC::OnBnClickedScriptEdit()
{
	CDlgSCEditor DlgEditor(this);
	DlgEditor.m_strFilePath = theApp.m_cScriptSrc.m_strFilePath;
	DlgEditor.DoModal();
	theApp.m_cScriptSrc.RefreshData();
	theApp.m_CacheRedirectFilter_None.RemoveAll();
}
void CDlgSetSEC::OnBnClickedBrowse1()
{
	FolderBrowse(IDC_EDIT_CustomBrowser1);
}
void CDlgSetSEC::OnBnClickedBrowse2()
{
	FolderBrowse(IDC_EDIT_CustomBrowser2);
}
void CDlgSetSEC::OnBnClickedBrowse3()
{
	FolderBrowse(IDC_EDIT_CustomBrowser3);
}
void CDlgSetSEC::OnBnClickedBrowse4()
{
	FolderBrowse(IDC_EDIT_CustomBrowser4);
}
void CDlgSetSEC::OnBnClickedBrowse5()
{
	FolderBrowse(IDC_EDIT_CustomBrowser5);
}
void CDlgSetSEC::FolderBrowse(UINT nID)
{
	CString szFilter;
	szFilter.LoadString(ID_FILE_TYPE_EXE);
	CFileDialog fileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter, this);
	CString strTitle;
	strTitle.LoadString(ID_OPEN_FILE_CHOOSER_TITLE);
	fileDlg.m_ofn.lpstrTitle = strTitle.GetString();
	if (fileDlg.DoModal() == IDOK)
	{
		SetDlgItemText(nID, fileDlg.GetPathName());
	}
}

void CDlgSetSEC::ChangeStateScriptEdit()
{
	BOOL bFlg = FALSE;

	if (((CButton*)GetDlgItem(IDC_CHECK_EnableZoneFilterScript))->GetCheck() == 1)
	{
		bFlg = TRUE;
	}
	else
	{
		bFlg = FALSE;
	}
	((CButton*)GetDlgItem(IDC_BUTTON_SCRIPT_EDIT))->EnableWindow(bFlg);
	((CButton*)GetDlgItem(IDC_COMBO1))->EnableWindow(bFlg);
	((CButton*)GetDlgItem(IDC_EDIT_CustomBrowser1))->EnableWindow(bFlg);
	((CButton*)GetDlgItem(IDC_EDIT_CustomBrowser2))->EnableWindow(bFlg);
	((CButton*)GetDlgItem(IDC_EDIT_CustomBrowser3))->EnableWindow(bFlg);
	((CButton*)GetDlgItem(IDC_EDIT_CustomBrowser4))->EnableWindow(bFlg);
	((CButton*)GetDlgItem(IDC_EDIT_CustomBrowser5))->EnableWindow(bFlg);
	((CButton*)GetDlgItem(IDC_BUTTON_BROWSE1))->EnableWindow(bFlg);
	((CButton*)GetDlgItem(IDC_BUTTON_BROWSE2))->EnableWindow(bFlg);
	((CButton*)GetDlgItem(IDC_BUTTON_BROWSE3))->EnableWindow(bFlg);
	((CButton*)GetDlgItem(IDC_BUTTON_BROWSE4))->EnableWindow(bFlg);
	((CButton*)GetDlgItem(IDC_BUTTON_BROWSE5))->EnableWindow(bFlg);
}

void CDlgSetSEC::OnBnClickedScript()
{
	ChangeStateScriptEdit();
}

BOOL CDlgSetSEC::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	//Script
	if (theApp.m_AppSettingsDlgCurrent.IsEnableURLRedirect())
		((CButton*)GetDlgItem(IDC_CHECK_EnableZoneFilterScript))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_EnableZoneFilterScript))->SetCheck(0);
	ChangeStateScriptEdit();

	SetDlgItemText(IDC_EDIT_CustomBrowser1, theApp.m_AppSettingsDlgCurrent.GetCustomBrowser());
	SetDlgItemText(IDC_EDIT_CustomBrowser2, theApp.m_AppSettingsDlgCurrent.GetCustomBrowser2());
	SetDlgItemText(IDC_EDIT_CustomBrowser3, theApp.m_AppSettingsDlgCurrent.GetCustomBrowser3());
	SetDlgItemText(IDC_EDIT_CustomBrowser4, theApp.m_AppSettingsDlgCurrent.GetCustomBrowser4());
	SetDlgItemText(IDC_EDIT_CustomBrowser5, theApp.m_AppSettingsDlgCurrent.GetCustomBrowser5());

	for (int i = 0; i < InfoDlgListMaxCnt; i++)
	{
		CString intoDlgListLabel;
		intoDlgListLabel.LoadString(gInfoDlgList[i]);
		m_ComboEmu.AddString(intoDlgListLabel);
	}
	int iInfoTime = 0;
	iInfoTime = theApp.m_AppSettingsDlgCurrent.GetRedirectMsgTimeout();
	double dIndexMod = 0.0;
	int iIndexMod = 0;
	dIndexMod = iInfoTime / 500;
	dIndexMod += 0.5;
	iIndexMod = (int)dIndexMod;
	if (iIndexMod < 0 || iIndexMod >= InfoDlgListMaxCnt)
		iIndexMod = 0;
	m_ComboEmu.SetCurSel(iIndexMod);

	return FALSE;
}
void CDlgSetSEC::OnDestroy()
{
	CPropertyPage::OnDestroy();
}
LRESULT CDlgSetSEC::Set_OK(WPARAM wParam, LPARAM lParam)
{
	//Script
	if (((CButton*)GetDlgItem(IDC_CHECK_EnableZoneFilterScript))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetEnableURLRedirect(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetEnableURLRedirect(0);

	//CustomBrowserPath
	CString strCustomBrowserPath;

	strCustomBrowserPath.Empty();
	GetDlgItemText(IDC_EDIT_CustomBrowser1, strCustomBrowserPath);
	theApp.m_AppSettingsDlgCurrent.SetCustomBrowser(strCustomBrowserPath);

	strCustomBrowserPath.Empty();
	GetDlgItemText(IDC_EDIT_CustomBrowser2, strCustomBrowserPath);
	theApp.m_AppSettingsDlgCurrent.SetCustomBrowser2(strCustomBrowserPath);

	strCustomBrowserPath.Empty();
	GetDlgItemText(IDC_EDIT_CustomBrowser3, strCustomBrowserPath);
	theApp.m_AppSettingsDlgCurrent.SetCustomBrowser3(strCustomBrowserPath);

	strCustomBrowserPath.Empty();
	GetDlgItemText(IDC_EDIT_CustomBrowser4, strCustomBrowserPath);
	theApp.m_AppSettingsDlgCurrent.SetCustomBrowser4(strCustomBrowserPath);

	strCustomBrowserPath.Empty();
	GetDlgItemText(IDC_EDIT_CustomBrowser5, strCustomBrowserPath);
	theApp.m_AppSettingsDlgCurrent.SetCustomBrowser5(strCustomBrowserPath);

	int iID = 0;
	iID = m_ComboEmu.GetCurSel();
	iID = iID * 500;
	theApp.m_AppSettingsDlgCurrent.SetRedirectMsgTimeout(iID);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgSetINIT ダイアログ

IMPLEMENT_DYNCREATE(CDlgSetINIT, CPropertyPage)
CDlgSetINIT::CDlgSetINIT() : CPropertyPage(CDlgSetINIT::IDD)
{
}

void CDlgSetINIT::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INIT_OP_PARAM, m_Combo);
}

BEGIN_MESSAGE_MAP(CDlgSetINIT, CPropertyPage)
	ON_WM_DESTROY()
	ON_MESSAGE(ID_SETTING_OK, Set_OK)
END_MESSAGE_MAP()

BOOL CDlgSetINIT::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	//起動時の強制パラメータを確認
	CString strEnforceInitURL;
	CString strEnforceInitParam;
	strEnforceInitURL = theApp.m_AppSettingsDlgCurrent.GetStartURL();
	strEnforceInitParam = theApp.m_AppSettingsDlgCurrent.GetEnforceInitParam();
	//整形
	strEnforceInitURL.TrimLeft();
	strEnforceInitURL.TrimRight();
	strEnforceInitURL.Replace(_T("\""), _T(""));

	strEnforceInitParam.TrimLeft();
	strEnforceInitParam.TrimRight();
	strEnforceInitParam.Replace(_T("\""), _T(""));

	SetDlgItemText(IDC_EDIT_INIT_URL_PARAM, strEnforceInitURL);
	SetDlgItemText(IDC_EDIT_INIT_OP_PARAM, strEnforceInitParam);

	m_Combo.AddString(_T("/MIN"));
	m_Combo.AddString(_T("/MAX"));
	m_Combo.AddString(_T("/NORMAL"));
	SHAutoComplete(GetDlgItem(IDC_EDIT_INIT_URL_PARAM)->m_hWnd, SHACF_URLALL | SHACF_AUTOSUGGEST_FORCE_ON | SHACF_AUTOAPPEND_FORCE_ON);

	CString strCustomMessage;
	strCustomMessage = theApp.m_AppSettings.GetInitMessage();
	if (!strCustomMessage.IsEmpty())
	{
		strCustomMessage.Replace(_T("\\n"), _T("\r\n"));
		SetDlgItemText(IDC_EDIT_INIT_MSG, strCustomMessage);
	}

	CString strStartUpProgram;
	CString strStartUpProgramArguments;
	strStartUpProgram = theApp.m_AppSettingsDlgCurrent.GetStartUpProgram();
	strStartUpProgramArguments = theApp.m_AppSettingsDlgCurrent.GetStartUpProgramArguments();

	strStartUpProgram.TrimLeft();
	strStartUpProgram.TrimRight();
	strStartUpProgram.Replace(_T("\""), _T(""));

	strStartUpProgramArguments.TrimLeft();
	strStartUpProgramArguments.TrimRight();
	strStartUpProgramArguments.Replace(_T("\""), _T(""));

	SetDlgItemText(IDC_EDIT_STARTUP_PROGRAM, strStartUpProgram);
	SetDlgItemText(IDC_EDIT_STARTUP_PROGRAM_ARGUMENTS, strStartUpProgramArguments);

	return FALSE;
}
void CDlgSetINIT::OnDestroy()
{
	CPropertyPage::OnDestroy();
}
LRESULT CDlgSetINIT::Set_OK(WPARAM wParam, LPARAM lParam)
{

	//起動時の強制パラメータを確認
	CString strEnforceInitURL;
	CString strEnforceInitParam;
	GetDlgItemText(IDC_EDIT_INIT_URL_PARAM, strEnforceInitURL);
	GetDlgItemText(IDC_EDIT_INIT_OP_PARAM, strEnforceInitParam);

	//整形
	strEnforceInitURL.TrimLeft();
	strEnforceInitURL.TrimRight();
	strEnforceInitURL.Replace(_T("\""), _T(""));

	strEnforceInitParam.TrimLeft();
	strEnforceInitParam.TrimRight();
	strEnforceInitParam.Replace(_T("\""), _T(""));

	theApp.m_AppSettingsDlgCurrent.SetStartURL(strEnforceInitURL);
	theApp.m_AppSettingsDlgCurrent.SetEnforceInitParam(strEnforceInitParam);

	CString strCustomMessage;
	GetDlgItemText(IDC_EDIT_INIT_MSG, strCustomMessage);
	strCustomMessage.TrimLeft();
	strCustomMessage.TrimRight();
	strCustomMessage.Replace(_T("\r\n"), _T("\\n"));
	theApp.m_AppSettingsDlgCurrent.SetInitMessage(strCustomMessage);

	CString strStartUpProgram;
	CString strStartUpProgramArguments;
	GetDlgItemText(IDC_EDIT_STARTUP_PROGRAM, strStartUpProgram);
	GetDlgItemText(IDC_EDIT_STARTUP_PROGRAM_ARGUMENTS, strStartUpProgramArguments);

	strStartUpProgram.TrimLeft();
	strStartUpProgram.TrimRight();
	strStartUpProgram.Replace(_T("\""), _T(""));

	strStartUpProgramArguments.TrimLeft();
	strStartUpProgramArguments.TrimRight();
	strStartUpProgramArguments.Replace(_T("\""), _T(""));

	theApp.m_AppSettingsDlgCurrent.SetStartUpProgram(strStartUpProgram);
	theApp.m_AppSettingsDlgCurrent.SetStartUpProgramArguments(strStartUpProgramArguments);

	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// CDlgSetCAP ダイアログ

IMPLEMENT_DYNCREATE(CDlgSetCAP, CPropertyPage)
CDlgSetCAP::CDlgSetCAP() : CPropertyPage(CDlgSetCAP::IDD)
{
}

void CDlgSetCAP::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgSetCAP, CPropertyPage)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_Enable_ProcessRunTime, &CDlgSetCAP::OnBnClickedPRT)
	ON_MESSAGE(ID_SETTING_OK, Set_OK)
END_MESSAGE_MAP()

void CDlgSetCAP::OnBnClickedPRT()
{
	this->ChangeStateLimitTime();
}

void CDlgSetCAP::ChangeStateLimitTime()
{
	if (((CButton*)GetDlgItem(IDC_Enable_ProcessRunTime))->GetCheck() == 1)
		GetDlgItem(IDC_EDIT_LIMIT_TIME)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_EDIT_LIMIT_TIME)->EnableWindow(FALSE);
}

BOOL CDlgSetCAP::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	//EnableDownloadRestriction
	if (theApp.m_AppSettingsDlgCurrent.IsEnableDownloadRestriction())
	{
		((CButton*)GetDlgItem(IDC_EnableDownloadRestriction))->SetCheck(1);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_EnableDownloadRestriction))->SetCheck(0);
	}

	//EnableUploadRestriction
	if (theApp.m_AppSettingsDlgCurrent.IsEnableUploadRestriction())
	{
		((CButton*)GetDlgItem(IDC_EnableUploadRestriction))->SetCheck(1);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_EnableUploadRestriction))->SetCheck(0);
	}

	//EnableFileExtChangeRestriction
	if (theApp.m_AppSettingsDlgCurrent.IsEnableFileExtChangeRestriction())
	{
		((CButton*)GetDlgItem(IDC_EnableFileExtChangeRestriction))->SetCheck(1);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_EnableFileExtChangeRestriction))->SetCheck(0);
	}

	//running
	if (theApp.m_AppSettingsDlgCurrent.IsEnableRunningTime())
	{
		((CButton*)GetDlgItem(IDC_Enable_ProcessRunTime))->SetCheck(1);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_Enable_ProcessRunTime))->SetCheck(0);
	}

	int iLimitTime = 0;
	iLimitTime = theApp.m_AppSettingsDlgCurrent.GetRunningLimitTime();
	SetDlgItemInt(IDC_EDIT_LIMIT_TIME, iLimitTime);

	this->ChangeStateLimitTime();

	int iW = 0;
	//メモリー 最大値(MB)
	iW = theApp.m_AppSettingsDlgCurrent.GetMemoryUsageLimit();
	if (iW < 1 || iW > 4096)
		iW = 1224;
	SetDlgItemInt(IDC_MemoryUsageLimit, iW);

	//ウィンドウ数 最大値
	iW = theApp.m_AppSettingsDlgCurrent.GetWindowCountLimit();
	if (iW < 1 || iW > 999)
		iW = 999;
	SetDlgItemInt(IDC_WindowCountLimit, iW);

	return FALSE;
}

void CDlgSetCAP::OnDestroy()
{
	CPropertyPage::OnDestroy();
}

LRESULT CDlgSetCAP::Set_OK(WPARAM wParam, LPARAM lParam)
{
	//EnableDownloadRestriction
	if (((CButton*)GetDlgItem(IDC_EnableDownloadRestriction))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetEnableDownloadRestriction(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetEnableDownloadRestriction(0);

	//EnableUploadRestriction
	if (((CButton*)GetDlgItem(IDC_EnableUploadRestriction))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetEnableUploadRestriction(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetEnableUploadRestriction(0);

	//EnableFileExtChangeRestriction
	if (((CButton*)GetDlgItem(IDC_EnableFileExtChangeRestriction))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetEnableFileExtChangeRestriction(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetEnableFileExtChangeRestriction(0);

	//running
	if (((CButton*)GetDlgItem(IDC_Enable_ProcessRunTime))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetEnableRunningTime(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetEnableRunningTime(0);

	int iLimitTime = 0;
	iLimitTime = GetDlgItemInt(IDC_EDIT_LIMIT_TIME);

	if (0 > iLimitTime)
		iLimitTime = 1440;
	theApp.m_AppSettingsDlgCurrent.SetRunningLimitTime(iLimitTime);

	int iW = 0;
	//メモリー 最大値(MB)
	iW = GetDlgItemInt(IDC_MemoryUsageLimit);
	if (iW < 1 || iW > 4096)
		iW = 1224;
	theApp.m_AppSettingsDlgCurrent.SetMemoryUsageLimit(iW);

	//ウィンドウ数 最大値
	iW = GetDlgItemInt(IDC_WindowCountLimit);
	if (iW < 1 || iW > 999)
		iW = 999;
	theApp.m_AppSettingsDlgCurrent.SetWindowCountLimit(iW);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgSetConnectionSetting ダイアログ

IMPLEMENT_DYNCREATE(CDlgSetConnectionSetting, CPropertyPage)
CDlgSetConnectionSetting::CDlgSetConnectionSetting() : CPropertyPage(CDlgSetConnectionSetting::IDD)
{
}

void CDlgSetConnectionSetting::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgSetConnectionSetting, CPropertyPage)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RAD_PX_IE, &CDlgSetConnectionSetting::OnBnClickedPxIE)
	ON_BN_CLICKED(IDC_RAD_PX_NA, &CDlgSetConnectionSetting::OnBnClickedPxNA)
	ON_BN_CLICKED(IDC_RAD_PX_EF, &CDlgSetConnectionSetting::OnBnClickedPxEF)
	ON_MESSAGE(ID_SETTING_OK, Set_OK)
END_MESSAGE_MAP()

void CDlgSetConnectionSetting::OnBnClickedPxIE()
{
	ChangeStateProxyEdit();
}

void CDlgSetConnectionSetting::OnBnClickedPxNA()
{
	ChangeStateProxyEdit();
}

void CDlgSetConnectionSetting::OnBnClickedPxEF()
{
	ChangeStateProxyEdit();
}

void CDlgSetConnectionSetting::ChangeStateProxyEdit()
{
	if (((CButton*)GetDlgItem(IDC_RAD_PX_EF))->GetCheck() == 1)
	{
		GetDlgItem(IDC_EDIT_ProxyAddress)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_ProxyBypassAddress)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_EDIT_ProxyAddress)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_ProxyBypassAddress)->EnableWindow(FALSE);
	}
}

BOOL CDlgSetConnectionSetting::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	//ProxyAddress
	CString strProxyAddress;
	strProxyAddress = theApp.m_AppSettingsDlgCurrent.GetProxyAddress();
	SetDlgItemText(IDC_EDIT_ProxyAddress, strProxyAddress);

	//ProxyBypassAddress
	CString strProxyBypassAddress;
	strProxyBypassAddress = theApp.m_AppSettingsDlgCurrent.GetProxyBypassAddress();
	strProxyBypassAddress.Replace(_T("\r"), _T(""));
	strProxyBypassAddress.Replace(_T("\n"), _T(";"));
	strProxyBypassAddress.Replace(_T(";;"), _T(";"));
	SetDlgItemText(IDC_EDIT_ProxyBypassAddress, strProxyBypassAddress);

	//TFProxyType
	int iProxyType = theApp.m_AppSettingsDlgCurrent.GetProxyType();
	if (iProxyType == CSG_PROXY_IE)
		((CButton*)GetDlgItem(IDC_RAD_PX_IE))->SetCheck(1);
	else if (iProxyType == CSG_PROXY_NA)
		((CButton*)GetDlgItem(IDC_RAD_PX_NA))->SetCheck(1);
	else if (iProxyType == CSG_PROXY_TF)
		((CButton*)GetDlgItem(IDC_RAD_PX_EF))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_RAD_PX_IE))->SetCheck(1);
	ChangeStateProxyEdit();

	//UserAgentAppend
	CString strUserAgentAppendStr;
	strUserAgentAppendStr = theApp.m_AppSettingsDlgCurrent.GetUserAgentAppendStr();
	SetDlgItemText(IDC_UserAgentAppendStr, strUserAgentAppendStr);

	return FALSE;
}
void CDlgSetConnectionSetting::OnDestroy()
{
	CPropertyPage::OnDestroy();
}
LRESULT CDlgSetConnectionSetting::Set_OK(WPARAM wParam, LPARAM lParam)
{
	//ProxyAddress
	CString strProxyAddress;
	GetDlgItemText(IDC_EDIT_ProxyAddress, strProxyAddress);
	theApp.m_AppSettingsDlgCurrent.SetProxyAddress(strProxyAddress);

	//ProxyBypassAddress
	CString strProxyBypassAddress;
	GetDlgItemText(IDC_EDIT_ProxyBypassAddress, strProxyBypassAddress);
	strProxyBypassAddress.TrimLeft();
	strProxyBypassAddress.TrimRight();
	strProxyBypassAddress.Replace(_T("\r"), _T(""));
	strProxyBypassAddress.Replace(_T("\n"), _T(";"));
	strProxyBypassAddress.Replace(_T(";;"), _T(";"));

	theApp.m_AppSettingsDlgCurrent.SetProxyBypassAddress(strProxyBypassAddress);

	//ProxyType
	int iProxyType = GetCheckedRadioButton(IDC_RAD_PX_IE, IDC_RAD_PX_EF);

	if (iProxyType == IDC_RAD_PX_IE)
		theApp.m_AppSettingsDlgCurrent.SetProxyType(CSG_PROXY_IE);
	else if (iProxyType == IDC_RAD_PX_NA)
		theApp.m_AppSettingsDlgCurrent.SetProxyType(CSG_PROXY_NA);
	else if (iProxyType == IDC_RAD_PX_EF)
	{
		theApp.m_AppSettingsDlgCurrent.SetProxyType(CSG_PROXY_TF);
		if (strProxyAddress.IsEmpty())
		{
			CString alertMsg;
			alertMsg.LoadString(ID_ALERT_EMPTY_DOMAIN);
			::MessageBox(this->m_hWnd, alertMsg, theApp.m_strThisAppName, MB_OK | MB_ICONERROR);
			return 1;
		}
	}
	else
		theApp.m_AppSettingsDlgCurrent.SetProxyType(CSG_PROXY_IE);

	//UserAgentAppend
	CString strUserAgentAppendStr;
	GetDlgItemText(IDC_UserAgentAppendStr, strUserAgentAppendStr);
	strUserAgentAppendStr.TrimLeft();
	strUserAgentAppendStr.TrimRight();
	theApp.m_AppSettingsDlgCurrent.SetUserAgentAppendStr(strUserAgentAppendStr);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgSetDomainFilter プロパティ ページ
#include "DlgDomainDetail.h"
IMPLEMENT_DYNCREATE(CDlgSetDomainFilter, CPropertyPage)
CDlgSetDomainFilter::CDlgSetDomainFilter() : CPropertyPage(CDlgSetDomainFilter::IDD)
{
}
CDlgSetDomainFilter::~CDlgSetDomainFilter()
{
}
void CDlgSetDomainFilter::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSetDomainFilter)
	DDX_Control(pDX, IDC_LIST1, m_List);
	//}}AFX_DATA_MAP
}

#pragma warning(push, 0)
//警告 C26454 演算のオーバーフロー : '-' の操作では、コンパイル時に負の符号なしの結果が生成されます(io .5)。
#pragma warning(disable : 26454)
BEGIN_MESSAGE_MAP(CDlgSetDomainFilter, CPropertyPage)
	//{{AFX_MSG_MAP(CDlgSetDomainFilter)
	ON_BN_CLICKED(IDC_BUTTON_INS, OnButtonPopIns)
	ON_BN_CLICKED(IDC_BUTTON_DEL, OnButtonPopDel)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDblclkList1)
	ON_BN_CLICKED(IDC_BUTTON_UP, OnButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, OnButtonDown)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_CHECK_ENABLE_URL_FILTER, OnEnableCtrl)
	//}}AFX_MSG_MAP
	ON_MESSAGE(ID_SETTING_OK, Set_OK)
END_MESSAGE_MAP()
#pragma warning(pop)

/////////////////////////////////////////////////////////////////////////////
// CDlgSetDomainFilter メッセージ ハンドラ
void CDlgSetDomainFilter::OnEnableCtrl()
{
	BOOL bChk = FALSE;
	bChk = ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_URL_FILTER))->GetCheck();

	if (bChk)
	{
		GetDlgItem(IDC_LIST1)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_INS)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_DEL)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_LIST1)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_INS)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DEL)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(FALSE);
	}
	return;
}

int CDlgSetDomainFilter::DuplicateChk(LPCTSTR sURL)
{
	int iRet = -1; //重複なしは、-1を返す。
	CString strURL;
	int iSelCount = -1;
	while ((iSelCount = m_List.GetNextItem(iSelCount, LVNI_ALL)) != -1)
	{
		strURL.Empty();
		strURL = m_List.GetItemText(iSelCount, URL);
		if (strURL == sURL)
		{
			iRet = iSelCount;
			break;
		}
	}
	return iRet;
}
void CDlgSetDomainFilter::InsertDlgShow(LPCTSTR sURL)
{
	CDlgDomainDetail Dlg(this);
	if (sURL)
		Dlg.m_strDomainName = sURL;
	Dlg.m_bEnable = TRUE;
	Dlg.m_ActionType = TF_ALLOW;

	if (Dlg.DoModal() == IDOK)
	{
		if (Dlg.m_strDomainName.IsEmpty())
			return;
		int iRet = DuplicateChk(Dlg.m_strDomainName);
		//重複なし。
		if (iRet == -1)
		{
			int index = m_List.GetItemCount();
			int iItem = m_List.InsertItem(index, _T(""));
			m_List.SetItemText(iItem, URL, Dlg.m_strDomainName);
			CString strMode;
			strMode.LoadString(Dlg.m_ActionType == TF_ALLOW ? ID_ACTION_LABEL_ALLOW : ID_ACTION_LABEL_DENY);
			m_List.SetItemText(iItem, ACTION, strMode);

			CString strTemp;
			strTemp = Dlg.m_bEnable ? _T("o") : _T("-");
			m_List.SetItemText(iItem, ENABLE, strTemp);
		}
		else
		{
			m_List.SetFocus();
			m_List.SetItemState(iRet, LVIS_SELECTED, LVIS_SELECTED);
			CString alertMsg;
			alertMsg.LoadString(ID_ALERT_ALREADY_ADDED_DOMAIN);
			CString strErrMsg;
			strErrMsg.Format(alertMsg, iRet + 1, Dlg.m_strDomainName);
			::MessageBox(this->m_hWnd, strErrMsg, theApp.m_strThisAppName, MB_OK | MB_ICONWARNING);
		}
	}
}
void CDlgSetDomainFilter::OnButtonPopIns()
{
	InsertDlgShow(NULL);
}
void CDlgSetDomainFilter::OnButtonPopDel()
{
	int iSelCount = 0;
	iSelCount = m_List.GetSelectedCount();
	if (iSelCount == 0)
		return;
	int nItemCount = m_List.GetItemCount();
	while (nItemCount--)
	{
		if (m_List.GetItemState(nItemCount, LVIS_SELECTED) == LVIS_SELECTED)
		{
			m_List.DeleteItem(nItemCount);
		}
	}
}

void CDlgSetDomainFilter::OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	int iSelCount = 0;
	iSelCount = m_List.GetSelectedCount();
	if (iSelCount != 1)
		return;
	iSelCount = m_List.GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED | LVNI_SELECTED);
	if (iSelCount != -1)
	{
		CDlgDomainDetail Dlg(this);
		BOOL bEnable = FALSE;
		CString strURL;
		CString strMode;
		CString strTemp;
		strURL = m_List.GetItemText(iSelCount, URL);
		strMode = m_List.GetItemText(iSelCount, ACTION);
		strTemp = m_List.GetItemText(iSelCount, ENABLE);
		bEnable = strTemp == _T("o") ? TRUE : FALSE;
		Dlg.m_strDomainName = strURL;
		CString allowLabel;
		allowLabel.LoadString(ID_ACTION_LABEL_ALLOW);
		Dlg.m_ActionType = strMode == allowLabel ? TF_ALLOW : TF_DENY;
		Dlg.m_bEnable = bEnable;
		if (Dlg.DoModal() == IDOK)
		{
			if (Dlg.m_strDomainName.IsEmpty())
				return;

			int iRet = DuplicateChk(Dlg.m_strDomainName);
			//重複なし。
			if (iRet == -1 || iRet == iSelCount)
			{
				m_List.SetItemText(iSelCount, URL, Dlg.m_strDomainName);
				CString strMode;
				strMode.LoadString(Dlg.m_ActionType == TF_ALLOW ? ID_ACTION_LABEL_ALLOW : ID_ACTION_LABEL_DENY);
				m_List.SetItemText(iSelCount, ACTION, strMode);

				CString strTemp;
				strTemp = Dlg.m_bEnable ? _T("o") : _T("-");
				m_List.SetItemText(iSelCount, ENABLE, strTemp);
			}
			else
			{
				m_List.SetFocus();
				m_List.SetItemState(iRet, LVIS_SELECTED, LVIS_SELECTED);
				CString alertMsg;
				alertMsg.LoadString(ID_ALERT_ALREADY_ADDED_DOMAIN);
				CString strErrMsg;
				strErrMsg.Format(alertMsg, iRet + 1, Dlg.m_strDomainName);
				::MessageBox(this->m_hWnd, strErrMsg, theApp.m_strThisAppName, MB_OK | MB_ICONWARNING);
			}
		}
	}
	*pResult = 0;
}

void CDlgSetDomainFilter::OnButtonUp()
{
	int iSelCount = 0;
	int iAfterPos = 0;
	iSelCount = m_List.GetSelectedCount();
	if (iSelCount != 1)
		return;
	iSelCount = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (iSelCount == -1)
		return;

	CString strURL;
	CString strMode;
	CString strEnable;
	strURL = m_List.GetItemText(iSelCount, URL);
	strMode = m_List.GetItemText(iSelCount, ACTION);
	strEnable = m_List.GetItemText(iSelCount, ENABLE);
	iAfterPos = iSelCount - 1;
	if (iAfterPos < 0)
		return;
	m_List.DeleteItem(iSelCount);
	int iItem = m_List.InsertItem(iAfterPos, _T(""));
	m_List.SetItemText(iItem, URL, strURL);
	m_List.SetItemText(iItem, ACTION, strMode);
	m_List.SetItemText(iItem, ENABLE, strEnable);
	m_List.SetItemState(iAfterPos, LVIS_SELECTED, LVIS_SELECTED);
	m_List.SetFocus();
}

void CDlgSetDomainFilter::OnButtonDown()
{
	int iSelCount = 0;
	int iAfterPos = 0;
	iSelCount = m_List.GetSelectedCount();
	if (iSelCount != 1)
		return;
	iSelCount = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (iSelCount == -1)
		return;

	CString strURL;
	CString strMode;
	CString strEnable;
	strURL = m_List.GetItemText(iSelCount, URL);
	strMode = m_List.GetItemText(iSelCount, ACTION);
	strEnable = m_List.GetItemText(iSelCount, ENABLE);
	iAfterPos = iSelCount + 1;
	if (iAfterPos >= m_List.GetItemCount())
		return;
	m_List.DeleteItem(iSelCount);
	int iItem = m_List.InsertItem(iAfterPos, _T(""));
	m_List.SetItemText(iItem, URL, strURL);
	m_List.SetItemText(iItem, ACTION, strMode);
	m_List.SetItemText(iItem, ENABLE, strEnable);
	m_List.SetItemState(iAfterPos, LVIS_SELECTED, LVIS_SELECTED);
	m_List.SetFocus();
}

void CDlgSetDomainFilter::OnSize(UINT nType, int cx, int cy)
{
	m_autoResize.Resize(this);
	CPropertyPage::OnSize(nType, cx, cy);
}
void CDlgSetDomainFilter::OnPaint()
{
	CPaintDC dc(this);
}

BOOL CDlgSetDomainFilter::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	CString columnLabelDomain;
	CString columnLabelAction;
	CString columnLabelEnable;
	columnLabelDomain.LoadString(ID_SETTINGS_COLUMN_HEADER_DOMAIN);
	columnLabelAction.LoadString(ID_SETTINGS_COLUMN_HEADER_ACTION);
	columnLabelEnable.LoadString(ID_SETTINGS_COLUMN_HEADER_ENABLED);
	m_List.InsertColumn(URL, columnLabelDomain, LVCFMT_LEFT, 480);
	m_List.InsertColumn(ACTION, columnLabelAction, LVCFMT_LEFT, 120);
	m_List.InsertColumn(ENABLE, columnLabelEnable, LVCFMT_CENTER, 50);
	ListView_SetExtendedListViewStyle(m_List.m_hWnd, LVS_EX_FULLROWSELECT);

	if (theApp.m_AppSettingsDlgCurrent.IsEnableURLFilter())
		((CButton*)GetDlgItem(IDC_CHECK_ENABLE_URL_FILTER))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_ENABLE_URL_FILTER))->SetCheck(0);

	_wsetlocale(LC_ALL, _T("jpn"));
	CStdioFile in;
	CString strPath;
	strPath = theApp.m_strDomainFilterFileFullPath;
	if (in.Open(strPath, CFile::modeReadWrite | CFile::shareDenyWrite | CFile::modeCreate | CFile::modeNoTruncate))
	{
		CString strTemp;
		CString strTemp2;
		CString strTemp3;
		CStringArray strArray;

		while (in.ReadString(strTemp))
		{
			strTemp2.Empty();
			strTemp3.Empty();
			strArray.RemoveAll();
			strTemp.TrimLeft();
			strTemp.TrimRight();
			if (strTemp.IsEmpty())
				continue;

			BOOL bEnable = TRUE;
			SBUtil::Split(&strArray, strTemp, _T("\t"));
			if (strArray.GetSize() >= 2)
			{
				strTemp2 = strArray.GetAt(0);
				strTemp2.TrimLeft();
				strTemp2.TrimRight();

				strTemp3 = strArray.GetAt(1);
				strTemp3.TrimLeft();
				strTemp3.TrimRight();
				if (strTemp2.Find(_T(";")) == 0)
				{
					bEnable = FALSE;
					strTemp2 = strTemp2.Mid(1);
				}
				else if (strTemp2.Find(_T("#")) == 0)
				{
					bEnable = FALSE;
					strTemp2 = strTemp2.Mid(1);
				}

				if (strTemp2.IsEmpty())
					continue;

				int iRet = DuplicateChk(strTemp2);
				//重複なし。
				if (iRet == -1)
				{
					int index = m_List.GetItemCount();
					int iItem = m_List.InsertItem(index, _T(""));
					CString strLowString;
					m_List.SetItemText(iItem, URL, strTemp2);

					CString strMode;
					strMode.LoadString(strTemp3 == _T("A") ? ID_ACTION_LABEL_ALLOW : ID_ACTION_LABEL_DENY);
					m_List.SetItemText(iItem, ACTION, strMode);

					strLowString = bEnable ? _T("o") : _T("-");
					m_List.SetItemText(iItem, ENABLE, strLowString);
				}
			}
		}
		in.Close();
	}
	else
	{
		CString alertMsg;
		alertMsg.LoadString(ID_CANNOT_SAVE_URL_FILTER);
		CString strErrMsg;
		strErrMsg.Format(alertMsg, strPath);
		::MessageBox(this->m_hWnd, strErrMsg, theApp.m_strThisAppName, MB_OK | MB_ICONERROR);
	}
	OnEnableCtrl();
	return TRUE;
}

LRESULT CDlgSetDomainFilter::Set_OK(WPARAM wParam, LPARAM lParam)
{
	if (((CButton*)GetDlgItem(IDC_CHECK_ENABLE_URL_FILTER))->GetCheck() == 1)
	{
		theApp.m_AppSettingsDlgCurrent.SetEnableURLFilter(1);
	}
	else
	{
		theApp.m_AppSettingsDlgCurrent.SetEnableURLFilter(0);
	}

	_wsetlocale(LC_ALL, _T("jpn"));
	CStdioFile out;
	CString strPath;
	strPath = theApp.m_strDomainFilterFileFullPath;
	if (out.Open(strPath, CFile::modeWrite | CFile::modeCreate))
	{
		CString strURL;
		CString strMode;
		CString strEnable;
		CString strLineData;
		BOOL bEnable = FALSE;
		int iSelCount = -1;
		while ((iSelCount = m_List.GetNextItem(iSelCount, LVNI_ALL)) != -1)
		{
			strLineData.Empty();
			strURL.Empty();
			strMode.Empty();
			strEnable.Empty();
			bEnable = FALSE;
			strURL = m_List.GetItemText(iSelCount, URL);
			int iRet = DuplicateChk(strURL);
			//重複なし。
			if (iRet == -1 || iRet == iSelCount)
			{
				strMode = m_List.GetItemText(iSelCount, ACTION);
				strEnable = m_List.GetItemText(iSelCount, ENABLE);
				bEnable = strEnable == _T("o") ? TRUE : FALSE;
				CString allowLabel;
				allowLabel.LoadString(ID_ACTION_LABEL_ALLOW);
				if (bEnable)
					strLineData.Format(_T("%s\t%s\n"), (LPCTSTR)strURL, strMode == allowLabel ? _T("A") : _T("D"));
				else
					strLineData.Format(_T(";%s\t%s\n"), (LPCTSTR)strURL, strMode == allowLabel ? _T("A") : _T("D"));
				out.WriteString(strLineData);
			}
		}
		out.Close();
		if (theApp.InVirtualEnvironment() == VE_THINAPP)
		{
			CString strSBFilePath;
			strSBFilePath = theApp.GetSandboxFilePath(theApp.m_strDomainFilterFileFullPath);
			if (!strSBFilePath.IsEmpty())
			{
				CString strFilePathCnf;
				strFilePathCnf = theApp.m_strDomainFilterFileFullPath;
				strFilePathCnf.Replace(_T("Default.conf"), _T(".conf"));
				theApp.Exec_SB2PYS_COPY(strSBFilePath, strFilePathCnf);
			}
		}
	}
	else
	{
		CString alertMsg;
		alertMsg.LoadString(ID_CANNOT_SAVE_URL_FILTER);
		CString strErrMsg;
		strErrMsg.Format(alertMsg, strPath);
		::MessageBox(this->m_hWnd, strErrMsg, theApp.m_strThisAppName, MB_OK | MB_ICONERROR);
		return 1;
	}
	return 0;
}

IMPLEMENT_DYNCREATE(CDlgSetPopupFilter, CPropertyPage)
CDlgSetPopupFilter::CDlgSetPopupFilter() : CPropertyPage(CDlgSetPopupFilter::IDD)
{
}
CDlgSetPopupFilter::~CDlgSetPopupFilter()
{
}
void CDlgSetPopupFilter::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSetPopupFilter)
	DDX_Control(pDX, IDC_LIST1, m_List);
	//}}AFX_DATA_MAP
}

#pragma warning(push, 0)
// 警告 C26454 演算のオーバーフロー : '-' の操作では、コンパイル時に負の符号なしの結果が生成されます(io .5)。
#pragma warning(disable : 26454)
BEGIN_MESSAGE_MAP(CDlgSetPopupFilter, CPropertyPage)
	//{{AFX_MSG_MAP(CDlgSetPopupFilter)
	ON_BN_CLICKED(IDC_BUTTON_INS, OnButtonPopIns)
	ON_BN_CLICKED(IDC_BUTTON_DEL, OnButtonPopDel)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDblclkList1)
	ON_BN_CLICKED(IDC_BUTTON_UP, OnButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, OnButtonDown)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_CHECK_ENABLE_POPUP_FILTER, OnEnableCtrl)
	//}}AFX_MSG_MAP
	ON_MESSAGE(ID_SETTING_OK, Set_OK)
END_MESSAGE_MAP()
#pragma warning(pop)

/////////////////////////////////////////////////////////////////////////////
// CDlgSetPopupFilter メッセージ ハンドラ
void CDlgSetPopupFilter::OnEnableCtrl()
{
	BOOL bChk = FALSE;
	bChk = ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_POPUP_FILTER))->GetCheck();

	if (bChk)
	{
		GetDlgItem(IDC_LIST1)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_INS)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_DEL)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_LIST1)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_INS)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DEL)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(FALSE);
	}
	return;
}

int CDlgSetPopupFilter::DuplicateChk(LPCTSTR sURL)
{
	int iRet = -1; // 重複なしは、-1を返す。
	CString strURL;
	int iSelCount = -1;
	while ((iSelCount = m_List.GetNextItem(iSelCount, LVNI_ALL)) != -1)
	{
		strURL.Empty();
		strURL = m_List.GetItemText(iSelCount, URL);
		if (strURL == sURL)
		{
			iRet = iSelCount;
			break;
		}
	}
	return iRet;
}
void CDlgSetPopupFilter::InsertDlgShow(LPCTSTR sURL)
{
	CDlgPopupDetail Dlg(this);
	if (sURL)
		Dlg.m_strDomainName = sURL;
	Dlg.m_bEnable = TRUE;
	Dlg.m_ActionType = TF_ALLOW;

	if (Dlg.DoModal() == IDOK)
	{
		if (Dlg.m_strDomainName.IsEmpty())
			return;
		int iRet = DuplicateChk(Dlg.m_strDomainName);
		// 重複なし。
		if (iRet == -1)
		{
			int index = m_List.GetItemCount();
			int iItem = m_List.InsertItem(index, _T(""));
			m_List.SetItemText(iItem, URL, Dlg.m_strDomainName);
			CString strMode;
			strMode.LoadString(Dlg.m_ActionType == TF_ALLOW ? ID_ACTION_LABEL_ALLOW : ID_ACTION_LABEL_DENY);
			m_List.SetItemText(iItem, ACTION, strMode);

			CString strTemp;
			strTemp = Dlg.m_bEnable ? _T("o") : _T("-");
			m_List.SetItemText(iItem, ENABLE, strTemp);
		}
		else
		{
			m_List.SetFocus();
			m_List.SetItemState(iRet, LVIS_SELECTED, LVIS_SELECTED);
			CString alertMsg;
			alertMsg.LoadString(ID_ALERT_ALREADY_ADDED_DOMAIN);
			CString strErrMsg;
			strErrMsg.Format(alertMsg, iRet + 1, Dlg.m_strDomainName);
			::MessageBox(this->m_hWnd, strErrMsg, theApp.m_strThisAppName, MB_OK | MB_ICONWARNING);
		}
	}
}
void CDlgSetPopupFilter::OnButtonPopIns()
{
	InsertDlgShow(NULL);
}
void CDlgSetPopupFilter::OnButtonPopDel()
{
	int iSelCount = 0;
	iSelCount = m_List.GetSelectedCount();
	if (iSelCount == 0)
		return;
	int nItemCount = m_List.GetItemCount();
	while (nItemCount--)
	{
		if (m_List.GetItemState(nItemCount, LVIS_SELECTED) == LVIS_SELECTED)
		{
			m_List.DeleteItem(nItemCount);
		}
	}
}

void CDlgSetPopupFilter::OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	int iSelCount = 0;
	iSelCount = m_List.GetSelectedCount();
	if (iSelCount != 1)
		return;
	iSelCount = m_List.GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED | LVNI_SELECTED);
	if (iSelCount != -1)
	{
		CDlgPopupDetail Dlg(this);
		BOOL bEnable = FALSE;
		CString strURL;
		CString strMode;
		CString strTemp;
		strURL = m_List.GetItemText(iSelCount, URL);
		strMode = m_List.GetItemText(iSelCount, ACTION);
		strTemp = m_List.GetItemText(iSelCount, ENABLE);
		bEnable = strTemp == _T("o") ? TRUE : FALSE;
		Dlg.m_strDomainName = strURL;
		CString allowLabel;
		allowLabel.LoadString(ID_ACTION_LABEL_ALLOW);
		Dlg.m_ActionType = strMode == allowLabel ? TF_ALLOW : TF_DENY;
		Dlg.m_bEnable = bEnable;
		if (Dlg.DoModal() == IDOK)
		{
			if (Dlg.m_strDomainName.IsEmpty())
				return;

			int iRet = DuplicateChk(Dlg.m_strDomainName);
			// 重複なし。
			if (iRet == -1 || iRet == iSelCount)
			{
				m_List.SetItemText(iSelCount, URL, Dlg.m_strDomainName);
				CString strMode;
				strMode.LoadString(Dlg.m_ActionType == TF_ALLOW ? ID_ACTION_LABEL_ALLOW : ID_ACTION_LABEL_DENY);
				m_List.SetItemText(iSelCount, ACTION, strMode);

				CString strTemp;
				strTemp = Dlg.m_bEnable ? _T("o") : _T("-");
				m_List.SetItemText(iSelCount, ENABLE, strTemp);
			}
			else
			{
				m_List.SetFocus();
				m_List.SetItemState(iRet, LVIS_SELECTED, LVIS_SELECTED);
				CString alertMsg;
				alertMsg.LoadString(ID_ALERT_ALREADY_ADDED_DOMAIN);
				CString strErrMsg;
				strErrMsg.Format(alertMsg, iRet + 1, Dlg.m_strDomainName);
				::MessageBox(this->m_hWnd, strErrMsg, theApp.m_strThisAppName, MB_OK | MB_ICONWARNING);
			}
		}
	}
	*pResult = 0;
}

void CDlgSetPopupFilter::OnButtonUp()
{
	int iSelCount = 0;
	int iAfterPos = 0;
	iSelCount = m_List.GetSelectedCount();
	if (iSelCount != 1)
		return;
	iSelCount = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (iSelCount == -1)
		return;

	CString strURL;
	CString strMode;
	CString strEnable;
	strURL = m_List.GetItemText(iSelCount, URL);
	strMode = m_List.GetItemText(iSelCount, ACTION);
	strEnable = m_List.GetItemText(iSelCount, ENABLE);
	iAfterPos = iSelCount - 1;
	if (iAfterPos < 0)
		return;
	m_List.DeleteItem(iSelCount);
	int iItem = m_List.InsertItem(iAfterPos, _T(""));
	m_List.SetItemText(iItem, URL, strURL);
	m_List.SetItemText(iItem, ACTION, strMode);
	m_List.SetItemText(iItem, ENABLE, strEnable);
	m_List.SetItemState(iAfterPos, LVIS_SELECTED, LVIS_SELECTED);
	m_List.SetFocus();
}

void CDlgSetPopupFilter::OnButtonDown()
{
	int iSelCount = 0;
	int iAfterPos = 0;
	iSelCount = m_List.GetSelectedCount();
	if (iSelCount != 1)
		return;
	iSelCount = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (iSelCount == -1)
		return;

	CString strURL;
	CString strMode;
	CString strEnable;
	strURL = m_List.GetItemText(iSelCount, URL);
	strMode = m_List.GetItemText(iSelCount, ACTION);
	strEnable = m_List.GetItemText(iSelCount, ENABLE);
	iAfterPos = iSelCount + 1;
	if (iAfterPos >= m_List.GetItemCount())
		return;
	m_List.DeleteItem(iSelCount);
	int iItem = m_List.InsertItem(iAfterPos, _T(""));
	m_List.SetItemText(iItem, URL, strURL);
	m_List.SetItemText(iItem, ACTION, strMode);
	m_List.SetItemText(iItem, ENABLE, strEnable);
	m_List.SetItemState(iAfterPos, LVIS_SELECTED, LVIS_SELECTED);
	m_List.SetFocus();
}

void CDlgSetPopupFilter::OnSize(UINT nType, int cx, int cy)
{
	m_autoResize.Resize(this);
	CPropertyPage::OnSize(nType, cx, cy);
}
void CDlgSetPopupFilter::OnPaint()
{
	CPaintDC dc(this);
}

BOOL CDlgSetPopupFilter::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	CString columnLabelPopup;
	CString columnLabelAction;
	CString columnLabelEnable;
	columnLabelPopup.LoadString(ID_SETTINGS_COLUMN_HEADER_DOMAIN);
	columnLabelAction.LoadString(ID_SETTINGS_COLUMN_HEADER_ACTION);
	columnLabelEnable.LoadString(ID_SETTINGS_COLUMN_HEADER_ENABLED);
	m_List.InsertColumn(URL, columnLabelPopup, LVCFMT_LEFT, 480);
	m_List.InsertColumn(ACTION, columnLabelAction, LVCFMT_LEFT, 120);
	m_List.InsertColumn(ENABLE, columnLabelEnable, LVCFMT_CENTER, 50);
	ListView_SetExtendedListViewStyle(m_List.m_hWnd, LVS_EX_FULLROWSELECT);

	if (theApp.m_AppSettingsDlgCurrent.IsEnablePopupFilter())
		((CButton*)GetDlgItem(IDC_CHECK_ENABLE_POPUP_FILTER))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_ENABLE_POPUP_FILTER))->SetCheck(0);

	_wsetlocale(LC_ALL, _T("jpn"));
	CStdioFile in;
	CString strPath;
	strPath = theApp.m_strPopupFilterFileFullPath;
	if (in.Open(strPath, CFile::modeReadWrite | CFile::shareDenyWrite | CFile::modeCreate | CFile::modeNoTruncate))
	{
		CString strTemp;
		CString strTemp2;
		CString strTemp3;
		CStringArray strArray;

		while (in.ReadString(strTemp))
		{
			strTemp2.Empty();
			strTemp3.Empty();
			strArray.RemoveAll();
			strTemp.TrimLeft();
			strTemp.TrimRight();
			if (strTemp.IsEmpty())
				continue;

			BOOL bEnable = TRUE;
			SBUtil::Split(&strArray, strTemp, _T("\t"));
			if (strArray.GetSize() >= 2)
			{
				strTemp2 = strArray.GetAt(0);
				strTemp2.TrimLeft();
				strTemp2.TrimRight();

				strTemp3 = strArray.GetAt(1);
				strTemp3.TrimLeft();
				strTemp3.TrimRight();
				if (strTemp2.Find(_T(";")) == 0)
				{
					bEnable = FALSE;
					strTemp2 = strTemp2.Mid(1);
				}
				else if (strTemp2.Find(_T("#")) == 0)
				{
					bEnable = FALSE;
					strTemp2 = strTemp2.Mid(1);
				}

				if (strTemp2.IsEmpty())
					continue;

				int iRet = DuplicateChk(strTemp2);
				// 重複なし。
				if (iRet == -1)
				{
					int index = m_List.GetItemCount();
					int iItem = m_List.InsertItem(index, _T(""));
					CString strLowString;
					m_List.SetItemText(iItem, URL, strTemp2);

					CString strMode;
					strMode.LoadString(strTemp3 == _T("A") ? ID_ACTION_LABEL_ALLOW : ID_ACTION_LABEL_DENY);
					m_List.SetItemText(iItem, ACTION, strMode);

					strLowString = bEnable ? _T("o") : _T("-");
					m_List.SetItemText(iItem, ENABLE, strLowString);
				}
			}
		}
		in.Close();
	}
	else
	{
		CString alertMsg;
		alertMsg.LoadString(ID_CANNOT_SAVE_POPUP_FILTER);
		CString strErrMsg;
		strErrMsg.Format(alertMsg, strPath);
		::MessageBox(this->m_hWnd, strErrMsg, theApp.m_strThisAppName, MB_OK | MB_ICONERROR);
	}
	OnEnableCtrl();
	return TRUE;
}

LRESULT CDlgSetPopupFilter::Set_OK(WPARAM wParam, LPARAM lParam)
{
	if (((CButton*)GetDlgItem(IDC_CHECK_ENABLE_POPUP_FILTER))->GetCheck() == 1)
	{
		theApp.m_AppSettingsDlgCurrent.SetEnablePopupFilter(1);
	}
	else
	{
		theApp.m_AppSettingsDlgCurrent.SetEnablePopupFilter(0);
	}

	_wsetlocale(LC_ALL, _T("jpn"));
	CStdioFile out;
	CString strPath;
	strPath = theApp.m_strPopupFilterFileFullPath;
	if (out.Open(strPath, CFile::modeWrite | CFile::modeCreate))
	{
		CString strURL;
		CString strMode;
		CString strEnable;
		CString strLineData;
		BOOL bEnable = FALSE;
		int iSelCount = -1;
		while ((iSelCount = m_List.GetNextItem(iSelCount, LVNI_ALL)) != -1)
		{
			strLineData.Empty();
			strURL.Empty();
			strMode.Empty();
			strEnable.Empty();
			bEnable = FALSE;
			strURL = m_List.GetItemText(iSelCount, URL);
			int iRet = DuplicateChk(strURL);
			// 重複なし。
			if (iRet == -1 || iRet == iSelCount)
			{
				strMode = m_List.GetItemText(iSelCount, ACTION);
				strEnable = m_List.GetItemText(iSelCount, ENABLE);
				bEnable = strEnable == _T("o") ? TRUE : FALSE;
				CString allowLabel;
				allowLabel.LoadString(ID_ACTION_LABEL_ALLOW);
				if (bEnable)
					strLineData.Format(_T("%s\t%s\n"), (LPCTSTR)strURL, strMode == allowLabel ? _T("A") : _T("D"));
				else
					strLineData.Format(_T(";%s\t%s\n"), (LPCTSTR)strURL, strMode == allowLabel ? _T("A") : _T("D"));
				out.WriteString(strLineData);
			}
		}
		out.Close();
		if (theApp.InVirtualEnvironment() == VE_THINAPP)
		{
			CString strSBFilePath;
			strSBFilePath = theApp.GetSandboxFilePath(theApp.m_strPopupFilterFileFullPath);
			if (!strSBFilePath.IsEmpty())
			{
				CString strFilePathCnf;
				strFilePathCnf = theApp.m_strPopupFilterFileFullPath;
				strFilePathCnf.Replace(_T("Default.conf"), _T(".conf"));
				theApp.Exec_SB2PYS_COPY(strSBFilePath, strFilePathCnf);
			}
		}
	}
	else
	{
		CString alertMsg;
		alertMsg.LoadString(ID_CANNOT_SAVE_POPUP_FILTER);
		CString strErrMsg;
		strErrMsg.Format(alertMsg, strPath);
		::MessageBox(this->m_hWnd, strErrMsg, theApp.m_strThisAppName, MB_OK | MB_ICONERROR);
		return 1;
	}
	return 0;
}

void CDlgSetDSP::OnBnClickedButton1()
{
	CStringArray straMenu;
	straMenu.Add(_T("500%"));
	straMenu.Add(_T("400%"));
	straMenu.Add(_T("300%"));
	straMenu.Add(_T("250%"));
	straMenu.Add(_T("200%"));
	straMenu.Add(_T("175%"));
	straMenu.Add(_T("150%"));
	straMenu.Add(_T("125%"));
	straMenu.Add(_T("110%"));

	straMenu.Add(_T("100%"));

	straMenu.Add(_T("90%"));
	straMenu.Add(_T("80%"));
	straMenu.Add(_T("75%"));
	straMenu.Add(_T("67%"));
	straMenu.Add(_T("50%"));
	straMenu.Add(_T("33%"));
	straMenu.Add(_T("25%"));

	CMenu menu;
	menu.LoadMenu(IDR_MENU_BACK);
	CMenu* pPopup = menu.GetSubMenu(0);
	while (pPopup->DeleteMenu(0, MF_BYPOSITION))
		;

	CString strName;
	int iCnt = 0;
	for (int i = 0; i < straMenu.GetSize(); i++)
	{
		strName = straMenu.GetAt(i);
		pPopup->AppendMenu(MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_BF_BACK1 + iCnt, strName);
		iCnt++;
	}
	//POINT point = { 0 };
	CRect rc;
	GetDlgItem(IDC_BUTTON1)->GetWindowRect(&rc);
	//::GetCursorPos(&point);
	long lResult = 0;
	lResult = TrackPopupMenuEx(pPopup->m_hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
				   rc.left, rc.bottom,
				   this->m_hWnd, NULL);
	if (lResult > 0)
	{
		lResult = lResult - ID_BF_BACK1;
		CString strRet;
		strRet = straMenu.GetAt(lResult);
		if (!strRet.IsEmpty())
		{
			strRet.Replace(_T("%"), _T(""));
			SetDlgItemText(IDC_EDIT_ZOOM, strRet);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDlgSetFileMgr ダイアログ

IMPLEMENT_DYNCREATE(CDlgSetFileMgr, CPropertyPage)
CDlgSetFileMgr::CDlgSetFileMgr() : CPropertyPage(CDlgSetFileMgr::IDD)
{
}

void CDlgSetFileMgr::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EnableOpenedOp, m_Combo);
}

BEGIN_MESSAGE_MAP(CDlgSetFileMgr, CPropertyPage)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_EnableUploadSync, &CDlgSetFileMgr::OnBnClickedULS)
	ON_MESSAGE(ID_SETTING_OK, Set_OK)
END_MESSAGE_MAP()

BOOL CDlgSetFileMgr::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	SetDlgItemText(IDC_RootPath, theApp.m_AppSettingsDlgCurrent.GetRootPath());
	SetDlgItemText(IDC_ExtFilter, theApp.m_AppSettingsDlgCurrent.GetExtFilter());
	SetDlgItemText(IDC_DisallowExt, theApp.m_AppSettingsDlgCurrent.GetDisallowExt());
	SetDlgItemText(IDC_DisallowTransferringExt, theApp.m_AppSettingsDlgCurrent.GetDisallowTransferringExt());
	SetDlgItemText(IDC_TransferPath, theApp.m_AppSettingsDlgCurrent.GetTransferPath());
	SetDlgItemText(IDC_TransferSubFolder, theApp.m_AppSettingsDlgCurrent.GetTransferSubFolder());
	SetDlgItemText(IDC_UploadPath, theApp.m_AppSettingsDlgCurrent.GetUploadPath());
	SetDlgItemInt(IDC_UploadSyncInterval, theApp.m_AppSettingsDlgCurrent.GetUploadSyncInterval());

	for (int i = 0; i < InfoOpenOpMaxCnt; i++)
	{
		CString intoOpenOpLabel;
		intoOpenOpLabel.LoadString(gInfoOpenOp[i]);
		m_Combo.AddString(intoOpenOpLabel);
	}
	int iEnableOpenedOp = 0;
	iEnableOpenedOp = theApp.m_AppSettingsDlgCurrent.GetEnableOpenedOp();
	if (iEnableOpenedOp < 0 || iEnableOpenedOp >= InfoOpenOpMaxCnt)
		iEnableOpenedOp = 0;
	m_Combo.SetCurSel(iEnableOpenedOp);

	if (theApp.m_AppSettingsDlgCurrent.IsDisableOpenedOpAlert())
		((CButton*)GetDlgItem(IDC_DisableOpenedOpAlert))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_DisableOpenedOpAlert))->SetCheck(0);

	if (theApp.m_AppSettingsDlgCurrent.IsShowUploadTab())
		((CButton*)GetDlgItem(IDC_ShowUploadTab))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_ShowUploadTab))->SetCheck(0);

	if (theApp.m_AppSettingsDlgCurrent.IsEnableUploadSync())
		((CButton*)GetDlgItem(IDC_EnableUploadSync))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_EnableUploadSync))->SetCheck(0);

	if (theApp.m_AppSettingsDlgCurrent.IsEnableUploadSyncMirror())
		((CButton*)GetDlgItem(IDC_EnableUploadSyncMirror))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_EnableUploadSyncMirror))->SetCheck(0);

	this->ChangeStateULS();
	return FALSE;
}
void CDlgSetFileMgr::OnBnClickedULS()
{
	this->ChangeStateULS();
}
void CDlgSetFileMgr::ChangeStateULS()
{
	if (((CButton*)GetDlgItem(IDC_EnableUploadSync))->GetCheck() == 1)
	{
		GetDlgItem(IDC_UploadSyncInterval)->EnableWindow(TRUE);
		GetDlgItem(IDC_EnableUploadSyncMirror)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_UploadSyncInterval)->EnableWindow(FALSE);
		GetDlgItem(IDC_EnableUploadSyncMirror)->EnableWindow(FALSE);
	}
}

LRESULT CDlgSetFileMgr::Set_OK(WPARAM wParam, LPARAM lParam)
{
	CString strValue;

	strValue.Empty();
	GetDlgItemText(IDC_RootPath, strValue);
	theApp.m_AppSettingsDlgCurrent.SetRootPath(strValue);

	strValue.Empty();
	GetDlgItemText(IDC_ExtFilter, strValue);
	theApp.m_AppSettingsDlgCurrent.SetExtFilter(strValue);

	strValue.Empty();
	GetDlgItemText(IDC_DisallowExt, strValue);
	theApp.m_AppSettingsDlgCurrent.SetDisallowExt(strValue);

	strValue.Empty();
	GetDlgItemText(IDC_DisallowTransferringExt, strValue);
	theApp.m_AppSettingsDlgCurrent.SetDisallowTransferringExt(strValue);

	strValue.Empty();
	GetDlgItemText(IDC_TransferPath, strValue);
	theApp.m_AppSettingsDlgCurrent.SetTransferPath(strValue);

	strValue.Empty();
	GetDlgItemText(IDC_TransferSubFolder, strValue);
	theApp.m_AppSettingsDlgCurrent.SetTransferSubFolder(strValue);

	strValue.Empty();
	GetDlgItemText(IDC_UploadPath, strValue);
	theApp.m_AppSettingsDlgCurrent.SetUploadPath(strValue);

	strValue.Empty();
	int iLimitTime = 0;
	iLimitTime = GetDlgItemInt(IDC_UploadSyncInterval);

	if (0 > iLimitTime)
		iLimitTime = 0;

	GetDlgItemText(IDC_UploadSyncInterval, strValue);
	theApp.m_AppSettingsDlgCurrent.SetUploadSyncInterval(iLimitTime);

	int iID = 0;
	iID = m_Combo.GetCurSel();
	theApp.m_AppSettingsDlgCurrent.SetEnableOpenedOp(iID);

	if (((CButton*)GetDlgItem(IDC_DisableOpenedOpAlert))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetDisableOpenedOpAlert(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetDisableOpenedOpAlert(0);

	if (((CButton*)GetDlgItem(IDC_ShowUploadTab))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetShowUploadTab(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetShowUploadTab(0);

	if (((CButton*)GetDlgItem(IDC_EnableUploadSync))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetEnableUploadSync(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetEnableUploadSync(0);

	if (((CButton*)GetDlgItem(IDC_EnableUploadSyncMirror))->GetCheck() == 1)
		theApp.m_AppSettingsDlgCurrent.SetEnableUploadSyncMirror(1);
	else
		theApp.m_AppSettingsDlgCurrent.SetEnableUploadSyncMirror(0);
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// CDlgSetCustomScript プロパティ ページ
IMPLEMENT_DYNCREATE(CDlgSetCustomScript, CPropertyPage)
CDlgSetCustomScript::CDlgSetCustomScript() : CPropertyPage(CDlgSetCustomScript::IDD)
{
}
CDlgSetCustomScript::~CDlgSetCustomScript()
{
}
void CDlgSetCustomScript::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSetCustomScript)
	DDX_Control(pDX, IDC_LIST1, m_List);
	//}}AFX_DATA_MAP
}

#pragma warning(push, 0)
//警告 C26454 演算のオーバーフロー : '-' の操作では、コンパイル時に負の符号なしの結果が生成されます(io .5)。
#pragma warning(disable : 26454)
BEGIN_MESSAGE_MAP(CDlgSetCustomScript, CPropertyPage)
	//{{AFX_MSG_MAP(CDlgSetCustomScript)
	ON_BN_CLICKED(IDC_BUTTON_INS, OnButtonPopIns)
	ON_BN_CLICKED(IDC_BUTTON_DEL, OnButtonPopDel)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDblclkList1)
	ON_BN_CLICKED(IDC_BUTTON_UP, OnButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, OnButtonDown)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_CHECK_ENABLE_CUSTOM_SCRIPT, OnEnableCtrl)
	//}}AFX_MSG_MAP
	ON_MESSAGE(ID_SETTING_OK, Set_OK)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgSetCustomScript::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_SHOW_DEV_TOOLS, &CDlgSetCustomScript::OnBnClickedShowDevTools)
END_MESSAGE_MAP()
#pragma warning(pop)

/////////////////////////////////////////////////////////////////////////////
// CDlgSetCustomScript メッセージ ハンドラ
void CDlgSetCustomScript::OnEnableCtrl()
{
	BOOL bChk = FALSE;
	bChk = ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_CUSTOM_SCRIPT))->GetCheck();

	if (bChk)
	{
		GetDlgItem(IDC_LIST1)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_INS)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_DEL)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_LIST1)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_INS)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DEL)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(FALSE);
	}
	return;
}

int CDlgSetCustomScript::DuplicateChk(LPCTSTR sURL, LPCTSTR sFileName)
{
	int iRet = -1; //重複なしは、-1を返す。
	CString strURL;
	CString strFileName;
	int iSelCount = -1;
	while ((iSelCount = m_List.GetNextItem(iSelCount, LVNI_ALL)) != -1)
	{
		strURL.Empty();
		strFileName.Empty();
		strURL = m_List.GetItemText(iSelCount, URL);
		if (strURL == sURL)
		{
			strFileName = m_List.GetItemText(iSelCount, FILENAME);
			if (strFileName == sFileName)
			{
				iRet = iSelCount;
				break;
			}
		}
	}
	return iRet;
}
void CDlgSetCustomScript::InsertDlgShow()
{
	CDlgCustomScriptDetail Dlg(this);
	Dlg.m_bEnable = TRUE;
	if (Dlg.DoModal() == IDOK)
	{
		if (Dlg.m_strURL.IsEmpty())
			return;
		if (Dlg.m_strFileName.IsEmpty())
			return;
		int iRet = DuplicateChk(Dlg.m_strURL, Dlg.m_strFileName);
		//重複なし。
		if (iRet == -1)
		{
			int index = m_List.GetItemCount();
			int iItem = m_List.InsertItem(index, _T(""));
			m_List.SetItemText(iItem, URL, Dlg.m_strURL);
			m_List.SetItemText(iItem, FILENAME, Dlg.m_strFileName);

			CString strTemp;
			strTemp = Dlg.m_bEnable ? _T("o") : _T("-");
			m_List.SetItemText(iItem, ENABLE, strTemp);
		}
		else
		{
			m_List.SetFocus();
			m_List.SetItemState(iRet, LVIS_SELECTED, LVIS_SELECTED);
			CString alreadyAddedMsg;
			alreadyAddedMsg.LoadString(ID_ALERT_ALREADY_ADDED_URL_OR_FILE);
			CString strErrMsg;
			strErrMsg.Format(alreadyAddedMsg, iRet + 1, Dlg.m_strURL, Dlg.m_strFileName);
			::MessageBox(this->m_hWnd, strErrMsg, theApp.m_strThisAppName, MB_OK | MB_ICONWARNING);
		}
	}
}
void CDlgSetCustomScript::OnButtonPopIns()
{
	InsertDlgShow();
}
void CDlgSetCustomScript::OnButtonPopDel()
{
	int iSelCount = 0;
	iSelCount = m_List.GetSelectedCount();
	if (iSelCount == 0)
		return;
	int nItemCount = m_List.GetItemCount();
	while (nItemCount--)
	{
		if (m_List.GetItemState(nItemCount, LVIS_SELECTED) == LVIS_SELECTED)
		{
			m_List.DeleteItem(nItemCount);
		}
	}
}

void CDlgSetCustomScript::OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	int iSelCount = 0;
	iSelCount = m_List.GetSelectedCount();
	if (iSelCount != 1)
		return;
	iSelCount = m_List.GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED | LVNI_SELECTED);
	if (iSelCount != -1)
	{
		CDlgCustomScriptDetail Dlg(this);
		BOOL bEnable = FALSE;
		CString strURL;
		CString strFILENAME;
		CString strTemp;
		strURL = m_List.GetItemText(iSelCount, URL);
		strFILENAME = m_List.GetItemText(iSelCount, FILENAME);
		strTemp = m_List.GetItemText(iSelCount, ENABLE);
		bEnable = strTemp == _T("o") ? TRUE : FALSE;
		Dlg.m_strURL = strURL;
		Dlg.m_strFileName = strFILENAME;
		Dlg.m_bEnable = bEnable;
		if (Dlg.DoModal() == IDOK)
		{
			if (Dlg.m_strURL.IsEmpty())
				return;
			if (Dlg.m_strFileName.IsEmpty())
				return;

			int iRet = DuplicateChk(Dlg.m_strURL, Dlg.m_strFileName);
			//重複なし。
			if (iRet == -1 || iRet == iSelCount)
			{
				m_List.SetItemText(iSelCount, URL, Dlg.m_strURL);
				m_List.SetItemText(iSelCount, FILENAME, Dlg.m_strFileName);

				CString strTemp;
				strTemp = Dlg.m_bEnable ? _T("o") : _T("-");
				m_List.SetItemText(iSelCount, ENABLE, strTemp);
			}
			else
			{
				m_List.SetFocus();
				m_List.SetItemState(iRet, LVIS_SELECTED, LVIS_SELECTED);
				CString alreadyAddedMsg;
				alreadyAddedMsg.LoadString(ID_ALERT_ALREADY_ADDED_URL_OR_FILE);
				CString strErrMsg;
				strErrMsg.Format(alreadyAddedMsg, iRet + 1, Dlg.m_strURL, Dlg.m_strFileName);
				::MessageBox(this->m_hWnd, strErrMsg, theApp.m_strThisAppName, MB_OK | MB_ICONWARNING);
			}
		}
	}
	*pResult = 0;
}

void CDlgSetCustomScript::OnButtonUp()
{
	int iSelCount = 0;
	int iAfterPos = 0;
	iSelCount = m_List.GetSelectedCount();
	if (iSelCount != 1)
		return;
	iSelCount = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (iSelCount == -1)
		return;

	CString strURL;
	CString strMode;
	CString strEnable;
	strURL = m_List.GetItemText(iSelCount, URL);
	strMode = m_List.GetItemText(iSelCount, FILENAME);
	strEnable = m_List.GetItemText(iSelCount, ENABLE);
	iAfterPos = iSelCount - 1;
	if (iAfterPos < 0)
		return;
	m_List.DeleteItem(iSelCount);
	int iItem = m_List.InsertItem(iAfterPos, _T(""));
	m_List.SetItemText(iItem, URL, strURL);
	m_List.SetItemText(iItem, FILENAME, strMode);
	m_List.SetItemText(iItem, ENABLE, strEnable);
	m_List.SetItemState(iAfterPos, LVIS_SELECTED, LVIS_SELECTED);
	m_List.SetFocus();
}

void CDlgSetCustomScript::OnButtonDown()
{
	int iSelCount = 0;
	int iAfterPos = 0;
	iSelCount = m_List.GetSelectedCount();
	if (iSelCount != 1)
		return;
	iSelCount = m_List.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (iSelCount == -1)
		return;

	CString strURL;
	CString strMode;
	CString strEnable;
	strURL = m_List.GetItemText(iSelCount, URL);
	strMode = m_List.GetItemText(iSelCount, FILENAME);
	strEnable = m_List.GetItemText(iSelCount, ENABLE);
	iAfterPos = iSelCount + 1;
	if (iAfterPos >= m_List.GetItemCount())
		return;
	m_List.DeleteItem(iSelCount);
	int iItem = m_List.InsertItem(iAfterPos, _T(""));
	m_List.SetItemText(iItem, URL, strURL);
	m_List.SetItemText(iItem, FILENAME, strMode);
	m_List.SetItemText(iItem, ENABLE, strEnable);
	m_List.SetItemState(iAfterPos, LVIS_SELECTED, LVIS_SELECTED);
	m_List.SetFocus();
}

void CDlgSetCustomScript::OnSize(UINT nType, int cx, int cy)
{
	m_autoResize.Resize(this);
	CPropertyPage::OnSize(nType, cx, cy);
}
void CDlgSetCustomScript::OnPaint()
{
	CPaintDC dc(this);
}

BOOL CDlgSetCustomScript::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	CString columnLabelURL;
	CString columnLabelFile;
	CString columnLabelEnable;
	columnLabelURL.LoadString(ID_SETTINGS_COLUMN_HEADER_URL);
	columnLabelFile.LoadString(ID_SETTINGS_COLUMN_HEADER_FILE);
	columnLabelEnable.LoadString(ID_SETTINGS_COLUMN_HEADER_ENABLED);
	m_List.InsertColumn(URL, columnLabelURL, LVCFMT_LEFT, 480);
	m_List.InsertColumn(FILENAME, columnLabelFile, LVCFMT_LEFT, 150);
	m_List.InsertColumn(ENABLE, columnLabelEnable, LVCFMT_CENTER, 50);
	ListView_SetExtendedListViewStyle(m_List.m_hWnd, LVS_EX_FULLROWSELECT);

	if (theApp.m_AppSettingsDlgCurrent.IsEnableCustomScript())
		((CButton*)GetDlgItem(IDC_CHECK_ENABLE_CUSTOM_SCRIPT))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_CHECK_ENABLE_CUSTOM_SCRIPT))->SetCheck(0);

	if (theApp.IsShowDevTools())
		GetDlgItem(IDC_SHOW_DEV_TOOLS)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_SHOW_DEV_TOOLS)->EnableWindow(FALSE);
	_wsetlocale(LC_ALL, _T("jpn"));
	CStdioFile in;
	CString strPath;
	strPath = theApp.m_strCustomScriptConfFullPath;
	if (in.Open(strPath, CFile::modeReadWrite | CFile::shareDenyWrite | CFile::modeCreate | CFile::modeNoTruncate))
	{
		CString strTemp;
		CString strTemp2;
		CString strTemp3;
		CStringArray strArray;

		while (in.ReadString(strTemp))
		{
			strTemp2.Empty();
			strTemp3.Empty();
			strArray.RemoveAll();
			strTemp.TrimLeft();
			strTemp.TrimRight();
			if (strTemp.IsEmpty())
				continue;

			BOOL bEnable = TRUE;
			SBUtil::Split(&strArray, strTemp, _T("\t"));
			if (strArray.GetSize() >= 2)
			{
				strTemp2 = strArray.GetAt(0);
				strTemp2.TrimLeft();
				strTemp2.TrimRight();

				strTemp3 = strArray.GetAt(1);
				strTemp3.TrimLeft();
				strTemp3.TrimRight();
				if (strTemp2.Find(_T(";")) == 0)
				{
					bEnable = FALSE;
					strTemp2 = strTemp2.Mid(1);
				}
				else if (strTemp2.Find(_T("#")) == 0)
				{
					bEnable = FALSE;
					strTemp2 = strTemp2.Mid(1);
				}

				if (strTemp2.IsEmpty())
					continue;

				if (strTemp3.IsEmpty())
					continue;

				//ファイル名に使えない文字を置き換える。
				strTemp3 = SBUtil::GetValidFileName(strTemp3);

				int iRet = DuplicateChk(strTemp2, strTemp3);
				//重複なし。
				if (iRet == -1)
				{
					int index = m_List.GetItemCount();
					int iItem = m_List.InsertItem(index, _T(""));
					m_List.SetItemText(iItem, URL, strTemp2);
					m_List.SetItemText(iItem, FILENAME, strTemp3);
					CString strLowString;
					strLowString = bEnable ? _T("o") : _T("-");
					m_List.SetItemText(iItem, ENABLE, strLowString);
				}
			}
		}
		in.Close();
	}
	else
	{
		CString alertMsg;
		alertMsg.LoadString(ID_CANNOT_LOAD_CUSTOM_SCRIPT);
		CString strErrMsg;
		strErrMsg.Format(alertMsg, strPath);
		::MessageBox(this->m_hWnd, strErrMsg, theApp.m_strThisAppName, MB_OK | MB_ICONERROR);
	}
	OnEnableCtrl();
	return TRUE;
}

LRESULT CDlgSetCustomScript::Set_OK(WPARAM wParam, LPARAM lParam)
{
	if (((CButton*)GetDlgItem(IDC_CHECK_ENABLE_CUSTOM_SCRIPT))->GetCheck() == 1)
	{
		theApp.m_AppSettingsDlgCurrent.SetEnableCustomScript(1);
	}
	else
	{
		theApp.m_AppSettingsDlgCurrent.SetEnableCustomScript(0);
	}

	_wsetlocale(LC_ALL, _T("jpn"));
	CStdioFile out;
	CString strPath;
	strPath = theApp.m_strCustomScriptConfFullPath;
	if (out.Open(strPath, CFile::modeWrite | CFile::modeCreate))
	{
		CString strURL;
		CString strFileName;
		CString strEnable;
		CString strLineData;
		BOOL bEnable = FALSE;
		int iSelCount = -1;
		while ((iSelCount = m_List.GetNextItem(iSelCount, LVNI_ALL)) != -1)
		{
			strLineData.Empty();
			strURL.Empty();
			strFileName.Empty();
			strEnable.Empty();
			bEnable = FALSE;
			strURL = m_List.GetItemText(iSelCount, URL);
			strFileName = m_List.GetItemText(iSelCount, FILENAME);
			//ファイル名に使えない文字を置き換える。
			strFileName = SBUtil::GetValidFileName(strFileName);

			int iRet = DuplicateChk(strURL, strFileName);
			//重複なし。
			if (iRet == -1 || iRet == iSelCount)
			{
				strEnable = m_List.GetItemText(iSelCount, ENABLE);
				bEnable = strEnable == _T("o") ? TRUE : FALSE;
				if (bEnable)
					strLineData.Format(_T("%s\t%s\n"), (LPCTSTR)strURL, (LPCTSTR)strFileName);
				else
					strLineData.Format(_T(";%s\t%s\n"), (LPCTSTR)strURL, (LPCTSTR)strFileName);
				out.WriteString(strLineData);
			}
		}
		out.Close();
		if (theApp.InVirtualEnvironment() == VE_THINAPP)
		{
			CString strSBFilePath;
			strSBFilePath = theApp.GetSandboxFilePath(theApp.m_strCustomScriptConfFullPath);
			if (!strSBFilePath.IsEmpty())
			{
				CString strFilePathCnf;
				strFilePathCnf = theApp.m_strCustomScriptConfFullPath;
				strFilePathCnf.Replace(_T("Default.conf"), _T(".conf"));
				theApp.Exec_SB2PYS_COPY(strSBFilePath, strFilePathCnf);
			}
		}
	}
	else
	{
		CString alertMsg;
		alertMsg.LoadString(ID_CANNOT_SAVE_CUSTOM_SCRIPT);
		CString strErrMsg;
		strErrMsg.Format(alertMsg, strPath);
		::MessageBox(this->m_hWnd, strErrMsg, theApp.m_strThisAppName, MB_OK | MB_ICONERROR);
		return 1;
	}
	return 0;
}

void CDlgSetCustomScript::OnBnClickedButton1()
{
	theApp.ShowDebugTraceDlg();
}

void CDlgSetCustomScript::OnBnClickedShowDevTools()
{
	theApp.ShowDevTools();
}

void CDlgSetDSP::OnBnClickedCheckDisableRebar()
{
	BOOL checked = ((CButton*)GetDlgItem(IDC_CHECK_DISABLE_REBAR))->GetCheck();
	GetDlgItem(IDC_CHECK_LOGO)->EnableWindow(checked);
}
