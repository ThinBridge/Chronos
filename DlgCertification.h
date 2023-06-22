#pragma once

// CDlgCertification ダイアログ

class CDlgCertification : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCertification)

private:
	int m_selectedIndex;

public:
	CDlgCertification(CWnd* pParent = nullptr); // 標準コンストラクター
	virtual ~CDlgCertification();

	CComboBox certificationComboBox;
	CefString m_host;

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_DLG_CERTIFICATION
	};
#endif

private:
	CString GetSerialNumberAsHexString(const CefRefPtr<CefX509Certificate> certificate);
	CString GetTimeString(const CefTime& value);
	CString GetPrincipalString(const CefRefPtr<CefX509CertPrincipal> principal);

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV サポート
	BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	std::vector<CefRefPtr<CefX509Certificate>> m_certificates;
	afx_msg void OnCbnSelchangeCertificationCombo();
	afx_msg void OnBnClickedOk();
	int SelectedIndex();
};
