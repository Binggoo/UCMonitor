#pragma once
#include <Windows.h>

#ifndef DEF_COMMU_CMD
#define DEF_COMMU_CMD const unsigned long
#endif

// 错误类型
typedef enum _ENUM_ERROR_TYPE
{
	ErrorType_System,
	ErrorType_Custom
}ErrorType;

//自定义错误
typedef enum _ENUM_CUSTOM_ERROR
{
	CustomError_OK = 0,
	CustomError_Cancel,
	CustomError_No_Master,
	CustomError_No_Target,
	CustomError_Compare_Failed,
	CustomError_All_Targets_Failed,
	CustomError_Master_Failed,
	CustomError_Image_Format_Error,
	CustomError_Compress_Error,
	CustomError_UnCompress_Error,
	CustomError_Speed_Too_Slow,
	CustomError_Unrecognized_Partition,
	CustomError_No_File_Select,
	CustomError_Target_Small,
	CustomError_Format_Error

}CustomError;

#pragma pack(push, 1)
typedef struct _STRUCT_CMD_IN
{
	DWORD dwCmdIn;
	DWORD dwSizeSend;
	BYTE  byStop;
}CMD_IN,*PCMD_IN;

typedef struct _STRUCT_CMD_OUT
{
	DWORD dwCmdOut;
	DWORD dwSizeSend;
	ErrorType errType;
	DWORD dwErrorCode;
}CMD_OUT,*PCMD_OUT;

typedef struct _STRUCT_CMD_SYNC_TIME_IN
{
	DWORD dwCmdIn;
	DWORD dwSizeSend;
	BYTE  byStop;
}SYNC_TIME_IN,*PSYNC_TIME_IN;

typedef struct _STRUCT_CMD_SYNC_TIME_OUT
{
	DWORD dwCmdOut;
	DWORD dwSizeSend;
	WORD wYmdHMS[6];
}SYNC_TIME_OUT,*PSYNC_TIME_OUT;

typedef struct _STRUCT_CMD_COPY_IMAGE_IN
{
	DWORD dwCmdIn;
	DWORD dwSizeSend;
	BYTE  byStop;
	char  *pszImageName;
	BYTE  byFlag;

}COPY_IMAGE_IN,*PCOPY_IMAGE_IN;

typedef struct _STRUCT_CMD_COPY_IMAGE_OUT
{
	DWORD dwCmdOut;
	DWORD dwSizeSend;
	ErrorType errType;
	DWORD dwErrorCode;
	BYTE  *pContext;
	BYTE  byFlag;
	
}COPY_IMAGE_OUT,*PCOPY_IMAGE_OUT;

typedef struct _STRUCT_CMD_MAKE_IMAGE_IN
{
	DWORD dwCmdIn;
	DWORD dwSizeSend;
	BYTE  byStop;
	char  *pszImageName;
	BYTE  *pContext;
	BYTE  byFlag;

}MAKE_IMAGE_IN,*PMAKE_IMAGE_IN;

typedef struct _STRUCT_CMD_MAKE_IMAGE_OUT
{
	DWORD dwCmdOut;
	DWORD dwSizeSend;
	ErrorType errType;
	DWORD dwErrorCode;
}MAKE_IMAGE_OUT,*PMAKE_IMAGE_OUT;

typedef struct _STRUCT_CMD_UPLOAD_LOG_IN
{
	DWORD dwCmdIn;
	DWORD dwSizeSend;
	BYTE  byStop;
	char  *pszImageName;
	BYTE  *pContext;
	BYTE  byFlag;

}UPLOAD_LOG_IN,*PUPLOAD_LOG_IN;

typedef struct _STRUCT_CMD_UPLOAD_LOG_OUT
{
	DWORD dwCmdOut;
	DWORD dwSizeSend;
	ErrorType errType;
	DWORD dwErrorCode;
}UPLOAD_LOG_OUT,*PUPLOAD_LOG_OUT;

typedef struct _STRUCT_CMD_QUERY_IMAGE_IN
{
	DWORD dwCmdIn;
	DWORD dwSizeSend;
	BYTE  byStop;
	char  *pszImageName;
	BYTE  byFlag;
}QUERY_IMAGE_IN,*PQUERY_IMAGE_IN;

typedef struct _STRUCT_CMD_QUERY_IMAGE_OUT
{
	DWORD dwCmdOut;
	DWORD dwSizeSend;
	ErrorType errType;
	DWORD dwErrorCode;
	BYTE  *pContext;
	BYTE  byFlag;
}QUERY_IMAGE_OUT,*PQUERY_IMAGE_OUT;

