
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>		//atoi random srandom
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>	//int flock(int fd,int operation);

#include <signal.h>

#include "wType.h"
#include "wLog.h"
#include "wFunction.h"

/**
 * ���������ڴ�
 * @param  filename [�����ڴ���ڵ��ļ���]
 * @param  pipe_id  [ftok���Ӳ���]
 * @param  size     [�����ڴ��С]
 * @return          [�����ڴ��ڽ�����ӳ���ַ]
 */
char *CreateShareMemory(const char *filename, int pipe_id, size_t size)
{
	LOG_DEBUG("default", "Try to alloc %lld bytes of share memory", size);

	//����Ҫ���빲���ڴ��keyֵ�������
	key_t key = ftok(filename, pipe_id);
	if (key < 0) 
	{
		printf("Initailize memory (ftok) failed: %s", strerror(errno));
		exit(1);
	}

	//���빲���ڴ�
	int shm_id = shmget(key, size, IPC_CREAT|IPC_EXCL|0666);

	//��������ڴ�ʧ��
	if (shm_id < 0) 
	{
		if (errno != EEXIST) 
		{
			LOG_ERROR("default", "Alloc share memory failed: %s", strerror(errno));
			exit(1);
		}

		LOG_DEBUG("default", "Share memory is exist now, try to attach it");

		//������ڴ��Ѿ������룬��������ʿ�����
		shm_id = shmget(key, size, 0666);

		//���ʧ��
		if (shm_id < 0) 
		{
			LOG_DEBUG("default", "Attach to share memory failed: %s, try to touch it", strerror(errno));
			
			//�²��Ƿ��Ǹ��ڴ��С̫С���Ȼ�ȡ�ڴ�ID
			shm_id = shmget(key, 0, 0666);
			
			//���ʧ�ܣ����޷��������ڴ棬ֻ���˳�
			if (shm_id < 0) 
			{
				LOG_ERROR("default", "Touch to share memory failed: %s", strerror(errno));
				exit(1);
			}
			else 
			{
				LOG_DEBUG("default", "Remove the exist share memory %d", shm_id);

				//����ɹ�������ɾ��ԭ�ڴ�
				if (shmctl(shm_id, IPC_RMID, NULL) < 0) 
				{
					LOG_ERROR("default", "Remove share memory failed: %s", strerror(errno));
					exit(1);
				}

				//�ٴ������ID���ڴ�
				shm_id = shmget(key, size, IPC_CREAT|IPC_EXCL|0666);
				if (shm_id < 0) 
				{
					LOG_ERROR("default", "Alloc share memory failed again: %s", strerror(errno));
					exit(1);
				}
			}
		}
		else
		{
			LOG_DEBUG("default", "Attach to share memory succeed");
		}
	}

	LOG_INFO("default", "Alloc %lld bytes of share memory succeed", size);

	return (char *)shmat(shm_id, NULL, 0);
}

/**
 * ��Ϊ�ػ�����
 * @param  filename [�����ļ���]
 */
void InitDaemon(const char *filename)
{
	//����Ҫ�������ļ�
	int lock_fd = open(filename, O_RDWR|O_CREAT, 0640);
	if (lock_fd < 0) 
	{
		printf("Open lock file failed when init daemon\n");
		exit(1);
	}
	//��ռʽ�����ļ�����ֹ����ͬ����Ľ����Ѿ�����
	int ret = flock(lock_fd, LOCK_EX | LOCK_NB);
	if (ret < 0) 
	{
		printf("Lock file failed, server is already running\n");
		exit(1);
	}

	//��ȡ��ǰ��Ŀ¼��Ϣ
	char dir_path[256] = {0};
	getcwd(dir_path, sizeof(dir_path));

	pid_t pid;

	//��һ��fork
	if ((pid = fork()) != 0) exit(0);

	//���ý�������һ���µĽ�������׽���
	setsid();

	//���������ź�
	signal(SIGINT,  SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	//�ٴ�fork
	if ((pid = fork()) != 0) exit(0);

	if (chdir(dir_path)) 
	{
		printf("Can not change run dir to %s, init daemon failed:%s\n", dir_path, strerror(errno));
		exit(1);
	}

	umask(0);
}
