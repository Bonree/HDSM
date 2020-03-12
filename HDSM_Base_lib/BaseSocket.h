#pragma once

#ifdef WIN32
#include <WinSock2.h>  
typedef int	socklen_t;  
#else
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <netdb.h>  
#include <fcntl.h>  
#include <unistd.h>  
#include <sys/stat.h>  
#include <sys/types.h>  
#include <arpa/inet.h>
#include <errno.h>
typedef int             SOCKET;
#define INVALID_SOCKET	(-1)
#define SOCKET_ERROR	(-1)
#endif

#include "TypeDefine.h"

class BaseSocket
{
public:
	BaseSocket();
	~BaseSocket(void);
public:
	HBOOL create(HINT32 af, HINT32 type, HINT32 protocol = 0);
	HINT32 connect(const HCHAR* ip, HUINT16 port);
	HINT32 bind(HUINT16 port);
	HINT32 listen(HINT32 backlog = 8);
	HBOOL accept(BaseSocket& s);
	HINT32 send(const HCHAR* buf, HINT32 len, HINT32 flags = 0);
	HINT32 recv(HCHAR* buf, HINT32 len, HINT32 flags = 0);
	HINT32 set_block_flag(HBOOL flag);
	HINT32 close();
	HBOOL is_valid();
	HBOOL wait_timeout(HUINT32 us);
	HUINT32 get_raw_socket();
public:
	static HINT32 init();
	static HINT32 clean();
	static HINT32 get_error();
public:
	BaseSocket& operator = (SOCKET s);
	operator SOCKET ();
protected:  
	SOCKET m_sock;  
};

