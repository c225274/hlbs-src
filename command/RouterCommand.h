
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_ROUTER_COMMAND_H_
#define _W_ROUTER_COMMAND_H_

#include "wHeadCmd.h"

#pragma pack(1)

//������ID
enum SERVER_TYPE
{
	SERVER_ROUTER = 1,
	SERVER_AGENT,
};

//Agent->Router����Ϣ��
const BYTE CMD_LOGIN          = 1;  //��¼������ָ��
struct LoginCmd_s : public wHeadCmd
{
	LoginCmd_s(BYTE para)
	{
		mCommand.cmd = CMD_LOGIN;
		mCommand.para = para;
	}
};

//Agent��¼Router����
const BYTE PARA_SERVER_LOGIN = 1;
struct LoginFromS_t : LoginCmd_s
{
	LoginFromS_t() : LoginCmd_s(PARA_SERVER_LOGIN), wdServerID(0), wdServerType(0) {}

	WORD wdServerID;
	WORD wdServerType;
};

#pragma pack()

#endif