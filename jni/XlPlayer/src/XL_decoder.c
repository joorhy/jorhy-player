#include "XL_log.h"
#include "XL_decoder.h"

#ifdef USE_FFMPEG
#pragma comment(lib, "lib\\avcodec.lib")
#pragma comment(lib, "lib\\avutil.lib")
#else
#pragma comment(lib, "lib\\libopenh264.lib")
#endif

#define MAX_YUV_SIZE (1024 * 1024 * 2)

H264Decoder *create_decoder()
{
	H264Decoder *decoder = (H264Decoder *)malloc(sizeof(H264Decoder));
	memset(decoder, 0, sizeof(H264Decoder));

	return decoder;
}

int initialize_decoder(H264Decoder *decoder)
{
#ifdef USE_FFMPEG
#if LIBAVCODEC_VERSION_MAJOR < 54
	avcodec_init();
#endif
	avcodec_register_all();

	decoder->codec = avcodec_find_decoder(CODEC_ID_H264);
	decoder->context = avcodec_alloc_context3(decoder->codec);
	decoder->picture = av_frame_alloc();
	av_init_packet(&decoder->packet);
	if (!decoder->codec || !decoder->context || !decoder->picture)
	{
		return 0;
	}

	if (decoder->codec->capabilities & CODEC_CAP_TRUNCATED)
	{
		decoder->context->flags |= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */
	}

	if (avcodec_open2(decoder->context, decoder->codec, NULL) < 0)
	{
		return 0;
	}
#else
	long rv = WelsCreateDecoder(&decoder->dec);
	if (rv != 0)
	{
		return 0;
	}

	SDecodingParam decParam;
	memset(&decParam, 0, sizeof(SDecodingParam));
	decParam.eOutputColorFormat = videoFormatI420;
	decParam.uiTargetDqLayer = UCHAR_MAX;
	decParam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
	decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;

	rv = (*decoder->dec)->Initialize(decoder->dec, &decParam);
	if (rv != 0)
	{
		return 0;
	}
#endif

	decoder->yuv = (char *)malloc(MAX_YUV_SIZE);

	return 0;
}

void destroy_decoder(H264Decoder *decoder)
{
#ifdef USE_FFMPEG
	if (decoder->context)
	{
		avcodec_close(decoder->context);
		av_free(decoder->context);
	}
	if (decoder->picture)
	{
		av_free(decoder->picture);
	}
	av_free_packet(&decoder->packet);
#else
	if (decoder->dec != NULL) 
	{
		(*decoder->dec)->Uninitialize(decoder->dec);
		WelsDestroyDecoder(decoder->dec);
	}
#endif
	if (decoder->yuv)
	{
		free(decoder->yuv);
	}

	free(decoder);
}

int decode_frame(H264Decoder *decoder, RtpStream *stream)
{
#ifdef USE_FFMPEG
	int ret = 0;
	int size = 0;
	decoder->packet.data = (uint8_t*)stream->frame_buffer;
	decoder->packet.size = stream->frame_len;
	ret = avcodec_decode_video2(decoder->context, decoder->picture, &size, &decoder->packet);
	if (ret < 0)
	{
		LOGE("CXPlDecodeH264::Decode error :%d\n", ret);
	}

	decoder->rect.w = decoder->context->width;
	decoder->rect.h = decoder->context->height;

	decoder->yuv_len = copy_data(decoder);
#else
	memset(&decoder->bufInfo, 0, sizeof(decoder->bufInfo));
	memset(decoder->data, 0, sizeof(decoder->data));
	memset(&decoder->bufInfo, 0, sizeof(SBufferInfo));

	DECODING_STATE rv = (*decoder->dec)->DecodeFrame2(decoder->dec, (unsigned char*)stream->frame_buffer, (int)stream->frame_len, decoder->data, &decoder->bufInfo);

	if (decoder->bufInfo.iBufferStatus == 1)
	{
		decoder->rect.w = decoder->bufInfo.UsrData.sSystemBuffer.iWidth;
		decoder->rect.h = decoder->bufInfo.UsrData.sSystemBuffer.iHeight;
	
		decoder->yuv_len = copy_data(decoder);
	}
#endif

	return 0;
}

int copy_data(H264Decoder *decoder)
{
	int len = 0;
	uint8_t *yuv_data = NULL;
	uint8_t *out_yuv = (uint8_t *)decoder->yuv;
	int height = 0;
	int width = 0;
	int i;

#ifdef USE_FFMPEG
	yuv_data = decoder->picture->data[0];
	if (yuv_data == NULL)
		return len;

	width = decoder->rect.w;
	height = decoder->rect.h;
	
	for (i = 0; i < height; i++)
	{
		memcpy(out_yuv, yuv_data, width);
		len += width;
		yuv_data += decoder->picture->linesize[0];
		out_yuv += width;
	}

	yuv_data = decoder->picture->data[1];
	width = decoder->rect.w / 2;
	height = decoder->rect.h / 2;
	for (i = 0; i < height; i++)
	{
		memcpy(out_yuv, yuv_data, width);
		len += width;
		yuv_data += decoder->picture->linesize[1];
		out_yuv += width;
	}

	yuv_data = decoder->picture->data[2];
	width = decoder->rect.w / 2;
	height = decoder->rect.h / 2;
	for (i = 0; i < height; i++)
	{
		memcpy(out_yuv, yuv_data, width);
		len += width;
		yuv_data += decoder->picture->linesize[2];
		out_yuv += width;
	}
#else
	yuv_data = decoder->data[0];
	if (yuv_data == NULL)
		return len;

	width = decoder->rect.w;
	height = decoder->rect.h;
	
	for (i = 0; i < height; i++)
	{
		memcpy(out_yuv, yuv_data, width);
		len += width;
		yuv_data += decoder->bufInfo.UsrData.sSystemBuffer.iStride[0];
		out_yuv += width;
	}

	yuv_data = decoder->data[1];
	width = decoder->rect.w / 2;
	height = decoder->rect.h / 2;
	for (i = 0; i < height; i++)
	{
		memcpy(out_yuv, yuv_data, width);
		len += width;
		yuv_data += decoder->bufInfo.UsrData.sSystemBuffer.iStride[1];
		out_yuv += width;
	}

	yuv_data = decoder->data[2];
	width = decoder->rect.w / 2;
	height = decoder->rect.h / 2;
	for (i = 0; i < height; i++)
	{
		memcpy(out_yuv, yuv_data, width);
		len += width;
		yuv_data += decoder->bufInfo.UsrData.sSystemBuffer.iStride[2];
		out_yuv += width;
	}
#endif

	return len;
}