#pragma once

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "BroFrame.h"
#include "MyComboBoxEx.h"
#include "APIHook.h"

class CMainFrame : public CFrameWnd
{

public:
	CMainFrame();
	HWND m_hHookDlg_hwnd;
	BOOL m_bAtomOpen;
	BOOL m_bGlobalMsgFlg;
	BOOL m_bGlobalMsgFlgMem;
	BOOL m_bCleanUPFinish;
	INT_PTR m_iMessageLoopTimerID;
	void CleanUP();

protected:
	DECLARE_DYNAMIC(CMainFrame)

public:
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL
	void AppExitExBT(CWnd* ptrW);
	void OnAppExitEx();
	void OnAppExitTimeLimit();
	void CloseMultiProcess(HWND hWndParent);
	BOOL CheckRecovery();
	BOOL CreateNewWindow(LPCTSTR pURL, BOOL bActive = TRUE);
	void RestoreWnd(LPCTSTR lpFile);

	void OnAppExitMaxMem();
	UINT GetWindowCount();
	CBrowserFrame* GetPrevFrmWnd(CBrowserFrame* ptdCurrent);
	CBrowserFrame* GetNextFrmWnd(CBrowserFrame* ptdCurrent);
	void GetMultiProcessWnd(HWND hWndParent);
	CPtrArray m_MultiProcessHWND;
	void SaveWindowList(LPCTSTR strPath, BOOL bAppendMode = FALSE);

public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
public:
	UINT Get_TabWindowCount();
	void Add_TabWindow(CBrowserFrame* Target);
	void Delete_TabWindow(CBrowserFrame* Target);
	INT Get_TabWindowIndex(CBrowserFrame* Target);
	void Get_TabWindowInfoArray(CStringArray& strATitle, CPtrArray& ptrAWnd);
	void Get_WindowInfoArrayWithPID(CStringArray& strATitle, CPtrArray& ptrAWnd, CUIntArray& ptrAPID);
	void Get_TabWindowInfoArray_hWnd(CPtrArray& ptrAWnd);
	void CloseTabWindowLeft(CBrowserFrame* pTarget);
	void CloseTabWindowRight(CBrowserFrame* pTarget);

	CString Get_TabWindowInfo_Title(HWND hWnd);
	void TabWindowMsgBSend(int iCommand, HWND hWnd);
	void SetWindowPlacementFrm(WINDOWPLACEMENT tFramePlacementMFrm, CBrowserFrame* Target);
	CBrowserFrame* GetNextGenerationActiveWindow(CBrowserFrame* pTarget);
	BOOL SetActiveFramePtr(CBrowserFrame* pTarget);
	void Tab_HScrollSync(HWND hWnd, DWORD dw1, DWORD dw2, DWORD dw3);
	void TabWindowChk();
	BOOL ReorderTab(int iOrg, int iAft);
	CBrowserFrame* GetBrowserFrameFromChildWnd(HWND hWnd);
	CBrowserFrame* m_pActiveWindow;
	CBrowserFrame* m_pPrevActiveWindow;

protected:
	CPtrList m_colBrowserWindows;
	CPtrList m_colBrowserTabList;
	void ShowTimeoutMessageBox(HWND hWnd, LPCTSTR strMsg, int iType, int iTimeOut);

	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	//}}AFX_MSG
	afx_msg void View_InitOK();
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnEndSession(BOOL bEnable);

public:
	LRESULT OnDBLAuthChk(WPARAM wParam, LPARAM lParam);
	LRESULT DeleteWindowList(WPARAM wParam, LPARAM lParam);
	LRESULT OnNewInstance(WPARAM nAtom, LPARAM lParam);
	LRESULT OnActiveHead(WPARAM wParam, LPARAM lParam);
	LRESULT OnActiveTail(WPARAM wParam, LPARAM lParam);
	LRESULT OnSaveWindowListMSG(WPARAM wParam, LPARAM lParam);
	CChildView* NewBrowserWindow(DWORD dwFlags);
	void HideOtherTabWindows(HWND hwndExclude);
	DECLARE_MESSAGE_MAP()
	void CheckBrowserWnd();

	INT_PTR m_iTimerID;
	INT_PTR m_iRecoveryTimerID;
	INT_PTR m_iTabTimerID;
	BOOL m_bTabTimerProcLock;
	BOOL ParseCommandLineAndNewWnd(CString strCommandLine);
	void InitFunc(CWnd* pFrame);
	void HideRebar(CBrowserFrame* pFrame);
};
