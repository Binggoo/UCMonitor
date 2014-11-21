// UCService.cpp : �������̨Ӧ�ó������ڵ㡣
//
// V1.01 2014-10-09 Binggoo  1.�޸ķ����Զ�����
// V1.02 2014-10-11 Binggoo  1.��������������
//                           2.�����ϴ��û�LOG�ļ�
// v1.03 2014-11-20 Binggoo  1.����MTPӳ�񿽱�

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

