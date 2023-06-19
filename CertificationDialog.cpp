// CertificationDialog.cpp : 実装ファイル
//

#include "pch.h"
#include "Sazabi.h"
#include "CertificationDialog.h"
#include "afxdialogex.h"


// CertificationDialog ダイアログ

IMPLEMENT_DYNAMIC(CertificationDialog, CDialogEx)

CertificationDialog::CertificationDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_CERTIFICATION, pParent)
{

}

CertificationDialog::~CertificationDialog()
{
}

void CertificationDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CertificationDialog, CDialogEx)
END_MESSAGE_MAP()


// CertificationDialog メッセージ ハンドラー
