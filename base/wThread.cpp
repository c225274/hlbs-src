
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

	if(pThread->PrepareToRun())
	{
		return NULL;
	}

	pThread->Run();

	return NULL;
}

wThread::wThread()
{
	m_iRunStatus = rt_init;
	memset((void *)&m_stLogCfg, 0, sizeof(m_stLogCfg));
}

wThread::~wThread() {}

int wThread::CreateThread()
{
	pthread_attr_init(&m_stAttr);
	pthread_attr_setscope(&m_stAttr, PTHREAD_SCOPE_SYSTEM);  // �����߳�״̬Ϊ��ϵͳ�������߳�һ����CPUʱ��
	pthread_attr_setdetachstate(&m_stAttr, PTHREAD_CREATE_JOINABLE);  // ���÷Ƿ�����߳�
	pthread_cond_init(&m_stCond, NULL);
	pthread_mutex_init(&m_stMutex, NULL);
	m_iRunStatus = rt_running;

	pthread_create(&m_hTrd, &m_stAttr, ThreadProc, (void *)this);

	return 0;
}

int wThread::CondBlock()
{
	pthread_mutex_lock(&m_stMutex);

	while(IsToBeBlocked() || m_iRunStatus == rt_stopped)  // �̱߳���������ֹͣ
	{
		if(m_iRunStatus == rt_stopped)  // ����߳���Ҫֹͣ����ֹ�߳�
		{
			ThreadLogDebug( "Thread exit.");
			pthread_exit((void *)m_abyRetVal);
		}
		ThreadLogDebug( "Thread would blocked." );
		m_iRunStatus = rt_blocked;
		pthread_cond_wait(&m_stCond, &m_stMutex);  //��������״̬
	}

	if(m_iRunStatus != rt_running)  
	{
		ThreadLogDebug("Thread waked up.");
	}
	
	m_iRunStatus = rt_running;  //�߳�״̬��Ϊrt_running

	pthread_mutex_unlock(&m_stMutex);  //�ù�����Ҫ���߳��������

	return 0;
}

int wThread::WakeUp()
{
	pthread_mutex_lock(&m_stMutex);

	if(!IsToBeBlocked() && m_iRunStatus == rt_blocked)
    {
		pthread_cond_signal( &m_stCond );  //���̷߳����ź��Ի���
	}

	pthread_mutex_unlock( &m_stMutex );

	return 0;
}

int wThread::StopThread()
{
	pthread_mutex_lock(&m_stMutex);

	m_iRunStatus = rt_stopped;
	pthread_cond_signal(&m_stCond);

	pthread_mutex_unlock(&m_stMutex);

	//�ȴ����߳���ֹ
	pthread_join(m_hTrd, NULL);
	ThreadLogDebug("Thread stopped.");

	return 0;
}

void wThread::ThreadLogInit(char *sPLogBaseName, long lPMaxLogSize, int iPMaxLogNum, int iShow, int iLevel/*= 0*/)
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