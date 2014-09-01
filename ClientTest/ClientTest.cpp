// ClientTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <winsock2.h>
#pragma comment(lib,"Ws2_32.lib")
#include "../UCService/CmdDefMgr.h"


#define DEFAULT_COUNT       20
#define DEFAULT_PORT        7788
#define DEFAULT_BUFFER      2048
#define DEFAULT_MESSAGE     "This is a test of the emergency \
broadcasting system"

#define MAX_CLINT_COUNT  100

char  szServer[128],          // Server to connect to
	szMessage[1024];        // Message to send to sever
int   iPort     = DEFAULT_PORT;  // Port on server to connect to
DWORD dwCount   = DEFAULT_COUNT; // Number of times to send message
BOOL  bSendOnly = FALSE;         // Send data only; don't receive

//
// Function: usage:
//
// Description:
//    Print usage information and exit
//
void usage()
{
	printf("usage: client [-p:x] [-s:IP] [-n:x] [-o]\n\n");
	printf("       -p:x      Remote port to send to\n");
	printf("       -s:IP     Server's IP address or hostname\n");
	printf("       -n:x      Number of times to send message\n");
	printf("       -o        Send messages only; don't receive\n");
	ExitProcess(1);
}

//
// Function: ValidateArgs
//
// Description:
//    Parse the command line arguments, and set some global flags 
//    to indicate what actions to perform
//
void ValidateArgs(int argc, char **argv)
{
	int                i;

	for(i = 1; i < argc; i++)
	{
		if ((argv[i][0] == '-') || (argv[i][0] == '/'))
		{
			switch (tolower(argv[i][1]))
			{
			case 'p':        // Remote port
				if (strlen(argv[i]) > 3)
					iPort = atoi(&argv[i][3]);
				break;
			case 's':       // Server
				if (strlen(argv[i]) > 3)
					strcpy(szServer, &argv[i][3]);
				break;
			case 'n':       // Number of times to send message
				if (strlen(argv[i]) > 3)
					dwCount = atol(&argv[i][3]);
				break;
			case 'o':       // Only send message; don't receive
				bSendOnly = TRUE;
				break;
			default:
				usage();
				break;
			}
		}
	}
}

int _tmain(int argc, char* argv[])
{
	WSADATA       wsd;
	char          szBuffer[DEFAULT_BUFFER];
	int           ret;
	struct sockaddr_in server;
	struct hostent    *host = NULL;

	// Parse the command line and load Winsock
	//
	ValidateArgs(argc, argv);
	if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
	{
		printf("Failed to load Winsock library!\n");
		return 1;
	}
	SOCKET sClient[MAX_CLINT_COUNT] = {INVALID_SOCKET};

	SYNC_TIME_IN SyncTimeIn = {0};
	SyncTimeIn.dwSizeSend = sizeof(SYNC_TIME_IN);
	SyncTimeIn.dwCmdIn = CMD_SYNC_TIME_IN;

	memcpy(szMessage,&SyncTimeIn,sizeof(SYNC_TIME_IN));

	for (int i = 0;i < MAX_CLINT_COUNT;i++)
	{
		//
		// Create the socket, and attempt to connect to the server
		//
		sClient[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sClient[i] == INVALID_SOCKET)
		{
			printf("socket(%d) failed: %d\n", i+1,WSAGetLastError());
			continue;
		}
		server.sin_family = AF_INET;
		server.sin_port = htons(iPort);
		server.sin_addr.s_addr = inet_addr(szServer);
		//
		// If the supplied server address wasn't in the form
		// "aaa.bbb.ccc.ddd" it's a hostname, so try to resolve it
		//
		if (server.sin_addr.s_addr == INADDR_NONE)
		{
			host = gethostbyname(szServer);
			if (host == NULL)
			{
				printf("Unable to resolve server: %s\n", szServer);
				return 1;
			}
			CopyMemory(&server.sin_addr, host->h_addr_list[0],
				host->h_length);
		}


		if (connect(sClient[i], (struct sockaddr *)&server, 
			sizeof(server)) == SOCKET_ERROR)
		{
			printf("connect(%d) failed: %d\n",i+1, WSAGetLastError());
			break;
		}
		// Send and receive data 
		//
		printf("Client %d connnect success.\n",i+1);
		
		
		ret = send(sClient[i], szMessage, sizeof(SYNC_TIME_IN), 0);
		if (ret == 0)
			break;
		else if (ret == SOCKET_ERROR)
		{
			printf("send() failed: %d\n", WSAGetLastError());
			break;
		}
		printf("Send %d bytes\n", ret);
		
		
		ret = recv(sClient[i], szBuffer, DEFAULT_BUFFER, 0);
		if (ret == 0)        // Graceful close
			break;
		else if (ret == SOCKET_ERROR)
		{
			printf("recv() failed: %d\n", WSAGetLastError());
			break;
		}
		szBuffer[ret] = '\0';

		SYNC_TIME_OUT SyncTimeOut = {0};
		memcpy(&SyncTimeOut,szBuffer,sizeof(SYNC_TIME_OUT));

		printf("RECV [%d bytes]: %04d-%02d-%02d %02d:%02d:%02d\n", ret
			,SyncTimeOut.wYmdHMS[0],SyncTimeOut.wYmdHMS[1],SyncTimeOut.wYmdHMS[2]
			,SyncTimeOut.wYmdHMS[3],SyncTimeOut.wYmdHMS[4],SyncTimeOut.wYmdHMS[5]);
		
		
		Sleep(1000);

	}

	system("pause");

	for (int i = 0;i < MAX_CLINT_COUNT;i++)
	{
		if (sClient[i] != INVALID_SOCKET)
		{
			closesocket(sClient[i]);
			sClient[i] = INVALID_SOCKET;
		}
	}


	WSACleanup();
	return 0;
}

