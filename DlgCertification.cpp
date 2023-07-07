// CDlgCertification.cpp : 実装ファイル
//
#include "stdafx.h"
#include "Sazabi.h"
#include "DlgCertification.h"
#include "afxdialogex.h"

// CDlgCertification ダイアログ

IMPLEMENT_DYNAMIC(CDlgCertification, CDialogEx)

CDlgCertification::CDlgCertification(CWnd* pParent /*=nullptr*/)
    : CDlgCertification(CefString(""),
			std::vector<CefRefPtr<CefX509Certificate>>(),
			pParent)
{
}

CDlgCertification::CDlgCertification(CefString host,
				     std::vector<CefRefPtr<CefX509Certificate>> certificates,
				     CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DLG_CERTIFICATION, pParent)
{
	m_selectedIndex = 0;
	m_host = host;
	m_certificates = certificates;
}

CDlgCertification::~CDlgCertification()
{
}

void CDlgCertification::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CERTIFICATION_COMBO, certificationComboBox);
}

CString CDlgCertification::GetSerialNumberAsHexString(const CefRefPtr<CefX509Certificate> certificate)
{
	auto serialNumber = certificate->GetSerialNumber();
	auto size = serialNumber->GetSize();

	CString serialNumberAsHexString;
	for (size_t i = 0; i < size; i++)
	{
		short buf = 0;
		int gotSize = serialNumber->GetData(&buf, 1, i);
		if (gotSize == 0)
			break;

		CString hex;
		hex.Format(_T("%02X"), buf);

		if (i > 0)
		{
			serialNumberAsHexString += ":";
		}
		serialNumberAsHexString += hex;
	}
	return serialNumberAsHexString;
}

CString CDlgCertification::GetTimeString(const CefTime& value)
{
	if (value.GetTimeT() == 0)
		return "*";
	if (value.month < 1 || value.month > 12)
		return "*";

	CString timeString;
	// CefTime is UTC.
	timeString.Format(_T("%d-%02d-%02dT%02d:%02d:%02dZ"),
			  value.year,
			  value.month,
			  value.day_of_month,
			  value.hour,
			  value.minute,
			  value.second);
	return timeString;
}

CString CDlgCertification::GetPrincipalString(const CefRefPtr<CefX509CertPrincipal> principal)
{
	CString principalString;
	std::vector<CefString> values;
	principalString += _T("CN=");
	principalString += (LPCWSTR)principal->GetCommonName().c_str();
	principalString += _T(", O=");
	principal->GetOrganizationNames(values);
	for (size_t i = 0; i < values.size(); i++)
	{
		CefString value = values[i];
		principalString += (LPCWSTR)value.c_str();
		if (i > 0)
		{
			principalString += _T(" ");
		}
	}
	principalString += _T(", OU=");
	principal->GetOrganizationUnitNames(values);
	for (size_t i = 0; i < values.size(); i++)
	{
		CefString value = values[i];
		principalString += (LPCWSTR)value.c_str();
		if (i > 0)
		{
			principalString += _T(" ");
		}
	}
#if CHROME_VERSION_MAJOR < 115
	principalString += _T(", STREET=");
	principal->GetStreetAddresses(values);
	for (size_t i = 0; i < values.size(); i++)
	{
		CefString value = values[i];
		principalString += (LPCWSTR)value.c_str();
		if (i > 0)
		{
			principalString += _T("-");
		}
	}
#endif
	principalString += _T(", L=");
	principalString += (LPCWSTR)principal->GetLocalityName().c_str();
	principalString += _T(", ST=");
	principalString += (LPCWSTR)principal->GetStateOrProvinceName().c_str();
	principalString += _T(", C=");
	principalString += (LPCWSTR)principal->GetCountryName().c_str();
	return principalString;
}

int CDlgCertification::SelectedIndex()
{
	return m_selectedIndex;
}

BEGIN_MESSAGE_MAP(CDlgCertification, CDialogEx)
	ON_CBN_SELCHANGE(IDC_CERTIFICATION_COMBO, &CDlgCertification::OnCbnSelchangeCertificationCombo)
	ON_BN_CLICKED(IDOK, &CDlgCertification::OnBnClickedOk)
END_MESSAGE_MAP()

// CDlgCertification メッセージ ハンドラー

void CDlgCertification::OnCbnSelchangeCertificationCombo()
{
	int curSel = certificationComboBox.GetCurSel();
	CefRefPtr<CefX509Certificate> certificate = m_certificates[curSel];

	CString certificationDetail;
	CString title;

	title.LoadString(ID_CERTIFICATION_ISSUER);
	certificationDetail += title;
	certificationDetail += GetPrincipalString(certificate->GetIssuer());
	certificationDetail += _T("\r\n");
	title.LoadString(ID_CERTIFICATION_SUBJECT);
	certificationDetail += title;
	certificationDetail += GetPrincipalString(certificate->GetSubject());
	certificationDetail += _T("\r\n");
	title.LoadString(ID_CERTIFICATION_SERIAL_NUMBER);
	certificationDetail += title;
	certificationDetail += GetSerialNumberAsHexString(certificate);

	CefTime validStart;
	cef_time_from_basetime(certificate->GetValidStart(), &validStart);
	CString validStartTimeString = GetTimeString(validStart);

	CefTime validExpiry;
	cef_time_from_basetime(certificate->GetValidExpiry(), &validExpiry);
	CString validExpiryString = GetTimeString(validExpiry);

	certificationDetail += _T("\r\n");
	title.LoadString(ID_CERTIFICATION_VALID_PERIOD);
	certificationDetail += title + validStartTimeString + _T(" - ") + validExpiryString;

	SetDlgItemText(IDC_CERTIFICATION_EDIT, certificationDetail);
}

void CDlgCertification::OnBnClickedOk()
{
	// DoModalなどで結果を返し終えた後、certificationComboBoxなどが解放されてしまって参照できない。
	// なので、このタイミングでクラス変数に結果を代入しておく。
	m_selectedIndex = certificationComboBox.GetCurSel();
	CDialogEx::OnOK();
}

BOOL CDlgCertification::OnInitDialog()
{
	BOOL superResult = CDialogEx::OnInitDialog();
	SetDlgItemText(IDC_CERTIFICATE_STATIC_SITE_INFO, (LPCWSTR)m_host.c_str());

	for (CefRefPtr<CefX509Certificate> certificate : m_certificates)
	{
		CString serialNumber = GetSerialNumberAsHexString(certificate);
		CString displayItemName;
		displayItemName.Format(
		    _T("%s [%s]"),
		    (LPCWSTR)certificate->GetSubject()->GetDisplayName().c_str(),
		    (LPCTSTR)serialNumber);

		certificationComboBox.AddString(displayItemName);
	}
	certificationComboBox.SetCurSel(0);
	OnCbnSelchangeCertificationCombo();
	return superResult;
}