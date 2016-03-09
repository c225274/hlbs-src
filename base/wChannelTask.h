
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_CHANNEL_TASK_H_
#define _W_CHANNEL_TASK_H_

#include "wType.h"
#include "wTask.h"
#include "wChannel.h"
#include "wLog.h"
#include "wMisc.h"
#include "wCommand.h"

class wChannelTask : public wTask
{
	public:
		wChannelTask();
		wChannelTask(wIO *pIO);
		void Initialize();
		virtual ~wChannelTask();
		
		virtual int HandleRecvMessage(char * pBuffer, int nLen) {}
};

#endif