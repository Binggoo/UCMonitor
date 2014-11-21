#pragma once
#include <vector>
#include "IOCP.H"
#include "CmdDefMgr.h"

struct DataBuf
{
	char *buf;
	int len;
	DataBuf *Next;
};

struct IncompleteData
{
	CIOCPContext *pContext;
	DWORD dwTotalSize;
	DWORD dwRemainSize;
	DataBuf *dataBuf;   // 用于存放不完整数据
	CRITICAL_SECTION Lock;
};

struct ClientData
{
	CIOCPContext *pContext;
	DWORD dwCmd;
	char *szFileName;
	volatile BOOL bEnd;
	volatile BOOL bStop;
	CRITICAL_SECTION Lock;
	DataBuf *dataBuf;
};

typedef std::vector<ClientData *> VecClientData;
typedef std::vector<ClientData *>::iterator ClientDataIteractor;

typedef std::vector<IncompleteData *> VecIncompleteData;
typedef std::vector<IncompleteData *>::iterator IncompleteDataIteractor;

typedef std::vector<char *> VecString;
typedef std::vector<char *>::iterator StringIteractor;

class CServer : public CIOCPServer
{
public:
	CServer();
	~CServer(void);

	void SetPath(HANDLE hLogFile,char *szImagePath,char *szRecordPath);
	void SetOtherPath(char *szPackagePath,char *szCustomLogPath);

private:
	VecClientData m_ClientDataVector;
	VecIncompleteData m_IncompleteDataVector; // 用于记录不完整数据命令。
	CRITICAL_SECTION m_ClientDataLock;
	CRITICAL_SECTION m_IncompleteDataLock;
	char m_szImagePath[MAX_PATH];
	char m_szRecordPath[MAX_PATH];
	char m_szPackagePath[MAX_PATH];
	char m_szCustomLogPath[MAX_PATH];
	HANDLE m_hThreadEnvent;

	volatile int m_nThreadCount;

	// 建立了一个新的连接
	virtual void OnConnectionEstablished(CIOCPContext *pContext, CIOCPBuffer *pBuffer);
	// 一个连接关闭
	virtual void OnConnectionClosing(CIOCPContext *pContext, CIOCPBuffer *pBuffer);
	// 在一个连接上发生了错误
	virtual void OnConnectionError(CIOCPContext *pContext, CIOCPBuffer *pBuffer, int nError);
	// 一个连接上的读操作完成
	virtual void OnReadCompleted(CIOCPContext *pContext, CIOCPBuffer *pBuffer);
	// 一个连接上的写操作完成
	virtual void OnWriteCompleted(CIOCPContext *pContext, CIOCPBuffer *pBuffer);

	void AddClientData(ClientData *clientData);
	void RemoveClientData(ClientData *clientData);
	void CleanupClientData();
	ClientData *GetClientData(CIOCPContext *pContext,DWORD dwCmd);

	void AddDataBuf(ClientData *clientData,char *buf,int len);
	void RemoveHeadDataBuf(ClientData *clientData);
	void ReleaseAllDataBuf(ClientData *clientData);
	void ReleaseDataBuf(DataBuf *dataBuf);
	DataBuf *GetHeadDataBuf(ClientData *clientData);

	void AddIncompleteData(IncompleteData *data);
	void AddIncompleteData(IncompleteData *data,char *buf,int len);
	void RemoveIncompleteData(IncompleteData *data);
	void RemoveIncompleteData(CIOCPContext *pContext);
	void CleanUpIncompleteData();
	IncompleteData *GetIncompleteData(CIOCPContext *pContext);

	static DWORD WINAPI CommandThreadProc(LPVOID lpParm);
	void HandleCommand(CIOCPContext *pContext,DWORD dwCmd);

	ULONGLONG EnumFile(const char *folder,VecString &vecFiles);
	void CleanVecString(VecString *pvecFiles);

};

typedef struct _STRUCT_VOID_PARM
{
	LPVOID lpVoid1;  //this
	LPVOID lpVoid2;  //CIOCPContext *
	DWORD  dwCmd;
}VOID_PARM,*LPVOID_PARM;

BOOL ReadFileAsyn(
	HANDLE hFile,
	ULONGLONG ullOffset,
	DWORD &dwSize,
	LPBYTE lpBuffer,
	LPOVERLAPPED lpOverlap,
	PDWORD pdwErrorCode);
BOOL WriteFileAsyn(
	HANDLE hFile,
	ULONGLONG ullOffset,
	DWORD &dwSize,
	LPBYTE lpBuffer,
	LPOVERLAPPED lpOverlap,
	PDWORD pdwErrorCode);

