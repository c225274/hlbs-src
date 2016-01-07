
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */
 
#include "wShareMemory.h"

wShareMemory::wShareMemory(const char *filename, int pipeid = 'i', size_t size)
{
	mFilename = filename;
	mPipeId = pipeid;
	mSize = size;
}

wShareMemory::wShareMemory()
{
	RemoveShareMemory();
}

char *wShareMemory::CreateShareMemory()
{
	LOG_DEBUG("default", "try to alloc %lld bytes of share memory", mSize);
	//open(mFilename,);
	
	mKey = ftok(mFilename, mPipeId);
	if (mKey < 0) 
	{
		LOG_ERROR("default", "create memory (ftok) failed: %s", strerror(errno));
		return 0;
	}

	//���빲���ڴ�
	mShmId = shmget(mKey, mSize, IPC_CREAT|IPC_EXCL|0666);
	
	//��������ڴ�ʧ��
	if (mShmId < 0) 
	{
		if (errno != EEXIST) 
		{
			LOG_ERROR("default", "Alloc share memory failed: %s", strerror(errno));
			return 0;
		}

		LOG_DEBUG("default", "share memory is exist now, try to attach it");

		//������ڴ��Ѿ������룬��������ʿ�����
		mShmId = shmget(mKey, mSize, 0666);

		//���ʧ��
		if (mShmId < 0) 
		{
			LOG_DEBUG("default", "attach to share memory failed: %s, try to touch it", strerror(errno));
			
			//�²��Ƿ��Ǹ��ڴ��С̫С���Ȼ�ȡ�ڴ�ID
			mShmId = shmget(mKey, 0, 0666);
			
			//���ʧ�ܣ����޷��������ڴ棬ֻ���˳�
			if (mShmId < 0) 
			{
				LOG_ERROR("default", "touch to share memory failed: %s", strerror(errno));
				return 0;
			}
			else 
			{
				LOG_DEBUG("default", "remove the exist share memory %d", mShmId);

				//����ɹ�������ɾ��ԭ�ڴ�
				if (shmctl(mShmId, IPC_RMID, NULL) < 0) 
				{
					LOG_ERROR("default", "remove share memory failed: %s", strerror(errno));
					return 0;
				}

				//�ٴ������ID���ڴ�
				mShmId = shmget(mKey, mSize, IPC_CREAT|IPC_EXCL|0666);
				if (mShmId < 0) 
				{
					LOG_ERROR("default", "alloc share memory failed again: %s", strerror(errno));
					return 0;
				}
			}
		}
		else
		{
			LOG_DEBUG("default", "attach to share memory succeed");
		}
	}

	LOG_INFO("default", "alloc %lld bytes of share memory succeed", mSize);
	
	mAddr = (char *)shmat(mShmId, NULL, 0);
    if (mAddr == (void *) -1) 
	{
		LOG_ERROR("default", "shmat() failed: %s", strerror(errno));
		return 0;
    }
	
	/*
    if (shmctl(mShmId, IPC_RMID, NULL) == -1) 
	{
		LOG_ERROR("default", "remove share memory failed: %s", strerror(errno));
		return 0;
    }
	*/
	return mAddr;
}

char *wShareMemory::AttachShareMemory()
{
	//����Ҫ���빲���ڴ��keyֵ�������
	mKey = ftok(mFilename, mPipeId);
	if (mKey < 0) 
	{
		LOG_ERROR("default", "create memory (ftok) failed: %s", strerror(errno));
		return 0;
	}

	// ���Ի�ȡ
	int mShmId = shmget(mKey, size, 0666);
	if( mShmId < 0 ) 
	{
		LOG_ERROR("default", "attach to share memory failed: %s", strerror(errno));
		return 0;
	}
	
	mAddr = (char *)shmat(mShmId, NULL, 0);
    if (mAddr == (void *) -1) 
	{
		LOG_ERROR("default", "shmat() failed: %s", strerror(errno));
		return 0;
    }
	
	/*
    if (shmctl(mShmId, IPC_RMID, NULL) == -1) 
	{
		LOG_ERROR("default", "remove share memory failed: %s", strerror(errno));
		return 0;
    }
	*/
	return mAddr;
}

void wShareMemory::RemoveShareMemory()
{
	if(mAddr ==0)
	{
		return;
	}
    if (shmdt(mAddr) == -1) 
	{
		LOG_ERROR("default", "shmdt(%d) failed", mAddr);
    }
	/*
    if (shmctl(mShmId, IPC_RMID, NULL) == -1) 
	{
		LOG_ERROR("default", "remove share memory failed: %s", strerror(errno));
    }
	*/
}
