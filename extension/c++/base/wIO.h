
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _W_IO_H_
#define _W_IO_H_

#include "wCore.h"
#include "wMisc.h"
#include "wNoncopyable.h"

enum IO_TYPE
{
	TYPE_UNKNOWN = -1,
	TYPE_SOCK,	//tcp|udp|http|unix
	TYPE_FILE,
	TYPE_BUF,
	TYPE_SHM
};

enum IO_FLAG
{
	FLAG_UNKNOWN = -1,
    FLAG_RECV,
    FLAG_SEND,
	FLAG_RVSD	//�շ�
};

/**	TASK_HTTPʹ��TASK_TCP����Э��*/
enum TASK_TYPE
{
	TASK_UNKNOWN = -1,
	TASK_UDP,
	TASK_TCP,
	TASK_UNIXS,
	TASK_UNIXD,
	TASK_HTTP
};

/**	δ��ʵ��������~*/
enum SOCK_STATUS
{
	STATUS_UNKNOWN = -1,
	STATUS_UNCONNECT,	//�ر�״̬
	STATUS_CONNECTED,	//�Ѿ���������
	STATUS_LISTEN		//�Ѿ����ڼ���״̬
};

//�׽���
enum SOCK_TYPE
{
	SOCK_UNKNOWN = -1,
	SOCK_LISTEN = 0,	//����sock
	SOCK_CONNECT		//����sock
};

class wIO : private wNoncopyable
{
	public:
		wIO();
		void Initialize();
		virtual ~wIO();
		
		int &FD() { return mFD; }
		int &Errno() { return mErr; }
		IO_TYPE &IOType() { return mIOType; }
		IO_FLAG &IOFlag() { return mIOFlag; }
		
		SOCK_TYPE &SockType() { return mSockType;}
		SOCK_STATUS &SockStatus() { return mSockStatus;}
		
		TASK_TYPE &TaskType() { return mTaskType;}
		
		unsigned long long &RecvTime() { return mRecvTime; }
		unsigned long long &SendTime() { return mSendTime; }
		unsigned long long &CreateTime() { return mCreateTime; }

		virtual string &Host() { return mHost; }
		virtual unsigned short &Port() { return mPort; }

		//30s
		virtual int SetTimeout(float fTimeout = 30) { return -1; }
		virtual int SetSendTimeout(float fTimeout = 30) { return -1; } 
		virtual int SetRecvTimeout(float fTimeout = 30) { return -1; }
		
		virtual int SetNonBlock(bool bNonblock = true);
		
		virtual int Open();
		virtual void Close();
		
		virtual ssize_t RecvBytes(char *vArray, size_t vLen) = 0;
		virtual ssize_t SendBytes(char *vArray, size_t vLen) = 0;
		
	protected:
		int mFD;
		int mErr;
		IO_TYPE mIOType;
		IO_FLAG mIOFlag;
		
		SOCK_TYPE mSockType;
		SOCK_STATUS mSockStatus;
		
		TASK_TYPE mTaskType;
		
		string mHost;
		unsigned short mPort;
		
		unsigned long long mRecvTime;	//�����յ����ݰ���ʱ���������
		unsigned long long mSendTime;	//��������ݰ�ʱ�������Ҫ�û�������⣩������
		unsigned long long mCreateTime;	//����ʱ�䣬����
};

#endif
