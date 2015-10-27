#include "XL_log.h"
#include "XL_rtp_stream.h"

#define MAX_FRAME_SIZE (1024 * 1024)

const char H264_HEAD[4] = { 0x00, 0x00, 0x00, 0x01 };

int get_frame_type(const char *data) {
	int frame_type = 0;
	switch (data[0] & 0x1F) {
	case 7:
		frame_type = frame_pps; break;
	case 8:
		frame_type = frame_sps; break;
	case 6:
		frame_type = frame_sei; break;
	case 5:
		frame_type = frame_i; break;
	case 1: 
		frame_type = frame_p; break;
	}
	return frame_type;
}

void process_rtp_packet(RtspPacket *pack, RtpStream *stream) {
	char *packet_data;
	char *frame_data;
	int forbidden_bit, nal_reference_idc, nal_unit_type;
	NaluHeader *nalu_hdr;

	if (pack->recv_len > 0 && (pack->recv_buffer[0] & 0xFF) == 0x24) {
		if ((pack->recv_buffer[1] & 0xFF) == 0x03) {// rtcp video packet
			pack->send_buffer = pack->recv_buffer;
			pack->send_len = pack->recv_len + 4;

			send_packet(pack);
		} else if ((pack->recv_buffer[1] & 0xFF) == 0x02) {// rtp video packet
			LOGI("video len = %d\n", pack->recv_len);

			packet_data = pack->recv_buffer + 16;
			if (pack->recv_buffer[5] & 0x80) {
				if (stream->stream_state == stream_begin) {
					frame_data = stream->frame_buffer + stream->frame_len;
					memcpy(frame_data, H264_HEAD, 4);
					stream->frame_len += 4;

					frame_data = stream->frame_buffer + stream->frame_len;
					memcpy(frame_data, packet_data, pack->recv_len - 12);
					stream->frame_len += pack->recv_len - 12;

					stream->frame_type = get_frame_type(packet_data);

					LOGI("this is complate frame\n");
				} else {
					frame_data = stream->frame_buffer + stream->frame_len;
					memcpy(frame_data, packet_data + 2, pack->recv_len - 12 - 2);
					stream->frame_len += pack->recv_len - 12 - 2;

					LOGI("this is a large frame\n");
				}
				stream->stream_state = stream_complate;
			} else {
				if (stream->stream_state == stream_begin) {
					forbidden_bit = (packet_data[0] & 0x80) >> 7;
					nal_reference_idc = (packet_data[0] & 0x60) >> 5;
					nal_unit_type = (packet_data[1]) & 0x1f;

					frame_data = stream->frame_buffer + stream->frame_len;
					memcpy(frame_data, H264_HEAD, 4);
					stream->frame_len += 4;

					frame_data = stream->frame_buffer + stream->frame_len;
					nalu_hdr = (NaluHeader *)frame_data;

					nalu_hdr->F = forbidden_bit;
					nalu_hdr->NRI = nal_reference_idc;
					nalu_hdr->TYPE = nal_unit_type;
					stream->frame_len += 1;

					frame_data = stream->frame_buffer + stream->frame_len;
					memcpy(frame_data, packet_data + 2, pack->recv_len - 12 - 2);
					stream->frame_len += pack->recv_len - 12 - 2;

					stream->frame_type = get_frame_type(packet_data + 1);
				} else {
					frame_data = stream->frame_buffer + stream->frame_len;
					memcpy(frame_data, packet_data + 2, pack->recv_len - 12 - 2);
					stream->frame_len += pack->recv_len - 12 - 2;
				}
				stream->stream_state = stream_continue;
			}
		} else if ((pack->recv_buffer[1] & 0xFF) == 0x01) { // rtp audio packet 
			pack->send_buffer = pack->recv_buffer;
			pack->send_len = pack->recv_len;

			send_packet(pack);
		} else if ((pack->recv_buffer[1] & 0xFF) == 0x00) {// rtp audio packet
			LOGI("audio len = %d\n", pack->recv_len);
		}
	}
}

RtpStream *create_rtp_stream() {
	RtpStream *stream = (RtpStream *)malloc(sizeof(RtpStream));
	memset(stream, 0, sizeof(RtpStream));
	stream->frame_buffer = (char *)malloc(MAX_FRAME_SIZE);
	stream->frame_len = 0;
	stream->packet_state = packet_init;
	stream->stream_state = stream_begin;

	return stream;
}

void destroy_rtp_stream(RtpStream *stream) {
	if (stream->frame_buffer)
		free(stream->frame_buffer);

	free(stream);
}

int read_rtp_packet(RtspPacket *pack, RtpStream *stream) {
	if (stream->packet_state == packet_init) {
		stream->packet_state = packet_body;

		pack->recv_len = 4;
		recv_packet(pack, 0);
	} else if (stream->packet_state == packet_head) {
		process_rtp_packet(pack, stream);
		clear_recv_buff(pack);
		stream->packet_state = packet_body;

		pack->recv_len = 4;
		recv_packet(pack, 0);
	} else {
		pack->recv_len = ntohs(*((short *)(pack->recv_buffer + 2)));

		stream->packet_state = packet_head;
		recv_packet(pack, 4);
	}

	return 0;
}