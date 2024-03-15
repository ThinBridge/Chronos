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
	const int32_t m_nTimerDelayPlaceholder = INT_MAX;
	// The maximum number of milliseconds we're willing to wait between calls to
	// DoWork().
	const int64_t m_nMaxTimerDelay = 1000 / 30; // 30fps

	HWND m_hWnd_;
	bool m_bTimerPending_;
	bool m_bIsActive_;
	bool m_bReentrancyDetected_;

	void SetTimer(int64_t delayMs);
	void KillTimer();
};
