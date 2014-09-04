#include "stdafx.h"
#include "Server.h"

#define SIZEOF_IMAGE_HEADER 1024
#define PKG_HEADER_SIZE     12
#define START_FLAG 0xBE
#define END_FLAG   0xED

#define LODWORD(_qw)    ((DWORD)(_qw))
#define HIDWORD(_qw)    ((DWORD)(((_qw) >> 32) & 0xffffffff))


CServer::CServer()
{
	::InitializeCriticalSection(&m_ClientDataLock);
	::InitializeCriticalSection(&m_IncompleteDataLock);

	memset(m_szImagePath,0,MAX_PATH);
	memset(m_szRecordPath,0,MAX_PATH);
	m_nThreadCount = 0;

	m_hThreadEnvent = ::CreateEvent(NULL,FALSE,TRUE,NULL);
}


CServer::~CServer(void)
{
	CleanupClientData();
	CleanUpIncompleteData();

	::DeleteCriticalSection(&m_ClientDataLock);
	::DeleteCriticalSection(&m_IncompleteDataLock);
}

void CServer::OnConnectionEstablished( CIOCPContext *pContext, CIOCPBuffer *pBuffer )
{
	printf("A client[%s:%d] come in.\n"
		,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port));
	WriteLogFile(TRUE,"A client[%s:%d] come in."
		,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port));

	// 判断是否是同步时间命令
	if (pBuffer->nLen == sizeof(SYNC_TIME_IN))
	{
		SYNC_TIME_IN syncTimeIn = {0};
		memcpy(&syncTimeIn,pBuffer->buff,sizeof(SYNC_TIME_IN));

		if (syncTimeIn.dwCmdIn == CMD_SYNC_TIME_IN && syncTimeIn.dwSizeSend == sizeof(SYNC_TIME_IN))
		{
			SYSTEMTIME SystemTime;
			GetSystemTime(&SystemTime);

			SYNC_TIME_OUT syncTimeOut = {0};
			syncTimeOut.dwCmdOut = CMD_SYNC_TIME_OUT;
			syncTimeOut.dwSizeSend = sizeof(SYNC_TIME_OUT);
			syncTimeOut.wYmdHMS[0] = SystemTime.wYear;
			syncTimeOut.wYmdHMS[1] = SystemTime.wMonth;
			syncTimeOut.wYmdHMS[2] = SystemTime.wDay;
			syncTimeOut.wYmdHMS[3] = SystemTime.wHour;
			syncTimeOut.wYmdHMS[4] = SystemTime.wMinute;
			syncTimeOut.wYmdHMS[5] = SystemTime.wSecond;

			int nLen = sizeof(SYNC_TIME_OUT);
			char *buf = new char[nLen];
			memset(buf,0,nLen);
			memcpy(buf,&syncTimeOut,nLen);

			SendText(pContext,buf,nLen);
		}
	}
}

void CServer::OnConnectionClosing( CIOCPContext *pContext, CIOCPBuffer *pBuffer )
{
	printf("A client[%s:%d] leave.\n"
		,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port));
	WriteLogFile(TRUE,"A client[%s:%d] leave."
		,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port));
}

void CServer::OnConnectionError( CIOCPContext *pContext, CIOCPBuffer *pBuffer, int nError )
{
	printf("A client[%s:%d] occured error.\n"
		,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port));
	WriteLogFile(TRUE,"A client[%s:%d] occured error."
		,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port));
}


