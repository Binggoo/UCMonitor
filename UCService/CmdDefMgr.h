#pragma once
#include <Windows.h>

#ifndef DEF_COMMU_CMD
#define DEF_COMMU_CMD const unsigned long
#endif

// ��������
typedef enum _ENUM_ERROR_TYPE
{
	ErrorType_System,
	ErrorType_Custom
}ErrorType;

//�Զ������
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

#pragma pack(pop)
/************************************************************************/
/* COMMAND:CMD_SYNC_TIME
 * <Input>:
 * DEF_COMMU_CMD	CMD_SYNC_TIME_IN;
 * DWORD dwSizeSend; //���͵��ֽ���;
 * <Output>:
 * DWORD dwSizeOfReturn; //���������ܴ�С��
 * DEF_COMMU_CMD	CMD_SYNC_TIME_OUT;
 * WORD wYmdHMS[4]; //������ʱ����*/
/************************************************************************/
DEF_COMMU_CMD CMD_SYNC_TIME_IN    = 0x43440000;
DEF_COMMU_CMD CMD_SYNC_TIME_OUT   = 0x43440100;

/************************************************************************/
/* COMMAND:CMD_COPY_IMAGE
 * <Input>:
 * DEF_COMMU_CMD CMD_COPY_IMAGE_IN;
 * DWORD dwSizeSend; //���͵��ֽ���;
 * BYTE byStop; //ֹͣ��־
 * char *pszImageName; //IMAGE����
 * <Output>:
 * DWORD dwSizeOfReturn; //���������ܴ�С��
 * DEF_COMMU_CMD CMD_COPY_IMAGE_OUT;
 * DWORD ErrorType; //��������
 * DWORD dwErrorCode; //������룬0��ʾ�ɹ�
 * BYTE *pContext; // IMAGE����
 * BYTE byFlag; //��ʼ������־����ʼ��BE,������ED*/
/************************************************************************/
DEF_COMMU_CMD CMD_COPY_IMAGE_IN   = 0x43440001;
DEF_COMMU_CMD CMD_COPY_IMAGE_OUT  = 0x43440101;

/************************************************************************/
/* COMMAND:CMD_MAKE_IMAGE
 * <Input>:
 * DEF_COMMU_CMD CMD_MAKE_IMAGE_IN;
 * DWORD dwSizeSend; //���͵��ֽ���;
 * BYTE byStop; //ֹͣ��־
 * char *pszImageName; //IMAGE���� 0�ַ���β
 * BYTE *pContext; //IMAGE����
 * BYTE byFlag; //��ʼ������־����ʼ��BE,������ED
 * <Output>:
 * DWORD dwSizeOfReturn; //���������ܴ�С��
 * DEF_COMMU_CMD CMD_MAKE_IMAGE_OUT;
 * DWORD ErrorType; //��������
 * DWORD dwErrorCode; //������룬0��ʾ�ɹ� */
/************************************************************************/
DEF_COMMU_CMD CMD_MAKE_IMAGE_IN   = 0x43440002;
DEF_COMMU_CMD CMD_MAKE_IMAGE_OUT  = 0x43440102;

/************************************************************************/
/* COMMAND:CMD_UPLOAD_LOG
 * <Input>:
 * DEF_COMMU_CMD CMD_UPLOAD_LOG_IN;
 * DWORD dwSizeSend; //���͵��ֽ���;
 * char *pszLogName; //LOG���ƣ�0�ַ���β
 * BYTE *pContext; //LOG�ļ�����
 * BYTE byFlag; //��ʼ������־����ʼ��BE,������ED
 * <Output>:
 * DWORD dwSizeOfReturn; //���������ܴ�С��
 * DEF_COMMU_CMD CMD_UPLOAD_LOG_OUT;
 * DWORD ErrorType; //��������
 * DWORD dwErrorCode; //������룬0��ʾ�ɹ� */
/************************************************************************/
DEF_COMMU_CMD CMD_UPLOAD_LOG_IN   = 0x43440003;
DEF_COMMU_CMD CMD_UPLOAD_LOG_OUT  = 0x43440103;

/************************************************************************/
/* COMMAND:CMD_QUERY_IMAGE
 * <Input>:
 * DEF_COMMU_CMD CMD_QUERY_IMAGE_IN;
 * DWORD dwSizeSend; //���͵��ֽ���;
 * char *pszImageName; //IMAGE����
 * <Output>:
 * DWORD dwSizeOfReturn; //���������ܴ�С��
 * DEF_COMMU_CMD CMD_QUERY_IMAGE_OUT;
 * DWORD ErrorType; //��������
 * DWORD dwErrorCode; //������룬0��ʾ�ɹ�
 * BYTE head[1024]; // 0�ַ���β
/************************************************************************/
DEF_COMMU_CMD CMD_QUERY_IMAGE_IN  = 0x43440004;
DEF_COMMU_CMD CMD_QUERY_IMAGE_OUT = 0x43440104;

/************************************************************************/
/* COMMAND:CMD_SHOW_SCREEN
 * <Input>:
 * DEF_COMMU_CMD CMD_SHOW_SCREEN_IN;
 * DWORD dwSizeSend; //���͵��ֽ���;
 * BYTE *bitmap
 * BYTE byFlag; //��ʼ������־����ʼ��BE,������ED
 * <Output>:
 * DWORD dwSizeOfReturn; //���������ܴ�С��
 * DEF_COMMU_CMD CMD_SHOW_SCREEN_OUT;
 * DWORD ErrorType; //��������
 * DWORD dwErrorCode; //������룬0��ʾ�ɹ�
/************************************************************************/
DEF_COMMU_CMD CMD_SHOW_SCREEN_IN  = 0x43440005;
DEF_COMMU_CMD CMD_SHOW_SCREEN_OUT = 0x43440105;

