
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_TIMER_H_
#define _W_TIMER_H_

#define TIME_OF_KEEP_ALIVE 30000

#include "wType.h"

/**
 *  简单定时器，单位为毫秒
 */
class wTimer
{
	public:
		wTimer()
		{
            mTimer = 0;
            mTimeRecord = 0;
		}

		wTimer(int vTimer): mTimer(vTimer), mTimeRecord(vTimer) {}

		~wTimer() {}

		bool CheckTimer(int vInterval)
		{
			int vPassTime = mTimer - vInterval;
			if( vPassTime <= 0 )
			{
				// 补差值时间
				mTimer = mTimeRecord + vPassTime;
				return true;
			}
			else
			{
				mTimer = vPassTime;
				return false;
			}
		}
		
	private:
		int mTimer;
		int mTimeRecord;
};

#endif
