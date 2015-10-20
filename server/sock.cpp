/**
 * winsock/BSD socket abstraction layer
 */
#include "sock.h"

#ifdef _MSC_VER
void printError( const char *msg )
{
	fprintf( stderr, "%s : %d\n", msg, WSAGetLastError() );
}
bool isSocketInvalid( SOCKET s )
{
	return s == INVALID_SOCKET;
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

#else
bool isSocketInvalid( SOCKET s )
{
	return s < 0;
}
void printError( const char *msg )
{
	perror( msg );
}
bool isError( int code )
{
	return code < 0;
}
void beNonBlockingMode( int sock0 )
{
	int val = 1;
	if( ioctl( sock0, FIONBIO, &val ) < 0 )
		perror( "ioctl" );

/*
	int flag = fcntl( sock0, F_GETFL, 0 );
	if( flag < 0 )
	{
		perror( "fcntl(GET) error" );
		return;
	}
	if( fcntl( sock0, F_SETFL, flag|O_NONBLOCK ) < 0 )
		perror( "fcntl(NONBLOCK) error" );
*/		
}
void Sleep( int ms )
{
	struct timespec treq;
	treq.tv_sec = (time_t)( ms / 1000 );
	treq.tv_nsec = 1000000 * ms;
	nanosleep( &treq, NULL );
}
#endif
