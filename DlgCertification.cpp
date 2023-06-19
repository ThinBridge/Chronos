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
	comboBoxValues.Clear();
	for (CefRefPtr<CefX509Certificate> x509Certificate : m_X509CertificateList)
	{
		CString item = x509Certificate->GetSubject()->GetDisplayName().c_str();
		comboBoxValues.AddString(item);
	}
	DDX_Control(pDX, IDC_COMBO1, comboBoxValues);
}


BEGIN_MESSAGE_MAP(DlgCertification, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO1, &DlgCertification::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// DlgCertification メッセージ ハンドラー

void DlgCertification::OnCbnSelchangeCombo1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
}
