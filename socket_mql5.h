#define SOCKET_CLIENT_STATUS_CONNECTED		1
#define SOCKET_CLIENT_STATUS_DISCONNECTED	2
#define SOCKET_CONNECT_STATUS_ERROR			1000
#define SOCKET_CONNECT_STATUS_OK			0
#define SOCKET_CLIENT_WAIT_REPLY_TIMEOUT	400


#define HTONS(a) (((0xFF&a)<<8) + ((0xFF00&a)>>8))
#define CELL_SIZE  8

#define DLLExport __declspec( dllexport )

#define BUFFER_SIZE 256

typedef struct _SOCKET_CLIENT
{
	BYTE status;
	USHORT sequence;
	ULONG sock;
} SOCKET_CLIENT, *PSOCKET_CLIENT;


typedef struct _SOCKET_DATA
{
	char symbol[8];
	double datetime;
	double bid;
	double ask;

} SOCKET_DATA, PSOCKET_DATA;