void CServer::OnReadCompleted( CIOCPContext *pContext, CIOCPBuffer *pBuffer )
{
// 	WaitForSingleObject(m_hThreadEnvent,INFINITE);
// 
// 	if (m_nThreadCount < 64)
// 	{
// 		SetEvent(m_hThreadEnvent);
// 	}

	// 是否存在上一轮不完整的command

	IncompleteData *incompleteData = GetIncompleteData(pContext);

	char *recvBuf = NULL;
	DWORD revLen = 0;

	if (incompleteData)
	{
		//存在不完整数据

		//判断这次数据是否完整
		if (incompleteData->dwRemainSize > pBuffer->nLen)
		{
			//不完整，把整个数据加入到不完整数据队列
			int len = pBuffer->nLen;
			char *buf = new char[len];

			memcpy(buf,pBuffer->buff,len);

			incompleteData->dwRemainSize -= len;

			AddIncompleteData(incompleteData,buf,len);

			return;

		}
		else
		{
			//把上次不完整数据补全
			revLen = incompleteData->dwTotalSize + pBuffer->nLen - incompleteData->dwRemainSize;
			recvBuf = new char[revLen];
			memset(recvBuf,0,revLen);

			// 拼接数据
			::EnterCriticalSection(&incompleteData->Lock);
			DataBuf *head = incompleteData->dataBuf;
			int readlen = 0;
			while (head)
			{
				DataBuf *next = head->Next;

				memcpy(recvBuf+readlen,head->buf,head->len);

				readlen += head->len;

				head = next;
			}
			::LeaveCriticalSection(&incompleteData->Lock);
			memcpy(recvBuf+readlen,pBuffer->buff,revLen - readlen);

			// 删除不完整数据
			RemoveIncompleteData(incompleteData);
		}

	}
	else
	{
		// 不存在不完整数据
		revLen = pBuffer->nLen;
		recvBuf = new char[revLen];
		memset(recvBuf,0,revLen);

		memcpy(recvBuf,pBuffer->buff,revLen);
	}
	
	// 判断接收到的数据是否多条数据


	CMD_IN cmdIn = {0};
	memcpy(&cmdIn,recvBuf,sizeof(CMD_IN));

	DWORD dwReadLen = 0;
	DWORD dwRemainLen = 0;
	DWORD dwLen = revLen;
	
	while (dwLen >= cmdIn.dwSizeSend)
	{
		ClientData *clientData = GetClientData(pContext,cmdIn.dwCmdIn);

		char *tmpBuffer = recvBuf + dwReadLen;
		int fileLen = strlen(tmpBuffer+sizeof(CMD_IN)) + 1;

		if (clientData == NULL)
		{
			clientData = new ClientData;
			memset(clientData,0,sizeof(ClientData));
			InitializeCriticalSection(&clientData->Lock);

			clientData->pContext = pContext;
			clientData->dwCmd = cmdIn.dwCmdIn;
			clientData->bStop = cmdIn.byStop;

			if ((BYTE)tmpBuffer[cmdIn.dwSizeSend-1] == END_FLAG)
			{
				clientData->bEnd = TRUE;
			}
			else
			{
				clientData->bEnd = FALSE;
			}

			clientData->szFileName = new char[fileLen];
			memset(clientData->szFileName,0,fileLen);
			strcpy_s(clientData->szFileName,fileLen,&tmpBuffer[sizeof(CMD_IN)]);

			AddClientData(clientData);

			// 创建处理线程
			DWORD dwThreadId = 0;
			LPVOID_PARM lpVoidParm = new VOID_PARM;
			lpVoidParm->lpVoid1 = this;
			lpVoidParm->lpVoid2 = pContext;
			lpVoidParm->dwCmd = cmdIn.dwCmdIn;

			HANDLE hThread = ::CreateThread(NULL,0,CommandThreadProc,lpVoidParm,0,&dwThreadId);
			::CloseHandle(hThread);
		}
		else
		{
			clientData->pContext = pContext;
			clientData->dwCmd = cmdIn.dwCmdIn;
			clientData->bStop = cmdIn.byStop;

			if ((BYTE)tmpBuffer[cmdIn.dwSizeSend-1] == END_FLAG)
			{
				clientData->bEnd = TRUE;
			}
			else
			{
				clientData->bEnd = FALSE;
			}

			clientData->szFileName = new char[fileLen];
			memset(clientData->szFileName,0,fileLen);
			strcpy_s(clientData->szFileName,fileLen,&tmpBuffer[sizeof(CMD_IN)]);
		}
			
		if (cmdIn.dwCmdIn == CMD_UPLOAD_LOG_IN || cmdIn.dwCmdIn == CMD_MAKE_IMAGE_IN)
		{
			int len = cmdIn.dwSizeSend - sizeof(CMD_IN) - fileLen - 1; // 不包含EndFlag;
			char *buf = new char[len];
			memset(buf,0,len);

			memcpy(buf,&tmpBuffer[cmdIn.dwSizeSend - len - 1],len);

			AddDataBuf(clientData,buf,len);

		}
	
		dwLen -= cmdIn.dwSizeSend;
		dwReadLen += cmdIn.dwSizeSend;

		if (dwLen > sizeof(CMD_IN))
		{
			memcpy(&cmdIn,&tmpBuffer[cmdIn.dwSizeSend],sizeof(cmdIn));
		}
		else
		{
			break;
		}

	}

	if (dwLen != 0)
	{
		//存在不完整性数据
		char *buf = new char[dwLen];
		memset(buf,0,dwLen);

		memcpy(buf,recvBuf + dwReadLen,dwLen);

		IncompleteData *data = new IncompleteData;
		memset(data,0,sizeof(IncompleteData));
		data->pContext = pContext;
		data->dwTotalSize = cmdIn.dwSizeSend;
		data->dwRemainSize = cmdIn.dwSizeSend - dwLen;
		
		::InitializeCriticalSection(&data->Lock);

		AddIncompleteData(data);
		AddIncompleteData(data,buf,dwLen);
	
	}

	delete []recvBuf;
	recvBuf = NULL;
	
}

void CServer::OnWriteCompleted( CIOCPContext *pContext, CIOCPBuffer *pBuffer )
{

}

void CServer::AddClientData( ClientData *clientData )
{
	::EnterCriticalSection(&m_ClientDataLock);
	m_ClientDataVector.push_back(clientData);
	::LeaveCriticalSection(&m_ClientDataLock);
}

