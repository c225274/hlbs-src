
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wTask.h"

wTask::wTask()
{
	Initialize();
}

wTask::wTask(wIO *pIO)
{
	Initialize();
	mIO = pIO;
}

void wTask::Initialize()
{
	mIO = NULL;
	mStatus = TASK_INIT;
	mHeartbeatTimes = 0;
	mRecvBytes = 0;
	mSendBytes = 0;
	mSendWrite = 0;
	memset(&mSendMsgBuff, 0, sizeof(mSendMsgBuff));
	memset(&mRecvMsgBuff, 0, sizeof(mRecvMsgBuff));
}

wTask::~wTask()
{
	SAFE_DELETE(mIO);
}

void wTask::CloseTask(int iReason)
{
	mStatus = TASK_QUIT;
	mIO->Close();
}

int wTask::TaskRecv()
{
	int iRecvLen = mIO->RecvBytes(mRecvMsgBuff + mRecvBytes, sizeof(mRecvMsgBuff) - mRecvBytes);
	
	LOG_DEBUG(ELOG_KEY, "recv data len: %d , %s", iRecvLen, mRecvMsgBuff);
	if(iRecvLen <= 0)
	{
		LOG_ERROR(ELOG_KEY, "recv data invalid len: %d , %s", iRecvLen, mRecvMsgBuff);
		return iRecvLen;	
	}
	mRecvBytes += iRecvLen;	

	char *pBuffer = mRecvMsgBuff;	//��ͷ��ʼ��ȡ
	int iBuffMsgLen = mRecvBytes;	//��Ϣ���ֽ���
	int iMsgLen = 0;
	
	while(true)
	{
		if(iBuffMsgLen < sizeof(int))
		{
			break;
		}

		iMsgLen = *(int *)pBuffer;	//������Ϣ�峤��

		//�ж���Ϣ����
		if(iMsgLen < MIN_CLIENT_MSG_LEN || iMsgLen > MAX_CLIENT_MSG_LEN )
		{
			LOG_ERROR(ELOG_KEY, "recv message invalid len: %d , fd(%d)", iMsgLen, mIO->IOFD());
			return -1;
		}

		iBuffMsgLen -= iMsgLen + sizeof(int);	//buf�г�ȥ��ǰ������Ϣʣ�����ݳ���
		pBuffer += iMsgLen + sizeof(int);		//λ�Ƶ�������Ϣ����λ�õ�ַ
		
		//һ����������Ϣ
		if(iBuffMsgLen < 0)
		{
			//����buf��ʶλ�����´�ѭ����׼��
			iBuffMsgLen += iMsgLen + sizeof(int);
			pBuffer -= iMsgLen + sizeof(int);
			
			LOG_DEBUG(ELOG_KEY, "recv a part of message: real len = %d, now len = %d", iMsgLen, iBuffMsgLen);
			break;
		}
		
		//ҵ���߼�
		HandleRecvMessage(pBuffer - iMsgLen, iMsgLen);	//ȥ��4�ֽ���Ϣ���ȱ�ʶλ
	}

	if(iBuffMsgLen == 0) //������������
	{
		mRecvBytes = 0;
	}
	else
	{
		//�ж�ʣ��ĳ���
		if(iBuffMsgLen < 0)
		{
			LOG_ERROR(ELOG_KEY, "the last msg len %d is impossible fd(%d)", iBuffMsgLen, mIO->IOFD());
			return -1;
		}
		
		mRecvBytes = iBuffMsgLen;
		memmove(mRecvMsgBuff, pBuffer, iBuffMsgLen);	//����Ѵ�����Ϣ
	}
	
	return iRecvLen;
}

