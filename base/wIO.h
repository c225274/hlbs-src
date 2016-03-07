
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_IO_H_
#define _W_IO_H_

#include "wType.h"
#include "wLog.h"
#include "wMisc.h"
#include "wNoncopyable.h"

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
	SOCK_LISTEN = 0,
	SOCK_CONNECT,
	SOCK_UNIX,
};

enum IO_TYPE
{
	TYPE_UNKNOWN = -1,
	TYPE_SOCK,	//tcp|udp|unix socket
	TYPE_FILE,
	TYPE_BUF,
	TYPE_SHM,
};

enum IO_FLAG
{
	FLAG_UNKNOWN = -1,
    FLAG_RECV,
    FLAG_SEND,
	FLAG_RVSD
};

class wIO : private wNoncopyable
{
	public:
		wIO();
		void Initialize();
		virtual ~wIO();
		
		int &FD() { return mFD; }
		IO_TYPE &IOType() { return mIOType; }
		IO_FLAG &IOFlag() { return mIOFlag; }
		
		SOCK_TYPE &SockType() { return mSockType;}
		SOCK_STATUS &SockStatus() { return mSockStatus;}
		
		unsigned long long &RecvTime() { return mRecvTime; }
		unsigned long long &SendTime() { return mSendTime; }
		unsigned long long &CreateTime() { return mCreateTime; }

		virtual string &Host() { return mHost; }
		virtual unsigned short &Port() { return mPort; }

		//30s
		virtual int SetTimeout(int iTimeout = 30) {}
		virtual int SetSendTimeout(int iTimeout = 30) {} 
		virtual int SetRecvTimeout(int iTimeout = 30) {}
		
		virtual int SetNonBlock(bool bNonblock = true);
		
		virtual int Open();
		virtual void Close();
		
		virtual ssize_t RecvBytes(char *vArray, size_t vLen) = 0;
		virtual ssize_t SendBytes(char *vArray, size_t vLen) = 0;
		
	protected:
		int mFD;
		IO_TYPE mIOType;
		IO_FLAG mIOFlag;
		
		SOCK_TYPE mSockType;
		SOCK_STATUS mSockStatus;

		string mHost;
		unsigned short mPort;
		
		unsigned long long mRecvTime;	//�����յ����ݰ���ʱ���������
		unsigned long long mSendTime;	//��������ݰ�ʱ�������Ҫ�û�������⣩������
		unsigned long long mCreateTime;	//����ʱ�䣬����
};

#endif
