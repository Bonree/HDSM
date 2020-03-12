#include "OplogSynchronizer.h"
#include "Utils.h"
#include "BaseSocket.h"
#include "../ConfigureMgr.h"
#include "../Logger.h"
#include "Task.h"
#include <string.h>

#define RECV_BUFFER_SIZE 1024
#define MAX_SN 100000000
#define RAISE_SN(_sn_) _sn_++;if (_sn_ > MAX_SN) _sn_ = 1;

OplogSynchronizer::OplogSynchronizer(void)
{
	m_ulAckSN = 1;
	m_ulSyncSN = 1;
	m_strMirrorPeerAddr = ConfigureMgr::get_mirror_peer_addr();
}

OplogSynchronizer::~OplogSynchronizer(void)
{
	m_SyncSocket.close();
}

HBOOL OplogSynchronizer::init_connection()
{
	HINT32 nPos = m_strMirrorPeerAddr.find(":");
	if (nPos == -1)
		return false;

	string sIP = m_strMirrorPeerAddr.substr(0, nPos);
	string sPort = m_strMirrorPeerAddr.substr(nPos+1, m_strMirrorPeerAddr.length()-nPos-1);

	if (!m_SyncSocket.is_valid())
		m_SyncSocket.create(AF_INET, SOCK_STREAM, 0);

	if (m_SyncSocket.set_block_flag(false) == SOCKET_ERROR)
	{
		m_SyncSocket.close();
		return false;
	}

	if (m_SyncSocket.connect(sIP.c_str(), atoi(sPort.c_str())) == SOCKET_ERROR)
	{
		m_SyncSocket.close();
		return false;
	}

	if (m_SyncSocket.wait_timeout(1000))
	{
		m_SyncSocket.close();
		return false;
	}

	if (m_SyncSocket.set_block_flag(true) == SOCKET_ERROR)
	{
		m_SyncSocket.close();
		return false;
	}

	return true;
}

HBOOL OplogSynchronizer::send_command(const string &strSent, HUINT32 ulSN, string &strRecv)
{
	if (!m_SyncSocket.is_valid())
	{
		if (!init_connection())
		{
			Logger::log_w("Try to conncet Mirror Peer failed!");
			return false;
		}
	}

#ifdef WIN32
	HINT32 flag = 0;
#else
	HINT32 flag = MSG_NOSIGNAL;
#endif
	HINT32 nSentLen = 0;
	HUINT32 nTotalSentLen = 0;
	const HCHAR *pBuf = strSent.c_str();
	while ((nSentLen = m_SyncSocket.send(pBuf+nTotalSentLen, strSent.size()-nTotalSentLen, flag)) != SOCKET_ERROR)
	{
		nTotalSentLen += nSentLen;
		if (nTotalSentLen >= strSent.size())
			break;
	}

	if (nSentLen == SOCKET_ERROR)
	{
		m_SyncSocket.close();
		return false;
	}

	if (m_SyncSocket.wait_timeout(1000))
	{
		m_SyncSocket.close();
		return false;
	}

	do 
	{
		HCHAR szRecvBuf[RECV_BUFFER_SIZE] = {0};
		HINT32 nRecvLen = 0;
		HUINT32 nTotalRecvLen = 0;

		HUINT32 ulDataLen = 0;
		m_SyncSocket.recv((HCHAR *)&ulDataLen, sizeof(HUINT32), 0);

		while (0 != (nRecvLen = m_SyncSocket.recv(szRecvBuf, RECV_BUFFER_SIZE, 0)))
		{
			if (nRecvLen == SOCKET_ERROR)
			{
				m_SyncSocket.close();
				return false;
			}

			strRecv.append(szRecvBuf, nRecvLen);
			nTotalRecvLen += nRecvLen;

			if (nTotalRecvLen >= ulDataLen)
				break;

			memset(szRecvBuf, 0, RECV_BUFFER_SIZE);
		}

		const HCHAR *pBuf = strRecv.c_str();
		HUINT8 checksum = pBuf[0];
		HINT32 nBufLen = strRecv.size();
		if (nBufLen == 0 || checksum != Utils::get_check_sum(pBuf+1, nBufLen-1))
		{
			m_SyncSocket.close();
			return false;
		}

		//buflen(4)+checksum(1)+bRet(4)+sn(4)+op(4)
		HUINT32 _ulSN = 0;
		memcpy(&_ulSN, pBuf+sizeof(HUINT8)+sizeof(HUINT32), sizeof(HUINT32));
		if (_ulSN == ulSN)
			break;

	} while (1);

	return true;
}

