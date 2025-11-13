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
		IDD = IDD_DLG_DOWNLOAD
	};
#endif
	CIconHelper m_hIcon;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	BOOL m_bDLComp;
	DECLARE_MESSAGE_MAP()
public:
	UINT m_nBrowserId;
	BOOL GetCompST() { return m_bDLComp; }
	void SetCompST(BOOL bComp, LPCTSTR strFileFullPath);
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
	afx_msg void OnBnClickedButtonFileOpen();
	afx_msg void OnBnClickedButtonDirectoryOpen();
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
	UINT m_iCnt;
#pragma warning(push)
// C26812 列挙型 'type-name' はスコープ外です。 'enum' より 'enum class' を優先します (Enum.3)
// これはダイアログを自動生成した際に作成されるenumなので、enum classにせず、警告の方を無視する。
#pragma warning(disable : 26812)
	enum
	{
		IDD = IDD_DLG_PROGRESS
	};
#pragma warning(pop)

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
};
