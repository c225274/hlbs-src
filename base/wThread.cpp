
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wThread.h"

/**
 *  �߳����
 */
void* ThreadProc(void *pvArgs)
{
	if(!pvArgs)
	{
		return NULL;
	}

	wThread *pThread = (wThread *)pvArgs;

	if(pThread->PrepareRun())
	{
		return NULL;
	}

	pThread->Run();
	return NULL;	//pthread_exit(0);
}

int wThread::StartThread(int join)
{
	pthread_attr_init(&mAttr);
	//�����߳�״̬Ϊ��ϵͳ�������߳�һ����CPUʱ��
	pthread_attr_setscope(&mAttr, PTHREAD_SCOPE_SYSTEM);
	if(join == 1)
	{
		pthread_attr_setdetachstate(&mAttr, PTHREAD_CREATE_JOINABLE);	//���÷Ƿ�����߳�
		mMutex = new wMutex();
		mCond = new wCond();
	}
	else
	{
		pthread_attr_setdetachstate(&mAttr, PTHREAD_CREATE_DETACHED);	//���÷�����߳�
	}
	
	mRunStatus = THREAD_RUNNING;
	pthread_create(&mTid, &mAttr, ThreadProc, (void *)this);
	
	pthread_attr_destroy(&mAttr);
	return 0;
}

int wThread::StopThread()
{
	mMutex->Lock();

	mRunStatus = THREAD_STOPPED;
	mCond->Signal();

	mMutex->Unlock();
	
	//�ȴ����߳���ֹ
	pthread_join(mTid, NULL);

	return 0;
}

int wThread::CondBlock()
{
	mMutex->Lock();		//������

	while(IsBlocked() || mRunStatus == THREAD_STOPPED)  //�̱߳���������ֹͣ
	{
		if(mRunStatus == THREAD_STOPPED)  //����߳���Ҫֹͣ����ֹ�߳�
		{
			pthread_exit((void *)GetRetVal());
		}
		
		mRunStatus = THREAD_BLOCKED;	//"blocked"
		
		mCond->Wait(*mMutex);	//��������״̬
	}

	if(mRunStatus != THREAD_RUNNING)  
	{
		//"Thread waked up"
	}
	
	mRunStatus = THREAD_RUNNING;  //�߳�״̬��ΪTHREAD_RUNNING

	mMutex->Unlock();	//�ù�����Ҫ���߳��������
	return 0;
}

int wThread::Wakeup()
{
	mMutex->Lock();

	if(!IsBlocked() && mRunStatus == THREAD_BLOCKED)
    {
		mCond->Signal();	//���̷߳����ź��Ի���
	}

	mMutex->Unlock();
	return 0;
}

inline int wThread::CancelThread()
{
	return pthread_cancel(mTid);
}

inline char* wThread::GetRetVal()
{
	memcpy(mRetVal, "pthread exited", sizeof("pthread exited"));
	return mRetVal;
}