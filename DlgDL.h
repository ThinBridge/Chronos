#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "sbcommon.h"

class CDlgDL : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgDL)

public:
	CDlgDL(CWnd* pParent, UINT nBrowserId);
	virtual ~CDlgDL();
	CWnd* m_pParent;

#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_DIALOG4
	};
#endif
	CIconHelper m_hIcon;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	BOOL m_bDLComp;
	DECLARE_MESSAGE_MAP()
public:
	UINT m_nBrowserId;
	void SetCompST(BOOL bComp, LPCTSTR strFileFullPath)
	{
		m_bDLComp = bComp;
		if (m_bDLComp)
		{
			m_strFileFullPath = strFileFullPath;
			TCHAR szFolder[MAX_PATH * 2 + 1] = {0};
			lstrcpyn(szFolder, strFileFullPath, MAX_PATH * 2);
			PathRemoveFileSpec(szFolder);
			m_strFileFolderPath = szFolder;
			m_FileName.SetWindowText(strFileFullPath);
			m_Prog.SetPos(100);
			m_Tf.SetWindowText(_T(""));
			GetDlgItem(IDC_BUTTON_FO)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_FO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_BUTTON_DIRO)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_DIRO)->ShowWindow(SW_SHOW);
			CString closeButtonLabel;
			closeButtonLabel.LoadString(ID_DOWNLOAD_COMPLETE_DIALOG_CLOSE);
			GetDlgItem(IDC_BUTTON1)->SetWindowText(closeButtonLabel);
			CString windowTitle;
			windowTitle.LoadString(ID_DOWNLOAD_COMPLETE_DIALOG_TITLE);
			this->SetWindowText(windowTitle);
			m_Msg.SetWindowText(windowTitle);
			if (!this->IsWindowVisible())
			{
				this->ShowWindow(SW_SHOW);
			}
			::SetWindowPos(this->m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			if (!m_iTimerID)
			{
				m_iTimerID = (INT_PTR)this;
				this->SetTimer(m_iTimerID, 30 * 1000, 0);
			}
		}
	}
	BOOL GetCompST() { return m_bDLComp; }
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	INT_PTR m_iTimerID;
	CProgressCtrl m_Prog;
	CStatic m_FileName;
	CStatic m_Msg;
	CStatic m_Tf;
	virtual BOOL OnInitDialog();
	BOOL m_bDownloadProgress;
	afx_msg void OnBnClickedButton1();
	BOOL m_bDLCancel;
	afx_msg void OnBnClickedButtonFo();
	afx_msg void OnBnClickedButtonDiro();
	CString m_strFileFullPath;
	CString m_strFileFolderPath;
	afx_msg void OnDestroy();
};

class ProgressDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ProgressDlg)
public:
	CWnd* m_pParent;
	ProgressDlg(CWnd* pParent = NULL);
	virtual ~ProgressDlg();
	void SetMsg(const CString& str);
	void SetMsgExec(const CString& str);
	UINT m_iCntg;
	enum
	{
		IDD = IDD_DIALOG1
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
};
