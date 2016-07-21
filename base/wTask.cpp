
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wTask.h"

wTask::~wTask()
{
	SAFE_DELETE(mSocket);
}

int wTask::Heartbeat()
{
	mHeartbeatTimes++;
	struct wCommand vCmd;
	int iRet = SyncSend((char*)&vCmd, sizeof(vCmd));
	return iRet;
}

int wTask::TaskRecv()
{
	int iRecvLen = mSocket->RecvBytes(mRecvMsgBuff + mRecvBytes, sizeof(mRecvMsgBuff) - mRecvBytes);
	
	if (iRecvLen <= 0) return iRecvLen;
	mRecvBytes += iRecvLen;	

	char *pBuffer = mRecvMsgBuff;	//��ͷ��ʼ��ȡ
	int iBuffMsgLen = mRecvBytes;	//��Ϣ���ֽ���
	int iMsgLen = 0;
	
	while (true)
	{
		if ((size_t)iBuffMsgLen < sizeof(int)) break;

		iMsgLen = *(int *)pBuffer;	//������Ϣ�峤��

		//�ж���Ϣ����
		if ((iMsgLen < MIN_CLIENT_MSG_LEN) || (iMsgLen > MAX_CLIENT_MSG_LEN))
		{
			LOG_ERROR(ELOG_KEY, "[system] recv message invalid len: %d , fd(%d)", iMsgLen, mSocket->FD());
			return ERR_MSGLEN;
		}

		iBuffMsgLen -= iMsgLen + sizeof(int);	//buf�г�ȥ��ǰ������Ϣʣ�����ݳ���
		pBuffer += iMsgLen + sizeof(int);		//λ�Ƶ�������Ϣ����λ�õ�ַ
		
		//һ����������Ϣ
		if (iBuffMsgLen < 0)
		{
			//����buf��ʶλ�����´�ѭ����׼��
			iBuffMsgLen += iMsgLen + sizeof(int);
			pBuffer -= iMsgLen + sizeof(int);
			
			LOG_DEBUG(ELOG_KEY, "[system] recv a part of message: real len = %d, now len = %d", iMsgLen, iBuffMsgLen);
			break;
		}
		
		//ҵ���߼�
		HandleRecvMessage(pBuffer - iMsgLen, iMsgLen);	//ȥ��4�ֽ���Ϣ���ȱ�ʶλ
	}

	if (iBuffMsgLen == 0) //������������
	{
		mRecvBytes = 0;
	}
	else
	{
		//�ж�ʣ��ĳ���
		if (iBuffMsgLen < 0)
		{
			LOG_ERROR(ELOG_KEY, "[system] the last msg len %d is impossible fd(%d)", iBuffMsgLen, mSocket->FD());
			return ERR_MSGLEN;
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
		if (iMsgLen <= 0) return 0;
		
		iSendLen = mSocket->SendBytes(mSendMsgBuff + mSendBytes, iMsgLen);
		if (iSendLen < 0) return iSendLen;

		mSendBytes += iSendLen;
		
		LOG_DEBUG(ELOG_KEY, "[system] send message len: %d, fd(%d)", iMsgLen, mSocket->FD());
	}
	
	if (mSendBytes > 0)
	{
		memmove(mSendMsgBuff, mSendMsgBuff + mSendBytes, mSendWrite - mSendBytes);	//����Ѵ�����Ϣ
		mSendWrite -= mSendBytes;
		mSendBytes = 0;
	}
	return mSendBytes;
}

int wTask::Send2Buf(const char *pCmd, int iLen)
{
	//�ж���Ϣ����
	if ((iLen <= MIN_CLIENT_MSG_LEN) || (iLen > MAX_CLIENT_MSG_LEN))
	{
		LOG_ERROR(ELOG_KEY, "[system] write message invalid len %d, fd(%d)", iLen, mSocket->FD());
		return -1;
	}
	
	int iMsgLen = iLen + sizeof(int);
	if ((int)(sizeof(mSendMsgBuff) - mSendWrite + mSendBytes) < iMsgLen) //ʣ��ռ䲻��
	{
		LOG_ERROR(ELOG_KEY, "[system] send buf not enough. send(%d) need(%d)", sizeof(mSendMsgBuff) - mSendWrite + mSendBytes, iMsgLen);
		return -2;
	}
	else if ((int)(sizeof(mSendMsgBuff) - mSendWrite) < iMsgLen) //д��ռ䲻��
	{
		memmove(mSendMsgBuff, mSendMsgBuff + mSendBytes, mSendWrite - mSendBytes);	//����Ѵ�����Ϣ
		mSendWrite -= mSendBytes;
		mSendBytes = 0;
	}
	
	*(int *)(mSendMsgBuff + mSendWrite) = iLen;
	memcpy(mSendMsgBuff + mSendWrite + sizeof(int), pCmd, iLen);
	mSendWrite += iMsgLen;

	LOG_DEBUG(ELOG_KEY, "[system] write message to buf(%d - %d), len(%d)", mSendWrite, mSendBytes, iLen);

	return mSendWrite - mSendBytes;
}

int wTask::SyncSend(const char *pCmd, int iLen)
{
	//�ж���Ϣ����
	if ((iLen < MIN_CLIENT_MSG_LEN) || (iLen > MAX_CLIENT_MSG_LEN))
	{
		LOG_ERROR(ELOG_KEY, "[system] send message invalid len %d, fd(%d)", iLen, mSocket->FD());
		return -1;
	}
	
	*(int *)mTmpSendMsgBuff = iLen;
	memcpy(mTmpSendMsgBuff + sizeof(int), pCmd, iLen);
	return mSocket->SendBytes(mTmpSendMsgBuff, iLen + sizeof(int));
}

int wTask::SyncRecv(char *pCmd, int iLen, int iTimeout)
{
	long long iSleep = 100;	//100us
	int iTryCount = iTimeout*1000000/iSleep;

	memset(mTmpRecvMsgBuff, 0, sizeof(mTmpRecvMsgBuff));
	int iCmdMsgLen = sizeof(int) + sizeof(struct wCommand);
	
	struct wCommand* pTmpCmd = 0;
	int iSize = 0, iRecvLen = 0;
	do {
		iSize = mSocket->RecvBytes(mTmpRecvMsgBuff + iRecvLen, iLen + sizeof(int));
		if (iSize < 0) break;
		iRecvLen += iSize;
		if ((iSize == 0) || (iRecvLen < iCmdMsgLen))
		{
			if (iTryCount-- < 0) break;
			
			usleep(iSleep);
			continue;
		}

		pTmpCmd = (struct wCommand*) (mTmpRecvMsgBuff + sizeof(int));	//���������ܳ����ڿ�ͷ
		if (pTmpCmd != NULL && pTmpCmd->GetCmd() == CMD_NULL && pTmpCmd->GetPara() == PARA_NULL)
		{
			iRecvLen -= iCmdMsgLen;
			memmove(mTmpRecvMsgBuff, mTmpRecvMsgBuff + iCmdMsgLen, iRecvLen);
		}
		
		if (((size_t)iRecvLen < iLen + sizeof(int)) && (iTryCount-- > 0))
		{
			usleep(iSleep);
			continue;
		}
		break;
	} while (true);
	
	int iMsgLen = *(int *)mTmpRecvMsgBuff;
	if ((iRecvLen <= 0) || (iMsgLen < MIN_CLIENT_MSG_LEN) || (iMsgLen > MAX_CLIENT_MSG_LEN))
	{
		LOG_ERROR(ELOG_KEY, "[system] sync recv message invalid len: %d, fd(%d)", iMsgLen, mSocket->FD());
		return ERR_MSGLEN;
	}

	if (iMsgLen > (int)(iRecvLen - sizeof(int)))	//��Ϣ������
	{
		LOG_DEBUG(ELOG_KEY, "[system] sync recv a part of message: real len = %d, now len = %d, call len = %d", iMsgLen, iRecvLen, iLen);
		return ERR_MSGLEN;
	}

	if (iMsgLen > iLen)
	{
		LOG_DEBUG(ELOG_KEY, "[system] sync recv error buffer len, it\'s to short!");
		return ERR_MSGLEN;
	}
	memcpy(pCmd, mTmpRecvMsgBuff + sizeof(int), iLen);
	return iRecvLen - sizeof(int);
}
