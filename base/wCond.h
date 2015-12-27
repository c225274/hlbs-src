
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_COND_H_
#define _W_COND_H_

#include <pthread.h>

#include "wNoncopyable.h"
#include "wMutex.h"

class wCond : private wNoncopyable
{

	public:
		wCond()
		{
			pthread_cond_init(&mCond, NULL);
		}
		
		~zCond()
		{
			pthread_cond_destroy(&mCond);
		}
		
		void Broadcast()
		{
			pthread_cond_broadcast(&mCond);
		}
		
		void Signal()
		{
			pthread_cond_signal(&mCond);
		}

		/**
		 * �ȴ��ض���������������
		 * @param stMutex ��Ҫ�ȴ��Ļ�����
		 */
		void Wait(wMutex &stMutex)
		{
			pthread_cond_wait(&mCond, &stMutex.mMutex);
		}

	private:

		pthread_cond_t mCond;		/**< ϵͳ�������� */

};


#endif