void CServer::RemoveClientData( ClientData *clientData )
{
	if (clientData == NULL)
	{
		return;
	}

	::EnterCriticalSection(&m_ClientDataLock);

	ClientDataIteractor it;
	for (it = m_ClientDataVector.begin();it != m_ClientDataVector.end();it++)
	{
		if (clientData == *it)
		{
			m_ClientDataVector.erase(it);

			if (clientData->szFileName)
			{
				delete []clientData->szFileName;
			}

			ReleaseAllDataBuf(clientData);

			DeleteCriticalSection(&clientData->Lock);

			delete clientData;

			clientData = NULL;

			break;
		}
	}

	::LeaveCriticalSection(&m_ClientDataLock);
}

void CServer::CleanupClientData()
{
	::EnterCriticalSection(&m_ClientDataLock);

	ClientDataIteractor it;
	for (it = m_ClientDataVector.begin();it != m_ClientDataVector.end();it++)
	{
		ClientData *clientData = *it;

		if (clientData->szFileName)
		{
			delete []clientData->szFileName;
		}

		ReleaseAllDataBuf(clientData);

		DeleteCriticalSection(&clientData->Lock);

		delete clientData;
		clientData = NULL;
	}

	m_ClientDataVector.clear();

	::LeaveCriticalSection(&m_ClientDataLock);
}

ClientData * CServer::GetClientData( CIOCPContext *pContext,DWORD dwCmd )
{
	ClientData *pBuffer = NULL;
	::EnterCriticalSection(&m_ClientDataLock);

	ClientDataIteractor it;
	for (it = m_ClientDataVector.begin();it != m_ClientDataVector.end();it++)
	{
		ClientData *clientData = *it;

		if (clientData->pContext == pContext && clientData->dwCmd == dwCmd)
		{
			pBuffer = clientData;
			break;
		}
	}

	::LeaveCriticalSection(&m_ClientDataLock);

	return pBuffer;
}

void CServer::AddDataBuf( ClientData *clientData,char *buf,int len )
{
	if (clientData == NULL)
	{
		return;
	}

	::EnterCriticalSection(&clientData->Lock);

	DataBuf *dataBuf = new DataBuf;
	memset(dataBuf,0,sizeof(DataBuf));

	dataBuf->buf = buf;
	dataBuf->len = len;
	dataBuf->Next = NULL;

	if (clientData->dataBuf == NULL)
	{
		clientData->dataBuf = dataBuf;
	}
	else
	{
		// 找到队列末尾
		DataBuf *pre = clientData->dataBuf;
		while (pre->Next)
		{
			pre = pre->Next;
		}

		pre->Next = dataBuf;
	}

	::LeaveCriticalSection(&clientData->Lock);
	
}

DataBuf * CServer::GetHeadDataBuf( ClientData *clientData )
{
	if (clientData == NULL)
	{
		return NULL;
	}

	if (clientData->dataBuf == NULL)
	{
		return NULL;
	}

	DataBuf *rec = NULL;

	::EnterCriticalSection(&clientData->Lock);

	rec = clientData->dataBuf;
	DataBuf *next = rec->Next;
	clientData->dataBuf = next;

	rec->Next = NULL;

	::LeaveCriticalSection(&clientData->Lock);

	return rec;
}

void CServer::RemoveHeadDataBuf( ClientData *clientData )
{
	if (clientData == NULL || clientData->dataBuf == NULL)
	{
		return;
	}

	::EnterCriticalSection(&clientData->Lock);

	DataBuf *head = clientData->dataBuf;
	clientData->dataBuf = head->Next;

	if (head->buf)
	{
		delete []head->buf;
		head->buf = NULL;
	}
	
	delete head;
	head = NULL;

	::LeaveCriticalSection(&clientData->Lock);
}

void CServer::ReleaseAllDataBuf( ClientData *clientData)
{
	if (clientData == NULL || clientData->dataBuf == NULL)
	{
		return;
	}

	::EnterCriticalSection(&clientData->Lock);

	DataBuf *head = clientData->dataBuf;
	while (head)
	{
		DataBuf *next = head->Next;

		if (head->buf)
		{
			delete []head->buf;
			head->buf = NULL;
		}

		delete head;
		head = NULL;

		head = next;
	}

	clientData->dataBuf = NULL;

	::LeaveCriticalSection(&clientData->Lock);
}

void CServer::ReleaseDataBuf( DataBuf *dataBuf )
{

	while (dataBuf)
	{
		DataBuf *next = dataBuf->Next;

		if (dataBuf->buf)
		{
			delete []dataBuf->buf;
			dataBuf->buf = NULL;
		}

		delete dataBuf;
		dataBuf = NULL;
		dataBuf = next;
	}
}



void CServer::SetPath(HANDLE hLogFile, char *szImagePath,char *szRecordPath )
{
	m_hLogFile = hLogFile;
	strcpy_s(m_szImagePath,strlen(szImagePath) + 1,szImagePath);
	strcpy_s(m_szRecordPath,strlen(szRecordPath) + 1,szRecordPath);
}

