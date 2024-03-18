#pragma once
#include "sbcommon.h"

class MessageLoopWorker
{
public:
	MessageLoopWorker();
	~MessageLoopWorker();
	HWND m_hWnd;
	void Run();

private:

	static const UINT_PTR m_nTimerID = 1;
	static const int32_t m_nTimerDelayPlaceholder = INT_MAX;
	// The maximum number of milliseconds we're willing to wait between calls to
	// DoWork().
	static const int64_t m_nMaxTimerDelay = 1000 / 30; // 30fps

	bool m_bTimerPending_;
	bool m_bIsActive_;
	bool m_bReentrancyDetected_;

	void SetTimer(int64_t delayMs);
	void KillTimer();
	void OnScheduleWork(int64_t delayMs);
	void OnTimerTimeout();
	void DoWork();
	void InitWindow();
	static LRESULT CALLBACK MessageLoopWindowHandler(HWND hwnd,
							 UINT msg,
							 WPARAM wparam,
							 LPARAM lparam);
};
