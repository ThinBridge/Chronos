#pragma once

// CDlgCertification ダイアログ

class CDlgCertification : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCertification)
private:
	CString GetSerialNumberAsHexString(CefRefPtr<CefX509Certificate> x509Certificate);
	CString GetTimeString(const CefTime& value);
	CString GetPrincipalString(const CefRefPtr<CefX509CertPrincipal> principal);

protected:
	void OnOk();

public:
	CDlgCertification(CWnd* pParent = nullptr);   // 標準コンストラクター
	virtual ~CDlgCertification();
	int CDlgCertification::GetSelectedIndex();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_CERTIFICATION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	std::vector<CefRefPtr<CefX509Certificate>> m_X509CertificateList;
	afx_msg void OnCbnSelchangeCertificationCombo();
	CComboBox certificationComboBox;
	int* m_selectedIndex;
	afx_msg void OnBnClickedOk();
};
