
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_AGENT_COMMAND_H_
#define _W_AGENT_COMMAND_H_

#include "wHeadCmd.h"

#pragma pack(1)

//Router->Agent����Ϣ��
const BYTE CMD_LOGIN          = 1;  //��¼������ָ��
struct LoginCmd_s : public wHeadCmd
{
	LoginCmd_s(BYTE para)
	{
		mCommand.cmd = CMD_LOGIN;
		mCommand.para = para;
	}
};

//Router��¼����
const BYTE PARA_SERVER_LOGIN = 1;
struct LoginFromS_t : LoginCmd_s
{
	LoginFromS_t() : LoginCmd_s(PARA_SERVER_LOGIN), wdServerID(0), wdServerType(0) {}

	WORD wdServerID;
	WORD wdServerType;
};

//�ͻ��˵�¼����
const BYTE PARA_CLIENT_LOGIN = 2;
struct LoginFromC_t : LoginCmd_s
{
	LoginFromC_t() : LoginCmd_s(PARA_CLIENT_LOGIN), wdServerID(0), wdServerType(0) {}
};

#pragma pack()

#endif