// 2014-10-11新增
typedef struct _STRUCT_CMD_DOWN_PKG_IN
{
	DWORD dwCmdIn;
	DWORD dwSizeSend;
	BYTE  byStop;
	char  *pszImageName;
	BYTE  byFlag;
}DOWN_PKG_IN,DOWN_LIST_IN,*PDOWN_PKG_IN,*PDOWN_LIST_IN;

typedef struct _STRUCT_CMD_DOWN_PKG_OUT
{
	DWORD dwCmdOut;
	DWORD dwSizeSend;
	ErrorType errType;
	DWORD dwErrorCode;
	char  *pszFileName;
	BYTE  *pContext;
	BYTE  byFlag;
}DOWN_PKG_OUT,*PDOWN_PKG_OUT;

typedef struct _STRUCT_CMD_DOWN_LIST_OUT
{
	DWORD dwCmdOut;
	DWORD dwSizeSend;
	ErrorType errType;
	DWORD dwErrorCode;
	BYTE  *pContext;
	BYTE  byFlag;
}DOWN_LIST_OUT,*PDOWN_LIST_OUT;

typedef struct _STRUCT_CMD_USER_LOG_IN
{
	DWORD dwCmdIn;
	DWORD dwSizeSend;
	BYTE  byStop;
	char  *pszImageName;
	BYTE  *pContext;
	BYTE  byFlag;
}USER_LOG_IN,*PUSER_LOG_IN;

typedef struct _STRUCT_CMD_USER_LOG_OUT
{
	DWORD dwCmdOut;
	DWORD dwSizeSend;
	ErrorType errType;
	DWORD dwErrorCode;
}USER_LOG_OUT,*PUSER_LOG_OUT;

typedef struct _STRUCT_CMD_QUERY_PKG_IN
{
	DWORD dwCmdIn;
	DWORD dwSizeSend;
	BYTE  byStop;
	char  *pszPkgName;
	BYTE  byFlag;
}QUERY_PKG_IN,QUERY_LIST_IN,*PQUERY_PKG_IN,*PQUERY_LIST_IN;

typedef struct _STRUCT_CMD_QUERY_PKG_OUT
{
	DWORD dwCmdOut;
	DWORD dwSizeSend;
	ErrorType errType;
	DWORD dwErrorCode;
	ULONGLONG ullFileSize;
	BYTE  byFlag;
}QUERY_PKG_OUT,QUERY_LIST_OUT,*PQUERY_PKG_OUT,*PQUERY_LIST_OUT;

#pragma pack(pop)
/************************************************************************/
/* COMMAND:CMD_SYNC_TIME
 * <Input>:
 * DEF_COMMU_CMD	CMD_SYNC_TIME_IN;
 * DWORD dwSizeSend; //发送的字节数;
 * <Output>:
 * DWORD dwSizeOfReturn; //返回数据总大小。
 * DEF_COMMU_CMD	CMD_SYNC_TIME_OUT;
 * WORD wYmdHMS[4]; //年月日时分秒*/
/************************************************************************/
DEF_COMMU_CMD CMD_SYNC_TIME_IN    = 0x43440000;
DEF_COMMU_CMD CMD_SYNC_TIME_OUT   = 0x43440100;

/************************************************************************/
/* COMMAND:CMD_COPY_IMAGE
 * <Input>:
 * DEF_COMMU_CMD CMD_COPY_IMAGE_IN;
 * DWORD dwSizeSend; //发送的字节数;
 * BYTE byStop; //停止标志
 * char *pszImageName; //IMAGE名称
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED
 * <Output>:
 * DWORD dwSizeOfReturn; //返回数据总大小。
 * DEF_COMMU_CMD CMD_COPY_IMAGE_OUT;
 * DWORD ErrorType; //错误类型
 * DWORD dwErrorCode; //错误代码，0表示成功
 * BYTE *pContext; // IMAGE内容
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED*/
/************************************************************************/
DEF_COMMU_CMD CMD_COPY_IMAGE_IN   = 0x43440001;
DEF_COMMU_CMD CMD_COPY_IMAGE_OUT  = 0x43440101;

/************************************************************************/
/* COMMAND:CMD_MAKE_IMAGE
 * <Input>:
 * DEF_COMMU_CMD CMD_MAKE_IMAGE_IN;
 * DWORD dwSizeSend; //发送的字节数;
 * BYTE byStop; //停止标志
 * char *pszImageName; //IMAGE名称 0字符结尾
 * BYTE *pContext; //IMAGE内容
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED
 * <Output>:
 * DWORD dwSizeOfReturn; //返回数据总大小。
 * DEF_COMMU_CMD CMD_MAKE_IMAGE_OUT;
 * DWORD ErrorType; //错误类型
 * DWORD dwErrorCode; //错误代码，0表示成功 */