DWORD WINAPI CServer::CommandThreadProc( LPVOID lpParm )
{
	LPVOID_PARM lpVoid_Parm = (LPVOID_PARM)lpParm;
	CServer *server = (CServer *)lpVoid_Parm->lpVoid1;
	CIOCPContext *pContext = (CIOCPContext *)lpVoid_Parm->lpVoid2;
	DWORD dwCmd = lpVoid_Parm->dwCmd;

	server->HandleCommand(pContext,dwCmd);

	return 1;

}

void CServer::HandleCommand( CIOCPContext *pContext,DWORD dwCmd )
{
	ClientData *clientData = GetClientData(pContext,dwCmd);

	m_nThreadCount++;

	char filename[MAX_PATH] = {NULL};
	DWORD dwErrorCode = 0;
	ErrorType errorType = ErrorType_System;
	ULONGLONG ullOffset = 0;
	DWORD dwLen = 0;
	BOOL bResult = TRUE;

	BOOL bEnd = FALSE;
	BOOL bStop = FALSE;

	OVERLAPPED overlapped = {NULL};
	overlapped.hEvent = ::CreateEvent(NULL,FALSE,TRUE,NULL);

	if (dwCmd == CMD_UPLOAD_LOG_IN)
	{
		CMD_OUT logOut = {NULL};
		logOut.dwCmdOut = CMD_UPLOAD_LOG_OUT;
		logOut.dwSizeSend = sizeof(CMD_OUT);
		logOut.errType = errorType;
		logOut.dwErrorCode = dwErrorCode;

		ullOffset = 0;

		printf("[%s:%d]upload log file %s start...\n"
			,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename);
		WriteLogFile(TRUE,"[%s:%d]upload log file %s start..."
			,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename);

		sprintf_s(filename,"%s\\%s",m_szRecordPath,clientData->szFileName);
		HANDLE hFile = ::CreateFile(filename,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_FLAG_OVERLAPPED,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			dwErrorCode = GetLastError();
			printf("[%s:%d]create record log file %s failed with system error code:%d\n"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);
			WriteLogFile(TRUE,"[%s:%d]create record log file %s failed with system error code:%d"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);

			logOut.errType = errorType;
			logOut.dwErrorCode = dwErrorCode;

			SendText(pContext,(char *)&logOut,sizeof(CMD_OUT));

			m_nThreadCount--;

			SetEvent(m_hThreadEnvent);

			RemoveClientData(clientData);
			RemoveIncompleteData(pContext);

			return;
		}

		do 
		{
			bEnd = clientData->bEnd;
			DataBuf *dataBuf = GetHeadDataBuf(clientData);
			
			if (dataBuf)
			{
				dwLen = dataBuf->len;
				if (!WriteFileAsyn(hFile,ullOffset,dwLen,(LPBYTE)dataBuf->buf,&overlapped,&dwErrorCode))
				{
					ReleaseDataBuf(dataBuf);

					printf("[%s:%d]write record log file %s failed with system error code:%d\n"
						,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);
					WriteLogFile(TRUE,"[%s:%d]write log file %s failed with system error code:%d"
						,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);

					logOut.errType = errorType;
					logOut.dwErrorCode = dwErrorCode;

					bResult = FALSE;

					break;
				}

				ReleaseDataBuf(dataBuf);

				ullOffset += dwLen;
			}
			else
			{
				Sleep(10);
				continue;
			}
			

		} while (!bEnd && (!pContext->bClosing && pContext->s != 0));

		CloseHandle(hFile);

		SendText(pContext,(char *)&logOut,sizeof(CMD_OUT));

		if (bResult)
		{
			printf("[%s:%d]upload log file %s success.\n"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename);
			WriteLogFile(TRUE,"[%s:%d]upload log file %s success."
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename);
		}

		m_nThreadCount--;

		SetEvent(m_hThreadEnvent);

		RemoveClientData(clientData);
		RemoveIncompleteData(pContext);

		return;
	}
	else if (dwCmd == CMD_MAKE_IMAGE_IN)
	{
		CMD_OUT makeImageOut = {NULL};
		makeImageOut.dwCmdOut = CMD_MAKE_IMAGE_OUT;
		makeImageOut.dwSizeSend = sizeof(CMD_OUT);
		makeImageOut.errType = errorType;
		makeImageOut.dwErrorCode = dwErrorCode;

		ullOffset = SIZEOF_IMAGE_HEADER;

		printf("[%s:%d]make image file %s start...\n"
			,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename);
		WriteLogFile(TRUE,"[%s:%d]make image file %s start..."
			,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename);

		sprintf_s(filename,"%s\\%s",m_szImagePath,clientData->szFileName);
		HANDLE hFile = ::CreateFile(filename,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_FLAG_OVERLAPPED,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			dwErrorCode = GetLastError();
			printf("[%s:%d]create make image file %s failed with system error code:%d\n"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);
			WriteLogFile(TRUE,"[%s:%d]create make image file %s failed with system error code:%d"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);

			makeImageOut.errType = errorType;
			makeImageOut.dwErrorCode = dwErrorCode;

			SendText(pContext,(char *)&makeImageOut,sizeof(CMD_OUT));

			m_nThreadCount--;

			SetEvent(m_hThreadEnvent);

			RemoveClientData(clientData);
			RemoveIncompleteData(pContext);

			return;
		}

		do 
		{
			bEnd = clientData->bEnd;
			bStop = clientData->bStop;

			DataBuf *dataBuf = GetHeadDataBuf(clientData);

			if (dataBuf)
			{
				dwLen = dataBuf->len;

				if (bEnd)
				{
					ullOffset = 0;
				}

				if (!WriteFileAsyn(hFile,ullOffset,dwLen,(LPBYTE)dataBuf->buf,&overlapped,&dwErrorCode))
				{
					ReleaseDataBuf(dataBuf);

					printf("[%s:%d]write make image file %s failed with system error code:%d\n"
						,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);
					WriteLogFile(TRUE,"[%s:%d]write make image file %s failed with system error code:%d"
						,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);

					makeImageOut.errType = errorType;
					makeImageOut.dwErrorCode = dwErrorCode;

					SendText(pContext,(char *)&makeImageOut,sizeof(CMD_OUT));

					bResult = FALSE;

					break;
				}

				ReleaseDataBuf(dataBuf);

				ullOffset += dwLen;

				SendText(pContext,(char *)&makeImageOut,sizeof(CMD_OUT));

			}
			else
			{
				Sleep(10);
				continue;
			}

		} while (!bEnd && (!pContext->bClosing && pContext->s != 0) && !bStop);

		CloseHandle(hFile);

		if (bStop)
		{
			errorType = ErrorType_Custom;
			dwErrorCode = CustomError_Cancel;
			bResult = FALSE;

			printf("[%s:%d]read copy image file %s canceled by user with custom error code:0x%X\n"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);
			WriteLogFile(TRUE,"[%s:%d]read copy image file %s canceled by user with custom error code:0x%X"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);
		}

		if (bResult)
		{
			char *imageName = new char[strlen(filename) + 1];
			strcpy_s(imageName,strlen(filename) + 1,filename);

			strcpy_s(imageName+strlen(imageName)-3,4,"IMG");

			MoveFileExA(filename,imageName,MOVEFILE_REPLACE_EXISTING);

			printf("[%s:%d]make image file %s success.\n"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),imageName);

			WriteLogFile(TRUE,"[%s:%d]make image file %s success."
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),imageName);
		}
		else
		{
			DeleteFileA(filename);
		}

		m_nThreadCount--;	

		SetEvent(m_hThreadEnvent);

		RemoveClientData(clientData);
		RemoveIncompleteData(pContext);

		return;
	}
	else if (dwCmd == CMD_COPY_IMAGE_IN)
	{
		CMD_OUT copyImageOut = {NULL};
		copyImageOut.dwCmdOut = CMD_COPY_IMAGE_OUT;
		copyImageOut.dwSizeSend = sizeof(CMD_OUT);
		copyImageOut.errType = errorType;
		copyImageOut.dwErrorCode = dwErrorCode;

		ULONGLONG ullReadSize = SIZEOF_IMAGE_HEADER;

		ullOffset = SIZEOF_IMAGE_HEADER;

		printf("[%s:%d]copy image file %s start...\n"
			,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename);
		WriteLogFile(TRUE,"[%s:%d]copy image file %s start..."
			,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename);

		sprintf_s(filename,"%s\\%s",m_szImagePath,clientData->szFileName);
		HANDLE hFile = ::CreateFile(filename,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			dwErrorCode = GetLastError();
			printf("[%s:%d]open copy image file %s failed with system error code:%d\n"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);
			WriteLogFile(TRUE,"[%s:%d]open copy image file %s failed with system error code:%d"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);

			copyImageOut.errType = errorType;
			copyImageOut.dwErrorCode = dwErrorCode;

			SendText(pContext,(char *)&copyImageOut,sizeof(CMD_OUT));

			m_nThreadCount--;

			SetEvent(m_hThreadEnvent);

			RemoveClientData(clientData);
			RemoveIncompleteData(pContext);

			return;
		}

		// 获取Image的大小
		LARGE_INTEGER liFillSize = {0};
		GetFileSizeEx(hFile,&liFillSize);

		while (ullReadSize < (ULONGLONG)liFillSize.QuadPart)
		{
			bStop = clientData->bStop;
			bEnd = clientData->bEnd;

			while (!bEnd && !bStop && !pContext->bClosing && pContext->s != 0)
			{
				Sleep(10);
				bStop = clientData->bStop;
				bEnd = clientData->bEnd;
			}

			clientData->bEnd = FALSE;

			if (bStop)
			{
				bResult = FALSE;
				break;
			}

			if (pContext->bClosing || pContext->s == 0)
			{
				bResult = FALSE;
				break;
			}

			BYTE pkgHead[PKG_HEADER_SIZE] = {NULL};
			dwLen = PKG_HEADER_SIZE;
			if (!ReadFileAsyn(hFile,ullOffset,dwLen,pkgHead,&overlapped,&dwErrorCode))
			{
				printf("[%s:%d]read copy image file %s failed with system error code:%d\n"
					,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);
				WriteLogFile(TRUE,"[%s:%d]read copy image file %s failed with system error code:%d"
					,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);

				copyImageOut.errType = errorType;
				copyImageOut.dwErrorCode = dwErrorCode;

				bResult = FALSE;

				SendText(pContext,(char *)&copyImageOut,sizeof(UPLOAD_LOG_OUT));

				break;
			}

			dwLen = *(PDWORD)&pkgHead[8];

			PBYTE pByte = new BYTE[dwLen];
			ZeroMemory(pByte,dwLen);

			if (!ReadFileAsyn(hFile,ullOffset,dwLen,pByte,&overlapped,&dwErrorCode))
			{
				bResult = FALSE;
				delete []pByte;

				printf("[%s:%d]read copy image file %s failed with system error code:%d\n"
					,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);
				WriteLogFile(TRUE,"[%s:%d]read copy image file %s failed with system error code:%d"
					,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);

				copyImageOut.errType = errorType;
				copyImageOut.dwErrorCode = dwErrorCode;

				SendText(pContext,(char *)&copyImageOut,sizeof(UPLOAD_LOG_OUT));
				
				break;
			}

			if (pByte[dwLen-1] != END_FLAG)
			{
				errorType = ErrorType_Custom;
				dwErrorCode = CustomError_Image_Format_Error;
				bResult = FALSE;
				delete []pByte;

				printf("[%s:%d]read copy image file %s format error with custom error code:0x%X\n"
					,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);
				WriteLogFile(TRUE,"[%s:%d]read copy image file %s format error with custom error code:0x%X"
					,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);

				copyImageOut.errType = errorType;
				copyImageOut.dwErrorCode = dwErrorCode;

				SendText(pContext,(char *)&copyImageOut,sizeof(UPLOAD_LOG_OUT));

				break;
			}

			ullOffset += dwLen;
			ullReadSize += dwLen;

			// 发送数据
			DWORD dwSndLength = sizeof(CMD_OUT) + dwLen + 1;
			BYTE *pSend = new BYTE[dwSndLength];
			ZeroMemory(pSend,dwSndLength);

			copyImageOut.dwSizeSend = dwSndLength;

			memcpy(pSend,&copyImageOut,sizeof(CMD_OUT));
			memcpy(pSend + sizeof(CMD_OUT),pByte,dwLen);

			if (ullReadSize >= (ULONGLONG)liFillSize.QuadPart)
			{
				//已经读完，加上end标志
				pSend[dwSndLength - 1] = END_FLAG;
			}

			SendText(pContext,(char *)pSend,dwSndLength);

			delete []pSend;
			delete []pByte;
			
		}

		CloseHandle(hFile);

		if (bStop)
		{

			errorType = ErrorType_Custom;
			dwErrorCode = CustomError_Cancel;
			bResult = FALSE;

			printf("[%s:%d]read copy image file %s canceled by user with custom error code:0x%X\n"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);
			WriteLogFile(TRUE,"[%s:%d]read copy image file %s canceled by user with custom error code:0x%X"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);

		}

		RemoveClientData(clientData);
		RemoveIncompleteData(pContext);

		if (bResult)
		{
			printf("[%s:%d]copy image file %s success.\n"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename);
			WriteLogFile(TRUE,"[%s:%d]make image file %s success."
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename);
		}

		m_nThreadCount--;

		SetEvent(m_hThreadEnvent);

		return;
	}
	else if (dwCmd == CMD_QUERY_IMAGE_IN)
	{
		CMD_OUT queryImageOut = {NULL};
		queryImageOut.dwCmdOut = CMD_QUERY_IMAGE_OUT;
		queryImageOut.dwSizeSend = sizeof(CMD_OUT);
		queryImageOut.errType = errorType;
		queryImageOut.dwErrorCode = dwErrorCode;

		printf("[%s:%d]query image file %s start...\n"
			,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename);
		WriteLogFile(TRUE,"[%s:%d]query image file %s start..."
			,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename);

		sprintf_s(filename,"%s\\%s",m_szImagePath,clientData->szFileName);
		HANDLE hFile = ::CreateFile(filename,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			dwErrorCode = GetLastError();
			printf("[%s:%d]open query image file %s failed with system error code:%d\n"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);
			WriteLogFile(TRUE,"[%s:%d]open query image file %s failed with system error code:%d"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);

			queryImageOut.errType = errorType;
			queryImageOut.dwErrorCode = dwErrorCode;

			SendText(pContext,(char *)&queryImageOut,sizeof(CMD_OUT));

			m_nThreadCount--;

			SetEvent(m_hThreadEnvent);

			RemoveClientData(clientData);
			RemoveIncompleteData(pContext);

			return;
		}

		BYTE pkgHead[SIZEOF_IMAGE_HEADER] = {NULL};
		dwLen = SIZEOF_IMAGE_HEADER;
		ullOffset = 0;
		if (!ReadFileAsyn(hFile,ullOffset,dwLen,pkgHead,&overlapped,&dwErrorCode))
		{
			printf("[%s:%d]read query image file %s failed with system error code:%d\n"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);
			WriteLogFile(TRUE,"[%s:%d]read query image file %s failed with system error code:%d"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);

			queryImageOut.errType = errorType;
			queryImageOut.dwErrorCode = dwErrorCode;

			bResult = FALSE;

			SendText(pContext,(char *)&queryImageOut,sizeof(CMD_OUT));

			m_nThreadCount--;

			SetEvent(m_hThreadEnvent);

			RemoveClientData(clientData);
			RemoveIncompleteData(pContext);

			CloseHandle(hFile);

			return;
		}

		if (dwLen < SIZEOF_IMAGE_HEADER)
		{
			errorType = ErrorType_Custom;
			dwErrorCode = CustomError_Image_Format_Error;
			bResult = FALSE;

			printf("[%s:%d]query copy image file %s format error with custom error code:0x%X\n"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);
			WriteLogFile(TRUE,"[%s:%d]query copy image file %s format error with custom error code:0x%X"
				,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename,dwErrorCode);

			queryImageOut.errType = errorType;
			queryImageOut.dwErrorCode = dwErrorCode;

			SendText(pContext,(char *)&queryImageOut,sizeof(CMD_OUT));

			bResult = FALSE;

			m_nThreadCount--;

			SetEvent(m_hThreadEnvent);

			RemoveClientData(clientData);
			RemoveIncompleteData(pContext);

			CloseHandle(hFile);

			return;
		}

		// 发送数据
		DWORD dwSndLength = sizeof(CMD_OUT) + SIZEOF_IMAGE_HEADER + 1;
		BYTE *pSend = new BYTE[dwSndLength];
		ZeroMemory(pSend,dwSndLength);
		pSend[dwSndLength - 1] = END_FLAG;

		queryImageOut.dwSizeSend = dwSndLength;

		memcpy(pSend,&queryImageOut,sizeof(CMD_OUT));
		memcpy(pSend + sizeof(CMD_OUT),&pkgHead,SIZEOF_IMAGE_HEADER);

		SendText(pContext,(char *)pSend,dwSndLength);
		delete []pSend;

		printf("[%s:%d]query image file %s success.\n"
			,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename);

		WriteLogFile(TRUE,"[%s:%d]query image file %s success."
			,::inet_ntoa(pContext->addrRemote.sin_addr),::ntohs(pContext->addrRemote.sin_port),filename);

		CloseHandle(hFile);

		RemoveClientData(clientData);
		RemoveIncompleteData(pContext);

		m_nThreadCount--;

		SetEvent(m_hThreadEnvent);

		return;

	}
}

