#include "BaseSocket.h"
#include <stdio.h>

#ifdef WIN32  
#pragma comment(lib,"Ws2_32.lib")
#endif

BaseSocket::BaseSocket()
{
	m_sock = INVALID_SOCKET;
}


BaseSocket::~BaseSocket(void)
{
}

HINT32 BaseSocket::init()
{
#ifdef WIN32
    WSADATA wsaData;  
	HINT32 ret = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (ret != 0)
        return -1;
#endif
    return 0;
}

HINT32 BaseSocket::clean()
{
#ifdef WIN32
	return (WSACleanup());
#endif
	return 0;
}

HBOOL BaseSocket::create(HINT32 af, HINT32 type, HINT32 protocol)
{  
	m_sock = socket(af, type, protocol);  
	return (m_sock != INVALID_SOCKET);
}

HINT32 BaseSocket::connect(const HCHAR* ip, HUINT16 port)  
{  
	struct sockaddr_in svraddr;  
	svraddr.sin_family = AF_INET;  
	svraddr.sin_addr.s_addr = inet_addr(ip);  
	svraddr.sin_port = htons(port);  
	return ::connect(m_sock, (struct sockaddr*)&svraddr, sizeof(svraddr));
}

HINT32 BaseSocket::bind(HUINT16 port)
{
	struct sockaddr_in svraddr;
	svraddr.sin_family = AF_INET;
	svraddr.sin_addr.s_addr = INADDR_ANY;
	svraddr.sin_port = htons(port);

	HINT32 opt = 1;
	if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (HCHAR*)&opt, sizeof(opt)) < 0)
		return SOCKET_ERROR;

	return ::bind(m_sock, (struct sockaddr*)&svraddr, sizeof(svraddr));
}

HINT32 BaseSocket::listen(HINT32 backlog /* = 8 */)
{
	return ::listen(m_sock, backlog);  
}

HBOOL BaseSocket::accept(BaseSocket& s)
{
	struct sockaddr_in cliaddr;  
	socklen_t addrlen = sizeof(cliaddr);  
	SOCKET sock = ::accept(m_sock, (struct sockaddr*)&cliaddr, &addrlen);  
	if (sock == SOCKET_ERROR)  
		return false;

	s = sock;
	return true;
}

HINT32 BaseSocket::send(const HCHAR* buf, HINT32 len, HINT32 flags /* = 0 */)
{
	HINT32 bytes = -1;
	HINT32 count = 0;
	while ( count < len ) {
		bytes = ::send(m_sock, buf + count, len - count, flags);  
		if (bytes == -1 || bytes == 0)
			return -1; 
		count += bytes;
	}
	return count;
}

HINT32 BaseSocket::recv(HCHAR* buf, HINT32 len, HINT32 flags /* = 0 */)
{
	return (::recv(m_sock, buf, len, flags));
}

HINT32 BaseSocket::close()
{
	HINT32 ret = -1;
	if (m_sock != INVALID_SOCKET)
	{
#ifdef WIN32
		ret = (::closesocket(m_sock));
#else  
		ret = (::close(m_sock));  
#endif
		m_sock = INVALID_SOCKET;
	}
	return ret;
}

HINT32 BaseSocket::get_error()
{  
#ifdef WIN32  
	return (WSAGetLastError());  
#else  
	return (errno);  
#endif  
} 

BaseSocket& BaseSocket::operator = (SOCKET s)
{
	m_sock = s;
	return (*this);
}

HBOOL BaseSocket::is_valid()
{
	return (m_sock != INVALID_SOCKET);
}

BaseSocket::operator SOCKET ()  
{  
	return m_sock;  
}

HINT32 BaseSocket::set_block_flag(HBOOL flag)
{
#ifdef WIN32
	HULONG ul = flag ? 0 : 1;
	return ioctlsocket(m_sock, FIONBIO, &ul);
#else
	HUINT32 mode = fcntl(m_sock, F_GETFL, 0);
	return fcntl(m_sock, F_SETFL, flag ? mode&~O_NONBLOCK : mode|O_NONBLOCK);
#endif
}

HBOOL BaseSocket::wait_timeout(HUINT32 us)
{
	//使用select模型设置超时   
	timeval timeout; 
	timeout.tv_sec  = 0;
	timeout.tv_usec = us;

	fd_set fd;
	FD_ZERO(&fd);   
	FD_SET(m_sock, &fd);  
	return (select(0, NULL, &fd, NULL, &timeout) <= 0);
}

HUINT32 BaseSocket::get_raw_socket()
{
	return m_sock;
}