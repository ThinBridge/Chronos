#pragma once
#include "resource.h"
#include <afxcmn.h>
typedef class _PageInfo PAGE_INFO;
class _PageInfo
{
public:
	BOOL bViewClass;
	UINT nID;
	CWnd* pWnd;
	CWnd* pWndParent;
	CString csCaption;
	CString csParentCaption;
};
namespace autoresize
{
	enum ResizeSpecifier
	{
		LEFT = 0x01,
		RIGHT = 0x02,
		TOP = 0x04,
		BOTTOM = 0x08,
		NO_HOR_RESIZE = 0x10,
		NO_VERT_RESIZE = 0x20,
	};

	struct ResizeSpec
	{
		HWND m_hwnd;
		CRect m_rc;
		UINT m_resizeSpec;
	};

	class CAutoResize
	{
	public:
		CAutoResize()
		{
		}
		virtual ~CAutoResize()
		{
			if (m_list.GetSize())
			{
				m_list.RemoveAll();
			}
		}

		bool AddSnap(CWnd* wnd, CRect rc, UINT resizeSpec)
		{
			HWND hwnd = wnd->GetSafeHwnd();
			if (!::IsWindow(hwnd))
			{
				return false;
			}
			int iMax = (int)m_list.GetSize();
			int i = 0;
			for (i = 0; i < iMax; ++i)
			{
				if (m_list[i].m_hwnd == hwnd)
				{
					break;
				}
			}
			if (resizeSpec == 0)
			{
				if (i < m_list.GetSize())
				{
					m_list.RemoveAt(i);
					return true;
				}
				return false;
			}

			if ((resizeSpec & (LEFT | RIGHT)) == (LEFT | RIGHT) && (resizeSpec & NO_HOR_RESIZE))
			{
				return false;
			}
			if ((resizeSpec & (TOP | BOTTOM)) == (TOP | BOTTOM) && (resizeSpec & NO_VERT_RESIZE))
			{
				return false;
			}

			if (rc == CRect(INT_MIN, INT_MIN, INT_MIN, INT_MIN))
			{
				CWnd* wndParent = wnd->GetParent();
				if (wndParent == NULL)
				{
					return false;
				}
				CRect rcParent;
				wndParent->GetClientRect(&rcParent);
				wnd->GetWindowRect(&rc);
				wndParent->ScreenToClient(&rc);
				rc.BottomRight() = rcParent.BottomRight() - rc.BottomRight();
			}
			else if (rc == CRect(INT_MAX, INT_MAX, INT_MAX, INT_MAX))
			{
				CWnd* wndParent = wnd->GetParent();
				if (wndParent == NULL)
				{
					return false;
				}
				CRect rcParent;
				wndParent->GetClientRect(&rcParent);
				wnd->GetWindowRect(&rc);
				wndParent->ScreenToClient(&rc);
				rc.BottomRight() = rcParent.BottomRight() - rc.BottomRight();
				rc.bottom = 0;
				rc.right = 0;
			}

			if (i >= m_list.GetSize())
			{
				autoresize::ResizeSpec spec;
				spec.m_hwnd = NULL;
				m_list.Add(spec);
			}

			m_list[i].m_hwnd = hwnd;
			m_list[i].m_rc = rc;
			m_list[i].m_resizeSpec = resizeSpec;
			return true;
		}

