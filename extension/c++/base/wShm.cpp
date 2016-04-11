
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */
 
#include "wShm.h"

wShm::wShm(const char *filename, int pipeid, size_t size)
{
	Initialize();

	mPipeId = pipeid;
	mSize = size + sizeof(struct shmhead_t);
	if(mPagesize > 0)
	{
		mSize = ALIGN(mSize, mPagesize);
	}
	
	memcpy(mFilename, filename, strlen(filename) +1);
}

void wShm::Initialize()
{
	mPagesize = getpagesize();
	memset(mFilename, 0, sizeof(mFilename));
	mPipeId = 0;
	mSize = 0;
	mShmId = 0;
	mKey = 0;
	mShmhead = NULL;
}

wShm::~wShm()
{
	FreeShm();
}

char *wShm::CreateShm()
{	
	int iFD = open(mFilename, O_CREAT);
	if (iFD < 0)
	{
		return NULL;
	}
	close(iFD);

	mKey = ftok(mFilename, mPipeId);
	if (mKey < 0) 
	{
		return NULL;
	}

	//���빲���ڴ�
	mShmId = shmget(mKey, mSize, IPC_CREAT| IPC_EXCL| 0666);
	
	//��������ڴ�ʧ��
	if (mShmId < 0) 
	{
		if (errno != EEXIST) 
		{
			return 0;
		}

		//������ڴ��Ѿ������룬��������ʿ�����
		mShmId = shmget(mKey, mSize, 0666);

		//���ʧ��
		if (mShmId < 0) 
		{			
			//�²��Ƿ��Ǹ��ڴ��С̫С���Ȼ�ȡ�ڴ�ID
			mShmId = shmget(mKey, 0, 0666);
			
			//���ʧ�ܣ����޷��������ڴ棬ֻ���˳�
			if (mShmId < 0) 
			{
				return 0;
			}
			else 
			{
				//����ɹ�������ɾ��ԭ�ڴ�
				if (shmctl(mShmId, IPC_RMID, NULL) < 0) 
				{
					return 0;
				}

				//�ٴ������ID���ڴ�
				mShmId = shmget(mKey, mSize, IPC_CREAT|IPC_EXCL|0666);
				if (mShmId < 0) 
				{
					return 0;
				}
			}
		}
	}
	
	char *pAddr = (char *)shmat(mShmId, NULL, 0);
    if (pAddr == (char *)-1) 
	{
		return 0;
    }

    //shmͷ
	mShmhead = (struct shmhead_t*) pAddr;
	mShmhead->mStart = pAddr;
	mShmhead->mEnd = pAddr + mSize;
	mShmhead->mUsedOff = pAddr + sizeof(struct shmhead_t);
	return mShmhead->mUsedOff;
}

char *wShm::AttachShm()
{	
	//����Ҫ���빲���ڴ��keyֵ�������
	mKey = ftok(mFilename, mPipeId);
	if (mKey < 0) 
	{
		return 0;
	}

	// ���Ի�ȡ
	int mShmId = shmget(mKey, mSize, 0666);
	if(mShmId < 0) 
	{
		return 0;
	}
	
	char *pAddr = (char *)shmat(mShmId, NULL, 0);
    if (pAddr == (char *) -1) 
	{
		return 0;
    }
	
    //shmͷ
	mShmhead = (struct shmhead_t*) pAddr;
	return mShmhead->mUsedOff;
}

char *wShm::AllocShm(size_t size)
{
	if (mShmhead == NULL)
	{
		return NULL;
	}

	if(mShmhead != NULL && mShmhead->mUsedOff + size < mShmhead->mEnd)
	{
		char *pAddr = mShmhead->mUsedOff;
		mShmhead->mUsedOff += size;
		memset(pAddr, 0, size);
		return pAddr;
	}
	return NULL;
}

void wShm::FreeShm()
{	
	if(mShmhead == NULL || mShmhead->mStart == NULL)
	{
		return;
	}

	//�Թ�����������������shmid_ds��ý��̹���������
    if (shmdt(mShmhead->mStart) == -1)
	{
    }
	
	//ɾ����shmid_ds����洢�Σ�ȫ�����̽����Ż�����ɾ����
    if (shmctl(mShmId, IPC_RMID, NULL) == -1)
	{
    }
	//unlink(mFilename);
}
