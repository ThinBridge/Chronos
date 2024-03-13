#include "stdafx.h"
#include "SazabiCefMessageLoopWorker.h"

SazabiCefMassageLoopWorker::SazabiCefMassageLoopWorker(HWND hTargetWnd, UINT_PTR nTimerID)
{
	m_bTimerPending_ = false;
	m_bIsActive_ = false;
	m_bReentrancyDetected_ = false;
	m_nTimerID = nTimerID;
	m_hWnd_ = hTargetWnd;
}

SazabiCefMassageLoopWorker::~SazabiCefMassageLoopWorker()
{
	KillTimer();
}

void SazabiCefMassageLoopWorker::OnScheduleWork(int64_t delayMs)
{
	if (delayMs <= 0)
	{
		DoWork();
	}
	else
	{
		// If | delayMs | is > 0 then the call should be scheduled to happen after the specified delay 
		// and any currently pending scheduled call should be cancelled. 
		KillTimer();
		SetTimer(delayMs);
	}
}

void SazabiCefMassageLoopWorker::OnTimerTimeout()
{
	KillTimer();
	DoWork();
}

void SazabiCefMassageLoopWorker::SetTimer(int64_t delayMs)
{
	m_bTimerPending_ = true;
	::SetTimer(m_hWnd_, m_nTimerID, static_cast<UINT>(delayMs), nullptr);
}

void SazabiCefMassageLoopWorker::KillTimer()
{
	if (m_bTimerPending_)
	{
		::KillTimer(m_hWnd_, m_nTimerID);
		m_bTimerPending_ = false;
	}
}

void SazabiCefMassageLoopWorker::DoWork()
{
	const bool was_reentrant = PerformMessageLoopWork();
	if (was_reentrant)
	{
		// Execute the remaining work as soon as possible.
		PostMessage(m_hWnd_, WM_SCHEDULE_CEF_WORK, NULL, 0);
	}
}

bool SazabiCefMassageLoopWorker::PerformMessageLoopWork()
{
	if (m_bIsActive_)
	{
		// When CefDoMessageLoopWork() is called there may be various callbacks
		// (such as paint and IPC messages) that result in additional calls to this
		// method. If re-entrancy is detected we must repost a request again to the
		// owner thread to ensure that the discarded call is executed in the future.
		m_bReentrancyDetected_ = true;
		return false;
	}

	m_bReentrancyDetected_ = false;

	m_bIsActive_ = true;
	CefDoMessageLoopWork();
	m_bIsActive_ = false;

	// |m_bReentrancy_detected_| may have changed due to re-entrant calls to this
	// method.
	return m_bReentrancyDetected_;
}