void CServer::AddIncompleteData( IncompleteData *data )
{
	::EnterCriticalSection(&m_IncompleteDataLock);
	m_IncompleteDataVector.push_back(data);
	::LeaveCriticalSection(&m_IncompleteDataLock);
}

void CServer::AddIncompleteData( IncompleteData *data,char *buf,int len )
{
	if (data == NULL)
	{
		return;
	}

	::EnterCriticalSection(&data->Lock);

	DataBuf *dataBuf = new DataBuf;
	memset(dataBuf,0,sizeof(DataBuf));

	dataBuf->buf = buf;
	dataBuf->len = len;
	dataBuf->Next = NULL;

	if (data->dataBuf == NULL)
	{
		data->dataBuf = dataBuf;
	}
	else
	{
		// 找到队列末尾
		DataBuf *pre = data->dataBuf;
		while (pre->Next)
		{
			pre = pre->Next;
		}

		pre->Next = dataBuf;
	}


	::LeaveCriticalSection(&data->Lock);
}

void CServer::RemoveIncompleteData( IncompleteData *data )
{
	::EnterCriticalSection(&m_IncompleteDataLock);
	IncompleteDataIteractor it;

	for (it = m_IncompleteDataVector.begin();it != m_IncompleteDataVector.end();it++)
	{
		if (*it == data)
		{
			m_IncompleteDataVector.erase(it);

			::EnterCriticalSection(&data->Lock);
			DataBuf *head = data->dataBuf;
			while (head)
			{
				DataBuf *next = head->Next;

				if (head->buf)
				{
					delete []head->buf;
					head->buf = NULL;
				}

				delete head;
				head = NULL;

				head = next;
			}

			data->dataBuf = NULL;

			::EnterCriticalSection(&data->Lock);

			::DeleteCriticalSection(&data->Lock);
			delete []data;
			data = NULL;

			break;
		}
	}
	::LeaveCriticalSection(&m_IncompleteDataLock);
}

