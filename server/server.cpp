/**
 * chat server
 */
#include "sock.h"
#include <list>
#include <vector>
#include <algorithm>
#include <string>

typedef int s32;

class Client
{
	SOCKET m_sock;
	int m_progress;
	bool m_alive;
	std::string m_recvBuf;

	void _finalize()
	{
		if( m_alive )
			closesocket( m_sock );
		m_alive = false;
	}

public:

	Client( SOCKET sock )
	{
		m_sock = sock;
		m_progress = 0;
		m_alive = true;
	}

	~Client()
	{
	}

	bool IsAlive(){ return m_alive; }

	void Recv()
	{
		if( !m_alive )
			return;

		char buf[ 512 ];
		int len;
		for(;;)
		{
			memset( buf, 0, sizeof(buf) );
			len = recv( m_sock, buf, sizeof(buf) - 1, 0 );
			if( isError( len ) )
			{
			//	printError( "recv" );
				return;
			}
			if( len == 0 )
				return;

			m_recvBuf += buf;
			printf( "recv(len=%d):%s", len, buf );
		}
	}

	bool GetPacket( std::string *buf )
	{
		if( !m_alive )
			return false;

		std::string::size_type pos = m_recvBuf.find( '\n' );
		if( pos == std::string::npos )
			return false;

		int size = pos + 1;

		*buf = m_recvBuf.substr( 0, size );

		m_recvBuf.erase( 0, size );

		return true;
	}

	void Send( std::string *buf )
	{
		if( !m_alive )
			return;

//		printf( "send(len=%d):%s", buf->length(), buf->c_str() );

		if( isError( send( m_sock, buf->c_str(), buf->size(), 0 ) ) )
		{
			printError( "send" );
			_finalize();
		}
	}
};

typedef std::list<Client*> ClientList;
static ClientList g_clientList;

static void _recvFromChilds()
{
	for( ClientList::iterator i = g_clientList.begin(); i != g_clientList.end(); i++ )
	{
		(*i)->Recv();
	}
}

static bool _sendToChilds()
{
	std::string bufToSend;
	bool result = false;

	for( ClientList::iterator i = g_clientList.begin(); i != g_clientList.end(); i++ )
	{
		std::string buf;
		while( (*i)->GetPacket( &buf ) )
		{
//			printf( "GetPacket(len=%d):%s", buf.length(), buf.c_str() );
			bufToSend += buf;
		}
	}

	if( bufToSend.size() > 0 )
	{
		for( ClientList::iterator j = g_clientList.begin(); j != g_clientList.end(); j++ )
		{
			(*j)->Send( &bufToSend );
			result = true;
		}
	}
	return result;
}

static void _cleanChilds()
{
	for( ClientList::iterator i = g_clientList.begin(); i != g_clientList.end(); )
	{
		if( !(*i)->IsAlive() )
		{
			delete *i;
			g_clientList.erase( i++ );
		}
		else
		{
			i++;
		}
	}
}

static void _progressChilds()
{
	_recvFromChilds();

	_cleanChilds();

	bool result = _sendToChilds();

	_cleanChilds();
	
	if( !result )
		Sleep( 10 );
}

static void _handleClients( SOCKET sock0 )
{
	struct sockaddr_in client;
	socklen_t len;
	SOCKET sock;

	len = sizeof(client);
	sock = accept(sock0, (struct sockaddr *)&client, &len);

	if( isSocketInvalid( sock ) )
	{
#if _MSC_VER
		int err = WSAGetLastError();
		if( err == WSAEWOULDBLOCK )
#else
		if( errno == EAGAIN )
#endif
		{
		//	printf( "accept : WSAEWOULDBLOCK  waiting... nClients=%d\n", g_clientList.size() );
			_progressChilds();
			return;
		}
		printError( "accept" );
	}
	else
	{
		beNonBlockingMode( sock );

		g_clientList.push_back( new Client( sock ) );
//		printf( "nClients=%d\n", g_clientList.size() );

/*
		memset(inbuf, 0, sizeof(inbuf));
		recv(sock, inbuf, sizeof(inbuf), 0);
		printf("%s", inbuf);

		send(sock, buf, (int)strlen(buf), 0);
*/
	}

}

/**
 * 
 */
static void _sub()
{
	SOCKET sock0;
	struct sockaddr_in addr;
	int yes = 1;

	char buf[2048];

	sock0 = socket(AF_INET, SOCK_STREAM, 0);
	if( isSocketInvalid( sock0 ) )
	{
		printError( "socket" );
		return;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	addr.sin_addr.s_addr = INADDR_ANY;

 setsockopt(sock0,
   SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

	if( isError( bind( sock0, (struct sockaddr *)&addr, sizeof(addr) ) ) )
	{
		printError( "bind" );
		return;
	}

	if( isError( listen( sock0, 5 ) ) )
	{
		printError( "listen" );
		return;
	}

	memset(buf, 0, sizeof(buf));
	strcpy(buf, 
	 "HTTP/1.0 200 OK\r\n"
	//	 "Content-Length: 20\r\n"
	 "Content-Type: text/html\r\n"
	 "\r\n"
	 "HELLO 0123456789012345678901234567890123456789\r\n");

	// non-blocking mode
	beNonBlockingMode( sock0 );

	for(;;)
	{
		int nClient = g_clientList.size();
		_handleClients( sock0 );
		if( nClient != g_clientList.size() )
			printf( "number of clients: %d\n", g_clientList.size() );
	}
}


/**
 * main
 */
int main( int argc, char *argv[])
{
	printf( "Petit Chat Server ver.1.0\n" );

	#ifdef _MSC_VER
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2,0), &wsaData);
	#else
		// http://www.paw.hi-ho.ne.jp/takadayouhei/technic/58.html
		signal( SIGPIPE , SIG_IGN ); 
	#endif

	try
	{
		_sub();
	}
	catch( std::bad_alloc )
	{
		fprintf( stderr, "out of memory!\n" );
	}

	g_clientList.resize( 0 );

	#ifdef _MSC_VER
		WSACleanup();
	#endif
}