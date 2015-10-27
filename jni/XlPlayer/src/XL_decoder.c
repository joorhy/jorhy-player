#include "XL_log.h"
#include "XL_decoder.h"

#ifdef WIN32

#ifdef USE_FFMPEG
#pragma comment(lib, "lib\\avcodec.lib")
#pragma comment(lib, "lib\\avutil.lib")
#else
#pragma comment(lib, "lib\\libopenh264.lib")
#endif

#endif

#define MAX_YUV_SIZE (1024 * 1024 * 2)

static bool YV12_to_RGB24(unsigned char* pYV12, unsigned char* pRGB24, int iWidth, int iHeight) {  
	const long nYLen = (long)(iHeight * iWidth);  
	const int nHfWidth = (iWidth >> 1); 

	int rgb[3];  
	int i, j, m, n, x, y;  
	unsigned char* yData, *uData, *vData;

	if(!pYV12 || !pRGB24)  
		return 0;   

	if(nYLen < 1 || nHfWidth < 1)   
		return 0;  

	yData = pYV12;  
	uData = &yData[nYLen];  
	vData = &uData[nYLen >> 2];  
	if(!uData || !vData)  
		return 0;  

	m = -iWidth;  
	n = -nHfWidth;  
	for(y = 0; y < iHeight; y++)  {  
		m += iWidth;  

		if(!(y % 2))  
			n += nHfWidth;  

		for(x=0; x < iWidth; x++) {  
			i = m + x;  
			j = n + (x>>1);  
			rgb[2] = (int)(yData[i] + 1.370705 * (vData[j] - 128)); // r分量值  
			rgb[1] = (int)(yData[i] - 0.698001 * (uData[j] - 128)  - 0.703125 * (vData[j] - 128)); // g分量值  
			rgb[0] = (int)(yData[i] + 1.732446 * (uData[j] - 128)); // b分量值  
			j = nYLen - iWidth - m + x;  
			i = (j<<1) + j;  
			for(j=0; j<3; j++) {  
				if(rgb[j]>=0 && rgb[j]<=255)  
					pRGB24[i + j] = rgb[j];  
				else  
					pRGB24[i + j] = (rgb[j] < 0) ? 0 : 255;  
			}  
		}  
	}  

	return 1;  
}  

