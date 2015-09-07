#ifndef _XL_rtsp_session_h
#define _XL_rtsp_session_h

#include "XL_packet.h"
#include "XL_rtp_stream.h"
#include "XL_decoder.h"
#include "XL_surface.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

enum session_state
{
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
struct _RtspSession
{
	RtspSession *next;
	RtspPacket *packet;
	RtpStream *stream;
	H264Decoder *decoder;
	int session_state;
	int is_set;
	int index;
	int cseq;
	char addr[16];
	short port;
	char uri[32];
};

extern RtspSession *create_session();
extern void initialize_session(RtspSession *session, const char *addr, short port, const char *vec_id, int channel);
extern void destroy_session(RtspSession *session);

extern void recv_rtsp_str(RtspSession *session);
extern void session_process(JoSurface *surface, RtspSession *session);

extern void session_start(RtspSession *session);
extern void session_stop(RtspSession *session);

extern void set_session_index(RtspSession *session, int index);

extern void rtsp_option(RtspSession *session);
extern void rtsp_describe(RtspSession *session);
extern void rtsp_setup_video(RtspSession *session);
extern void rtsp_setup_audio(RtspSession *session);
extern void rtsp_play(RtspSession *session);
extern void rtsp_teardown(RtspSession *session);

#ifdef __cplusplus
}
#endif

#endif /* _XL_rtsp_session_h */