		bool AddSnapToRight(CWnd* wnd, UINT opt = 0, int cx = INT_MIN)
		{
			return AddSnap(wnd, CRect(INT_MIN, INT_MIN, cx, INT_MIN), RIGHT | opt);
		}
		bool AddSnapToBottom(CWnd* wnd, UINT opt = 0, int cy = INT_MIN)
		{
			return AddSnap(wnd, CRect(INT_MIN, INT_MIN, INT_MIN, cy), BOTTOM | opt);
		}
		bool AddSnapToRightBottom(CWnd* wnd, int cx = INT_MIN, int cy = INT_MIN)
		{
			return AddSnap(wnd, CRect(INT_MIN, INT_MIN, cx, cy), RIGHT | BOTTOM);
		}
		bool AddSnapToFullBottom(CWnd* wnd)
		{
			return AddSnap(wnd, CRect(INT_MAX, INT_MAX, INT_MAX, INT_MAX), RIGHT | BOTTOM);
		}
		void Resize(CWnd* parent)
		{
			HDWP hdwp = BeginDeferWindowPos((int)m_list.GetSize());
			if (hdwp == NULL)
			{
				return;
			}

			CRect rc;
			parent->GetClientRect(&rc);
			int iMax = (int)m_list.GetSize();
			for (int i = 0; i < iMax; ++i)
			{
				autoresize::ResizeSpec& spec = m_list[i];
				ASSERT(spec.m_resizeSpec != 0);

				CWnd* wnd = CWnd::FromHandle(spec.m_hwnd);
				CRect rcNew;
				wnd->GetWindowRect(&rcNew);
				parent->ScreenToClient(&rcNew);

				int width = rcNew.Width();
				int height = rcNew.Height();

				if (spec.m_resizeSpec & LEFT)
				{
					rcNew.left = rc.left + spec.m_rc.left;
					if (spec.m_resizeSpec & NO_HOR_RESIZE)
					{
						rcNew.right = rcNew.left + width;
					}
				}
				if (spec.m_resizeSpec & RIGHT)
				{
					rcNew.right = rc.right - spec.m_rc.right;
					if (spec.m_resizeSpec & NO_HOR_RESIZE)
					{
						rcNew.left = rcNew.right - width;
					}
				}
				if (spec.m_resizeSpec & TOP)
				{
					rcNew.top = rc.top + spec.m_rc.right;
					if (spec.m_resizeSpec & NO_VERT_RESIZE)
					{
						rcNew.bottom = rcNew.top + height;
					}
				}
				if (spec.m_resizeSpec & BOTTOM)
				{
					rcNew.bottom = rc.bottom - spec.m_rc.bottom;
					if (spec.m_resizeSpec & NO_VERT_RESIZE)
					{
						rcNew.top = rcNew.bottom - height;
					}
				}
				DeferWindowPos(hdwp, wnd->GetSafeHwnd(), NULL, rcNew.left, rcNew.top, rcNew.Width(), rcNew.Height(), SWP_NOZORDER);
			}

			EndDeferWindowPos(hdwp);
		}
		CArray<autoresize::ResizeSpec, autoresize::ResizeSpec&> m_list;
	};

} // namespace autoresize
class CPrefsStatic : public CStatic
{
public:
	CPrefsStatic()
	{
		m_textClr = ::GetSysColor(COLOR_3DFACE);
		m_fontWeight = FW_NORMAL;
		m_fontSize = 12;
	}

public:
	CString m_csFontName;

	void SetConstantText(LPCTSTR pText) { m_csConstantText = pText; }

	int m_fontSize, m_fontWeight;
	BOOL m_grayText;
	COLORREF m_textClr;

public:
	virtual ~CPrefsStatic()
	{
		if (m_bm.GetSafeHandle())
		{
			m_bm.DeleteObject();
		}

		if (m_captionFont.GetSafeHandle())
		{
			m_captionFont.DeleteObject();
		}

		if (m_nameFont.GetSafeHandle())
		{
			m_nameFont.DeleteObject();
		}
	}

protected:
	DECLARE_DYNAMIC(CPrefsStatic)
	CFont m_captionFont, m_nameFont;
	CBitmap m_bm;
	CString m_csConstantText;
	void MakeCaptionBitmap();

protected:
	//{{AFX_MSG(CPrefsStatic)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#define WM_SETTINGSDIALOG_CLOSE WM_USER + 5

typedef CTypedPtrArray<CPtrArray, PAGE_INFO*> PAGE_LIST;
typedef CMap<CWnd*, CWnd*, INT_PTR, INT_PTR&> WNDTREE_MAP;

class CSettingsDialog : public CDialog
{
public:
	CSettingsDialog(CWnd* pParent = NULL);
	virtual ~CSettingsDialog();

public:
	BOOL Create();
	void ExpandTree();
	BOOL DestroyPages();
	BOOL CreatePage(const PAGE_INFO* pInfo);
	void ShowPage(const PAGE_INFO* pInfo, UINT nShow = SW_SHOW);
	void SetLogoText(CString sText);
	void SetTitle(CString sTitle);
	HTREEITEM GetNextItemCOrS(HTREEITEM hItem);
	HTREEITEM FindItem(const CString& csCaption);
	HTREEITEM FindItem(CWnd* pWnd);
	CWnd* AddPage(CRuntimeClass* pWndClass, LPCTSTR pCaption, UINT nID, LPCTSTR pParentCaption);
	CWnd* AddPage(CRuntimeClass* pWndClass, LPCTSTR pCaption, UINT nID = 0, CWnd* pDlgParent = NULL);
	autoresize::CAutoResize m_autoResize;

