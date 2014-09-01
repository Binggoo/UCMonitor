
// UCMonitor.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#define WM_SHOWTASK (WM_USER + 1)

// CUCMonitorApp:
// See UCMonitor.cpp for the implementation of this class
//

class CUCMonitorApp : public CWinApp
{
public:
	CUCMonitorApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CUCMonitorApp theApp;