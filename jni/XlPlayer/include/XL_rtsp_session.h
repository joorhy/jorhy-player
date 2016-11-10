#ifndef _XL_rtsp_session_h
#define _XL_rtsp_session_h

#include "XL_packet.h"
#include "XL_rtp_stream.h"
#include "XL_decoder.h"
#include "XL_surface.h"
#include "XL_record.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

enum session_state {
	init_state = 1,
	option_state,
	describe_state,
	setup_video_state,
	setup_audio_state,
	play_state,
	stream_state,
	teardown_state,
};

typedef struct _RtspSession RtspSession;
struct _RtspSession {
	RtspSession *next;
	RtspPacket *packet;
	RtpStream *stream;
	H264Decoder *decoder;
	Record *record;
	int session_state;
	int is_set;
	int index;
	int cseq;
	char addr[16];
	short port;
	char uri[32];
};

extern RtspSession *create_session(const char *addr, short port, const char *vec_id, int channel, int index);
extern void destroy_session(RtspSession *session);
extern void session_start(RtspSession *session);
extern void session_process(JoSurface *surface, RtspSession *session);
extern void session_stop(RtspSession *session);

#ifdef __cplusplus
}
#endif

#endif /* _XL_rtsp_session_h */