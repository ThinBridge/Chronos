// CDlgCertification.cpp : 実装ファイル
//
#include "stdafx.h"
#include "Sazabi.h"
#include "DlgCertification.h"
#include "afxdialogex.h"

// CDlgCertification ダイアログ

IMPLEMENT_DYNAMIC(CDlgCertification, CDialogEx)

CDlgCertification::CDlgCertification(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DLG_CERTIFICATION, pParent)
{
	m_selectedIndex = 0;
}

CDlgCertification::~CDlgCertification()
{
}

void CDlgCertification::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CERTIFICATION_COMBO, certificationComboBox);
	certificationComboBox.Clear();
	for (CefRefPtr<CefX509Certificate> x509Certificate : m_X509CertificateList)
	{
		CString serialNumber = GetSerialNumberAsHexString(x509Certificate);
		CString displayItemName;
		displayItemName.Format(
		    _T("%s [%s]"),
		    (LPCTSTR)x509Certificate->GetSubject()->GetDisplayName().c_str(),
		    (LPCTSTR)serialNumber);

		certificationComboBox.AddString(displayItemName);
	}
	certificationComboBox.SetCurSel(0);
	OnCbnSelchangeCertificationCombo();
}

CString CDlgCertification::GetSerialNumberAsHexString(CefRefPtr<CefX509Certificate> x509Certificate)
{
	auto serialNumber = x509Certificate->GetSerialNumber();
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
	principalString += principal->GetCommonName().c_str();
	principalString += _T(", O=");
	principal->GetOrganizationNames(values);
	for (size_t i = 0; i < values.size(); i++)
	{
		CefString value = values[i];
		principalString += value.c_str();
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
		principalString += value.c_str();
		if (i > 0)
		{
			principalString += _T(" ");
		}
	}
	principalString += _T(", STREET=");
	principal->GetStreetAddresses(values);
	for (size_t i = 0; i < values.size(); i++)
	{
		CefString value = values[i];
		principalString += value.c_str();
		if (i > 0)
		{
			principalString += _T("-");
		}
	}
	principalString += _T(", L=");
	principalString += principal->GetLocalityName().c_str();
	principalString += _T(", ST=");
	principalString += principal->GetStateOrProvinceName().c_str();
	principalString += _T(", C");
	principalString += principal->GetCountryName().c_str();
	return principalString;
}

int CDlgCertification::GetSelectedIndex()
{
	return certificationComboBox.GetCurSel();
}

BEGIN_MESSAGE_MAP(CDlgCertification, CDialogEx)
	ON_CBN_SELCHANGE(IDC_CERTIFICATION_COMBO, &CDlgCertification::OnCbnSelchangeCertificationCombo)
	ON_BN_CLICKED(IDOK, &CDlgCertification::OnBnClickedOk)
END_MESSAGE_MAP()

// CDlgCertification メッセージ ハンドラー

void CDlgCertification::OnCbnSelchangeCertificationCombo()
{
	int curSel = certificationComboBox.GetCurSel();
	CefRefPtr<CefX509Certificate> x509Certificate = m_X509CertificateList[curSel];

	CString certificationDetail;

	certificationDetail += _T("発行元: ");
	certificationDetail += GetPrincipalString(x509Certificate->GetIssuer());
	certificationDetail += _T("\r\n");
	certificationDetail += _T("発行先: ");
	certificationDetail += GetPrincipalString(x509Certificate->GetSubject());
	certificationDetail += _T("\r\n");
	certificationDetail += _T("シリアル番号: ");
	certificationDetail += GetSerialNumberAsHexString(x509Certificate);

	CefTime validStart;
	cef_time_from_basetime(x509Certificate->GetValidStart(), &validStart);
	CString validStartTimeString = GetTimeString(validStart);
	
	CefTime validExpiry;
	cef_time_from_basetime(x509Certificate->GetValidExpiry(), &validExpiry);
	CString validExpiryString = GetTimeString(validExpiry);

	certificationDetail += _T("\r\n");
	certificationDetail += _T("有効期間: ") + validStartTimeString + _T(" - ") + validExpiryString;

	SetDlgItemText(IDC_CERTIFICATION_EDIT, certificationDetail);
}

void CDlgCertification::OnBnClickedOk()
{
	// DoModalなどで結果を返し終えた後、comboBoxValuesなどは解放されてしまって参照できない。
	// なので、このタイミングでクラス変数に結果を代入しておく。
	int curSel = certificationComboBox.GetCurSel();
	*m_selectedIndex = curSel;
	CDialogEx::OnOK();
}
