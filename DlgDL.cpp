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
	int iRet = ::MessageBox(this->m_hWnd, _T("ダウンロードをキャンセルしますか？"), theApp.m_strThisAppName, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
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
	int iRet = ::MessageBox(this->m_hWnd, _T("ダウンロードをキャンセルしますか？"), theApp.m_strThisAppName, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
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
	SetDlgItemText(IDC_STATIC_TX, _T("起動中"));
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
			CString strErrMsg;
			if (hRet <= 32)
			{
				switch (hRet)
				{
				case 0:
					strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"), _T("メモリまたはリソースが不足している恐れがあります。"));
					break;
				case ERROR_FILE_NOT_FOUND:
					strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"), _T("指定されたファイルが見つかりませんでした。"));
					break;
				case ERROR_PATH_NOT_FOUND:
					strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"), _T("指定されたパスが見つかりませんでした。"));
					break;
				case ERROR_BAD_FORMAT:
					strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"), _T(".exeファイルが無効です。Win32の.exeではないか、.exeイメージ内にエラーがあります。"));
					break;
				case SE_ERR_ACCESSDENIED:
					strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"), _T("オペレーティングシステムが、指定されたファイルへのアクセスを拒否しました。"));
					break;
				case SE_ERR_ASSOCINCOMPLETE:
					strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"), _T("ファイル名の関連付けが不完全または無効です。"));
					break;
				case SE_ERR_DDEBUSY:
					strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"), _T("ほかのDDEトランザクションが現在処理中なので、DDEトランザクションを完了できませんでした。"));
					break;
				case SE_ERR_DDEFAIL:
					strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"), _T("DDEトランザクションが失敗しました。"));
					break;
				case SE_ERR_DDETIMEOUT:
					strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"), _T("	要求がタイムアウトしたので、DDEトランザクションを完了できませんでした。"));
					break;
				case SE_ERR_DLLNOTFOUND:
					strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"), _T("指定されたダイナミックリンクライブラリ（DLL）が見つかりませんでした。"));
					break;
					//			case SE_ERR_FNF:
					//				strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"),_T("指定されたファイルが見つかりませんでした。"));
					//				break;
				case SE_ERR_NOASSOC:
					strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"), _T("指定されたファイル拡張子に関連付けられたアプリケーションがありません。"));
					break;
				case SE_ERR_OOM:
					strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"), _T("操作を完了するのに十分なメモリがありません。"));
					break;
					//			case SE_ERR_PNF:
					//				strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"),_T("指定されたパスが、見つかりませんでした。"));
					//				break;
				case SE_ERR_SHARE:
					strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"), _T("共有違反が発生しました。"));
					break;
				default:
					strErrMsg.Format(_T("アプリケーションを起動できません。\n%s"), _T("例外が発生しました。"));
					break;
				}
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
