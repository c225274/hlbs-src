
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _W_TASK_H_
#define _W_TASK_H_

#include "wCore.h"
#include "wCommand.h"
#include "wIO.h"
#include "wLog.h"
#include "wMisc.h"
#include "wNoncopyable.h"

class wTask : private wNoncopyable
{
	public:
		wTask() {}
		wTask(wIO *pIO) : mIO(pIO) {}
		virtual ~wTask() {}
		
		wIO *IO() { return mIO; }
		void DeleteIO();
		TASK_STATUS &Status() { return mStatus; }
		bool IsRunning() { return mStatus == TASK_RUNNING; }
		
		virtual void CloseTask(int iReason);	//iReason�ر�ԭ��
		virtual int VerifyConn() { return 0;}	//��֤���յ�����
		virtual int Verify() {return 0;}		//����������֤����
		
		virtual int Heartbeat();
		virtual int HeartbeatOutTimes() { return mHeartbeatTimes > KEEPALIVE_CNT; }
		virtual int ClearbeatOutTimes() { return mHeartbeatTimes = 0; }
		/**
		 *  ������ܵ�����
		 *  ÿ����Ϣ��С[2b,128k)
		 *  �����߼�������������Ϣ��Ȼ������û������ҵ����HandleRecvMessage
		 *  return ��<0 �Զ˷�������|��Ϣ���� =0 �Զ˹ر�(FIN_WAIT1) >0 �����ַ�
		 */
		virtual int TaskRecv();
		/**
		 *  �첽���Ϳͻ�����Ϣ
		 *  return 
		 *  -1 ����Ϣ���Ȳ��Ϸ�
		 *  -2 �����ͻ���ʣ��ռ䲻�㣬���Ժ�����
		 *   0 : ���ͳɹ�
		 */
		virtual int SendToBuf(const char *pCmd, int iLen);
		/**
		 * ���ͻ�����������
		 */
		int WritableLen() { return mSendWrite - mSendBytes; }
		virtual int TaskSend();
		
		/**
		 *  ͬ������ȷ�г�����Ϣ
		 */
		int SyncSend(const char *pCmd, int iLen);
		/**
		 *  ͬ������ȷ�г�����Ϣ(�豣֤��sockδ����epoll�У���ֹ���־�������)
		 *  ȷ��pCmd���㹻���Ŀռ�����Դ�ͬ����Ϣ
		 */
		int SyncRecv(char *pCmd, int iLen, int iTimeout = 10/*s*/);
		
		//ҵ���߼���ں���
		virtual int HandleRecvMessage(char * pBuffer, int nLen) {}
		
	protected:
		wIO	*mIO {NULL};
		TASK_STATUS mStatus {TASK_INIT};
		int mHeartbeatTimes {0};
		
		//������Ϣ�Ļ����� 32M
		int mRecvBytes {0};	//���յ��ֽ���
		char mRecvMsgBuff[MAX_RECV_BUFFER_LEN] {'\0'};	
		
		//������Ϣʱ����ʱ������ 32M
		int mSendBytes {0};						//�ѷ����ֽ����������̸߳��£�
		int mSendWrite {0};						//���ͻ��屻д���ֽ�����д���̸߳��£�
		char mSendMsgBuff[MAX_SEND_BUFFER_LEN] {'\0'};
		
		char mTmpSendMsgBuff[MAX_CLIENT_MSG_LEN + sizeof(int)] {'\0'};	//ͬ�����ͣ���ʱ������
		char mTmpRecvMsgBuff[MAX_CLIENT_MSG_LEN + sizeof(int)] {'\0'};	//ͬ�����ܣ���ʱ������
};

#endif

