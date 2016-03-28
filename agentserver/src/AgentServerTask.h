
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _AGENT_SERVER_TASK_H_
#define _AGENT_SERVER_TASK_H_

#include <arpa/inet.h>
#include <functional>

#include "wCore.h"
#include "wLog.h"
#include "wTcpTask.h"
#include "wDispatch.h"
#include "Common.h"
#include "SvrCmd.h"
#include "LoginCmd.h"
#include "AgentConfig.h"
#include "AgentServer.h"

#define AGENT_REG_DISP(cmdid, paraid, func) mDispatch.Register("AgentServerTask", CMD_ID(cmdid, paraid), REG_FUNC(CMD_ID(cmdid, paraid), func));

class AgentServer;
class AgentServerTask : public wTcpTask
{
	public:
		AgentServerTask();
		AgentServerTask(wIO *pIO);
		~AgentServerTask();
		void Initialize();
		
		virtual int VerifyConn();
		virtual int Verify();

		virtual int HandleRecvMessage(char * pBuffer, int nLen);
		
		int ParseRecvMessage(struct wCommand* pCommand ,char *pBuffer,int iLen);
		
		DEC_FUNC(ReloadSvrReq);
		DEC_FUNC(GetSvrAll);
		DEC_FUNC(GetSvrByGXid);	//获取一个可用svr
		DEC_FUNC(ReportSvr);	//上报

		DEC_FUNC(SyncSvrReq);

	protected:
		DEC_DISP(mDispatch);

		AgentConfig *mConfig;
		AgentServer *mServer;
};

#endif
