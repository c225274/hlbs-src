
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_MTCP_CLIENT_H_
#define _W_MTCP_CLIENT_H_

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>
#include <string>
#include <string.h>
#include <map>
#include <vector>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "wType.h"
#include "wMisc.h"
#include "wLog.h"
#include "wSingleton.h"
#include "wTcpClient.h"
#include "wTcpTask.h"

template <typename T, typename TCPTASK = wTcpTask>
class wMTcpClient: public wSingleton<T>
{
	public:
		wMTcpClient();
		~wMTcpClient();
		
		void Initialize();
		
		bool AddTcpClient(int iType, wTcpClient *pTcpClient);
		bool CleanTcpClient(int iType, wTcpClient *pTcpClient = NULL);
		
		//void PrepareStart();
		void Start();
		
		virtual void Run();
			
		bool IsRunning()
		{
			return CLIENT_STATUS_INIT == mStatus;
		}
		
		void SetStatus(CLIENT_STATUS eStatus = CLIENT_STATUS_QUIT)
		{
			mStatus = eStatus;
		}
		
		CLIENT_STATUS GetStatus()
		{
			return mStatus;
		}
		
	protected:
		CLIENT_STATUS mStatus;	//��������ǰ״̬
	
		map<int, wTcpClient<TCPTASK>**> mTcpClient;	//ÿ�����Ϳͻ��ˣ��ɹ��ض������
		typedef map<int, vector<wTcpClient<TCPTASK>*> >::iterator MapIt;
		typedef map<int, vector<wTcpClient<TCPTASK>*> >::value_type MapValue;
		typedef pair<int, vector<wTcpClient<TCPTASK>*> > MapPair;
		
};

template <typename T,typename TCPTASK>
wMTcpClient<T,TCPTASK>::wMTcpClient()
{
	mStatus = CLIENT_STATUS_INIT;
	Initialize();
}

template <typename T,typename TCPTASK>
void wMTcpClient<T,TCPTASK>::Initialize()
{
	//...
}

template <typename T,typename TCPTASK>
bool wMTcpClient<T,TCPTASK>::AddTcpClient(int iType, wTcpClient *pTcpClient)
{
	if(pTcpClient == NULL)
	{
		return false;
	}
	vector<wTcpClient*> vTcpClient;
	MapIt mt = mTcpClient.find(iType);
	if(mt != mTcpClient.end())
	{
		vTcpClient = mt->second();
		mTcpClient.erase(mt);
	}
	vTcpClient.push_back(pTcpClient);
	mTcpClient.insert(MapValue(iType, vTcpClient));
	return true;
}

template <typename T,typename TCPTASK>
bool wMTcpClient<T,TCPTASK>::CleanTcpClient(int iType, wTcpClient *pTcpClient)
{
	MapIt mt = mTcpClient.find(iType);
	if(mt != mTcpClient.end())
	{
		if(pTcpClient == NULL)
		{
			return mTcpClient.erase(mt) == 1;
		}
		else
		{
			vector<wTcpClient*> vTcpClient = mt->second();
			vector<wTcpClient*>::iterator it = find(vTcpClient.begin(), vTcpClient.end(), pTcpClient);
			if(it != vTcpClient.end())
			{
				vTcpClient.erase(it);
				SAFE_DELETE(*it);
				mTcpClient.erase(mt);
				mTcpClient.insert(MapValue(iType, vTcpClient));
				return true;
			}
			return false;
		}
	}
	return false;
}

template <typename T,typename TCPTASK>
void wMTcpClient<T,TCPTASK>::Start()
{
	mStatus = CLIENT_STATUS_RUNNING;
	LOG_INFO("default", "Server start succeed");
	//���������ѭ��
	while(IsRunning())
	{
		//���ܿͻ�����Ϣ
		Recv();
		
		Run();
	}
}

template <typename T,typename TCPTASK>
void wMTcpClient<T,TCPTASK>::Recv()
{
	for(MapIt it = mTcpClient.begin(); it!= mTcpClient.end(); it++)
	{
		vector<wTcpClient*> vTcpClient = mt->second();
		for(vector<wTcpClient*>::iterator vIt = vTcpClient.begin(); vIt != vTcpClient.end() ;vIt++)
		{
			if(*vIt == NULL)
			{
				SAFE_DELETE(*vIt);
			}
			else
			{
				(*vIt)->Recv();
			}
		}
	}
}

template <typename T,typename TCPTASK>
void wMTcpClient<T,TCPTASK>::Run()
{
	//...
}

#endif