#pragma once

// DlgCertification ダイアログ

class DlgCertification : public CDialogEx
{
	DECLARE_DYNAMIC(DlgCertification)

public:
	DlgCertification(CWnd* pParent = nullptr);   // 標準コンストラクター
	virtual ~DlgCertification();

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
	CComboBox comboBoxValues;
};
