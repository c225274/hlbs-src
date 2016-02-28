
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <stdarg.h>
#include <string.h>

#include "wThread.h"
#include "wLog.h"

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

wThread::wThread()
{
	mRunStatus = RT_BLOCKED;	//����
}

wThread::~wThread() 
{
	//
}

int wThread::StartThread(int join)
{
	pthread_attr_init(&mAttr);
	pthread_attr_setscope(&mAttr, PTHREAD_SCOPE_SYSTEM);  //�����߳�״̬Ϊ��ϵͳ�������߳�һ����CPUʱ��
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
	
	mRunStatus = RT_RUNNING;
	pthread_create(&mTid, &mAttr, ThreadProc, (void *)this);
	
	pthread_attr_destry(&mAttr);
	return 0;
}

int wThread::StopThread()
{
	mMutex->Lock();

	mRunStatus = RT_STOPPED;
	mCond->Signal();

	mMutex->Unlock();
	
	//�ȴ����߳���ֹ
	pthread_join(mTid, NULL);

	return 0;
}

int wThread::CondBlock()
{
	mMutex->Lock();		//������

	while(IsBlocked() || mRunStatus == RT_STOPPED)  //�̱߳���������ֹͣ
	{
		if(mRunStatus == RT_STOPPED)  //����߳���Ҫֹͣ����ֹ�߳�
		{
			pthread_exit((void *)GetRetVal());
		}
		
		mRunStatus = RT_BLOCKED;	//"blocked"
		
		mCond->Wait(*mMutex);	//��������״̬
	}

	if(mRunStatus != RT_RUNNING)  
	{
		//"Thread waked up"
	}
	
	mRunStatus = RT_RUNNING;  //�߳�״̬��ΪRT_RUNNING

	mMutex->Unlock();	//�ù�����Ҫ���߳��������
	return 0;
}

int wThread::Wakeup()
{
	mMutex->Lock();

	if(!IsBlocked() && mRunStatus == RT_BLOCKED)
    {
		mCond->Signal();	//���̷߳����ź��Ի���
	}

	mMutex->Unlock();
	return 0;
}

int wThread::CancelThread()
{
	return pthread_cancel(mTid);
}