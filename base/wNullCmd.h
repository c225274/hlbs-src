
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

/**
 * ����������ڲ�����ʹ�ã��Ϳͻ��˽�����ָ����Ҫ���ⶨ��
 */
#ifndef _W_NULL_CMD_H_
#define _W_NULL_CMD_H_

#include "wType.h"

#pragma pack(1)

const BYTE CMD_NULL = 0;		/**< �յ�ָ�� */
const BYTE PARA_NULL = 0;		/**< �յ�ָ����� */

struct NullCmd_t
{
	BYTE cmd;					/**< ָ����� */
	BYTE para;					/**< ָ������ӱ�� */

	BYTE GetCmdType() const { return cmd; }
	BYTE GetParaType() const { return para; }
	
	NullCmd_t(const BYTE cmd = CMD_NULL, const BYTE para = PARA_NULL) : cmd(cmd), para(para) {};
};

//socket����˿�ָ���ӱ��
const BYTE SERVER_PARA_NULL = 0;

//socket����˿ղ���ָ������źźͶ�ʱ��ָ��
struct ServerNullCmd_t:public NullCmd_t
{
	ServerNullCmd_t():NullCmd_t(CMD_NULL,SERVER_PARA_NULL) {}
};

//socket�ͻ��˿�ָ���ӱ��
const BYTE CLIENT_PARA_NULL = 1;

//socket�ͻ��˿ղ���ָ������źźͶ�ʱ��ָ��
struct ClientNullCmd_t:public NullCmd_t
{
	ClientNullCmd_t():NullCmd_t(CMD_NULL,CLIENT_PARA_NULL) {}
};

#pragma pack()

#endif