void CServer::RemoveIncompleteData( CIOCPContext *pContext )
{
	::EnterCriticalSection(&m_IncompleteDataLock);
	IncompleteDataIteractor it;

	for (it = m_IncompleteDataVector.begin();it != m_IncompleteDataVector.end();it++)
	{
		IncompleteData *data = *it;

		if (data == NULL)
		{
			continue;
		}

		if (data->pContext == pContext)
		{
			m_IncompleteDataVector.erase(it);

			::EnterCriticalSection(&data->Lock);
			DataBuf *head = data->dataBuf;
			while (head)
			{
				DataBuf *next = head->Next;

				if (head->buf)
				{
					delete []head->buf;
					head->buf = NULL;
				}

				delete head;
				head = NULL;

				head = next;
			}

			data->dataBuf = NULL;

			::EnterCriticalSection(&data->Lock);

			::DeleteCriticalSection(&data->Lock);
			delete []data;
			data = NULL;
		}
	}
	::LeaveCriticalSection(&m_IncompleteDataLock);
}

void CServer::CleanUpIncompleteData()
{
	::EnterCriticalSection(&m_IncompleteDataLock);

	IncompleteDataIteractor it;

	for (it = m_IncompleteDataVector.begin();it != m_IncompleteDataVector.end();it++)
	{
		IncompleteData *data = *it;	

		::EnterCriticalSection(&data->Lock);
		DataBuf *head = data->dataBuf;
		while (head)
		{
			DataBuf *next = head->Next;

			if (head->buf)
			{
				delete []head->buf;
				head->buf = NULL;
			}

			delete head;
			head = NULL;

			head = next;
		}

		data->dataBuf = NULL;

		delete []data;
		data = NULL;
		::EnterCriticalSection(&data->Lock);
	}
	m_IncompleteDataVector.clear();

	::LeaveCriticalSection(&m_IncompleteDataLock);
}

