// This class is based on CEF's main_message_loop_external_pump_win.cc.
// https://github.com/chromiumembedded/cef/blob/master/tests/shared/browser/main_message_loop_external_pump_win.cc
// main_message_loop_external_pump_win.cc is governed by a BSD-style license.
// https://github.com/chromiumembedded/cef/blob/master/LICENSE.txt

#include "stdafx.h"
#include "MessageLoopWorker.h"

MessageLoopWorker::MessageLoopWorker(HWND hTargetWnd, UINT_PTR nTimerID)
{
	m_bTimerPending_ = false;
	m_bIsActive_ = false;
	m_bReentrancyDetected_ = false;
	m_nTimerID = nTimerID;
	m_hWnd_ = hTargetWnd;
}

MessageLoopWorker::~MessageLoopWorker()
{
	KillTimer();
}

void MessageLoopWorker::OnScheduleWork(int64_t delayMs)
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

void MessageLoopWorker::OnTimerTimeout()
{
	KillTimer();
	DoWork();
}

void MessageLoopWorker::SetTimer(int64_t delayMs)
{
	m_bTimerPending_ = true;
	::SetTimer(m_hWnd_, m_nTimerID, static_cast<UINT>(delayMs), nullptr);
}

void MessageLoopWorker::KillTimer()
{
	if (m_bTimerPending_)
	{
		::KillTimer(m_hWnd_, m_nTimerID);
		m_bTimerPending_ = false;
	}
}

void MessageLoopWorker::DoWork()
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