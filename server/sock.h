/**
 * winsock/BSD socket abstraction layer
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#include <winsock2.h>
typedef int socklen_t;
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
typedef int SOCKET;
#define closesocket close
#endif

void printError( const char *msg );
bool isSocketInvalid( SOCKET s );
bool isError( int code );
void beNonBlockingMode( SOCKET sock0 );

#ifdef _MSC_VER
#else
void Sleep( int ms );
#endif