string OplogSynchronizer::get_ack_buffer(HUINT64 ullAckIndex)
{
	//checksum(1)+sn(4)+op(4)+ackindex(8)
	string _str;
	_str.append((const HCHAR *)&m_ulAckSN, sizeof(HUINT32));

	HUINT32 op = OP_ACK;
	_str.append((const HCHAR *)&op, sizeof(HUINT32));
	_str.append((const HCHAR *)&ullAckIndex, sizeof(HUINT64));

	HUINT8 checksum = Utils::get_check_sum(_str.c_str(), _str.size());
	HUINT32 dataLen = sizeof(HUINT8) + _str.size();

	string str;
	str.append((const HCHAR *)&dataLen, sizeof(HUINT32));
	str += checksum;
	str += _str;

	return str;
}

string OplogSynchronizer::get_sync_buffer()
{
	//checksum(1)+sn(4)+op(4)
	string _str;
	_str.append((const HCHAR *)&m_ulSyncSN, sizeof(HUINT32));

	HUINT32 op = OP_SYNC;
	_str.append((const HCHAR *)&op, sizeof(HUINT32));

	HUINT8 checksum = Utils::get_check_sum(_str.c_str(), _str.size());
	HUINT32 dataLen = sizeof(HUINT8) + _str.size();

	string str;
	str.append((const HCHAR *)&dataLen, sizeof(HUINT32));
	str += checksum;
	str += _str;

	return str;
}

HBOOL OplogSynchronizer::ack(HUINT64 ullAckIndex)
{
	RAISE_SN(m_ulAckSN);
	string str;
	return send_command(get_ack_buffer(ullAckIndex), m_ulAckSN, str);
}

HUINT64 OplogSynchronizer::sync(string &str)
{
	RAISE_SN(m_ulSyncSN);
	string strRecv;
	HUINT64 ullAckIndex = 0;
	if (send_command(get_sync_buffer(), m_ulSyncSN, strRecv))
	{
		//checksum(1)
		//bRet(4)
		//sn(4)
		//op(4)
		//totaltime(4)
		//processtime(4)
		//errinfolen(4)
		//errinfo(errinfolen)
		//ackindex(8)
		//[datalen(4)]
		//[data(datalen)]
		const HCHAR *pBuf = strRecv.c_str();

		HUINT32 bRet = 0;
		memcpy(&bRet, pBuf+sizeof(HUINT8), sizeof(HUINT32));

		HUINT32 offset = sizeof(HUINT8) +
			sizeof(HUINT32) +
			sizeof(HUINT32) +
			sizeof(HUINT32) +
			sizeof(HUINT32) +
			sizeof(HUINT32);

		HUINT32 ulErrInfoLen = 0;
		memcpy(&ulErrInfoLen, pBuf+offset, sizeof(HUINT32));
		offset += sizeof(HUINT32);

		//errinfo
		offset += ulErrInfoLen;

		memcpy(&ullAckIndex, pBuf+offset, sizeof(HUINT64));
		offset += sizeof(HUINT64);

		if (bRet > 0 && ullAckIndex > 0)
		{
			HUINT32 ulDataLen = 0;
			memcpy(&ulDataLen, pBuf+offset, sizeof(HUINT32));
			offset += sizeof(HUINT32);

			str.append(pBuf+offset, ulDataLen);
			offset += ulDataLen;
		}
		else if (bRet > 0)
		{
			str = "NODATA";
		}
	}

	return ullAckIndex;
}
