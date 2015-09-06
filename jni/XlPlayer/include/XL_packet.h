#ifndef _XL_packet_h
#define _XL_packet_h

#include "stdio.h"

#ifdef WIN32
#include "windows.h"

typedef SOCKET j_socket_t;
#define j_invalid_sosket INVALID_SOCKET
#define j_close_socket	closesocket

#else
#include <unistd.h>
#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <arpa/inet.h>

typedef int j_socket_t;
#define j_invalid_sosket -1
#define j_close_socket	close

#endif

typedef struct RtspPacket
{
	j_socket_t sock;
	char *send_buffer;
	int send_len;
	char *recv_buffer;
	int recv_len;
} RtspPacket;

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

extern RtspPacket *create_packet();
extern int initialize_packet(RtspPacket *sock, const char *addr, short port);
extern void destroy_packet(RtspPacket *sock);
extern void clear_recv_buff(RtspPacket *sock);
extern int send_packet(RtspPacket *sock);
extern int recv_packet(RtspPacket *sock, int offset);

#ifdef __cplusplus
}
#endif

#endif /* _XL_packet_h */