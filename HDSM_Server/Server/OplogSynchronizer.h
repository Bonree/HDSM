#pragma once
#include <string>
#include "BaseSocket.h"
#include "TypeDefine.h"

using namespace std;

class OplogSynchronizer
{
public:
	OplogSynchronizer(void);
	virtual ~OplogSynchronizer(void);
public:
	HUINT64 sync(string &str);
	HBOOL ack(HUINT64 ullAckIndex);
private:
	HBOOL init_connection();
	HBOOL send_command(const string &strSent, HUINT32 ulSN, string &strRecv);
private:
	string get_ack_buffer(HUINT64 ullAckIndex);
	string get_sync_buffer();
private:
	BaseSocket	m_SyncSocket;
	HUINT32		m_ulAckSN;
	HUINT32		m_ulSyncSN;
	string		m_strMirrorPeerAddr;
};

