/**
 * socket manager for client
 */
#pragma once
#include <windows.h>
#include <string>

class SockClient
{
	SOCKET m_sock;
	std::string m_buf;
public:
	void Finalize();
	SockClient();
	bool Start( const char *target );
	void Main();
	void Send( const char *str );
	bool IsAlive();
	std::string Recv();
};
