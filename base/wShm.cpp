
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */
 
#include "wShm.h"

wShm::wShm(const char *filename, int pipeid, size_t size)
{
	Initialize();

	mPipeId = pipeid;
	mSize = size;
	if(mPagesize > 0)
	{
		mSize = ALIGN(size, mPagesize);
	}
	
	//����ͷ������
	mSize += sizeof(shmhead_t);
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
	LOG_DEBUG(ELOG_KEY, "[runtime] try to alloc %lld bytes of share memory", mSize);
	
	int fd = open(mFilename, O_CREAT);
	if (fd < 0)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] open file(%s) failed: %s", mFilename, strerror(errno));
		return NULL;
	}
	close(fd);

	mKey = ftok(mFilename, mPipeId);
	if (mKey < 0) 
	{
		LOG_ERROR(ELOG_KEY, "[runtime] create memory (ftok) failed: %s", strerror(errno));
		return NULL;
	}

	//���빲���ڴ�
	mShmId = shmget(mKey, mSize, IPC_CREAT| IPC_EXCL| 0666);
	
	//��������ڴ�ʧ��
	if (mShmId < 0) 
	{
		if (errno != EEXIST) 
		{
			LOG_ERROR(ELOG_KEY, "[runtime] alloc share memory failed: %s", strerror(errno));
			return 0;
		}

		LOG_DEBUG(ELOG_KEY, "[runtime] share memory is exist now, try to attach it");

		//������ڴ��Ѿ������룬��������ʿ�����
		mShmId = shmget(mKey, mSize, 0666);

		//���ʧ��
		if (mShmId < 0) 
		{
			LOG_DEBUG(ELOG_KEY, "[runtime] attach to share memory failed: %s, try to touch it", strerror(errno));
			
			//�²��Ƿ��Ǹ��ڴ��С̫С���Ȼ�ȡ�ڴ�ID
			mShmId = shmget(mKey, 0, 0666);
			
			//���ʧ�ܣ����޷��������ڴ棬ֻ���˳�
			if (mShmId < 0) 
			{
				LOG_ERROR(ELOG_KEY, "[runtime] touch to share memory failed: %s", strerror(errno));
				return 0;
			}
			else 
			{
				LOG_DEBUG(ELOG_KEY, "[runtime] remove the exist share memory %d", mShmId);

				//����ɹ�������ɾ��ԭ�ڴ�
				if (shmctl(mShmId, IPC_RMID, NULL) < 0) 
				{
					LOG_ERROR(ELOG_KEY, "[runtime] remove share memory failed: %s", strerror(errno));
					return 0;
				}

				//�ٴ������ID���ڴ�
				mShmId = shmget(mKey, mSize, IPC_CREAT|IPC_EXCL|0666);
				if (mShmId < 0) 
				{
					LOG_ERROR(ELOG_KEY, "[runtime] alloc share memory failed again: %s", strerror(errno));
					return 0;
				}
			}
		}
		else
		{
			LOG_DEBUG(ELOG_KEY, "[runtime] attach to share memory succeed");
		}
	}

	LOG_DEBUG(ELOG_KEY, "[runtime] alloc %lld bytes of share memory succeed", mSize);
	
	char *pAddr = (char *)shmat(mShmId, NULL, 0);
    if (pAddr == (char *)-1) 
	{
		LOG_ERROR(ELOG_KEY, "[runtime] shmat failed: %s", strerror(errno));
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
	LOG_DEBUG(ELOG_KEY, "[runtime] try to attach %lld bytes of share memory", mSize);
	
	//����Ҫ���빲���ڴ��keyֵ�������
	mKey = ftok(mFilename, mPipeId);
	if (mKey < 0) 
	{
		LOG_ERROR(ELOG_KEY, "create memory (ftok) failed: %s", strerror(errno));
		return 0;
	}

	// ���Ի�ȡ
	int mShmId = shmget(mKey, mSize, 0666);
	if(mShmId < 0) 
	{
		LOG_ERROR(ELOG_KEY, "attach to share memory failed: %s", strerror(errno));
		return 0;
	}
	
	char *pAddr = (char *)shmat(mShmId, NULL, 0);
    if (pAddr == (char *) -1) 
	{
		LOG_ERROR(ELOG_KEY, "shmat() failed: %s", strerror(errno));
		return 0;
    }
	
    //shmͷ
	mShmhead = (struct shmhead_t*) pAddr;
	mShmhead->mStart = pAddr;
	mShmhead->mEnd = pAddr + mSize;
	mShmhead->mUsedOff = pAddr + sizeof(struct shmhead_t);
	return mShmhead->mUsedOff;
}

char *AllocShm(size_t iLen)
{
	if(mShmhead.mUsedOff + iLen < mShmhead.mEnd)
	{
		char *pAddr = mShmhead.mUsedOff;
		mShmhead.mUsedOff += iLen;
		return pAddr;
	}

	LOG_ERROR(ELOG_KEY, "alloc shm failed: shm space not enough");
	return 0;
}

void wShm::FreeShm()
{
	if(mShmhead == 0 || mShmhead.mStart == 0)
	{
		return;
	}

	//�Թ�����������������shmid_ds��ý��̹���������
    if (shmdt(mShmhead.mStart) == -1)
	{
		LOG_ERROR(ELOG_KEY, "shmdt(%d) failed", mShmhead.mStart);
    }
	
	//ɾ����shmid_ds����洢��
    if (shmctl(mShmId, IPC_RMID, NULL) == -1)
	{
		LOG_ERROR(ELOG_KEY, "remove share memory failed: %s", strerror(errno));
    }
	//unlink(mFilename);
}
