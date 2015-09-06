#include "XL_rtsp_session.h"

const char *rtsp_end = "\r\n\r\n";
const char *content_length = "Content-Length:";

/*Option method for rtsp protocol*/
static const char *option = 
"OPTIONS rtsp://222.214.218.237:6601/LDMsMTI4MTIsMSwxLDAsMA== RTSP/1.0\r\n"
"CSeq: 1\r\n"
"User-Agent: GPSViewer (6.1.0.1 2012-06-14)\r\n\r\n";

/*Describe method for rtsp protocol*/
static const char *describe = 
"DESCRIBE rtsp://222.214.218.237:6601/LDMsMTI4MTIsMSwxLDAsMA== RTSP/1.0\r\n"
"CSeq: 2\r\n"
"User-Agent: GPSViewer (6.1.0.1 2012-06-14)\r\n\r\n";

/*Setup method for rtsp protocol*/
static const char *setup_a = 
"SETUP rtsp://222.214.218.237:6601/LDMsMTI4MTIsMSwxLDAsMA== RTSP/1.0\r\n"
"CSeq: 3\r\n"
"User-Agent: GPSViewer (6.1.0.1 2012-06-14)\r\n"
"Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n\r\n";

/*Setup method for rtsp protocol*/
static const char *setup_v = 
"SETUP rtsp://222.214.218.237:6601/LDMsMTI4MTIsMSwxLDAsMA== RTSP/1.0\r\n"
"CSeq: 4\r\n"
"User-Agent: GPSViewer (6.1.0.1 2012-06-14)\r\n"
"Session: AAF8C703304D46E8BF366802C1D5CF7E\r\n"
"Transport: RTP/AVP/TCP;unicast;interleaved=2-3\r\n\r\n";

/*Play method for rtsp progocol*/
static const char *play = 
"PLAY rtsp://222.214.218.237:6601/LDMsMTI4MTIsMSwxLDAsMA== RTSP/1.0\r\n"
"CSeq: 5\r\n"
"User-Agent: GPSViewer (6.1.0.1 2012-06-14)\r\n"
"Session: 21B46086FD2E4240BB1ACCD307E8A351\r\n"
"Range: npt=0.000-\r\n\r\n";

RtspSession *create_session()
{
	RtspSession *session = (RtspSession *)malloc(sizeof(RtspSession));
	memset(session, 0, sizeof(RtspSession));

	return session;
}

void initialize_session(RtspSession *session, const char *addr, short port)
{
	session->session_state = init_state;

	session->packet = create_packet();
	initialize_packet(session->packet, addr, port);

	session->stream = create_rtp_stream();
	initialize_rtp_stream(session->stream);

	session->decoder = create_decoder();
	initialize_decoder(session->decoder);
}

void destroy_session(RtspSession *session)
{
	destroy_packet(session->packet);
	destroy_rtp_stream(session->stream);
	destroy_decoder(session->decoder);
}

void recv_rtsp_str(RtspSession *session)
{
	int offset = 0;
	char *pContent;
	clear_recv_buff(session->packet);
	while (strstr(session->packet->recv_buffer, rtsp_end) == NULL)
	{
		session->packet->recv_len = 1;
		recv_packet(session->packet, offset);
		offset++;
	}

	pContent = strstr(session->packet->recv_buffer, content_length);
	if (pContent != NULL)
	{
		pContent += strlen(content_length);
		while (*pContent == ' ')
		{
			pContent++;
		}
		session->packet->recv_len = atoi(pContent);
		recv_packet(session->packet, offset);
	}
}

void session_process(JoSurface *surface, RtspSession *session)
{
	if (session->session_state == stream_state)
	{
		read_rtp_packet(session->packet, session->stream);
		if (session->stream->stream_state == stream_complate)
		{
			if (session->stream->frame_type == frame_i || session->stream->frame_type == frame_p)
			{
				decode_frame(session->decoder, session->stream);
				render_frame(surface, session->decoder, session->index);

				session->stream->stream_state = stream_begin;
				session->stream->frame_len = 0;
				session->stream->frame_type = frame_unknow;
			}
			else
			{
				session->stream->stream_state = stream_begin;
			}
		}
	}
	else
	{
		recv_rtsp_str(session);
		switch (session->session_state)
		{
		case option_state:
			rtsp_describe(session);
			break;
		case describe_state:
			rtsp_setup_video(session);
			break;
		case setup_video_state:
			rtsp_play(session);
			break;
		case play_state:
			session->session_state = stream_state;
			break;
		default:
			break;
		}
	}
}

void session_start(RtspSession *session)
{
	rtsp_option(session);
}

 void session_stop(RtspSession *session)
 {

 }

 void rtsp_option(RtspSession *session)
 {
	session->session_state = option_state;

	session->packet->send_buffer = (char *)option;
	session->packet->send_len = strlen(option);
	send_packet(session->packet);
 }

void rtsp_describe(RtspSession *session)
{
	session->session_state = describe_state;

	session->packet->send_buffer = (char *)describe;
	session->packet->send_len = strlen(describe);
	send_packet(session->packet);
}

void rtsp_setup_video(RtspSession *session)
{
	session->session_state = setup_video_state;

	session->packet->send_buffer = (char *)setup_v;
	session->packet->send_len = strlen(setup_v);
	send_packet(session->packet);
}

void rtsp_setup_audio(RtspSession *session)
{
	session->session_state = setup_audio_state;

	session->packet->send_buffer = (char *)setup_a;
	session->packet->send_len = strlen(setup_a);
	send_packet(session->packet);
}

void rtsp_play(RtspSession *session)
{
	session->session_state = play_state;

	session->packet->send_buffer = (char *)play;
	session->packet->send_len = strlen(play);
	send_packet(session->packet);
}

void rtsp_teardown(RtspSession *session)
{
}