int wTask::TaskSend()
{
	while(true)
	{
		int iMsgLen = mSendWrite - mSendBytes;
		
		if(iMsgLen <= 0)
		{
			return 0;
		}
		
		int iSendLen = mIO->SendBytes(mSendMsgBuff + mSendBytes, iMsgLen);
		if(iSendLen < 0)
		{
			return iSendLen;
		}
		
		if(iSendLen < iMsgLen)
		{
			mSendBytes += iSendLen;
			continue;
		}
		
		LOG_DEBUG(ELOG_KEY, "send message len: %d, fd(%d)", iMsgLen, mIO->IOFD());
	}
	
	int iSendLen = mSendBytes;
	if(mSendBytes > 0)
	{
		memmove(mSendMsgBuff, mSendMsgBuff + mSendBytes, mSendWrite - mSendBytes);	//����Ѵ�����Ϣ
		mSendWrite -= mSendBytes;
		mSendBytes = 0;
	}
	return iSendLen;
}

int wTask::WriteToSendBuf(const char *pCmd, int iLen)
{
	//�ж���Ϣ����
	if(iLen <= MIN_CLIENT_MSG_LEN || iLen > MAX_CLIENT_MSG_LEN )
	{
		LOG_ERROR(ELOG_KEY, "write message invalid len %d, fd(%d)", iLen, mIO->IOFD());
		return -1;
	}
	
	int iMsgLen = iLen + sizeof(int);
	if(sizeof(mSendMsgBuff) - mSendWrite + mSendBytes < iMsgLen) //ʣ��ռ䲻��
	{
		return -2;
	}
	else if(sizeof(mSendMsgBuff) - mSendWrite < iMsgLen) //д��ռ䲻��
	{
		memmove(mSendMsgBuff, mSendMsgBuff + mSendBytes, mSendWrite - mSendBytes);	//����Ѵ�����Ϣ
		mSendWrite -= mSendBytes;
		mSendBytes = 0;
	}
	
	*(int *)(mSendMsgBuff + mSendWrite)= iLen;
	memcpy(mSendMsgBuff + mSendWrite + sizeof(int), pCmd, iLen);
	return 0;
}

int wTask::SyncSend(const char *pCmd, int iLen)
{
	//�ж���Ϣ����
	if(iLen < MIN_CLIENT_MSG_LEN || iLen > MAX_CLIENT_MSG_LEN )
	{
		LOG_ERROR(ELOG_KEY, "send message invalid len %d, fd(%d)", iLen, mIO->IOFD());
		return -1;
	}
	
	*(int *)mTmpSendMsgBuff = iLen;
	memcpy(mTmpSendMsgBuff + sizeof(int), pCmd, iLen);
	return mIO->SendBytes(mTmpSendMsgBuff, iLen + sizeof(int));
}

int wTask::SyncRecv(char *pCmd, int iLen)
{
	int iRecvLen = 0, iMsgLen = 0;
	struct wCommand* pTmpCmd = 0;
	
	memset(mTmpRecvMsgBuff, 0, sizeof(mTmpRecvMsgBuff));
	do
	{
		iRecvLen = mIO->RecvBytes(mTmpRecvMsgBuff, iLen + sizeof(int));
		if(iRecvLen <= 0)
		{
			LOG_ERROR(ELOG_KEY, "recv data invalid len:%d ,fd(%d)", iRecvLen, mIO->IOFD());
			return iRecvLen;	
		}
		//���˵�����
		pTmpCmd = (struct wCommand*) mTmpRecvMsgBuff;
	} while(pTmpCmd != NULL && pTmpCmd->GetCmd() == CMD_NULL && pTmpCmd->GetPara() == PARA_NULL);
	
	iMsgLen = *(int *)mTmpRecvMsgBuff;
	if(iMsgLen < MIN_CLIENT_MSG_LEN || iMsgLen > MAX_CLIENT_MSG_LEN)
	{
		LOG_ERROR(ELOG_KEY, "get message invalid len: %d, fd(%d)", iMsgLen, mIO->IOFD());
		return -1;
	}

	if (iMsgLen > iRecvLen)	//��Ϣ������
	{
		LOG_DEBUG(ELOG_KEY, "recv a part of message: real len = %d, now len = %d", iMsgLen, iRecvLen);
		return -1;
	}

	if (iMsgLen > iLen)
	{
		LOG_DEBUG(ELOG_KEY, "error buffer len, it\'s to short!");
		return -1;
	}
	memcpy(pCmd, mTmpRecvMsgBuff + sizeof(int), iLen);
	return iRecvLen - sizeof(int);
}
