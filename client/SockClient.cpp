/**
 * socket manager for client
 */
#include "SockClient.h"

#define PORT 12345

void printError( const char *msg )
{
	fprintf( stderr, "%s : %d\n", msg, WSAGetLastError() );
}
bool isError( int code )
{
	return code == SOCKET_ERROR;
}
void beNonBlockingMode( SOCKET sock0 )
{
	u_long iMode = 1;
	if( isError( ioctlsocket( sock0, FIONBIO, &iMode ) ) )
		printError( "ioctlsocket" );
}



SockClient::SockClient()
{
	m_sock = INVALID_SOCKET;
}

bool SockClient::IsAlive()
{
	return m_sock != INVALID_SOCKET;
}

void SockClient::Main()
{
	if( !IsAlive() )
		return;

	for(;;)
	{
		char buf[32];
		memset( buf, 0, sizeof(buf) );
		int n = recv( m_sock, buf, sizeof(buf) - 1, 0 );
	 
		if( n < 1 )
		{
			if( WSAGetLastError() == WSAEWOULDBLOCK )
			{
			}
			else
			{
				printf( "recv : %d\n", WSAGetLastError() );
				Finalize();
			}
			break;
		}
		
		m_buf += buf;
	}
}

std::string SockClient::Recv()
{
	std::string::size_type pos = m_buf.find( '\n' );
	if( std::string::npos == pos )
		return "";

	int size = pos + 1;
	std::string tmp( m_buf.begin(), m_buf.begin() + size );
	m_buf.erase( 0, size );
	return tmp;
}

void SockClient::Send( const char *str )
{
	if( !IsAlive() )
		return;

	std::string s = str;
	s += "\n";

	int n = send( m_sock, s.c_str(), s.size(), 0 );
	if( n < 0 )
	{
		printf( "send : %d\n", WSAGetLastError() );
	}
}

void SockClient::Finalize()
{
	if( m_sock != INVALID_SOCKET )
	{
		closesocket( m_sock );
	}
	m_sock = INVALID_SOCKET;
	m_buf = "";
}

bool SockClient::Start( const char *target )
{
	if( IsAlive() )
		Finalize();

	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET)
	{
		printf("socket : %d\n", WSAGetLastError());
		return false;
	}

	struct sockaddr_in server;

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);

	server.sin_addr.S_un.S_addr = inet_addr(target);
 
	if (server.sin_addr.S_un.S_addr == 0xffffffff)
	{
		struct hostent *host;

		host = gethostbyname(target);
		if (host == NULL)
		{
			if (WSAGetLastError() == WSAHOST_NOT_FOUND)
			{
				 printf("host not found : %s\n", target);
			}
			return false;
		}

		unsigned int **addrptr;
		addrptr = (unsigned int **)host->h_addr_list;

		while (*addrptr != NULL)
		{
			server.sin_addr.S_un.S_addr = *(*addrptr);

			if (connect(m_sock,
				(struct sockaddr *)&server,
				sizeof(server)) == 0) {
				break;	// success
			}

			addrptr++;	// go next address if connect failed
		}

		if (*addrptr == NULL)
		{
			printf("connect : %d\n", WSAGetLastError());
			return false;
		}
	}
	else
	{
		if (connect(m_sock,
					 (struct sockaddr *)&server,
					 sizeof(server)) != 0)
		{
			printf("connect : %d\n", WSAGetLastError());
			return false;
		}
	}
	beNonBlockingMode( m_sock );

	return true;	
}