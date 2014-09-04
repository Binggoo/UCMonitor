// myservice.cpp

#include "stdafx.h"
#include "Server.h"
#include "myservice.h"
#include "Ini.h"
#include <shlwapi.h>
#include <shlobj.h>

#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"shell32.lib")


CMyService::CMyService()
:CNTService(MY_SERVICE_NAME)
{
	m_iStartParam = 0;
	m_iIncParam = 1;
	m_iState = m_iStartParam;
}

BOOL CMyService::OnInit()
{
	// Read the registry parameters
    // Try opening the registry key:
    // HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\<AppName>\Parameters
    HKEY hkey;
	char szKey[1024];
	sprintf_s(szKey,"SYSTEM\\CurrentControlSet\\Services\\%s\\Parameters",m_szServiceName);
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     szKey,
                     0,
                     KEY_QUERY_VALUE,
                     &hkey) == ERROR_SUCCESS) {
        // Yes we are installed
        DWORD dwType = 0;
        DWORD dwSize = sizeof(m_iStartParam);
        RegQueryValueEx(hkey,
                        "Start",
                        NULL,
                        &dwType,
                        (BYTE*)&m_iStartParam,
                        &dwSize);
        dwSize = sizeof(m_iIncParam);
        RegQueryValueEx(hkey,
                        "Inc",
                        NULL,
                        &dwType,
                        (BYTE*)&m_iIncParam,
                        &dwSize);
        RegCloseKey(hkey);
    }

	// Set the initial state
	m_iState = m_iStartParam;

	return TRUE;
}

void CMyService::Run()
{
	char szFilePath[_MAX_PATH] = {NULL};
	::GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));

	strcpy_s(szFilePath+strlen(szFilePath)-3,4,"log");

	HANDLE hLogFile = CreateFile(szFilePath,GENERIC_WRITE | GENERIC_READ,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_ALWAYS,0,NULL);

	strcpy_s(szFilePath+strlen(szFilePath)-3,4,"ini");

	CServer server;
	CIni ini;
	ini.SetPathName(szFilePath);

	char szImagePath[MAX_PATH] = {NULL};
	char szRecordPath[MAX_PATH] = {NULL};
	char szIpAddress[256] = {NULL};
	int nPort = 7788;

	ini.GetString(_T("PathSetting"),_T("ImagePath"),szImagePath,MAX_PATH);
	ini.GetString(_T("PathSetting"),_T("RecordPath"),szRecordPath,MAX_PATH);

	if (!PathFileExistsA(szImagePath))
	{
		SHCreateDirectoryExA(NULL,szImagePath,NULL);
	}

	if (!PathFileExistsA(szRecordPath))
	{
		SHCreateDirectoryExA(NULL,szRecordPath,NULL);
	}

	server.SetPath(hLogFile,szImagePath,szRecordPath);

	system("cls");
	printf("%s Version %d.%d\n",m_szServiceName, m_iMajorVersion, m_iMinorVersion);
	server.WriteLogFile(FALSE,"\r\n%s Version %d.%d",m_szServiceName, m_iMajorVersion, m_iMinorVersion);

	ini.GetString(_T("ServerSetting"),_T("ServerIP"),szIpAddress,256);
	nPort = ini.GetInt(_T("ServerSetting"),_T("ListenPort"),7788);

	BOOL bRet = server.Start(szIpAddress,nPort);

	while (!bRet && m_bIsRunning)
	{
		printf("Start server(%s:%d) failed ! Please check the ip address and the listen port is right.\n"
			,szIpAddress,nPort);

		printf("Wait 60s to retry......\n");

		server.WriteLogFile(TRUE,"Start server(%s:%d) failed ! Please check the ip address and the listen port is right."
			,szIpAddress,nPort);
		server.WriteLogFile(TRUE,"Wait 60s to retry......");

		Sleep(60000);

		ini.GetString(_T("ServerSetting"),_T("ServerIP"),szIpAddress,256);
		nPort = ini.GetInt(_T("ServerSetting"),_T("ListenPort"),7788);

		bRet = server.Start(szIpAddress,nPort);
	}

	printf("Start server(%s:%d) success...\n",szIpAddress,nPort);
	server.WriteLogFile(TRUE,"Start server(%s:%d) success...",szIpAddress,nPort);

	while (m_bIsRunning)
	{
		Sleep(1000);
	}

	server.Shutdown();

	printf("The server(%s:%d) closed.\n",szIpAddress,nPort);
	server.WriteLogFile(TRUE,"The server(%s:%d) closed.",szIpAddress,nPort);

	CloseHandle(hLogFile);
	
}

// Process user control requests
BOOL CMyService::OnUserControl(DWORD dwOpcode)
{
    switch (dwOpcode) {
    case SERVICE_CONTROL_USER + 0:

        // Save the current status in the registry
        SaveStatus();
        return TRUE;

    default:
        break;
    }
    return FALSE; // say not handled
}

// Save the current status in the registry
void CMyService::SaveStatus()
{
    DebugMsg("Saving current status");
    // Try opening the registry key:
    // HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\<AppName>\...
    HKEY hkey = NULL;
	char szKey[1024];
	sprintf_s(szKey,"SYSTEM\\CurrentControlSet\\Services\\%s\\Status",m_szServiceName);
    DWORD dwDisp;
	DWORD dwErr;
    DebugMsg("Creating key: %s", szKey);
    dwErr = RegCreateKeyEx(	HKEY_LOCAL_MACHINE,
                           	szKey,
                   			0,
                   			"",
                   			REG_OPTION_NON_VOLATILE,
                   			KEY_WRITE,
                   			NULL,
                   			&hkey,
                   			&dwDisp);
	if (dwErr != ERROR_SUCCESS) {
		DebugMsg("Failed to create Status key (%lu)", dwErr);
		return;
	}	

    // Set the registry values
	DebugMsg("Saving 'Current' as %ld", m_iState); 
    RegSetValueEx(hkey,
                  "Current",
                  0,
                  REG_DWORD,
                  (BYTE*)&m_iState,
                  sizeof(m_iState));


    // Finished with key
    RegCloseKey(hkey);

}
