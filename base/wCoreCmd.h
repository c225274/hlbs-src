
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_CORE_CMD_H_
#define _W_CORE_CMD_H_

#include "wBaseCmd.h"

#pragma pack(1)

//++++++++++++�������ݽṹ
const BYTE CMD_CHANNEL_REQ = 50;
struct ChannelReqCmd_s : public wCommand
{
	ChannelReqCmd_s(BYTE para)
	{
		mCmd = CMD_CHANNEL_REQ;
		mPara = para;
		
		mPid = 0;
		mSlot = 0;
		mFD = -1;
	}
	
	int mPid;	//���ͷ�����id
	int mSlot;	//���ͷ����̱���ƫ��(�±�)
	int mFD;	//���ͷ�ch[0]������
};

const BYTE CHANNEL_REQ_OPEN = 1;
struct ChannelReqOpen_t : ChannelReqCmd_s 
{
	ChannelReqOpen_t() : ChannelReqCmd_s(CHANNEL_REQ_OPEN)
	{
		//
	}
};

const BYTE CHANNEL_REQ_CLOSE = 2;
struct ChannelReqClose_t : ChannelReqCmd_s 
{
	ChannelReqClose_t() : ChannelReqCmd_s(CHANNEL_REQ_CLOSE)
	{
		//
	}
};

const BYTE CHANNEL_REQ_QUIT = 3;
struct ChannelReqQuit_t : ChannelReqCmd_s 
{
	ChannelReqQuit_t() : ChannelReqCmd_s(CHANNEL_REQ_QUIT)
	{
		//
	}
};

const BYTE CHANNEL_REQ_TERMINATE = 4;
struct ChannelReqTerminate_t : ChannelReqCmd_s 
{
	ChannelReqTerminate_t() : ChannelReqCmd_s(CHANNEL_REQ_TERMINATE)
	{
		//
	}
};

const BYTE CHANNEL_REQ_REOPEN = 5;
struct ChannelReqReopen_t : ChannelReqCmd_s 
{
	ChannelReqReopen_t() : ChannelReqCmd_s(CHANNEL_REQ_REOPEN)
	{
		//
	}
};

#pragma pack()

#endif