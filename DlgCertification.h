#pragma once

// DlgCertification ダイアログ

class DlgCertification : public CDialogEx
{
	DECLARE_DYNAMIC(DlgCertification)
private:
	CString GetSerialNumberAsHexString(CefRefPtr<CefX509Certificate> x509Certificate);
	CString GetTimeString(const CefTime& value);
	CString GetPrincipalString(const CefRefPtr<CefX509CertPrincipal> principal);

protected:
	void OnOk();

public:
	DlgCertification(CWnd* pParent = nullptr);   // 標準コンストラクター
	virtual ~DlgCertification();
	int DlgCertification::GetSelectedIndex();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_CERTIFICATION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	std::vector<CefRefPtr<CefX509Certificate>> m_X509CertificateList;
	afx_msg void OnCbnSelchangeCombo1();
	CComboBox certificationComboBox;
	int* m_selectedIndex;
	afx_msg void OnBnClickedOk();
};
