#include "stdafx.h"
#include "Sazabi.h"
#include "DlgDL.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CDlgDL, CDialogEx)
CDlgDL::CDlgDL(CWnd* pParent, UINT nBrowserId)
	: CDialogEx(IDD_DIALOG4, pParent)
{
	m_pParent = pParent;
	m_bDLCancel = FALSE;
	m_bDownloadProgress = FALSE;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bDLComp = FALSE;
	m_nBrowserId = nBrowserId;
	m_iTimerID = 0;
}

CDlgDL::~CDlgDL()
{
}
void CDlgDL::OnDestroy()
{
	if (m_iTimerID)
	{
		this->KillTimer(m_iTimerID);
		m_iTimerID = 0;
	}
	CDialogEx::OnDestroy();
}

void CDlgDL::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_Prog);
	DDX_Control(pDX, IDC_STATIC_FN, m_FileName);
	DDX_Control(pDX, IDC_STATIC_DL_MSG, m_Msg);
	DDX_Control(pDX, IDC_STATIC_TL, m_Tf);
}

BEGIN_MESSAGE_MAP(CDlgDL, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CDlgDL::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgDL::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgDL::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_FO, &CDlgDL::OnBnClickedButtonFo)
	ON_BN_CLICKED(IDC_BUTTON_DIRO, &CDlgDL::OnBnClickedButtonDiro)
END_MESSAGE_MAP()

void CDlgDL::OnBnClickedOk()
{
	//	CDialogEx::OnOK();
}

void CDlgDL::OnTimer(UINT_PTR nIDEvent)
{
	if (m_iTimerID == nIDEvent)
	{
		if (m_iTimerID)
		{
			this->KillTimer(m_iTimerID);
			m_iTimerID = 0;
		}
		CDialog::OnOK();
	}
}
void CDlgDL::OnBnClickedCancel()
{
	//DLが終了している。
	if (m_bDLComp)
	{
		m_bDLCancel = FALSE;
		if (m_iTimerID)
		{
			this->KillTimer(m_iTimerID);
			m_iTimerID = 0;
		}
		CDialogEx::OnCancel();
		return;
	}
	CString confirmMsg;
	confirmMsg.LoadString(ID_CONFIRM_CANCEL_DOWNLOAD);
	int iRet = ::MessageBox(this->m_hWnd, confirmMsg, theApp.m_strThisAppName, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
	if (iRet == IDYES)
	{
		m_bDLCancel = TRUE;
		CDialogEx::OnCancel();
		return;
	}
	m_bDLCancel = FALSE;
}

BOOL CDlgDL::OnInitDialog()
{
	ModifyStyleEx(0, WS_EX_APPWINDOW);
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	if (m_pParent)
	{
		this->CenterWindow((CWnd*)theApp.GetActiveBFramePtr());
	}
	m_Prog.SetRange(0, 100);
	m_Prog.SetStep(1);
	m_bDLCancel = FALSE;
	if (!m_bDLComp)
	{
		GetDlgItem(IDC_BUTTON_FO)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_FO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BUTTON_DIRO)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DIRO)->ShowWindow(SW_HIDE);
	}
	return FALSE;
}

