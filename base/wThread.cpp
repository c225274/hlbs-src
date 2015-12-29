
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <stdarg.h>
#include <string.h>

#include "wThread.h"
#include "wLog.h"

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

	return NULL;
}

wThread::wThread()
{
	mRunStatus = rt_init;
}

wThread::~wThread() 
{
	//...
}

int wThread::CreateThread()
{
	pthread_attr_init(&mPthreadAttr);
	pthread_attr_setscope(&mPthreadAttr, PTHREAD_SCOPE_SYSTEM);  // �����߳�״̬Ϊ��ϵͳ�������߳�һ����CPUʱ��
	pthread_attr_setdetachstate(&mPthreadAttr, PTHREAD_CREATE_JOINABLE);  // ���÷Ƿ�����߳�
	
	pthread_cond_init(&mCond, NULL);
	pthread_mutex_init(&mMutex, NULL);
	
	mRunStatus = rt_running;

	pthread_create(&mPhreadId, &mPthreadAttr, ThreadProc, (void *)this);

	return 0;
}

int wThread::CondBlock()
{
	pthread_mutex_lock(&mMutex);

	while(IsBlocked() || mRunStatus == rt_stopped)  // �̱߳���������ֹͣ
	{
		if(mRunStatus == rt_stopped)  // ����߳���Ҫֹͣ����ֹ�߳�
		{
			//"Thread exit."
			pthread_exit((void *)mAbyRetVal);
		}
		//"Thread would blocked."
		mRunStatus = rt_blocked;
		pthread_cond_wait(&mCond, &mMutex);  //��������״̬
	}

	if(mRunStatus != rt_running)  
	{
		//"Thread waked up."
	}
	
	mRunStatus = rt_running;  //�߳�״̬��Ϊrt_running

	pthread_mutex_unlock(&mMutex);  //�ù�����Ҫ���߳��������

	return 0;
}

int wThread::WakeUp()
{
	pthread_mutex_lock(&mMutex);

	if(!IsBlocked() && mRunStatus == rt_blocked)
    {
		pthread_cond_signal(&mCond);  //���̷߳����ź��Ի���
	}

	pthread_mutex_unlock(&mMutex);

	return 0;
}

int wThread::StopThread()
{
	pthread_mutex_lock(&mMutex);

	mRunStatus = rt_stopped;
	pthread_cond_signal(&mCond);

	pthread_mutex_unlock(&mMutex);

	//�ȴ����߳���ֹ
	pthread_join(mPhreadId, NULL);

	return 0;
}