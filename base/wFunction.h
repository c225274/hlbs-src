
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_FUNCTION_H_
#define _W_FUNCTION_H_

#include "wType.h"

/**
 * ��Ϊ�ػ�����
 * @param  filename [�����ļ���]
 */
void InitDaemon(const char *filename);

/**
 * ���������ڴ�
 * @param  filename [�����ڴ���ڵ��ļ���]
 * @param  pipe_id  [ftok���Ӳ���]
 * @param  size     [�����ڴ��С]
 * @return          [�����ڴ��ڽ�����ӳ���ַ]
 */
char *CreateShareMemory(const char *filename, int pipe_id, size_t size);

#endif
