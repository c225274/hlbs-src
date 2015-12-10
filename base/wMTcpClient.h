
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

template <typename T,typename TASK>
class wMTcpClient: public wSingleton<T>
{
	public:
		wMTcpClient();
		~wMTcpClient();
		
		void Initialize();
		
		wTcpClient<TASK>* CreateClient(string sClientName, char *vIPAddress, unsigned short vPort);
		bool AddTcpClient(int iType, wTcpClient<TASK> *pTcpClient);
		bool CleanTcpClient(int iType, wTcpClient<TASK> *pTcpClient = NULL);
		
		bool GenerateClient(int iType, string sClientName, char *vIPAddress, unsigned short vPort);
		
		//void PrepareStart();
		void Start();
		
		virtual void Run();

		virtual void Recv();
		
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
		
		void CheckTimer();
		void CheckReconnect();

		vector<wTcpClient<TASK>*> TcpClients(int iType);
		wTcpClient<TASK>* OneTcpClient(int iType);

	protected:
		CLIENT_STATUS mStatus;	//服务器当前状态
		
		//定时记录器
		unsigned long long mLastTicker;	//服务器当前时间
		//服务器重连计时器
		wTimer mServerReconnectTimer;
		int mReconnectTimes;

        std::map<int, vector<wTcpClient<TASK>*> > mTcpClient;	//每种类型客户端，可挂载多个连接
};

template <typename T,typename TASK>
wMTcpClient<T,TASK>::wMTcpClient()
{
	mStatus = CLIENT_STATUS_INIT;
	Initialize();
}

template <typename T,typename TASK>
wMTcpClient<T,TASK>::~wMTcpClient()
{
    //...
}

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::Initialize()
{
	mServerReconnectTimer = wTimer(RECONNECT_TIME);
	mLastTicker = GetTickCount();
	mReconnectTimes = 0;
}

template <typename T,typename TASK>
bool wMTcpClient<T,TASK>::GenerateClient(int iType, string sClientName, char *vIPAddress, unsigned short vPort)
{
	wTcpClient<TASK>* pTcpClient = CreateClient(sClientName, vIPAddress , vPort);
	if(pTcpClient != NULL)
	{
	    return AddTcpClient(iType, pTcpClient);
	}
	return false;
}

template <typename T,typename TASK>
wTcpClient<TASK>* wMTcpClient<T,TASK>::CreateClient(string sClientName, char *vIPAddress, unsigned short vPort)
{
	wTcpClient<TASK>* pTcpClient = new wTcpClient<TASK>(sClientName);
	int iRet = pTcpClient->ConnectToServer(vIPAddress, vPort);
	if(iRet >= 0)
	{
		return pTcpClient;
	}
	return NULL;
}

template <typename T,typename TASK>
bool wMTcpClient<T,TASK>::AddTcpClient(int iType, wTcpClient<TASK> *pTcpClient)
{
	if(pTcpClient == NULL)
	{
		return false;
	}
	vector<wTcpClient<TASK>*> vTcpClient;
    typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClient.find(iType);
	if(mt != mTcpClient.end())
	{
		vTcpClient = mt->second;
		mTcpClient.erase(mt);
	}
	vTcpClient.push_back(pTcpClient);
	mTcpClient.insert(pair<int, vector<wTcpClient<TASK>*> >(iType, vTcpClient));
	return true;
}

template <typename T,typename TASK>
bool wMTcpClient<T,TASK>::CleanTcpClient(int iType, wTcpClient<TASK> *pTcpClient)
{
	typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClient.find(iType);
	if(mt != mTcpClient.end())
	{
		if(pTcpClient == NULL)
		{
			return mTcpClient.erase(mt) == 1;
		}
		else
		{
			vector<wTcpClient<TASK>*> vTcpClient = mt->second;
			typename vector<wTcpClient<TASK>*>::iterator it = find(vTcpClient.begin(), vTcpClient.end(), pTcpClient);
			if(it != vTcpClient.end())
			{
				vTcpClient.erase(it);
				SAFE_DELETE(*it);
				mTcpClient.erase(mt);
				mTcpClient.insert(pair<int, vector<wTcpClient<TASK>*> >(iType, vTcpClient));
				return true;
			}
			return false;
		}
	}
	return false;
}

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::Start()
{
	//进入服务主服务
	//while(IsRunning())
	if(IsRunning())
	{
		Recv();
		
		Run();
	}
}

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::Recv()
{
    typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClient.begin();
	for(mt; mt != mTcpClient.end(); mt++)
	{
		vector<wTcpClient<TASK>* > vTcpClient = mt->second;
        
        typename vector<wTcpClient<TASK>*>::iterator vIt = vTcpClient.begin();
		for(vIt ; vIt != vTcpClient.end() ;vIt++)
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

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::CheckReconnect()
{
	int iNowTime = time(NULL);
	int iIntervalTime;

    typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClient.begin();
	for(mt; mt != mTcpClient.end(); mt++)
	{
		vector<wTcpClient<TASK>* > vTcpClient = mt->second;
        
        typename vector<wTcpClient<TASK>*>::iterator vIt = vTcpClient.begin();
		for(vIt ; vIt != vTcpClient.end() ;vIt++)
		{
			//task
			wTcpTask *pTcpTask = (*vIt)->TcpTask();
			if (pTcpTask == NULL)
			{
				continue;
			}
			if (pTcpTask->Socket()->SocketType() != CONNECT_SOCKET)
			{
				continue;
			}
			iIntervalTime = iNowTime - pTcpTask->Socket()->Stamp();
			if (iIntervalTime >= KEEPALIVE_TIME)
			{
				if(pTcpTask->Socket()->SocketFD() < 0 && mReconnectTimes < 5)
				{
					if((*vIt)->ReConnectToServer() == 0)
					{
						mReconnectTimes = 0;
						continue;
					}
				}
				mReconnectTimes++;
			}
		}
	}
}

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::CheckTimer()
{
	int iInterval = (int)(GetTickCount() - mLastTicker);

	if(iInterval < 100) 	//100ms
	{
		return;
	}

	//加上间隔时间
	mLastTicker += iInterval;
	
	//检测客户端超时
	if(mServerReconnectTimer.CheckTimer(iInterval))
	{
		CheckReconnect();
	}
}

template <typename T,typename TASK>
void wMTcpClient<T,TASK>::Run()
{
	//...
}

template <typename T,typename TASK>
vector<wTcpClient<TASK>*> wMTcpClient<T,TASK>::TcpClients(int iType)
{
	vector<wTcpClient<TASK>*> vTcpClient;
    typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClient.find(iType);
	if(mt != mTcpClient.end())
	{
		vTcpClient = mt->second;
	}
	return vTcpClient;
}

template <typename T,typename TASK>
wTcpClient<TASK>* wMTcpClient<T,TASK>::OneTcpClient(int iType)
{
	vector<wTcpClient<TASK>*> vTcpClient;
    typename map<int, vector<wTcpClient<TASK>*> >::iterator mt = mTcpClient.find(iType);
	if(mt != mTcpClient.end())
	{
		vTcpClient = mt->second;
		return vTcpClient[0];
	}
	return NULL;
}

#endif
