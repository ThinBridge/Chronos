#pragma once


// CertificationDialog ダイアログ

class CertificationDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CertificationDialog)

public:
	CertificationDialog(CWnd* pParent = nullptr);   // 標準コンストラクター
	virtual ~CertificationDialog();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_CERTIFICATION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
};
