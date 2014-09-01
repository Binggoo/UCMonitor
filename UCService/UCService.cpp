// UCService.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "myservice.h"

int _tmain(int argc, _TCHAR* argv[])
{
	// Create the service object
	CMyService MyService;

	// Parse for standard arguments (install, uninstall, version etc.)
	if (!MyService.ParseStandardArgs(argc, argv)) 
	{

		// Didn't find any standard args so start the service
		// Uncomment the DebugBreak line below to enter the debugger
		// when the service is started.
		//DebugBreak();
		MyService.StartService();
	}

	// When we get here, the service has been stopped
	return MyService.m_Status.dwWin32ExitCode;
}

