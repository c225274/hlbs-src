
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_SHM_H_
#define _W_SHM_H_

#include <sys/mman.h>

#include "wType.h"
#include "wLog.h"
#include "wNoncopyable.h"

//�ڴ�ӳ��
//PAGE_SIZE
class wMmap : private wNoncopyable
{
	public:
		/**
		 *  mmap������PAGE_SIZEΪ��λ����ӳ�䣬���ڴ�Ҳֻ����ҳΪ��λ����ӳ�䣬
		 *  ��Ҫӳ���PAGE_SIZE�������ĵ�ַ��Χ��Ҫ�Ƚ����ڴ���룬ǿ����PAGE_SIZE�ı�����С����ӳ��
		 *  
		 *  fd = -1 & flags = MAP_ANONYMOUS ����ӳ�䣬ֻ���ھ�����Ե��ϵ�Ľ��̼�ͨ��
		 */
		wMmap(const char *filename, int flags = MAP_SHARED, size_t length = 0, int prot = PROT_READ| PROT_WRITE,  off_t offset = 0, void *start = NULL) 
		{
			memcpy(mFilename, filename, strlen(filename)+1);
			int pagesize = getpagesize();
			if(pagesize > 0)
			{
				mSize = ALIGN(size, pagesize);
			}
			else
			{
				mSize = size;
			}
		}
		
		void * CreateShm()
		{
			mStart = mmap(start, length, prot, flags, fd, offset);
		}
		
		int RemoveShm()
		{
			return munmap(mStart, mSize);
		}
		
		~wMmap()
		{
			RemoveShm();
		}
	
	private:
		char mFilename[255];
		void *mStart;
		size_t mSize;
		
		int mFD;
}