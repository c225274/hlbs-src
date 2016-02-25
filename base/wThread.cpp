
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

int wThread::StartThread()
{
	pthread_attr_init(&mPthreadAttr);
	pthread_attr_setscope(&mPthreadAttr, PTHREAD_SCOPE_SYSTEM);  //�����߳�״̬Ϊ��ϵͳ�������߳�һ����CPUʱ��
	pthread_attr_setdetachstate(&mPthreadAttr, PTHREAD_CREATE_JOINABLE);  //���÷Ƿ�����߳�
	
	mMutex = new wMutex(NULL);
	mCond = new wCond(NULL);

	mRunStatus = rt_running;

	pthread_create(&mPhreadId, &mPthreadAttr, ThreadProc, (void *)this);

	return 0;
}

int wThread::CondBlock()
{
	mMutex->Lock();

	while(IsBlocked() || mRunStatus == rt_stopped)  //�̱߳���������ֹͣ
	{
		if(mRunStatus == rt_stopped)  //����߳���Ҫֹͣ����ֹ�߳�
		{
			pthread_exit((void *)mAbyRetVal);	//"Thread exit"
		}
		
		mRunStatus = rt_blocked;	//"Thread would blocked"
		
		mCond->Wait(*mMutex);	//��������״̬
	}

	if(mRunStatus != rt_running)  
	{
		//"Thread waked up"
	}
	
	mRunStatus = rt_running;  //�߳�״̬��Ϊrt_running

	mMutex->Unlock();	//�ù�����Ҫ���߳��������
	return 0;
}

int wThread::WakeUp()
{
	mMutex->Lock();

	if(!IsBlocked() && mRunStatus == rt_blocked)
    {
		mCond->Signal();	//���̷߳����ź��Ի���
	}

	mMutex->Unlock();
	return 0;
}

int wThread::StopThread()
{
	mMutex->Lock();

	mRunStatus = rt_stopped;
	mCond->Signal();

	mMutex->Unlock();
	
	//�ȴ����߳���ֹ
	pthread_join(mPhreadId, NULL);

	return 0;
}