void CDlgDL::OnBnClickedButton1()
{
	if (m_bDLComp)
	{
		CDialogEx::OnCancel();
		if (m_iTimerID)
		{
			this->KillTimer(m_iTimerID);
			m_iTimerID = 0;
		}

		return;
	}
	CString confirmMsg;
	confirmMsg.LoadString(ID_CONFIRM_CANCEL_DOWNLOAD);
	int iRet = ::MessageBox(this->m_hWnd, confirmMsg, theApp.m_strThisAppName, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
	if (iRet == IDYES)
	{
		m_bDLCancel = TRUE;
		return;
	}
	m_bDLCancel = FALSE;
}
IMPLEMENT_DYNAMIC(ProgressDlg, CDialogEx)

ProgressDlg::ProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(ProgressDlg::IDD, pParent)
{
	m_iCntg = 0;
	m_pParent = pParent;
}

ProgressDlg::~ProgressDlg()
{
}

void ProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void ProgressDlg::SetMsg(const CString& str)
{
	SetDlgItemText(IDC_STATIC_MSG, str);
	CString strCnt;
	if (m_iCntg > 999999)
		m_iCntg = 1;
	m_iCntg++;
}

void ProgressDlg::SetMsgExec(const CString& str)
{
	CString startingMsg;
	startingMsg.LoadString(ID_DL_START_APP_IN_PROGRESS);
	SetDlgItemText(IDC_STATIC_TX, startingMsg);
	SetDlgItemText(IDC_STATIC_MSG, str);
	CString strCnt;
	m_iCntg++;
}
BEGIN_MESSAGE_MAP(ProgressDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &ProgressDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &ProgressDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

void ProgressDlg::OnBnClickedOk()
{
	//CDialog::OnOK();
}

void ProgressDlg::OnBnClickedCancel()
{
	CDialog::OnCancel();
}

BOOL ProgressDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (m_pParent)
	{
		this->CenterWindow(m_pParent);
	}
	return TRUE;
}

void CDlgDL::OnBnClickedButtonFo()
{
	if (!theApp.IsSGMode())
	{
		if (::PathFileExists(m_strFileFullPath))
		{
			ULONGLONG hRet = 0;
			SetLastError(NO_ERROR);
			hRet = (ULONGLONG)ShellExecute(this->m_hWnd, NULL, m_strFileFullPath, NULL, m_strFileFolderPath, SW_SHOW);
			CString errMsg;
			if (hRet <= 32)
			{
				switch (hRet)
				{
				case 0:
					errMsg.LoadString(ID_DL_START_APP_FAILED_LOW_RESOURCE);
					break;
				case ERROR_FILE_NOT_FOUND:
					errMsg.LoadString(ID_DL_START_APP_FAILED_FILE_NOT_FOUND);
					break;
				case ERROR_PATH_NOT_FOUND:
					errMsg.LoadString(ID_DL_START_APP_FAILED_PATH_NOT_FOUND);
					break;
				case ERROR_BAD_FORMAT:
					errMsg.LoadString(ID_DL_START_APP_FAILED_BAD_FORMAT);
					break;
				case SE_ERR_ACCESSDENIED:
					errMsg.LoadString(ID_DL_START_APP_FAILED_ACCESSDENIED);
					break;
				case SE_ERR_ASSOCINCOMPLETE:
					errMsg.LoadString(ID_DL_START_APP_FAILED_ASSOCINCOMPLETE);
					break;
				case SE_ERR_DDEBUSY:
					errMsg.LoadString(ID_DL_START_APP_FAILED_DDEBUSY);
					break;
				case SE_ERR_DDEFAIL:
					errMsg.LoadString(ID_DL_START_APP_FAILED_DDEFAIL);
					break;
				case SE_ERR_DDETIMEOUT:
					errMsg.LoadString(ID_DL_START_APP_FAILED_DDETIMEOUT);
					break;
				case SE_ERR_DLLNOTFOUND:
					errMsg.LoadString(ID_DL_START_APP_FAILED_DLLNOTFOUND);
					break;
					//			case SE_ERR_FNF:
					//				errMsg.LoadString(ID_DL_START_APP_FAILED_FILE_NOT_FOUND);
					//				break;
				case SE_ERR_NOASSOC:
					errMsg.LoadString(ID_DL_START_APP_FAILED_NOASSOC);
					break;
				case SE_ERR_OOM:
					errMsg.LoadString(ID_DL_START_APP_FAILED_OOM);
					break;
					//			case SE_ERR_PNF:
					//				errMsg.LoadString(ID_DL_START_APP_FAILED_PATH_NOT_FOUND);
					//				break;
				case SE_ERR_SHARE:
					errMsg.LoadString(ID_DL_START_APP_FAILED_SHARE);
					break;
				default:
					errMsg.LoadString(ID_DL_START_APP_FAILED_DEFAULT);
					break;
				}
				CString errMsgBase;
				errMsgBase.LoadString(ID_DL_START_APP_FAILED_BASE);
				CString strErrMsg;
				strErrMsg.Format(errMsgBase, errMsg);
				strErrMsg += _T("\n");
				strErrMsg += m_strFileFullPath;
				::MessageBox(this->m_hWnd, strErrMsg, theApp.m_pszAppName, MB_OK | MB_ICONWARNING);
			}
			CDialog::OnOK();
		}
		return;
	}
	//B:\のファイルを開く場合は、filerに任せる。
	//filerのルートを取得
	if (PathIsSameRoot(m_strFileFullPath, theApp.m_AppSettings.GetRootPath()))
	{
		if (::PathFileExists(m_strFileFullPath))
		{
			theApp.OpenChFiler(m_strFileFullPath);
			CDialog::OnOK();
		}
	}
}

void CDlgDL::OnBnClickedButtonDiro()
{
	if (!theApp.IsSGMode())
	{
		//[フォルダーを開く]の場合
		if (::PathFileExists(m_strFileFolderPath) && !::PathIsDirectory(m_strFileFolderPath))
		{
			// 指定されたパスにファイルが存在、かつディレクトリでない
			;
		}
		else if (::PathFileExists(m_strFileFolderPath))
		{
			ShellExecute(this->m_hWnd, NULL, m_strFileFolderPath, NULL, NULL, SW_SHOW);
			CDialog::OnOK();
		}
		return;
	}
	//B:\のファイルを開く場合は、filerに任せる。
	//filerのルートを取得
	if (PathIsSameRoot(m_strFileFolderPath, theApp.m_AppSettings.GetRootPath()))
	{
		//[フォルダーを開く]の場合
		if (::PathFileExists(m_strFileFolderPath) && !::PathIsDirectory(m_strFileFolderPath))
		{
			// 指定されたパスにファイルが存在、かつディレクトリでない
			;
		}
		else if (::PathFileExists(m_strFileFolderPath))
		{
			theApp.OpenChFiler(m_strFileFolderPath);
			CDialog::OnOK();
		}
	}
}
