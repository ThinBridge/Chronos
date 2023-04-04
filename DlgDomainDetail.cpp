#include "stdafx.h"
#include "Sazabi.h"
#include "DlgDomainDetail.h"
#include "afxdialogex.h"
IMPLEMENT_DYNAMIC(CDlgDomainDetail, CDialogEx)
CDlgDomainDetail::CDlgDomainDetail(CWnd* pParent /*=NULL*/)
    : CDialogEx(IDD_DIALOG5, pParent)
{
}

CDlgDomainDetail::~CDlgDomainDetail()
{
}

void CDlgDomainDetail::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgDomainDetail, CDialogEx)
ON_BN_CLICKED(IDOK, &CDlgDomainDetail::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &CDlgDomainDetail::OnBnClickedCancel)
ON_BN_CLICKED(IDC_CHECK1, &CDlgDomainDetail::OnBnClickedCheck1)
END_MESSAGE_MAP()

void CDlgDomainDetail::OnBnClickedOk()
{
	GetDlgItemText(IDC_EDIT1, m_strDomainName);
	m_strDomainName.TrimLeft();
	m_strDomainName.TrimRight();
	if (((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck())
		m_bEnable = TRUE;
	else
		m_bEnable = FALSE;

	if (((CButton*)GetDlgItem(IDC_RADIO1))->GetCheck())
		m_ActionType = TF_ALLOW;
	else
		m_ActionType = TF_DENY;

	if (m_strDomainName.IsEmpty() && m_bEnable)
	{
		CString alertMsg;
		alertMsg.LoadString(ID_ALERT_EMPTY_DOMAIN);
		::MessageBox(this->m_hWnd, alertMsg, theApp.m_strThisAppName, MB_OK | MB_ICONWARNING);
		((CButton*)GetDlgItem(IDC_EDIT1))->SetFocus();
		return;
	}
	CDialogEx::OnOK();
}

void CDlgDomainDetail::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}

void CDlgDomainDetail::OnBnClickedCheck1()
{
	SetState();
}

void CDlgDomainDetail::SetState()
{
	BOOL bFlg = FALSE;
	if (((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck() == 1)
		bFlg = TRUE;
	else
		bFlg = FALSE;
	GetDlgItem(IDC_EDIT1)->EnableWindow(bFlg);
	GetDlgItem(IDC_RADIO1)->EnableWindow(bFlg);
	GetDlgItem(IDC_RADIO2)->EnableWindow(bFlg);
}
BOOL CDlgDomainDetail::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetDlgItemText(IDC_EDIT1, m_strDomainName);
	if (m_bEnable)
	{
		((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(1);
		GetDlgItem(IDC_EDIT1)->SetFocus();
	}
	else
		((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(0);

	if (m_ActionType == TF_ALLOW)
		((CButton*)GetDlgItem(IDC_RADIO1))->SetCheck(1);
	else
		((CButton*)GetDlgItem(IDC_RADIO2))->SetCheck(1);
	SetState();
	return FALSE;
}
////////////////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CDlgCustomScriptDetail, CDialogEx)
CDlgCustomScriptDetail::CDlgCustomScriptDetail(CWnd* pParent /*=NULL*/)
    : CDialogEx(IDD_DIALOG6, pParent)
{
}

CDlgCustomScriptDetail::~CDlgCustomScriptDetail()
{
}

void CDlgCustomScriptDetail::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgCustomScriptDetail, CDialogEx)
ON_BN_CLICKED(IDOK, &CDlgCustomScriptDetail::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &CDlgCustomScriptDetail::OnBnClickedCancel)
ON_BN_CLICKED(IDC_CHECK1, &CDlgCustomScriptDetail::OnBnClickedCheck1)
ON_BN_CLICKED(IDC_BUTTON1, &CDlgCustomScriptDetail::OnBnClickedButton1)
END_MESSAGE_MAP()

void CDlgCustomScriptDetail::OnBnClickedOk()
{
	GetDlgItemText(IDC_EDIT1, m_strURL);
	GetDlgItemText(IDC_EDIT2, m_strFileName);
	if (((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck())
		m_bEnable = TRUE;
	else
		m_bEnable = FALSE;

	m_strURL.TrimLeft();
	m_strURL.TrimRight();
	m_strFileName.TrimLeft();
	m_strFileName.TrimRight();

	if (m_strURL.IsEmpty() && m_bEnable)
	{
		CString alertMsg;
		alertMsg.LoadString(ID_ALERT_EMPTY_URL);
		::MessageBox(this->m_hWnd, alertMsg, theApp.m_strThisAppName, MB_OK | MB_ICONWARNING);
		((CButton*)GetDlgItem(IDC_EDIT1))->SetFocus();
		return;
	}
	if (m_strFileName.IsEmpty() && m_bEnable)
	{
		CString alertMsg;
		alertMsg.LoadString(ID_ALERT_EMPTY_FILENAME);
		::MessageBox(this->m_hWnd, alertMsg, theApp.m_strThisAppName, MB_OK | MB_ICONWARNING);
		((CButton*)GetDlgItem(IDC_EDIT2))->SetFocus();
		return;
	}
	else
	{
		//ファイル名に使えない文字を置き換える。
		m_strFileName = SBUtil::GetValidFileName(m_strFileName);
	}
	CDialogEx::OnOK();
}

void CDlgCustomScriptDetail::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}

void CDlgCustomScriptDetail::OnBnClickedCheck1()
{
	SetState();
}
void CDlgCustomScriptDetail::OnBnClickedButton1()
{
	CString szFilter;
	szFilter.LoadString(ID_FILE_TYPE_JS);
	CFileDialog fileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter, this);
	CString strTitle;
	strTitle.LoadString(ID_OPEN_FILE_CHOOSER_TITLE);
	fileDlg.m_ofn.lpstrTitle = strTitle.GetString();
	fileDlg.m_ofn.lpstrInitialDir = theApp.m_strExeFolderPath.GetString();
	if (fileDlg.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_EDIT2, fileDlg.GetFileName());
	}
}

void CDlgCustomScriptDetail::SetState()
{
	BOOL bFlg = FALSE;
	if (((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck() == 1)
		bFlg = TRUE;
	else
		bFlg = FALSE;
	GetDlgItem(IDC_EDIT1)->EnableWindow(bFlg);
	GetDlgItem(IDC_EDIT2)->EnableWindow(bFlg);
	GetDlgItem(IDC_BUTTON1)->EnableWindow(bFlg);
}
BOOL CDlgCustomScriptDetail::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetDlgItemText(IDC_EDIT1, m_strURL);
	SetDlgItemText(IDC_EDIT2, m_strFileName);
	if (m_bEnable)
	{
		((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(1);
		GetDlgItem(IDC_EDIT1)->SetFocus();
	}
	else
		((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(0);

	SetState();
	return FALSE;
}
