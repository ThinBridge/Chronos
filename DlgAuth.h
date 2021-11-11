#pragma once
class CDlgAuth : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgAuth)

public:
	CDlgAuth(CWnd* pParent = NULL);
	virtual ~CDlgAuth();
	CString m_strCaption;
	CString m_strID;
	CString m_strPW;
	CString m_strMsgTxt;
	CWnd* m_pWnd;
	HWND m_hStopWnd;
	WNDCLASSEX m_wndClass;

#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_DLG_AUTH
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	int RunModalLoop(DWORD dwFlags = 0);
	virtual INT_PTR DoModal();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
};

class CDlgAuthPWC : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgAuthPWC)

public:
	CDlgAuthPWC(CWnd* pParent = NULL);
	virtual ~CDlgAuthPWC();
	CString m_strCaption;
	CString m_strID;
	CString m_strPW_Current;
	CString m_strPW_NEW;
	CString m_strPW_NEW2;

	CString m_strMsgTxt;
	CWnd* m_pWnd;
	HWND m_hStopWnd;
	WNDCLASSEX m_wndClass;

#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD_DLG_AUTH_PWC
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	virtual INT_PTR DoModal();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
};