/************************************************************************/
DEF_COMMU_CMD CMD_MAKE_IMAGE_IN   = 0x43440002;
DEF_COMMU_CMD CMD_MAKE_IMAGE_OUT  = 0x43440102;

/************************************************************************/
/* COMMAND:CMD_UPLOAD_LOG
 * <Input>:
 * DEF_COMMU_CMD CMD_UPLOAD_LOG_IN;
 * DWORD dwSizeSend; //发送的字节数;
 * char *pszLogName; //LOG名称，0字符结尾
 * BYTE *pContext; //LOG文件内容
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED
 * <Output>:
 * DWORD dwSizeOfReturn; //返回数据总大小。
 * DEF_COMMU_CMD CMD_UPLOAD_LOG_OUT;
 * DWORD ErrorType; //错误类型
 * DWORD dwErrorCode; //错误代码，0表示成功 */
/************************************************************************/
DEF_COMMU_CMD CMD_UPLOAD_LOG_IN   = 0x43440003;
DEF_COMMU_CMD CMD_UPLOAD_LOG_OUT  = 0x43440103;

/************************************************************************/
/* COMMAND:CMD_QUERY_IMAGE
 * <Input>:
 * DEF_COMMU_CMD CMD_QUERY_IMAGE_IN;
 * DWORD dwSizeSend; //发送的字节数;
 * char *pszImageName; //IMAGE名称
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED
 * <Output>:
 * DWORD dwSizeOfReturn; //返回数据总大小。
 * DEF_COMMU_CMD CMD_QUERY_IMAGE_OUT;
 * DWORD ErrorType; //错误类型
 * DWORD dwErrorCode; //错误代码，0表示成功
 * BYTE head[1024]; // 
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED */
/************************************************************************/
DEF_COMMU_CMD CMD_QUERY_IMAGE_IN  = 0x43440004;
DEF_COMMU_CMD CMD_QUERY_IMAGE_OUT = 0x43440104;

/************************************************************************/
/* COMMAND:CMD_SHOW_SCREEN
 * <Input>:
 * DEF_COMMU_CMD CMD_SHOW_SCREEN_IN;
 * DWORD dwSizeSend; //发送的字节数;
 * BYTE *bitmap
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED
 * <Output>:
 * DWORD dwSizeOfReturn; //返回数据总大小。
 * DEF_COMMU_CMD CMD_SHOW_SCREEN_OUT;
 * DWORD ErrorType; //错误类型
 * DWORD dwErrorCode; //错误代码，0表示成功
/************************************************************************/
DEF_COMMU_CMD CMD_SHOW_SCREEN_IN  = 0x43440005;
DEF_COMMU_CMD CMD_SHOW_SCREEN_OUT = 0x43440105;

/************************************************************************/
/* COMMAND:CMD_DOWN_PKG
 * <Input>:
 * DEF_COMMU_CMD CMD_DOWN_PKG_IN;
 * DWORD dwSizeSend; //发送的字节数;
 * BYTE byStop; //停止标志
 * char *pszPkgName; //Package名称
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED
 * <Output>:
 * DWORD dwSizeOfReturn; //返回数据总大小。
 * DEF_COMMU_CMD CMD_DOWN_PKG_OUT;
 * DWORD ErrorType; //错误类型
 * DWORD dwErrorCode; //错误代码，0表示成功
 * ULONGLONG ullFileSize;//Package文件大小
 * BYTE *pContext; // Package内容
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED*/
/************************************************************************/
DEF_COMMU_CMD CMD_DOWN_PKG_IN  = 0x43440006;
DEF_COMMU_CMD CMD_DOWN_PKG_OUT = 0x43440106;

/************************************************************************/
/* COMMAND:CMD_USER_LOG
 * <Input>:
 * DEF_COMMU_CMD CMD_USER_LOG_IN;
 * DWORD dwSizeSend; //发送的字节数;
 * char *pszLogName; //LOG名称，0字符结尾
 * BYTE *pContext; //LOG文件内容
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED
 * <Output>:
 * DWORD dwSizeOfReturn; //返回数据总大小。
 * DEF_COMMU_CMD CMD_USER_LOG_OUT;
 * DWORD ErrorType; //错误类型
 * DWORD dwErrorCode; //错误代码，0表示成功 */
/************************************************************************/
DEF_COMMU_CMD CMD_USER_LOG_IN   = 0x43440007;
DEF_COMMU_CMD CMD_USER_LOG_OUT  = 0x43440107;

