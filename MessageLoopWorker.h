#pragma once
#include "sbcommon.h"

class MassageLoopWorker
{
public:
	UINT_PTR m_nTimerID;

	MassageLoopWorker(HWND hTargetWnd, UINT_PTR nTimerId);
	~MassageLoopWorker();
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
