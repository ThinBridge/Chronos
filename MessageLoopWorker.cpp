#include "stdafx.h"
#include "MessageLoopWorker.h"

MassageLoopWorker::MassageLoopWorker(HWND hTargetWnd, UINT_PTR nTimerID)
{
	m_bTimerPending_ = false;
	m_bIsActive_ = false;
	m_bReentrancyDetected_ = false;
	m_nTimerID = nTimerID;
	m_hWnd_ = hTargetWnd;
}

MassageLoopWorker::~MassageLoopWorker()
{
	KillTimer();
}

void MassageLoopWorker::OnScheduleWork(int64_t delayMs)
{
	if (delayMs == m_nTimerDelayPlaceholder && m_bTimerPending_)
	{
		// Don't set the maximum timer requested from DoWork() if a timer event is
		// currently pending.
		return;
	}

	if (delayMs <= 0)
	{
		DoWork();
	}
	else
	{
		// If | delayMs | is > 0 then the call should be scheduled to happen after the specified delay 
		// and any currently pending scheduled call should be cancelled. 
		KillTimer();

		if (delayMs > m_nMaxTimerDelay)
		{
			delayMs = m_nMaxTimerDelay;
		}
		SetTimer(delayMs);
	}
}

void MassageLoopWorker::OnTimerTimeout()
{
	KillTimer();
	DoWork();
}

void MassageLoopWorker::SetTimer(int64_t delayMs)
{
	m_bTimerPending_ = true;
	::SetTimer(m_hWnd_, m_nTimerID, static_cast<UINT>(delayMs), nullptr);
}

void MassageLoopWorker::KillTimer()
{
	if (m_bTimerPending_)
	{
		::KillTimer(m_hWnd_, m_nTimerID);
		m_bTimerPending_ = false;
	}
}

void MassageLoopWorker::DoWork()
{
	if (m_bIsActive_)
	{
		// When CefDoMessageLoopWork() is called there may be various callbacks
		// (such as paint and IPC messages) that result in additional calls to this
		// method. If re-entrancy is detected we must repost a request again to the
		// owner thread to ensure that the discarded call is executed in the future.
		m_bReentrancyDetected_ = true;
	}
	else
	{
		m_bReentrancyDetected_ = false;

		m_bIsActive_ = true;
		CefDoMessageLoopWork();
		m_bIsActive_ = false;
		
		// Note: |m_bReentrancy_detected_| may have changed due to re-entrant calls to 
		// this method.
	}

	if (m_bReentrancyDetected_)
	{
		// Execute the remaining work as soon as possible.
		PostMessage(m_hWnd_, WM_SCHEDULE_CEF_WORK, NULL, 0);
	}
	else if (!m_bTimerPending_)
	{
		PostMessage(m_hWnd_, WM_SCHEDULE_CEF_WORK, NULL, static_cast<LPARAM>(m_nTimerDelayPlaceholder));
	}
}
