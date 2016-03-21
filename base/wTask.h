
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
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
		wTask();
		wTask(wIO *pIO);
		void Initialize();
		virtual ~wTask();
		
		wIO *IO() { return mIO; }
		void DeleteIO();
		TASK_STATUS &Status() { return mStatus; }
		bool IsRunning() { return mStatus == TASK_RUNNING; }
		
		virtual void CloseTask(int iReason);	//iReason�ر�ԭ��
		virtual int VerifyConn() { return 0;}	//��֤���յ�����
		virtual int Verify() {return 0;}		//����������֤����
		
		virtual int Heartbeat() { return 0; }
		virtual int HeartbeatOutTimes() { return 0; }
		
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
		virtual int WriteToSendBuf(const char *pCmd, int iLen);
		/**
		 * ���ͻ�����������
		 */
		int IsWritting() { return mSendWrite - mSendBytes; }
		virtual int TaskSend();
		
		/**
		 *  ͬ������ȷ�г�����Ϣ
		 */
		int SyncSend(const char *pCmd, int iLen);
		/**
		 *  ͬ������ȷ�г�����Ϣ
		 *  ���������Ϊ����ģʽ�����øú������Ͼ�ֻ�г�ʱ��(30s)���߽��ܲ�������Ϣ�ų���
		 *  ȷ��pCmd���㹻���Ŀռ�����Դ�ͬ����Ϣ
		 */
		int SyncRecv(char *pCmd, int iLen);
		
		virtual int HandleRecvMessage(char * pBuffer, int nLen) = 0 ; //ҵ���߼���ں���
		
	protected:
		wIO	*mIO;
		TASK_STATUS mStatus;
		int mHeartbeatTimes;
		
		//������Ϣ�Ļ����� 32M
		int mRecvBytes;	//���յ��ֽ���
		char mRecvMsgBuff[MAX_RECV_BUFFER_LEN];	
		
		//������Ϣʱ����ʱ������ 32M
		int mSendBytes;						//�ѷ����ֽ����������̸߳��£�
		int mSendWrite;						//���ͻ��屻д���ֽ�����д���̸߳��£�
		char mSendMsgBuff[MAX_SEND_BUFFER_LEN];
		
		char mTmpSendMsgBuff[MAX_CLIENT_MSG_LEN + sizeof(int)];	//ͬ�����ͣ���ʱ������
		char mTmpRecvMsgBuff[MAX_CLIENT_MSG_LEN + sizeof(int)];	//ͬ�����ܣ���ʱ������
};

#endif

