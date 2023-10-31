#pragma once
class CDlgDomainDetail : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgDomainDetail)

public:
	CDlgDomainDetail(CWnd* pParent = NULL);
	virtual ~CDlgDomainDetail();
	CString m_strDomainName;
	BOOL m_bEnable;
	UINT m_ActionType;
	void SetState();
// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_DLG_DOMAIN
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedCheck1();
	virtual BOOL OnInitDialog();
};
class CDlgCustomScriptDetail : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCustomScriptDetail)

public:
	CDlgCustomScriptDetail(CWnd* pParent = NULL);
	virtual ~CDlgCustomScriptDetail();
	CString m_strURL;
	CString m_strFileName;
	BOOL m_bEnable;
	void SetState();
	// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_DLG_CUSTOMSCRIPT
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedButton1();
	virtual BOOL OnInitDialog();
};
