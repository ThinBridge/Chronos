#pragma once

// CDlgCertification �_�C�A���O

class CDlgCertification : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCertification)

private:
	int m_selectedIndex;

public:
	CDlgCertification(CWnd* pParent = nullptr); // �W���R���X�g���N�^�[
	virtual ~CDlgCertification();
	int SelectedIndex();

	std::vector<CefRefPtr<CefX509Certificate>> m_certificates;
	CefString m_host;

// �_�C�A���O �f�[�^
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
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV �T�|�[�g
	BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeCertificationCombo();
	afx_msg void OnBnClickedOk();

	CComboBox certificationComboBox;
};
