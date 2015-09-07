#include "XL_rtsp_session.h"
#include "XL_base64.h"

const char *rtsp_end = "\r\n\r\n";
const char *content_length = "Content-Length:";

/*uri source string*/
static const char *src_uri = ",3,%s,%d,1,0,0";

/*Option method for rtsp protocol*/
static const char *option = 
"OPTIONS rtsp://%s:%d/%s RTSP/1.0\r\n"
"CSeq: %d\r\n"
"User-Agent: GPSViewer (6.1.0.1 2012-06-14)\r\n\r\n";

/*Describe method for rtsp protocol*/
static const char *describe = 
"DESCRIBE rtsp://%s:%d/%s RTSP/1.0\r\n"
"CSeq: %d\r\n"
"User-Agent: GPSViewer (6.1.0.1 2012-06-14)\r\n\r\n";

/*Setup method for rtsp protocol*/
static const char *setup_a = 
"SETUP rtsp://%s:%d/%s RTSP/1.0\r\n"
"CSeq: %d\r\n"
"User-Agent: GPSViewer (6.1.0.1 2012-06-14)\r\n"
"Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n\r\n";

/*Setup method for rtsp protocol*/
static const char *setup_v = 
"SETUP rtsp://%s:%d/%s RTSP/1.0\r\n"
"CSeq: %d\r\n"
"User-Agent: GPSViewer (6.1.0.1 2012-06-14)\r\n"
"Session: AAF8C703304D46E8BF366802C1D5CF7E\r\n"
"Transport: RTP/AVP/TCP;unicast;interleaved=2-3\r\n\r\n";

/*Play method for rtsp protocol*/
static const char *play = 
"PLAY rtsp://%s:%d/%s RTSP/1.0\r\n"
"CSeq: %d\r\n"
"User-Agent: GPSViewer (6.1.0.1 2012-06-14)\r\n"
"Session: 21B46086FD2E4240BB1ACCD307E8A351\r\n"
"Range: npt=0.000-\r\n\r\n";

/*Teardown method for rtsp protocol*/
static const char *teardown =
"TEARDOWN rtsp://%s:%d/%s RTSP/1.0\r\n"
"CSeq: %d\r\n"
"User-Agent: GPSViewer (6.1.0.1 2012-06-14)\r\n\r\n";

RtspSession *create_session()
{
	RtspSession *session = (RtspSession *)malloc(sizeof(RtspSession));
	memset(session, 0, sizeof(RtspSession));

	return session;
}

void initialize_session(RtspSession *session, const char *addr, short port, const char *vec_id, int channel)
{
	session->session_state = init_state;
	session->cseq = 1;
	memcpy(session->addr, addr, strlen(addr));
	session->port = port;

	/*make base64 code of uri*/
	char uri[64] = { 0 };
#ifdef WIN32
	sprintf_s(uri, sizeof(uri), src_uri, vec_id, channel);
#else
	sprintf(uri, src_uri, vec_id, channel);
#endif
	base64_in(uri, session->uri, strlen(uri));

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

 void set_session_index(RtspSession *session, int index)
 {
	 session->index = index;
 }

 void rtsp_option(RtspSession *session)
 {
	char cmd_buffer[1024] = { 0 };
	session->session_state = option_state;

#ifdef WIN32
	sprintf_s(cmd_buffer, sizeof(cmd_buffer), option, session->addr, session->port, session->uri, session->cseq++);
#else
	sprintf(cmd_buffer, option, session->addr, session->port, session->uri, session->cseq++);
#endif
	session->packet->send_buffer = (char *)cmd_buffer;
	session->packet->send_len = strlen(cmd_buffer);
	send_packet(session->packet);
 }

void rtsp_describe(RtspSession *session)
{
	char cmd_buffer[1024] = { 0 };
	session->session_state = describe_state;

#ifdef WIN32
	sprintf_s(cmd_buffer, sizeof(cmd_buffer), describe, session->addr, session->port, session->uri, session->cseq++);
#else
	sprintf(cmd_buffer, describe, session->addr, session->port, session->uri, session->cseq++);
#endif
	session->packet->send_buffer = (char *)cmd_buffer;
	session->packet->send_len = strlen(cmd_buffer);
	send_packet(session->packet);
}

void rtsp_setup_video(RtspSession *session)
{
	char cmd_buffer[1024] = { 0 };
	session->session_state = setup_video_state;

#ifdef WIN32
	sprintf_s(cmd_buffer, sizeof(cmd_buffer), setup_v, session->addr, session->port, session->uri, session->cseq++);
#else
	sprintf(cmd_buffer, setup_v, session->addr, session->port, session->uri, session->cseq++);
#endif
	session->packet->send_buffer = (char *)cmd_buffer;
	session->packet->send_len = strlen(cmd_buffer);
	send_packet(session->packet);
}

void rtsp_setup_audio(RtspSession *session)
{
	char cmd_buffer[1024] = { 0 };
	session->session_state = setup_audio_state;

#ifdef WIN32
	sprintf_s(cmd_buffer, sizeof(cmd_buffer), setup_a, session->addr, session->port, session->uri, session->cseq++);
#else
	sprintf(cmd_buffer, setup_a, session->addr, session->port, session->uri, session->cseq++);
#endif
	session->packet->send_buffer = (char *)cmd_buffer;
	session->packet->send_len = strlen(cmd_buffer);
	send_packet(session->packet);
}

void rtsp_play(RtspSession *session)
{
	char cmd_buffer[1024] = { 0 };
	session->session_state = play_state;

#ifdef WIN32
	sprintf_s(cmd_buffer, sizeof(cmd_buffer), play, session->addr, session->port, session->uri, session->cseq++);
#else
	sprintf(cmd_buffer, play, session->addr, session->port, session->uri, session->cseq++);
#endif
	session->packet->send_buffer = (char *)cmd_buffer;
	session->packet->send_len = strlen(cmd_buffer);
	send_packet(session->packet);
}

void rtsp_teardown(RtspSession *session)
{
	char cmd_buffer[1024] = { 0 };
	session->session_state = teardown_state;

#ifdef WIN32
	sprintf_s(cmd_buffer, sizeof(cmd_buffer), teardown, session->addr, session->port, session->uri, session->cseq++);
#else
	sprintf(cmd_buffer, teardown, session->addr, session->port, session->uri, session->cseq++);
#endif
	session->packet->send_buffer = (char *)cmd_buffer;
	session->packet->send_len = strlen(cmd_buffer);
	send_packet(session->packet);
}