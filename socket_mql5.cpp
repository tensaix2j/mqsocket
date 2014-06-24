//--------------------------------------------------------------------------//
//	SOCKET client DLL Library
//--------------------------------------------------------------------------//

#define _CRT_SECURE_NO_DEPRECATE
#include <windows.h>
#include "socket_mql5.h"
#pragma comment(lib, "wsock32.lib")

//--------------------------------------------------------------------------//
ULONG my_rand()
//--------------------------------------------------------------------------//
{
	_asm
	{
		rdtsc
	}
	
}


//--------------------------------------------------------------------------//
ULONG Host2Ip(char * host)
//--------------------------------------------------------------------------//
{
	struct hostent * p;
	ULONG ret;
	p = gethostbyname(host);
	if (p)
	{
		ret = *(ULONG*)(p->h_addr);
	}
	else
	{
		ret = INADDR_NONE;
	}
	return ret;
}

//--------------------------------------------------------------------------//
ULONG ConnectToServer(char * host, USHORT port)
//--------------------------------------------------------------------------//
{
	struct sockaddr_in addr;
	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);

	ULONG ip;
	ULONG sock = INVALID_SOCKET;
	
	ip = Host2Ip(host);
	if (ip != INADDR_NONE)
	{
		addr.sin_addr.S_un.S_addr = ip;
		addr.sin_port = HTONS(port);
		
		if (addr.sin_addr.S_un.S_addr != INADDR_NONE)
		{
			addr.sin_family = AF_INET;
			sock = (ULONG)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//IPPROTO_TCP
			
			int option = 1;
			setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option) );
			

			if (sock != INVALID_SOCKET)
			{
				if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)))
				{
					closesocket(sock);
					sock = INVALID_SOCKET;
				}
			}
		}
	}
	
	return sock;
}

//--------------------------------------------------------------------------//
ULONG DLLExport SocketOpen(PSOCKET_CLIENT client, wchar_t * wc_host, USHORT port)
//--------------------------------------------------------------------------//
{
	ULONG ret;

	char *host  = new char[wcslen(wc_host) + 1]; 
	wcstombs(host, wc_host, wcslen(wc_host) + 1);
	
	client->status = SOCKET_CLIENT_STATUS_DISCONNECTED;
	client->sequence = (USHORT)my_rand();
	
	client->sock = ConnectToServer(host, port);
	
	if (client->sock == INVALID_SOCKET)
	{	
		ret = SOCKET_CONNECT_STATUS_ERROR;
		closesocket(client->sock);
	}
	else
	{
		client->status = SOCKET_CLIENT_STATUS_CONNECTED;
		ret = SOCKET_CONNECT_STATUS_OK;
	}
	delete(host);

	return(ret);
}
//--------------------------------------------------------------------------//
void DLLExport SocketClose(PSOCKET_CLIENT client)
//--------------------------------------------------------------------------//
{
	if (client->status == SOCKET_CLIENT_STATUS_CONNECTED)
	{
		closesocket(client->sock);
		client->status = SOCKET_CLIENT_STATUS_DISCONNECTED;
	}
}

//--------------------------------------------------------------------------//
ULONG DLLExport SocketWriteData(PSOCKET_CLIENT client, wchar_t *symbol, double td, double bid, double ask)
//--------------------------------------------------------------------------//
{
	char data[CELL_SIZE*4];
	ULONG ret = SOCKET_CONNECT_STATUS_ERROR;
	
	char * val_para  = new char[wcslen(symbol) + 1]; 
	wcstombs(val_para, symbol, wcslen(symbol) + 1);

	if (client->status == SOCKET_CLIENT_STATUS_CONNECTED)
	{
		// transform string
		memset(data, 0, CELL_SIZE*4); 
		memcpy(&data[0], val_para, min(strlen(val_para), CELL_SIZE ));
		memcpy(&data[CELL_SIZE]  ,  &td, CELL_SIZE );
		memcpy(&data[CELL_SIZE*2], &bid, CELL_SIZE );
		memcpy(&data[CELL_SIZE*3], &ask, CELL_SIZE );

		// send message
		if (send(client->sock, data, CELL_SIZE*4, 0) != CELL_SIZE*4)
		{
			client->status = SOCKET_CLIENT_STATUS_DISCONNECTED;
			ret = SOCKET_CONNECT_STATUS_ERROR;
			closesocket(client->sock);
			
		}
		else
		{
			ret = SOCKET_CONNECT_STATUS_OK;
		}
	}

	delete (val_para);
	return ret;
}