/************************************************************************/
/* COMMAND:CMD_QUERY_PKG
 * <Input>:
 * DEF_COMMU_CMD CMD_QUERY_PKG_IN;
 * DWORD dwSizeSend; //发送的字节数;
 * char *pszPkgName; //IMAGE名称
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED
 * <Output>:
 * DWORD dwSizeOfReturn; //返回数据总大小。
 * DEF_COMMU_CMD CMD_QUERY_PKG_OUT;
 * DWORD ErrorType; //错误类型
 * DWORD dwErrorCode; //错误代码，0表示成功
 * ULONGLONG ullFileSize; //文件大小
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED */
/************************************************************************/
DEF_COMMU_CMD CMD_QUERY_PKG_IN  = 0x43440008;
DEF_COMMU_CMD CMD_QUERY_PKG_OUT = 0x43440108;

/************************************************************************/
/* COMMAND:CMD_DOWN_LIST
 * <Input>:
 * DEF_COMMU_CMD CMD_DOWN_LIST_IN;
 * DWORD dwSizeSend; //发送的字节数;
 * BYTE byStop; //停止标志
 * char *pszPkgName; //Package名称
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED
 * <Output>:
 * DWORD dwSizeOfReturn; //返回数据总大小。
 * DEF_COMMU_CMD CMD_DOWN_LIST_OUT;
 * DWORD ErrorType; //错误类型
 * DWORD dwErrorCode; //错误代码，0表示成功
 * ULONGLONG ullFileSize;//Package文件大小
 * BYTE *pContext; // Package内容
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED*/
/************************************************************************/
DEF_COMMU_CMD CMD_DOWN_LIST_IN  = 0x43440009;
DEF_COMMU_CMD CMD_DOWN_LIST_OUT = 0x43440109;

/************************************************************************/
/* COMMAND:CMD_QUERY_LIST
 * <Input>:
 * DEF_COMMU_CMD CMD_QUERY_LIST_IN;
 * DWORD dwSizeSend; //发送的字节数;
 * char *pszPkgName; //IMAGE名称
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED
 * <Output>:
 * DWORD dwSizeOfReturn; //返回数据总大小。
 * DEF_COMMU_CMD CMD_QUERY_LIST_OUT;
 * DWORD ErrorType; //错误类型
 * DWORD dwErrorCode; //错误代码，0表示成功
 * ULONGLONG ullFileSize; //文件大小
 * BYTE byFlag; //开始结束标志，开始：BE,结束：ED */
/************************************************************************/
DEF_COMMU_CMD CMD_QUERY_LIST_IN  = 0x4344000A;
DEF_COMMU_CMD CMD_QUERY_LIST_OUT = 0x4344010A;


// IMAGE define
#define LEN_DIGEST 64
#define SIZEOF_IMAGE_HEADER 1024
#define SIZEOF_IMAGE_DISCRP 256
#define IMAGE_FLAG "PY-IMG"
#define END_FLAG 0xED
#define START_FLAG 0xBE
#define APP_VER_LENGTH						(16)	// this is the length of the FILE_HANDLER_VER
#define PKG_HEADER_SIZE  12
#define ZIP_VERSION   "1.28"
#define APP_VERSION   "1.0.0"

#pragma pack(push, 1)
typedef struct _STRUCT_IMAGE_HEADER
{
	char      szImageFlag[8];  //8
	ULONGLONG ullImageSize;    //8
	char	  szAppVersion[APP_VER_LENGTH]; //16
	DWORD	  dwImageType;  //4
	DWORD	  dwMaxSizeOfPackage; //4
	ULONGLONG ullCapacitySize; //8
	DWORD     dwBytesPerSector; //4
	char	  szZipVer[APP_VER_LENGTH]; //16
	ULONGLONG ullPkgCount; //8
	ULONGLONG ullValidSize; //8
	DWORD     dwHashLen; //4
	DWORD	  dwHashType; //4
	BYTE	  byImageDigest[LEN_DIGEST]; // 64
	char      szDescription[SIZEOF_IMAGE_DISCRP]; //256
	DWORD     dwPkgOffset; // 4
	DWORD     dwFolderCount; // 4
	DWORD     dwFileCount; // 4
	DWORD     dwListSize; // 4
	BYTE      byReserved[SIZEOF_IMAGE_HEADER-8*5-4*9-APP_VER_LENGTH*2-LEN_DIGEST-SIZEOF_IMAGE_DISCRP-1]; //595
	BYTE      byEnd; //1
}IMAGE_HEADER,*PIMAGE_HEADER;

#pragma pack(pop)