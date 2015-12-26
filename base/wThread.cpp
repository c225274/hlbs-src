
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
	memset((void *)&m_stLogCfg, 0, sizeof(m_stLogCfg));
}

wThread::~wThread() {}

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

	while(IsToBeBlocked() || mRunStatus == rt_stopped)  // �̱߳���������ֹͣ
	{
		if(mRunStatus == rt_stopped)  // ����߳���Ҫֹͣ����ֹ�߳�
		{
			//ThreadLogDebug( "Thread exit.");
			pthread_exit((void *)mAbyRetVal);
		}
		//ThreadLogDebug( "Thread would blocked." );
		mRunStatus = rt_blocked;
		pthread_cond_wait(&mCond, &mMutex);  //��������״̬
	}

	if(mRunStatus != rt_running)  
	{
		//ThreadLogDebug("Thread waked up.");
	}
	
	mRunStatus = rt_running;  //�߳�״̬��Ϊrt_running

	pthread_mutex_unlock(&mMutex);  //�ù�����Ҫ���߳��������

	return 0;
}

int wThread::WakeUp()
{
	pthread_mutex_lock(&mMutex);

	if(!IsToBeBlocked() && mRunStatus == rt_blocked)
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
	//ThreadLogDebug("Thread stopped.");

	return 0;
}

/*
void wThread::ThreadLogInit(char *sPLogBaseName, long lPMaxLogSize, int iPMaxLogNum, int iShow, int iLevel)
{
	memset(m_stLogCfg.szLogBaseName, 0, sizeof(m_stLogCfg.szLogBaseName));
	strncpy(m_stLogCfg.szLogBaseName, sPLogBaseName, sizeof(m_stLogCfg.szLogBaseName)-1);
	m_stLogCfg.lMaxLogSize = lPMaxLogSize;
	m_stLogCfg.iMaxLogNum = iPMaxLogNum;
	strncpy( m_stLogCfg.szThreadKey, m_stLogCfg.szLogBaseName, sizeof( m_stLogCfg.szThreadKey )-1 ) ;
	
	INIT_ROLLINGFILE_LOG( m_stLogCfg.szThreadKey, m_stLogCfg.szLogBaseName, (LogLevel) iLevel, m_stLogCfg.lMaxLogSize, m_stLogCfg.iMaxLogNum ); 
}

void wThread::ThreadLogDebug(const char *sFormat, ...)
{
	va_list va;
	va_start(va, sFormat);
	LogDebug_va(m_stLogCfg.szThreadKey, sFormat, va);
	va_end(va);
}

void wThread::ThreadLogInfo(const char *sFormat, ...)
{
	va_list va;
	va_start(va, sFormat);
	LogInfo_va(m_stLogCfg.szThreadKey, sFormat, va);
	va_end(va);
}

void wThread::ThreadLogNotice(const char *sFormat, ...)
{
	va_list va;
	va_start(va, sFormat);
	LogNotice_va(m_stLogCfg.szThreadKey, sFormat, va);
	va_end(va);
}

void wThread::ThreadLogWarn(const char *sFormat, ...)
{
	va_list va;
	va_start(va, sFormat);
	LogWarn_va(m_stLogCfg.szThreadKey, sFormat, va);
	va_end(va);
}

void wThread::ThreadLogError(const char *sFormat, ...)
{
	va_list va;
	va_start(va, sFormat);
	LogError_va(m_stLogCfg.szThreadKey, sFormat, va);
	va_end(va);
}

void wThread::ThreadLogFatal(const char *sFormat, ...)
{
	va_list va;
	va_start(va, sFormat);
	LogFatal_va(m_stLogCfg.szThreadKey, sFormat, va);
	va_end(va);
}
*/