#include "pch.h"
#include "..\..\StdAfx.h"
#include "..\..\sbcommon.h"
TEST(Test_Sbcommon, CLogDispatcher_GetOpStr)
{
	CLogDispatcher clogDispatcher;
	EXPECT_EQ(clogDispatcher.GetOpStr(LOG_UPLOAD), _T("upload"));
	EXPECT_EQ(clogDispatcher.GetOpStr(LOG_DOWNLOAD), _T("download"));
	EXPECT_EQ(clogDispatcher.GetOpStr(LOG_BROWSING), _T("browsing"));
	EXPECT_EQ(clogDispatcher.GetOpStr(LOG_M_GET), _T("upload"));
	EXPECT_EQ(clogDispatcher.GetOpStr(LOG_M_POST), _T("download"));
	EXPECT_EQ(clogDispatcher.GetOpStr(-1), _T("download"));
}