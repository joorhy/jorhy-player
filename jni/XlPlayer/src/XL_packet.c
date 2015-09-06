#include "XL_packet.h"

#define MAX_PACKET_SIZE 8192

RtspPacket *create_packet()
{
	RtspPacket *pack = (RtspPacket *)malloc(sizeof(RtspPacket));
	memset(pack, 0, sizeof(RtspPacket));
	return pack;
}

int initialize_packet(RtspPacket *pack, const char *addr, short port)
{
	struct sockaddr_in sockaddrServer;
	int ret;
	int addr_len;

#ifdef WIN32
	WSADATA wsa={0};
    WSAStartup(MAKEWORD(2,2),&wsa);
#endif 
	pack->sock = socket(AF_INET, SOCK_STREAM, 0);

	addr_len = sizeof(sockaddrServer);
	memset(&sockaddrServer, 0, sizeof(sockaddrServer));
	sockaddrServer.sin_family = AF_INET;
	sockaddrServer.sin_addr.s_addr = inet_addr(addr);
	sockaddrServer.sin_port = htons(port);
	
	ret = connect(pack->sock, (struct sockaddr*)&sockaddrServer, addr_len);

	pack->recv_buffer = (char *)malloc(MAX_PACKET_SIZE);

	return 0;
}

void destroy_packet(RtspPacket *pack)
{
	if (pack->sock != j_invalid_sosket)
	{
		j_close_socket(pack->sock);
	}
	free(pack);
}

void clear_recv_buff(RtspPacket *pack)
{
	memset(pack->recv_buffer, 0, MAX_PACKET_SIZE);
}

int send_packet(RtspPacket *pack)
{
	send(pack->sock, pack->send_buffer, pack->send_len, 0);

	return 0;
}

int recv_packet(RtspPacket *pack, int offset)
{
	recv(pack->sock, pack->recv_buffer + offset, pack->recv_len, 0);
	printf("recv len = %d\n", pack->recv_len);

	return 0;
}