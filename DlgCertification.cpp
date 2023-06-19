// DlgCertification.cpp : 実装ファイル
//
#include "stdafx.h"
#include "Sazabi.h"
#include "DlgCertification.h"
#include "afxdialogex.h"


// DlgCertification ダイアログ

IMPLEMENT_DYNAMIC(DlgCertification, CDialogEx)

DlgCertification::DlgCertification(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_CERTIFICATION, pParent)
{

}

DlgCertification::~DlgCertification()
{
}

void DlgCertification::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, comboBoxValues);
	comboBoxValues.Clear();
	for (CefRefPtr<CefX509Certificate> x509Certificate : m_X509CertificateList)
	{
		CString serialNumber = GetSerialNumberAsHexString(x509Certificate);
		CString displayItemName;
		displayItemName.Format(
			_T("%s [%s]"), 
			(LPCTSTR)x509Certificate->GetSubject()->GetDisplayName().c_str(),
			(LPCTSTR)serialNumber);

		comboBoxValues.AddString(displayItemName);
	}
	comboBoxValues.SetCurSel(0);
}

CString DlgCertification::GetSerialNumberAsHexString(CefRefPtr<CefX509Certificate> x509Certificate)
{
	auto serialNumber = x509Certificate->GetSerialNumber();
	auto size = serialNumber->GetSize();

	CString serialNumberAsHexString;
	for (int i = 0; i < size; i++)
	{
		short buf = 0;
		int gotSize = serialNumber->GetData(&buf, 1, i);
		if (gotSize == 0)
			break;

		CString hex;
		hex.Format(_T("%02X"), buf);

		if (i > 0)
			serialNumberAsHexString += ":";
		serialNumberAsHexString += hex;
	}
	return serialNumberAsHexString;
}

BEGIN_MESSAGE_MAP(DlgCertification, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO1, &DlgCertification::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// DlgCertification メッセージ ハンドラー

void DlgCertification::OnCbnSelchangeCombo1()
{
	int curSel = comboBoxValues.GetCurSel();
	CefRefPtr<CefX509Certificate> x509Certificate = m_X509CertificateList[curSel];

	CString certificationDetail;
	std::vector<CefString> values;
	certificationDetail += x509Certificate->GetSubject()->GetCommonName().c_str();
	certificationDetail += _T("\n");
	certificationDetail += x509Certificate->GetSubject()->GetCountryName().c_str();
	certificationDetail += _T("\n");
	x509Certificate->GetSubject()->GetDomainComponents(values);
	for (int i = 0; i < values.size(); i++)
	{
		CefString value = values[i];
		certificationDetail += value.c_str();
		if (i > 0)
		{
			certificationDetail += _T(",");
		}
	}
	certificationDetail += _T("\n");
	certificationDetail += x509Certificate->GetSubject()->GetLocalityName().c_str();
	certificationDetail += _T("\n");
	x509Certificate->GetSubject()->GetOrganizationNames(values);
	for (int i = 0; i < values.size(); i++)
	{
		CefString value = values[i];
		certificationDetail += value.c_str();
		if (i > 0)
		{
			certificationDetail += _T(",");
		}
	}
	certificationDetail += _T("\n");
	x509Certificate->GetSubject()->GetOrganizationUnitNames(values);
	for (int i = 0; i < values.size(); i++)
	{
		CefString value = values[i];
		certificationDetail += value.c_str();
		if (i > 0)
		{
			certificationDetail += _T(",");
		}
	}
	certificationDetail += _T("\n");
	certificationDetail += x509Certificate->GetSubject()->GetStateOrProvinceName().c_str();
	certificationDetail += _T("\n");
	x509Certificate->GetSubject()->GetStreetAddresses(values);
	for (int i = 0; i < values.size(); i++)
	{
		CefString value = values[i];
		certificationDetail += value.c_str();
		if (i > 0)
		{
			certificationDetail += _T(",");
		}
	}
	SetDlgItemText(IDC_EDIT1, certificationDetail);
}
