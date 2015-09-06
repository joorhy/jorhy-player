#ifndef _XL_decoder_h
#define _XL_decoder_h
#include "XL_rtp_stream.h"

#ifdef WIN32
#define inline __inline
#endif

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif
#include "SDL.h"
#include "libavcodec\avcodec.h"

typedef struct H264Decoder
{
	AVCodecContext *context;
	AVFrame *picture;
	AVPacket packet;
	AVCodec *codec;
	char *yuv;
	int yuv_len;
	SDL_Rect rect;
} H264Decoder;

extern H264Decoder *create_decoder();
extern int initialize_decoder(H264Decoder *decoder);
extern void destroy_decoder(H264Decoder *decoder);
extern int decode_frame(H264Decoder *decoder, RtpStream *stream);
extern int copy_data(H264Decoder *decoder);

#ifdef __cplusplus
}
#endif

#endif /* _XL_decoder_h */