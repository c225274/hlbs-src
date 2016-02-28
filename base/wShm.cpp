
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */
 
#include "wShm.h"

wShm::wShm(const char *filename, int pipeid, size_t size)
{
	mPipeId = pipeid;
	int pagesize = getpagesize();
	if(pagesize > 0)
	{
		mSize = ALIGN(size, pagesize);
	}
	else
	{
		mSize = size;
	}
	memcpy(mFilename, filename, strlen(filename)+1);
}

wShm::~wShm()
{
	RemoveShm();
}

char *wShm::CreateShm()
{
	LOG_DEBUG("default", "[runtime] try to alloc %lld bytes of share memory", mSize);
	
	int fd = open(mFilename, O_CREAT);
	if (fd < 0)
	{
		LOG_ERROR("error", "[runtime] open file(%s) failed: %s", mFilename, strerror(errno));
		return 0;
	}
	close(fd);

	mKey = ftok(mFilename, mPipeId);
	if (mKey < 0) 
	{
		LOG_ERROR("error", "[runtime] create memory (ftok) failed: %s", strerror(errno));
		return 0;
	}

	//���빲���ڴ�
	mShmId = shmget(mKey, mSize, IPC_CREAT|IPC_EXCL|0666);
	
	//��������ڴ�ʧ��
	if (mShmId < 0) 
	{
		if (errno != EEXIST) 
		{
			LOG_ERROR("error", "[runtime] alloc share memory failed: %s", strerror(errno));
			return 0;
		}

		LOG_DEBUG("default", "[runtime] share memory is exist now, try to attach it");

		//������ڴ��Ѿ������룬��������ʿ�����
		mShmId = shmget(mKey, mSize, 0666);

		//���ʧ��
		if (mShmId < 0) 
		{
			LOG_DEBUG("default", "[runtime] attach to share memory failed: %s, try to touch it", strerror(errno));
			
			//�²��Ƿ��Ǹ��ڴ��С̫С���Ȼ�ȡ�ڴ�ID
			mShmId = shmget(mKey, 0, 0666);
			
			//���ʧ�ܣ����޷��������ڴ棬ֻ���˳�
			if (mShmId < 0) 
			{
				LOG_ERROR("default", "[runtime] touch to share memory failed: %s", strerror(errno));
				return 0;
			}
			else 
			{
				LOG_DEBUG("default", "[runtime] remove the exist share memory %d", mShmId);

				//����ɹ�������ɾ��ԭ�ڴ�
				if (shmctl(mShmId, IPC_RMID, NULL) < 0) 
				{
					LOG_ERROR("default", "[runtime] remove share memory failed: %s", strerror(errno));
					return 0;
				}

				//�ٴ������ID���ڴ�
				mShmId = shmget(mKey, mSize, IPC_CREAT|IPC_EXCL|0666);
				if (mShmId < 0) 
				{
					LOG_ERROR("default", "[runtime] alloc share memory failed again: %s", strerror(errno));
					return 0;
				}
			}
		}
		else
		{
			LOG_DEBUG("default", "[runtime] attach to share memory succeed");
		}
	}

	LOG_INFO("default", "[runtime] alloc %lld bytes of share memory succeed", mSize);
	
	mAddr = (char *)shmat(mShmId, NULL, 0);
    if (mAddr == (void *) -1) 
	{
		LOG_ERROR("error", "[runtime] shmat failed: %s", strerror(errno));
		return 0;
    }
	
	return mAddr;
}

char *wShm::AttachShm()
{
	LOG_DEBUG("default", "[runtime] try to attach %lld bytes of share memory", mSize);
	
	//����Ҫ���빲���ڴ��keyֵ�������
	mKey = ftok(mFilename, mPipeId);
	if (mKey < 0) 
	{
		LOG_ERROR("default", "create memory (ftok) failed: %s", strerror(errno));
		return 0;
	}

	// ���Ի�ȡ
	int mShmId = shmget(mKey, mSize, 0666);
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
	
	return mAddr;
}

void wShm::RemoveShm()
{
	if(mAddr ==0)
	{
		return;
	}
	
	//IPC�����ں˼����ݽṹ
    if (shmdt(mAddr) == -1)		//�Թ�����������������shmid_ds��ý��̹���������
	{
		LOG_ERROR("default", "shmdt(%d) failed", mAddr);
    }
    if (shmctl(mShmId, IPC_RMID, NULL) == -1)	//ɾ����shmid_ds����洢��
	{
		LOG_ERROR("default", "remove share memory failed: %s", strerror(errno));
    }
	//unlink(mFilename);
}
