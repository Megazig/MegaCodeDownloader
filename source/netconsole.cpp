#include "netconsole.h"

s32 connection;
u8 connected = 0;
char *buffer;

void NCconnect(u32 ipaddress)
{
	buffer = (char*)malloc(1024);
	connection = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if(connection < 0) {
		//printf("Unable to create socket\n");
		return;
	}
	
	struct sockaddr_in connect_addr;
	memset(&connect_addr, 0, sizeof(connect_addr));
	connect_addr.sin_family = AF_INET;
	connect_addr.sin_port = 10001;
	connect_addr.sin_addr.s_addr = ipaddress;
	
	if(net_connect(connection, (struct sockaddr*)&connect_addr, sizeof(connect_addr)) == -1) {
		net_close(connection);
		//printf("Unable to connect\n");
		return;
	}
	connected = 1;
	//printf("Connected... \n");
	
}

void NCsend(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if(connected) {
		vsprintf(buffer, fmt, args);
		net_write(connection, buffer, strlen(buffer));
	}
}
