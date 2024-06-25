#pragma once
#include "sbcommon.h"

class MessageLoopWorker
{
public:
	MessageLoopWorker(HINSTANCE hInstance);
	~MessageLoopWorker();
	bool PostScheduleMessage(int64_t delayMs);
	void Run();
	void Quit();

private:
	static const UINT_PTR m_nTimerID = 1;
	static const int32_t m_nTimerDelayPlaceholder = INT_MAX;
	// The maximum number of milliseconds we're willing to wait between calls to
	// DoWork().
	static const int64_t m_nMaxTimerDelay = 1000 / 30; // 30fps

	HWND m_hWnd_;
	bool m_bTimerPending_;
	bool m_bIsActive_;
	bool m_bReentrancyDetected_;
	HINSTANCE m_hInstance_;
	bool m_bRunning_;

	void SetTimer(int64_t delayMs);
	void KillTimer();
	void OnScheduleWork(int64_t delayMs);
	void OnTimerTimeout();
	bool PerformMessageLoopWork();
	void DoWork();
	void InitWindow();
	static LRESULT CALLBACK WindowProcesser(HWND hwnd,
						UINT msg,
						WPARAM wparam,
						LPARAM lparam);
};
