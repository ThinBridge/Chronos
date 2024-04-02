#pragma once
#include "afxwin.h"
#include "Sazabi.h"
#include "afxdialogex.h"
class DebugWndLogData
{
public:
	DebugWndLogData()
	{
		CTime timeDlg = CTime::GetCurrentTime();
		CString strTemp(timeDlg.Format(_T("%X")));
		mDATE_TIME = strTemp;
	}
	CString GetString(BOOL bAppendTime = FALSE)
	{
		CString strMSG;
		if (bAppendTime)
		{
			strMSG = mDATE_TIME;
		}
		if (!mHWND.IsEmpty())
		{
			if (bAppendTime)
				strMSG += _T("\t");
			strMSG += mHWND;
		}
		if (!mFUNCTION_NAME.IsEmpty())
		{
			strMSG += _T("\t");
			strMSG += mFUNCTION_NAME;
		}
		if (!mMESSAGE1.IsEmpty())
		{
			strMSG += _T("\t");
			strMSG += mMESSAGE1;
		}
		if (!mMESSAGE2.IsEmpty())
		{
			strMSG += _T("\t");
			strMSG += mMESSAGE2;
		}
		if (!mMESSAGE3.IsEmpty())
		{
			strMSG += _T("\t");
			strMSG += mMESSAGE3;
		}
		if (!mMESSAGE4.IsEmpty())
		{
			strMSG += _T("\t");
			strMSG += mMESSAGE4;
		}
		if (!mMESSAGE5.IsEmpty())
		{
			strMSG += _T("\t");
			strMSG += mMESSAGE5;
		}
		if (!mMESSAGE6.IsEmpty())
		{
			strMSG += _T("\t");
			strMSG += mMESSAGE6;
		}
		return strMSG;
	}
	virtual ~DebugWndLogData() {}
	CString mDATE_TIME;
	CString mHWND;
	CString mFUNCTION_NAME;
	CString mMESSAGE1;
	CString mMESSAGE2;
	CString mMESSAGE3;
	CString mMESSAGE4;
	CString mMESSAGE5;
	CString mMESSAGE6;
};

// 項目のデータクラス
class OWNERDATA
{
public:
	OWNERDATA() { iIndex = 0; }
	~OWNERDATA() {}
	INT_PTR iIndex;
	CString mDATE_TIME;
	CString mHWND;
	CString mFUNCTION_NAME;
	CString mMESSAGE1;
	CString mMESSAGE2;
	CString mMESSAGE3;
	CString mMESSAGE4;
	CString mMESSAGE5;
	CString mMESSAGE6;
};

#pragma warning(push)
// C26812 列挙型 'type-name' はスコープ外です。 'enum' より 'enum class' を優先します (Enum.3)
// これはリスト用に自動生成されたenumなのか、独自定義したものなのか不明。
// そのため、警告の抑止により対応する。
#pragma warning(disable : 26812)
enum
{
	LIST_INDEX,
	LIST_DATE_TIME,
	LIST_HWND,
	LIST_FUNCTION_NAME,
	LIST_MESSAGE1,
	LIST_MESSAGE2,
	LIST_MESSAGE3,
	LIST_MESSAGE4,
	LIST_MESSAGE5,
	LIST_MESSAGE6,
	LIST_MAX,
};
#pragma warning(pop)

class CDlgDebugWnd : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgDebugWnd)

public:
	CDlgDebugWnd(CWnd* pParent = NULL); // 標準コンストラクター
	virtual ~CDlgDebugWnd();
	autoresize::CAutoResize m_autoResize;
	HANDLE m_hEventLogDebugWnd;
	CString m_strEventLogDebugWnd;

	// ダイアログ データ
#pragma warning(push)
// C26812 列挙型 'type-name' はスコープ外です。 'enum' より 'enum class' を優先します (Enum.3)
// これはダイアログを自動生成した際に作成されるenumなので、enum classにせず、警告の方を無視する。
#pragma warning(disable : 26812)
	enum
	{
		IDD = IDD_DLG_DEBUG_WND
	};
#pragma warning(pop)

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV サポート
	// リストビューのオーナーデータ
	CTypedPtrArray<CPtrArray, OWNERDATA*> m_pOwnerData;
	virtual void CreateListView();
	void ClearData();
	CString m_strTemp;
	BOOL m_bAutoScFlg;
	void DWToString(INT_PTR in, CString& strNumber)
	{
		strNumber.Empty();
		strNumber.Format(_T("%d"), in);
		return;
	}
	DECLARE_MESSAGE_MAP()
public:
	void SetLogMsg(const DebugWndLogData& Data)
	{
		SetLogMsg(Data.mDATE_TIME,
			  Data.mHWND,
			  Data.mFUNCTION_NAME,
			  Data.mMESSAGE1,
			  Data.mMESSAGE2,
			  Data.mMESSAGE3,
			  Data.mMESSAGE4,
			  Data.mMESSAGE5,
			  Data.mMESSAGE6);
	}
	void SetLogMsg(LPCTSTR pDATE_TIME,
		       LPCTSTR pHWND,
		       LPCTSTR pFUNCTION_NAME,
		       LPCTSTR pMESSAGE1,
		       LPCTSTR pMESSAGE2,
		       LPCTSTR pMESSAGE3,
		       LPCTSTR pMESSAGE4,
		       LPCTSTR pMESSAGE5,
		       LPCTSTR pMESSAGE6);

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnPaint();

	afx_msg void OnBnClickedButton1();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual afx_msg void OnGetdispinfoList1(NMHDR* pNMHDR, LRESULT* pResult);
	CListCtrl m_List;
	afx_msg void OnBnClickedCheck1();
};

class CDlgSCEditor : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSCEditor)

public:
	CDlgSCEditor(CWnd* pParent = NULL); // 標準コンストラクター
	virtual ~CDlgSCEditor();
	autoresize::CAutoResize m_autoResize;
	CString m_strFilePath;
	// ダイアログ データ
#pragma warning(push)
// C26812 列挙型 'type-name' はスコープ外です。 'enum' より 'enum class' を優先します (Enum.3)
// これはダイアログを自動生成した際に作成されるenumなので、enum classにせず、警告の方を無視する。
#pragma warning(disable : 26812)
	enum
	{
		IDD = IDD_DLG_SC_EDITOR
	};
#pragma warning(pop)

protected:
	BOOL bFirstFlg;
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV サポート
	void ReadText();
	void WriteText();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CEdit m_Edit;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonFunc();
	afx_msg void OnBnClickedButtonTop();
	afx_msg void OnBnClickedButtonUrl();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonTraceLog();
	afx_msg void OnBnClickedButton1();

	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnPaint();
	int m_SelPosS;
	int m_SelPosE;
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
