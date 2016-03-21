
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_MEM_POOL_H_
#define _W_MEM_POOL_H_

#include <malloc.h>
#include <new>

#include "wCore.h"
#include "wLog.h"
#include "wNoncopyable.h"

class wMemPool : private wNoncopyable
{
	struct extra_t
	{
		extra_t	*mNext;
		char	*mAddr;
	};

	public:
		wMemPool();
		virtual ~wMemPool();
		void Initialize();
		
		char *Create(size_t size);
		void *Alloc(size_t size);
		void Destroy();
		void Reset();
		
	protected:
		char	*mStart; //��ʼ��ַ
		char	*mLast;	//�ѷ��䵽�ĵ�ַ
		char	*mEnd;	//������ַ
		int		mSize;
		struct extra_t *mExtra;
};

#endif
