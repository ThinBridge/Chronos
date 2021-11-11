#pragma once
#include "resource.h"
#define ID_MYCOMBO_SELENDOK 33800
/////////////////////////////////////////////////////////////////////////////
// CIconWnd window

class CIconWnd : public CStatic
{
	DECLARE_DYNCREATE(CIconWnd)

	// Construction/destruction
public:
	CIconWnd();
	virtual ~CIconWnd();

	// Operations
public:
	void SetIcon(HICON icon);

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

private:
	// Attributes
	CIconHelper m_icon;
};

/////////////////////////////////////////////////////////////////////////////
// CIconEdit window

class CIconEdit : public CEdit
{
	DECLARE_DYNCREATE(CIconEdit)
	// Construction/destruction
public:
	CIconEdit();
	virtual ~CIconEdit();
	CWnd* m_pParentWnd;
	BOOL m_bFontCreated;
	CFont m_fontUI;

	// Operations
public:
	void SetIcon(HICON icon);
	void SetIcon(UINT iconres);

protected:
	virtual void PreSubclassWindow();

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam); // Maps to WM_SETFONT
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

private:
	void Prepare();
	void CreateIcon();

	// Attributes
	CIconWnd m_icon;
	CIconHelper m_internalIcon;
};

class CMyComboEdit : public CEdit
{
	DECLARE_DYNCREATE(CMyComboEdit)
public:
	CMyComboEdit()
	{
		m_pParentFrmWnd = NULL;
	};
	virtual ~CMyComboEdit(){};
	CWnd* m_pParentFrmWnd;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CComboEditXP)
public:
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CComboEditXP)
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	BOOL m_bIME;
	DECLARE_MESSAGE_MAP()
};
class CMyComboBoxEx : public CComboBoxEx
{
public:
	CMyComboBoxEx();
	CWnd* m_pParentWnd;
	CMyComboEdit m_Edit;

public:
	//{{AFX_VIRTUAL(CMyComboBoxEx)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
	{
		m_pParentWnd = pParentWnd;
		BOOL bRet = CComboBoxEx::Create(dwStyle, rect, pParentWnd, nID);
		HWND hEdit = (HWND)::SendMessage(m_hWnd, CBEM_GETEDITCONTROL, 0, 0);
		if (hEdit != NULL)
		{
			m_Edit.SubclassWindow(hEdit);
			m_Edit.m_pParentFrmWnd = pParentWnd;
			SHAutoComplete(m_Edit.m_hWnd, SHACF_URLALL | SHACF_AUTOSUGGEST_FORCE_ON | SHACF_AUTOAPPEND_FORCE_ON);
		}
		return bRet;
	}
	void AppendString(LPTSTR lpText)
	{
		if (!lpText)
			return;
		COMBOBOXEXITEM cbei = {0};
		memset(&cbei, 0x00, sizeof(cbei));
		cbei.mask = CBEIF_TEXT;
		cbei.iItem = 0;
		cbei.pszText = lpText;
		InsertItem(&cbei);
	}

public:
	virtual ~CMyComboBoxEx();

protected:
	//{{AFX_MSG(CMyComboBoxEx)
	afx_msg void OnSelendok();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