int copy_data(H264Decoder *decoder) {
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

	for (i = 0; i < height; i++) {
		memcpy(out_yuv, yuv_data, width);
		len += width;
		yuv_data += decoder->picture->linesize[0];
		out_yuv += width;
	}

	yuv_data = decoder->picture->data[1];
	width = decoder->rect.w / 2;
	height = decoder->rect.h / 2;
	for (i = 0; i < height; i++) {
		memcpy(out_yuv, yuv_data, width);
		len += width;
		yuv_data += decoder->picture->linesize[1];
		out_yuv += width;
	}

	yuv_data = decoder->picture->data[2];
	width = decoder->rect.w / 2;
	height = decoder->rect.h / 2;
	for (i = 0; i < height; i++) {
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

	for (i = 0; i < height; i++) {
		memcpy(out_yuv, yuv_data, width);
		len += width;
		yuv_data += decoder->bufInfo.UsrData.sSystemBuffer.iStride[0];
		out_yuv += width;
	}

	yuv_data = decoder->data[1];
	width = decoder->rect.w / 2;
	height = decoder->rect.h / 2;
	for (i = 0; i < height; i++) {
		memcpy(out_yuv, yuv_data, width);
		len += width;
		yuv_data += decoder->bufInfo.UsrData.sSystemBuffer.iStride[1];
		out_yuv += width;
	}

	yuv_data = decoder->data[2];
	width = decoder->rect.w / 2;
	height = decoder->rect.h / 2;
	for (i = 0; i < height; i++) {
		memcpy(out_yuv, yuv_data, width);
		len += width;
		yuv_data += decoder->bufInfo.UsrData.sSystemBuffer.iStride[2];
		out_yuv += width;
	}
#endif
	YV12_to_RGB24((unsigned char *)decoder->yuv, (unsigned char *)decoder->rgb, decoder->rect.w, decoder->rect.h);

	return len;
}

H264Decoder *create_decoder() {
	H264Decoder *decoder = (H264Decoder *)malloc(sizeof(H264Decoder));
	memset(decoder, 0, sizeof(H264Decoder));

#ifdef USE_FFMPEG

#if LIBAVCODEC_VERSION_MAJOR < 54
	avcodec_init();
#endif
	avcodec_register_all();

	decoder->codec = avcodec_find_decoder(CODEC_ID_H264);
	decoder->context = avcodec_alloc_context3(decoder->codec);
	decoder->picture = av_frame_alloc();
	av_init_packet(&decoder->packet);
	if (!decoder->codec || !decoder->context || !decoder->picture) {
		return 0;
	}

	if (decoder->codec->capabilities & CODEC_CAP_TRUNCATED) {
		decoder->context->flags |= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */
	}

	if (avcodec_open2(decoder->context, decoder->codec, NULL) < 0) {
		return 0;
	}
#else
	SDecodingParam decParam;
	long rv = WelsCreateDecoder(&decoder->dec);
	if (rv != 0) {
		return 0;
	}

	memset(&decParam, 0, sizeof(SDecodingParam));
	decParam.eOutputColorFormat = videoFormatI420;
	decParam.uiTargetDqLayer = UCHAR_MAX;
	decParam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
	decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;

	rv = (*decoder->dec)->Initialize(decoder->dec, &decParam);
	if (rv != 0) {
		return 0;
	}
#endif

	decoder->yuv = (char *)malloc(MAX_YUV_SIZE);
	decoder->rgb= (char *)malloc(MAX_YUV_SIZE);

	return decoder;
}

void destroy_decoder(H264Decoder *decoder) {
#ifdef USE_FFMPEG
	if (decoder->context) {
		avcodec_close(decoder->context);
		av_free(decoder->context);
	}
	if (decoder->picture) {
		av_free(decoder->picture);
	}
	av_free_packet(&decoder->packet);
#else
	if (decoder->dec != NULL) {
		(*decoder->dec)->Uninitialize(decoder->dec);
		WelsDestroyDecoder(decoder->dec);
	}
#endif
	if (decoder->yuv) {
		free(decoder->yuv);
	}

	if (decoder->rgb) {
		free(decoder->rgb);
	}

	free(decoder);
}

int decode_frame(H264Decoder *decoder, RtpStream *stream) {
#ifdef USE_FFMPEG
	int ret = 0;
	int size = 0;
	decoder->packet.data = (uint8_t*)stream->frame_buffer;
	decoder->packet.size = stream->frame_len;
	ret = avcodec_decode_video2(decoder->context, decoder->picture, &size, &decoder->packet);
	if (ret < 0) {
		LOGE("CXPlDecodeH264::Decode error :%d\n", ret);
	}

	decoder->rect.w = decoder->context->width;
	decoder->rect.h = decoder->context->height;

	//decoder->yuv_len = copy_data(decoder);
#else
	DECODING_STATE rv;
	memset(&decoder->bufInfo, 0, sizeof(decoder->bufInfo));
	memset(decoder->data, 0, sizeof(decoder->data));
	memset(&decoder->bufInfo, 0, sizeof(SBufferInfo));

	rv = (*decoder->dec)->DecodeFrame2(decoder->dec, (unsigned char*)stream->frame_buffer, (int)stream->frame_len, decoder->data, &decoder->bufInfo);

	if (decoder->bufInfo.iBufferStatus == 1) {
		LOGI("CXPlDecodeH264::Decode success");
		decoder->rect.w = decoder->bufInfo.UsrData.sSystemBuffer.iWidth;
		decoder->rect.h = decoder->bufInfo.UsrData.sSystemBuffer.iHeight;

		//decoder->yuv_len = copy_data(decoder);
	}
#endif

	return 0;
}