	PAGE_LIST m_pInfo;
	CRect m_FrameRect;
	CString m_csTitle;
	CString m_csLogoText;
	WNDTREE_MAP m_wndMap;
	CWnd* m_pParent;
	//{{AFX_DATA(CSettingsDialog)
	enum
	{
		IDD = IDD_SETTINGS_DLG
	};
	CStatic m_PageFrame;
	CTreeCtrl m_TreeCtrl;
	CPrefsStatic m_CaptionBarCtrl;
	//}}AFX_DATA

	// Overrides
	virtual BOOL CreateWnd(CWnd* pWnd, CCreateContext* pContext = NULL);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsDialog)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	    //}}AFX_VIRTUAL

	// Implementation
protected:
	void ExpandBranch(HTREEITEM hti);
	void InitTreeCtrl();
	BOOL RefreshData();

	// Generated message map functions
	//{{AFX_MSG(CSettingsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnGetDispInfoTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg void OnPreferenceHelp();
	afx_msg void OnApply();

	DECLARE_MESSAGE_MAP()
};

class CDlgSetLog : public CPropertyPage
{
	DECLARE_DYNCREATE(CDlgSetLog)
public:
	CDlgSetLog();
	//{{AFX_DATA(CDlgSetTab1)
	enum
	{
		IDD = IDD_SETTINGS_DLG_LOG
	};
	//}}AFX_DATA

	// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CDlgSetTab1)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV サポート
	    //}}AFX_VIRTUAL

	// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CDlgSetTab1)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnEnableLog();
	afx_msg void OnEnableLogging();

