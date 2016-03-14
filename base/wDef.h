
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_DEF_H_
#define _W_DEF_H_

//名字的最大长度
#define MAX_NAME_LEN 32

//最长的IP地址字符串长度
#define MAX_IP_LEN 16

//消息缓冲区大小(每条信息消息主体最大长度)
#define MAX_PACKAGE_LEN 131072	/* 128k */

//客户端消息的最长长度
#define MAX_CLIENT_MSG_LEN MAX_PACKAGE_LEN

//客户端消息主体最短长度(一字节消息类型)
#define MIN_CLIENT_MSG_LEN 1

//每一个服务器之间的发送接收缓冲区的大小
#define MAX_SEND_BUFFER_LEN 524288 /* 512k */

#define MAX_RECV_BUFFER_LEN 261120 /* 255k */

//心跳包间隔时间
#define KEEPALIVE_TIME 3000 /* 3*1000 */

//服务器检测心跳时间
#define CHECK_CLIENT_TIME 5000 /* 5*1000 */

//超时时间
#define WAITRES_TIME 30000 /* 30*1000 */

//消息队列的长度
#define MSG_QUEUE_LEN 16777216 /* 16m */

#define LISTEN_BACKLOG	511

//主机名长度
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN  256
#endif

#define FD_UNKNOWN	-1
#define SETPROCTITLE_PAD '\0'
#define LF (u_char) '\n'
#define CR (u_char) '\r'
#define CRLF "\r\n"

//整形长度
#define NGX_INT32_LEN   (sizeof("-2147483648") - 1)		/* int32长度*/
#define NGX_INT64_LEN   (sizeof("-9223372036854775808") - 1) 	/* int64长度*/


#define PROCESS_SINGLE     0 	//单独进程
#define PROCESS_MASTER     1 	//主进程
//#define PROCESS_SIGNALLER  2 	//信号进程
#define PROCESS_WORKER     3 	//工作进程

#define PROCESS_NORESPAWN     -1	//子进程退出时，父进程不再创建
//#define PROCESS_JUST_SPAWN    -2	//正在重启
#define PROCESS_RESPAWN       -3	//子进程异常退出时，master会重新创建它。如当worker或cache manager异常退出时，父进程会重新创建它
#define PROCESS_DETACHED	  -5	//热代码替换? TODO

#define MAX_PROCESSES         1024

enum MASTER_STATUS
{
	MASTER_INIT = -1,
	MASTER_RUNNING,
	MASTER_HUP,
	MASTER_EXITING,
	MASTER_EXITED
};

enum WORKER_STATUS
{
	WORKER_INIT = -1,
	WORKER_RUNNING,
	WORKER_HUP,
	WORKER_EXITING,
	WORKER_EXITED
};

enum SERVER_STATUS
{
	SERVER_INIT = -1,	//服务器的初始化状态
	SERVER_QUIT,	 	//服务器进入关闭状态
	SERVER_RUNNING	 	//正常运行状态模式
};

enum TASK_STATUS
{
	TASK_INIT = -1,
	TASK_QUIT,
	TASK_RUNNING
};

#define USER  "nobody"
#define GROUP  "nobody"

//define
#define PREFIX  "/home/hlfs/disvr/routerserver/"
#define CONF_PREFIX  "/home/hlfs/disvr/routerserver/conf/"
 
#define PID_PATH  "/home/hlfs/disvr/routerserver/bin/disvr.pid"
#define LOCK_PATH  "/home/hlfs/disvr/routerserver/bin/disvr.lock"
#define WAIT_MUTEX	"/home/hlfs/disvr/routerserver/bin/wait_mutex.shm.ipc"

#endif