//--------------------------------------------------------------------------//
ULONG DLLExport SocketWriteString(PSOCKET_CLIENT client, wchar_t *wstr)
//--------------------------------------------------------------------------//
{
	
	ULONG ret = SOCKET_CONNECT_STATUS_ERROR;
	
	char * str  = new char[wcslen(wstr) + 1]; 
	wcstombs(str, wstr, wcslen(wstr) + 1);

	if (client->status == SOCKET_CLIENT_STATUS_CONNECTED)
	{
		// send message
		if ( send(client->sock, str, strlen(str), 0) != (int)strlen(str))
		{
			client->status = SOCKET_CLIENT_STATUS_DISCONNECTED;
			ret = SOCKET_CONNECT_STATUS_ERROR;
			closesocket(client->sock);
			
		}
		else
		{
			ret = SOCKET_CONNECT_STATUS_OK;
		}
	}

	delete (str);
	return ret;
}


//-----
void DLLExport TestFillString( wchar_t *input ) {
	
	char *mystring = "hello world";
	mbstowcs( input, mystring, strlen(mystring));
}


//------------------------
ULONG DLLExport SocketReadString( PSOCKET_CLIENT client, wchar_t *buffer , int timeout_ms ) {
	
	ULONG ret = SOCKET_CONNECT_STATUS_ERROR;

	fd_set read_set;
	struct timeval timeout;

	timeout.tv_sec =   timeout_ms / 1000; // Time out after a minute
	timeout.tv_usec = (timeout_ms % 1000) * 1000;
	
	FD_ZERO(&read_set);
	FD_SET( client->sock , &read_set);

	int r = select( 1, &read_set, NULL, NULL, &timeout);

	if( r<0 ) {
		// Handle the error
		client->status = SOCKET_CLIENT_STATUS_DISCONNECTED;
		ret = SOCKET_CONNECT_STATUS_ERROR;
		closesocket(client->sock);
	}

	if( r==0 ) {
		// Timeout - handle that. You could try waiting again, close the socket...
		ret = SOCKET_CLIENT_WAIT_REPLY_TIMEOUT;
	}

	if( r>0 ) {
		// The socket is ready for reading - call read() on it.
		char strbuffer[BUFFER_SIZE];
		int bytesread = recv( client->sock, strbuffer, BUFFER_SIZE, 0) ;
		strbuffer[bytesread] = 0;
		mbstowcs (buffer, strbuffer, strlen(strbuffer) );
			
		ret = SOCKET_CONNECT_STATUS_OK;		
	}
		
	return ret;
}



//--------------------------------------------------------------------------//
ULONG DLLExport SocketWriteString_ExpectReply(PSOCKET_CLIENT client, wchar_t *wstr , wchar_t *reply , int timeout_ms )
//--------------------------------------------------------------------------//
{
	
	ULONG ret = SOCKET_CONNECT_STATUS_ERROR;
	
	char * str  = new char[wcslen(wstr) + 1]; 
	wcstombs(str, wstr, wcslen(wstr) + 1);

	if (client->status == SOCKET_CLIENT_STATUS_CONNECTED)
	{
		// send message
		if ( send(client->sock, str, strlen(str), 0) != (int)strlen(str))
		{
			client->status = SOCKET_CLIENT_STATUS_DISCONNECTED;
			ret = SOCKET_CONNECT_STATUS_ERROR;
			closesocket(client->sock);
			
		}
		else
		{
			ret = SocketReadString( client, reply , timeout_ms ) ;
		}
	}

	delete (str);
	return ret;
}



//--------------------------------------------------------------------------//
BOOL __stdcall DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
//--------------------------------------------------------------------------//
{	
	WSADATA ws;
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH: 
			WSAStartup(0x202, &ws);			
			break;
		case DLL_PROCESS_DETACH:
			WSACleanup();
			break;
	}
	return 1;
}
