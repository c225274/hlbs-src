
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wPing.h"

wPing::wPing()
{
	Initialize();
}

wPing::wPing(const char *ip, int timeout)
{
	mStrIp = ip;
	mTimeout = timeout;
}

wPing::~wPing()
{
	Close();
}

int wPing::Close()
{
	if(mFD != FD_UNKNOWN)
	{
		close(mFD);
		mFD = FD_UNKNOWN;
	}
	return 0;
}

void wPing::Initialize()
{
	mPid = getpid();
	mFD = FD_UNKNOWN;
}

int wPing::Open()
{
	struct protoent *protocol;
	if ((protocol = getprotobyname("icmp")) == NULL)
	{
		return FD_UNKNOWN;
	}
	/** ����ʹ��ICMP��ԭʼ�׽���,�����׽���ֻ��root�������� */
	if ((mFD = socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0)	//IPPROTO_ICMP
	{
		return FD_UNKNOWN;
	}
	
	/** ����rootȨ��,���õ�ǰ�û�Ȩ�� */
	//setuid(getuid());
	 
	/** �����׽��ֽ��ջ�������50K����ҪΪ�˼�С���ջ���������ĵĿ����ԣ���������pingһ���㲥��ַ��ಥ��ַ��������������Ӧ�� */
	int size = 50*1024;
	if (setsockopt(mFD, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) != 0)
	{
		//set socket receive buf failed:%d
		return FD_UNKNOWN;
	}
	
	bzero(&mDestAddr, sizeof(mDestAddr));
	mDestAddr.sin_family = AF_INET;
	
	struct hostent *host;
	unsigned long inaddr = 0l;
	if (inaddr = inet_addr(mStrIp.c_str()) == INADDR_NONE)  
	{
		/** �������� */
		if ((host = gethostbyname(mStrIp.c_str())) == NULL)
		{
			return false;
		}
		memcpy((char *) &mDestAddr.sin_addr, host->h_addr, host->h_length);
	}  
	else
	{
		/** ��ip��ַ */
		memcpy((char *) &mDestAddr, (char *) &inaddr, host->h_length);
	}
	
	return mFD;
}

bool wPing::Ping(int times)
{
	if (mFD == FD_UNKNOWN)
	{
		return false;
	}
	
	while (i < times)
    {
        i++;
		
		/*��������ICMP����*/
        SendPacket(1);
		
		/*��������ICMP����*/
        RecvPacket();
    }
}

/** ����num��ICMP���� */
void wPing::SendPacket(int num)
{
	if (num > MAX_NO_PACKETS)
	{
		num = MAX_NO_PACKETS;
	}
	
	int packetsize;
	int i = 0;
	while (i < num)
	{
		i++;
		mSend++;
		
		/** ����ICMP��ͷ */
		packetsize = Pack(mSend);
		if (sendto(mFD, mSendpacket, packetsize, 0, (struct sockaddr *)&mDestAddr, sizeof(mDestAddr)) < 0)
		{
			continue;
		}
		/** ÿ��һ�뷢��һ��ICMP���� */
		sleep(1);
	}
}

/** ��������ICMP���� */
void wPing::RecvPacket()  
{
	int n, fromlen;
    fromlen = sizeof(mFrom);
    while (mRecv < mSend)
    {
        alarm(MAX_WAIT_TIME);
        if ((n = recvfrom(mFD, mRecvpacket, sizeof(mRecvpacket), 0, (struct sockaddr *)&mFrom, (struct socklen_t *)&fromlen)) < 0)  
        {
            if (errno == EINTR)
			{
				continue;
			}
			//recvfrom error
            continue;  
        }
		
		/** ��¼����ʱ�� */
        gettimeofday(&mRecvtv, NULL);
        if (Unpack(mRecvpacket, n) == -1)
		{
			continue;
		}
        mRecv++;
    }
}  

/** У��� */
unsigned short wPing::CalChksum(unsigned short *addr, int len)
{
	int sum = 0;
	int nleft = len;
	unsigned short *w = addr;
	unsigned short answer = 0;
			
	/** ��ICMP��ͷ������������2�ֽ�Ϊ��λ�ۼ����� */
	while(nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}
	
	/** ��ICMP��ͷΪ�������ֽڣ���ʣ�����һ�ֽڡ������һ���ֽ���Ϊһ��2�ֽ����ݵĸ��ֽڣ����2�ֽ����ݵĵ��ֽ�Ϊ0�������ۼ� */
	if(nleft == 1)
	{      
		*(unsigned char *)(&answer) = *(unsigned char *)w;
		sum += answer;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;
	return answer;
}

/** ����ICMP����ͷ */
int wPing::Pack(int pack_no)
{
	struct icmp *icmp;
	icmp = (struct icmp*) mSendpacket;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_cksum = 0;
	icmp->icmp_seq = pack_no;
	icmp->icmp_id = mPid;
	
	/** ��¼����ʱ�� */
	struct timeval *tval;
	tval = (struct timeval *) icmp->icmp_data;
	gettimeofday(tval, NULL);
	
	int packsize = 8 + SEND_DATA_LEN;
	
	/** У���㷨 */
	icmp->icmp_cksum = CalChksum((unsigned short *)icmp, packsize);
	return packsize;
}

/** ��ȥICMP��ͷ */
int wPing::Unpack(char *buf, int len)
{
	int i,iphdrlen;
	struct ip *ip;
	struct icmp *icmp;
	struct timeval *tvsend;
	double rtt;
	ip = (struct ip *)buf;
	
	/** ��ip��ͷ����,��ip��ͷ�ĳ��ȱ�־��4 */
	iphdrlen = ip->ip_hl << 2;
	
	/** Խ��ip��ͷ,ָ��ICMP��ͷ */
	icmp = (struct icmp *) (buf + iphdrlen); 
	
	/** ICMP��ͷ��ICMP���ݱ����ܳ��� */
	len -= iphdrlen;
	
	/** С��ICMP��ͷ�����򲻺��� */
	if (len < 8)
	{
		//ICMP packets/'s length is less than 8/n
		return -1;  
	}
	
	/** ȷ�������յ����������ĵ�ICMP�Ļ�Ӧ */
	if ((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == mPid))  
	{  
		tvsend = (struct timeval *) icmp->icmp_data;  
		
		/** ���պͷ��͵�ʱ��� */
		TvSub(&mRecvtv, tvsend);
		
		/** �Ժ���Ϊ��λ����rtt */
		rtt = mRecvtv.tv_sec * 1000 + mRecvtv.tv_usec / 1000;
		mTotalResponseTimes += rtt;
		if (mFasterResponseTime == -1)
		{
			mFasterResponseTime = rtt;
		}
		else if(mFasterResponseTime > rtt)
		{
			mFasterResponseTime = rtt;  
		}
		
		if(mLowerResponseTime == -1)  
		{  
			mLowerResponseTime = rtt;  
		}  
		else if(mLowerResponseTime < rtt)  
		{
			mLowerResponseTime = rtt;  
		}
		/** ��ʾ�����Ϣ */  
		//printf("%d/tbyte from %s/t: icmp_seq=%u/tttl=%d/trtt=%.3f/tms/n", len, inet_ntoa(m_from.sin_addr), icmp->icmp_seq, ip->ip_ttl, rtt);  
		
		return 0;
	}
	return -1;
}

void wPing::TvSub(struct timeval *out,struct timeval *in)  
{         
    if((out->tv_usec -= in->tv_usec) < 0)
    {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}
