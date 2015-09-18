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

#ifdef USE_FFMPEG
#include "libavcodec\avcodec.h"
#else
#include "codec_api.h"
#include "codec_def.h"
#include "codec_app_def.h"
#include "codec_ver.h"
#endif

typedef struct H264Decoder
{
#ifdef USE_FFMPEG
	// for decoder resource(ffmpeg)
	AVCodecContext *context;
	AVFrame *picture;
	AVPacket packet;
	AVCodec *codec;
#else
	// for decoder resource(open h264)
	ISVCDecoder *dec;
	unsigned char* data[3];
	SBufferInfo bufInfo;
#endif
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