	//}}AFX_MSG
	LRESULT Set_OK(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButton1();
	DECLARE_MESSAGE_MAP()
};

class CDlgSetDSP : public CPropertyPage
{
	DECLARE_DYNCREATE(CDlgSetDSP)
public:
	CDlgSetDSP();
	enum
	{
		IDD = IDD_SETTINGS_DLG_DSP
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	LRESULT Set_OK(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
};

class CDlgSetGen : public CPropertyPage
{
	DECLARE_DYNCREATE(CDlgSetGen)
public:
	CDlgSetGen();
	enum
	{
		IDD = IDD_SETTINGS_DLG_GEN
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnKeyCombiTest();
	LRESULT Set_OK(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

class CDlgSetSEC : public CPropertyPage
{
	DECLARE_DYNCREATE(CDlgSetSEC)
public:
	CDlgSetSEC();
	enum
	{
		IDD = IDD_SETTINGS_DLG_SEC
	};
	CComboBox m_ComboEmu;
	void ChangeStateScriptEdit();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	void FolderBrowse(UINT nID);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBrowse1();
	afx_msg void OnBnClickedBrowse2();
	afx_msg void OnBnClickedBrowse3();
	afx_msg void OnBnClickedBrowse4();
	afx_msg void OnBnClickedBrowse5();
	afx_msg void OnBnClickedScript();
	afx_msg void OnBnClickedScriptEdit();

	LRESULT Set_OK(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

class CDlgSetConnectionSetting : public CPropertyPage
{
	DECLARE_DYNCREATE(CDlgSetConnectionSetting)
public:
	CDlgSetConnectionSetting();
	enum
	{
		IDD = IDD_SETTINGS_DLG_CONNECTION
	};
	void ChangeStateProxyEdit();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedPxIE();
	afx_msg void OnBnClickedPxNA();
	afx_msg void OnBnClickedPxEF();
	LRESULT Set_OK(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

class CDlgSetCAP : public CPropertyPage
{
	DECLARE_DYNCREATE(CDlgSetCAP)
public:
	CDlgSetCAP();
	enum
	{
		IDD = IDD_SETTINGS_DLG_CAP
	};
	void ChangeStateLimitTime();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedPRT();
	LRESULT Set_OK(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

class CDlgSetINIT : public CPropertyPage
{
	DECLARE_DYNCREATE(CDlgSetINIT)
public:
	CDlgSetINIT();
	enum
	{
		IDD = IDD_SETTINGS_DLG_INIT
	};
	CComboBox m_Combo;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	LRESULT Set_OK(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

class CDlgSetDomainFilter : public CPropertyPage
{
	DECLARE_DYNCREATE(CDlgSetDomainFilter)
public:
	CDlgSetDomainFilter();
	virtual ~CDlgSetDomainFilter();
	autoresize::CAutoResize m_autoResize;
	enum LIST_INDEX
	{
		URL,
		ACTION,
		ENABLE
	};
	int DuplicateChk(LPCTSTR sURL);
	enum
	{
		IDD = IDD_SETTINGS_DLG_URL_FILTER
	};
	CListCtrl m_List;
	//	CComboBox	m_Combo;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	void InsertDlgShow(LPCTSTR sURL);

protected:
	//	afx_msg void OnButtonPopAdd();
	afx_msg void OnButtonPopIns();
	afx_msg void OnButtonPopDel();

	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy() { CPropertyPage::OnDestroy(); }
	afx_msg void OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonUp();
	afx_msg void OnButtonDown();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnEnableCtrl();

	LRESULT Set_OK(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

class CDlgSetFileMgr : public CPropertyPage
{
	DECLARE_DYNCREATE(CDlgSetFileMgr)
public:
	CDlgSetFileMgr();
	virtual ~CDlgSetFileMgr(){};
	autoresize::CAutoResize m_autoResize;
	enum LIST_INDEX
	{
		URL,
		ACTION,
		ENABLE
	};
	enum
	{
		IDD = IDD_SETTINGS_DLG_FILEMGR
	};
	CComboBox m_Combo;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	//	afx_msg void OnButtonPopAdd();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy() { CPropertyPage::OnDestroy(); }
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnEnableCtrl();
	afx_msg void OnBnClickedULS();
	LRESULT Set_OK(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
	void ChangeStateULS();
};

class CDlgSetCustomScript : public CPropertyPage
{
	DECLARE_DYNCREATE(CDlgSetCustomScript)
public:
	CDlgSetCustomScript();
	virtual ~CDlgSetCustomScript();
	autoresize::CAutoResize m_autoResize;
	enum LIST_INDEX
	{
		URL,
		FILENAME,
		ENABLE
	};
	int DuplicateChk(LPCTSTR sURL, LPCTSTR sFileName);
	enum
	{
		IDD = IDD_SETTINGS_DLG_CUSTOM_SCRIPT
	};
	CListCtrl m_List;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	void InsertDlgShow();

protected:
	//	afx_msg void OnButtonPopAdd();
	afx_msg void OnButtonPopIns();
	afx_msg void OnButtonPopDel();

	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy() { CPropertyPage::OnDestroy(); }
	afx_msg void OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonUp();
	afx_msg void OnButtonDown();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnEnableCtrl();

	LRESULT Set_OK(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedShowDevTools();
};