IncompleteData * CServer::GetIncompleteData( CIOCPContext *pContext )
{
	IncompleteData *data = NULL;
	::EnterCriticalSection(&m_IncompleteDataLock);
	IncompleteDataIteractor it;

	for (it = m_IncompleteDataVector.begin();it != m_IncompleteDataVector.end();it++)
	{
		data = *it;
		if (data->pContext == pContext)
		{
			break;
		}
	}
	::LeaveCriticalSection(&m_IncompleteDataLock);

	return data;
}

BOOL ReadFileAsyn( HANDLE hFile,
	ULONGLONG ullOffset,DWORD &dwSize,LPBYTE lpBuffer,LPOVERLAPPED lpOverlap,PDWORD pdwErrorCode )
{
	DWORD dwReadLen = 0;
	DWORD dwErrorCode = 0;

	if (lpOverlap)
	{
		lpOverlap->Offset = LODWORD(ullOffset);
		lpOverlap->OffsetHigh = HIDWORD(ullOffset);
	}

	if (!ReadFile(hFile,lpBuffer,dwSize,&dwReadLen,lpOverlap))
	{
		dwErrorCode = ::GetLastError();

		if(dwErrorCode == ERROR_IO_PENDING) // 结束异步I/O
		{
			if (WaitForSingleObject(lpOverlap->hEvent, INFINITE) != WAIT_FAILED)
			{
				if(!::GetOverlappedResult(hFile, lpOverlap, &dwReadLen, FALSE))
				{
					*pdwErrorCode = ::GetLastError();
					return FALSE;
				}
				else
				{
					dwSize = dwReadLen;
					return TRUE;
				}
			}
			else
			{
				*pdwErrorCode = ::GetLastError();
				return FALSE;
			}

		}
		else
		{
			*pdwErrorCode = dwErrorCode;
			return FALSE;
		}
	}
	else
	{
		dwSize = dwReadLen;
		return TRUE;
	}
}

