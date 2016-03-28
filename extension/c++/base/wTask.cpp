
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

wTask::~wTask() {}

void wTask::DeleteIO()
{
	SAFE_DELETE(mIO);	//mIO�����ڶ���
}

void wTask::CloseTask(int iReason)
{
	mStatus = TASK_QUIT;
	mIO->Close();
}

int wTask::TaskRecv()
{
	int iRecvLen = mIO->RecvBytes(mRecvMsgBuff + mRecvBytes, sizeof(mRecvMsgBuff) - mRecvBytes);
	
	if(iRecvLen <= 0)
	{
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
			return -1;
		}
		
		mRecvBytes = iBuffMsgLen;
		memmove(mRecvMsgBuff, pBuffer, iBuffMsgLen);	//����Ѵ�����Ϣ
	}
	
	return iRecvLen;
}

int wTask::TaskSend()
{
	int iSendLen = 0;
	int iMsgLen = 0;
	while(true)
	{
		iMsgLen = mSendWrite - mSendBytes;
		if(iMsgLen <= 0)
		{
			return 0;
		}
		
		iSendLen = mIO->SendBytes(mSendMsgBuff + mSendBytes, iMsgLen);
		if(iSendLen < 0)
		{
			return iSendLen;
		}
		mSendBytes += iSendLen;
		
	}
	
	if(mSendBytes > 0)
	{
		memmove(mSendMsgBuff, mSendMsgBuff + mSendBytes, mSendWrite - mSendBytes);	//����Ѵ�����Ϣ
		mSendWrite -= mSendBytes;
		mSendBytes = 0;
	}
	return mSendBytes;
}

int wTask::SendToBuf(const char *pCmd, int iLen)
{
	//�ж���Ϣ����
	if(iLen <= MIN_CLIENT_MSG_LEN || iLen > MAX_CLIENT_MSG_LEN )
	{
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
	mSendWrite += iMsgLen;

	return mSendWrite - mSendBytes;
}

int wTask::SyncSend(const char *pCmd, int iLen)
{
	//�ж���Ϣ����
	if(iLen < MIN_CLIENT_MSG_LEN || iLen > MAX_CLIENT_MSG_LEN )
	{
		return -1;
	}
	
	*(int *)mTmpSendMsgBuff = iLen;
	memcpy(mTmpSendMsgBuff + sizeof(int), pCmd, iLen);
	return mIO->SendBytes(mTmpSendMsgBuff, iLen + sizeof(int));
}

int wTask::SyncRecv(char *pCmd, int iLen, int iTimeout)
{
	int iSize = 0, iRecvLen = 0, iMsgLen = 0, iTryCount = 0; /*ÿ����Ϣ��౻��Ϊ���ٸ���*/
	long long iSleep = 5000;	//5ms

	iTryCount = iTimeout*1000000 / iSleep;
	memset(mTmpRecvMsgBuff, 0, sizeof(mTmpRecvMsgBuff));
	
	struct wCommand* pTmpCmd = 0;
	int iCmdMsgLen = sizeof(int) + sizeof(struct wCommand);

	do {
		iSize = mIO->RecvBytes(mTmpRecvMsgBuff + iRecvLen, iLen + sizeof(int));
		if (iSize <= 0)
		{
			break;
		}
		iRecvLen += iSize;

		if ((iRecvLen < iCmdMsgLen) && (iTryCount-- > 0))	//����һ����Ϣ�峤��
		{
			usleep(iSleep);
			continue;
		}

		pTmpCmd = (struct wCommand*) (mTmpRecvMsgBuff + sizeof(int));
		if (pTmpCmd != NULL && pTmpCmd->GetCmd() == CMD_NULL && pTmpCmd->GetPara() == PARA_NULL)	//���˵�����
		{
			memmove(mTmpRecvMsgBuff, mTmpRecvMsgBuff + iCmdMsgLen, iRecvLen - iCmdMsgLen);
			iRecvLen -= iCmdMsgLen;
		}
		
		if ((iRecvLen < iLen + sizeof(int)) && (iTryCount-- > 0))
		{
			usleep(iSleep);
			continue;
		}
		break;
	} while(true);
	
	iMsgLen = *(int *)mTmpRecvMsgBuff;
	if(iMsgLen < MIN_CLIENT_MSG_LEN || iMsgLen > MAX_CLIENT_MSG_LEN)
	{
		return -1;
	}

	if (iMsgLen > iRecvLen)	//��Ϣ������
	{
		return -1;
	}

	if (iMsgLen > iLen)
	{
		return -1;
	}
	memcpy(pCmd, mTmpRecvMsgBuff + sizeof(int), iLen);
	return iRecvLen - sizeof(int);
}
