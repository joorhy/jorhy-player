#ifndef _XL_rtp_stream_h
#define _XL_rtp_stream_h

#include "XL_packet.h"

enum packet_state
{
	packet_init = 1,
	packet_head,
	packet_body,
};

enum stream_state
{
	stream_begin = 1,
	stream_continue,
	stream_complate,
};

enum frame_type
{
	frame_unknow = 1,
	frame_pps,
	frame_sps,
	frame_sei,
	frame_i,
	frame_p,
};

#pragma pack(push)
#pragma pack(1)
typedef struct RtpHeader
{
	unsigned short rtsp_head;
	unsigned short length;

	char csrc_count : 4;
	char x : 1;
	char p : 1;
	char vertion : 2;

	char m : 1;
	char payload_type : 7;

	unsigned short seq;
	unsigned int timestamp;
	unsigned int ssrc;
} RtpHeader;

typedef struct  NaluHeader
{
	//byte 0
	unsigned char TYPE : 5;
	unsigned char NRI : 2;
	unsigned char F : 1;
} NaluHeader; /**//* 1 BYTES */

typedef struct FuIndicator
{
	//byte 0
	unsigned char TYPE : 5;
	unsigned char NRI : 2;
	unsigned char F : 1;
} FuIndicator; /**//* 1 BYTES */

typedef struct FuHeader
{
	//byte 0
	unsigned char TYPE : 5;
	unsigned char R : 1;
	unsigned char E : 1;
	unsigned char S : 1;
} FuHeader; /**//* 1 BYTES */


typedef struct RtpStream
{
	char *frame_buffer;
	int frame_len;
	int packet_state;
	int stream_state;
	int frame_type;
} RtpStream;
#pragma pack(pop)

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

extern RtpStream *create_rtp_stream();
extern int initialize_rtp_stream(RtpStream *stream);
extern void destroy_rtp_stream(RtpStream *stream);

extern int read_rtp_packet(RtspPacket *pack, RtpStream *stream);
extern void process_rtp_packet(RtspPacket *sock, RtpStream *stream);

#ifdef __cplusplus
}
#endif

#endif /* _XL_rtp_stream_h */