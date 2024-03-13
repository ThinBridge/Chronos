#pragma once
#include "sbcommon.h"

class SazabiCefMassageLoopWorker
{
public:
	UINT_PTR m_nTimerID;

	SazabiCefMassageLoopWorker(HWND hTargetWnd, UINT_PTR nTimerId);
	~SazabiCefMassageLoopWorker();
	void OnScheduleWork(int64_t delayMs);
	void OnTimerTimeout();
	void DoWork();

private:
	HWND m_hWnd_;
	bool m_bTimerPending_;
	bool m_bIsActive_;
	bool m_bReentrancyDetected_;

	void SetTimer(int64_t delayMs);
	void KillTimer();
	bool PerformMessageLoopWork();
};
