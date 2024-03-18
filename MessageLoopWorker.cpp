// This class is based on CEF's main_message_loop_external_pump_win.cc.
// https://github.com/chromiumembedded/cef/blob/master/tests/shared/browser/main_message_loop_external_pump_win.cc
// main_message_loop_external_pump_win.cc is governed by a BSD-style license.
// https://github.com/chromiumembedded/cef/blob/master/LICENSE.txt

#include "stdafx.h"
#include "MessageLoopWorker.h"

MessageLoopWorker::MessageLoopWorker(HINSTANCE hInstance)
{
	m_bTimerPending_ = false;
	m_bIsActive_ = false;
	m_bReentrancyDetected_ = false;
	m_hWnd_ = NULL;
	m_hInstance_ = hInstance;
}

MessageLoopWorker::~MessageLoopWorker()
{
	KillTimer();
	::SetWindowLongPtr(m_hWnd_, GWLP_USERDATA, NULL);
	DestroyWindow(m_hWnd_);
	m_hWnd_ = NULL;
}

void MessageLoopWorker::Run()
{
	InitWindow();
	OnScheduleWork(0);
}

BOOL MessageLoopWorker::PostScheduleMessage(int64_t delayMs)
{
	if (!m_hWnd_)
	{
		return FALSE;
	}
	return PostMessage(m_hWnd_, WM_SCHEDULE_CEF_WORK, NULL, static_cast<LPARAM>(delayMs));
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
		PostScheduleMessage(0);
	}
	else if (!m_bTimerPending_)
	{
		PostScheduleMessage(m_nTimerDelayPlaceholder);
	}
}

void MessageLoopWorker::InitWindow()
{
	const wchar_t* className = _T("MessageLoopWindow");
	HINSTANCE hInstance = m_hInstance_;
	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = MessageLoopWorker::WindowProcesser;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = className;
	RegisterClassEx(&wcex);

	m_hWnd_ = CreateWindow(className, nullptr, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInstance, nullptr);
	::SetWindowLongPtr(m_hWnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

// static
LRESULT CALLBACK MessageLoopWorker::WindowProcesser(HWND hwnd,
						    UINT msg,
						    WPARAM wparam,
						    LPARAM lparam)
{
	MessageLoopWorker* messageLoopWorker = reinterpret_cast<MessageLoopWorker*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (!messageLoopWorker)
	{
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	switch (msg)
	{
	case WM_TIMER:
		// Timer timed out.
		if (wparam == m_nTimerID)
		{
			messageLoopWorker->OnTimerTimeout();
			return 0;
		}
		break;
	case WM_SCHEDULE_CEF_WORK:
		// OnScheduleMessagePumpWork() request.
		const int64_t delayMs = static_cast<int64_t>(lparam);
		messageLoopWorker->OnScheduleWork(delayMs);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