BOOL WriteFileAsyn( HANDLE hFile,
	ULONGLONG ullOffset,DWORD &dwSize,LPBYTE lpBuffer,LPOVERLAPPED lpOverlap,PDWORD pdwErrorCode )
{
	DWORD dwWriteLen = 0;
	DWORD dwErrorCode = 0;

	if (lpOverlap)
	{
		lpOverlap->Offset = LODWORD(ullOffset);
		lpOverlap->OffsetHigh = HIDWORD(ullOffset);
	}


	if (!WriteFile(hFile,lpBuffer,dwSize,&dwWriteLen,lpOverlap))
	{
		dwErrorCode = ::GetLastError();

		if(dwErrorCode == ERROR_IO_PENDING) // 结束异步I/O
		{
			if (WaitForSingleObject(lpOverlap->hEvent, INFINITE) != WAIT_FAILED)
			{
				if(!::GetOverlappedResult(hFile, lpOverlap, &dwWriteLen, FALSE))
				{
					*pdwErrorCode = ::GetLastError();
					return FALSE;
				}
				else
				{
					dwSize = dwWriteLen;
					return TRUE;
				}
			}
			else
			{
				*pdwErrorCode = ::GetLastError();
				return FALSE;
			}

		}
		else
		{
			*pdwErrorCode = dwErrorCode;
			return FALSE;
		}
	}
	else
	{
		dwSize = dwWriteLen;
		return TRUE;
	}
}
