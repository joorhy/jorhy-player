#include "XL_decoder.h"

#pragma comment(lib, "lib\\avcodec.lib")
#pragma comment(lib, "lib\\avutil.lib")

#define MAX_YUV_SIZE (1024 * 1024 * 2)

H264Decoder *create_decoder()
{
	H264Decoder *decoder = (H264Decoder *)malloc(sizeof(H264Decoder));
	memset(decoder, 0, sizeof(H264Decoder));
	return decoder;
}

int initialize_decoder(H264Decoder *decoder)
{
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

	decoder->yuv = (char *)malloc(MAX_YUV_SIZE);

	return 0;
}

void destroy_decoder(H264Decoder *decoder)
{
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

	if (decoder->yuv)
	{
		free(decoder->yuv);
	}

	free(decoder);
}

int decode_frame(H264Decoder *decoder, RtpStream *stream)
{
	int ret = 0;
	int size = 0;
	decoder->packet.data = (uint8_t*)stream->frame_buffer;
	decoder->packet.size = stream->frame_len;
	ret = avcodec_decode_video2(decoder->context, decoder->picture, &size, &decoder->packet);
	if (ret < 0)
	{
		printf("CXPlDecodeH264::Decode error :%d\n", ret);
	}

	decoder->rect.w = decoder->context->width;
	decoder->rect.h = decoder->context->height;

	//decoder->yuv_len = copy_data(decoder);

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

	return len;
}