
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
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
	LOG_DEBUG(ELOG_KEY, "[runtime] try to alloc %lld bytes of share memory", mSize);
	
	int iFD = open(mFilename, O_CREAT);
	if (iFD < 0)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] open file(%s) failed: %s", mFilename, strerror(errno));
		return NULL;
	}
	close(iFD);

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
		LOG_ERROR(ELOG_KEY, "[runtime] create memory (ftok) failed: %s", strerror(errno));
		return 0;
	}

	// ���Ի�ȡ
	int mShmId = shmget(mKey, mSize, 0666);
	if(mShmId < 0) 
	{
		LOG_ERROR(ELOG_KEY, "[runtime] attach to share memory failed: %s", strerror(errno));
		return 0;
	}
	
	char *pAddr = (char *)shmat(mShmId, NULL, 0);
    if (pAddr == (char *) -1) 
	{
		LOG_ERROR(ELOG_KEY, "[runtime] shmat() failed: %s", strerror(errno));
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
		LOG_ERROR(ELOG_KEY, "[runtime] shm need exec function CreateShm or AttachShm first");
		return NULL;
	}

	if(mShmhead != NULL && mShmhead->mUsedOff + size < mShmhead->mEnd)
	{
		char *pAddr = mShmhead->mUsedOff;
		mShmhead->mUsedOff += size;
		memset(pAddr, 0, size);
		return pAddr;
	}
	LOG_ERROR(ELOG_KEY, "[runtime] alloc(%d) shm failed,shm space not enough, real free(%d)", size, mShmhead->mEnd - mShmhead->mUsedOff);
	return NULL;
}

void wShm::FreeShm()
{
	LOG_DEBUG(ELOG_KEY, "[runtime] free %lld bytes of share memory", mSize);
	
	if(mShmhead == NULL || mShmhead->mStart == NULL)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] free shm failed: shm head illegal");
		return;
	}

	//�Թ�����������������shmid_ds��ý��̹���������
    if (shmdt(mShmhead->mStart) == -1)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] shmdt(%d) failed", mShmhead->mStart);
    }
	
	//ɾ����shmid_ds����洢�Σ�ȫ�����̽����Ż�����ɾ����
    if (shmctl(mShmId, IPC_RMID, NULL) == -1)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] remove share memory failed: %s", strerror(errno));
    }
	//unlink(mFilename);
}
