#include "XL_log.h"
#include "XL_decoder.h"
#include "png.h"

#ifdef WIN32

#ifdef USE_FFMPEG
#pragma comment(lib, "lib\\avcodec.lib")
#pragma comment(lib, "lib\\avutil.lib")
#else
#pragma comment(lib, "lib\\libopenh264.lib")
#endif

#endif

#define MAX_YUV_SIZE (1024 * 1024 * 2)

static int write_png(const char *filename, const char *data, int width, int height) {
	png_FILE_p fp = NULL;
	png_structp write_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep *row_pointers = NULL;
	png_colorp palette = NULL;
	int i;

	fp = fopen(filename, "wb");
	if (!fp)
	{
		LOGE("Could not open File %s\n", filename);
		return -1;
	}

	write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!write_ptr)
	{
		fclose(fp);
		return -1;
	}

	info_ptr = png_create_info_struct(write_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		return -1;
	}
	if (setjmp(png_jmpbuf(write_ptr)))
	{
		fclose(fp);
		return -1;
	}
	png_init_io(write_ptr, fp);

	png_set_IHDR(write_ptr, info_ptr, width, height, 8,			//8bit
		PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	palette = (png_colorp)png_malloc(write_ptr, PNG_MAX_PALETTE_LENGTH * png_sizeof(png_color));
	png_set_PLTE(write_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);

	png_write_info(write_ptr, info_ptr);

	png_set_packing(write_ptr);
	row_pointers = (png_bytep *)malloc(height * sizeof(png_bytep));
	for (i = 0; i<height; i++)
		row_pointers[i] = (png_bytep)data + width*i;
	png_write_image(write_ptr, row_pointers);
	png_free(write_ptr, palette);
	free(row_pointers);
	fclose(fp);
}

bool YV12ToBGR24_Native(const char *file_name, unsigned char* pYUV, int width,int height)
{
	const long len = width * height;
    unsigned char* yData = pYUV;
    unsigned char* vData = &yData[len];
    unsigned char* uData = &vData[len >> 2];

    int bgr[3];
    int yIdx,uIdx,vIdx,idx;
	int i, j, k;
	unsigned char* pBGR24 = (unsigned char*)malloc(1024 * 1024);

    if (width < 1 || height < 1 || pYUV == NULL || pBGR24 == NULL)
        return FALSE;

    for (i = 0;i < height;i++){
        for (j = 0;j < width;j++){
            yIdx = i * width + j;
            vIdx = (i/2) * (width/2) + (j/2);
            uIdx = vIdx;

            bgr[0] = (int)(yData[yIdx] + 1.732446 * (uData[vIdx] - 128));                                    // b分量
            bgr[1] = (int)(yData[yIdx] - 0.698001 * (uData[uIdx] - 128) - 0.703125 * (vData[vIdx] - 128));    // g分量
            bgr[2] = (int)(yData[yIdx] + 1.370705 * (vData[uIdx] - 128));                                    // r分量

            for (k = 0;k < 3;k++){
                idx = (i * width + j) * 3 + k;
                if(bgr[k] >= 0 && bgr[k] <= 255)
                    pBGR24[idx] = bgr[k];
                else
                    pBGR24[idx] = (bgr[k] < 0)?0:255;
            }
        }
    }
	write_png(file_name, (const char *)pBGR24, width, height);
	free(pBGR24);

    return TRUE;
}

static bool YV12_to_RGB24(const char *file_name, unsigned char* pYV12, int iWidth, int iHeight) {  
	const long nYLen = (long)(iHeight * iWidth);  
	const int nHfWidth = (iWidth >> 1); 
	unsigned char* pRGB24 = (unsigned char*)malloc(1024 * 1024);

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
			rgb[2] = (int)((yData[i] & 0xFF) + 1.4075 * ((vData[j] & 0xFF) - 128)); // b分量值  
			rgb[1] = (int)((yData[i] & 0xFF) - 0.3455  * ((uData[j] & 0xFF) - 128)  - 0.7169 * ((vData[j] & 0xFF) - 128)); // g分量值  
			rgb[0] = (int)((yData[i] & 0xFF) + 1.779 * ((uData[j] & 0xFF)- 128)); // r分量值  
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
	write_png(file_name, (const char *)pRGB24, iWidth, iHeight);
	free(pRGB24);

	return 1;  
}  

int save_png(const char *file_name, H264Decoder *decoderA, H264Decoder *decoderB) {
	//FILE *fp = NULL;
	H264Decoder *decoder;
	int len = 0;
	uint8_t *yuv_data = NULL;
	uint8_t *tmp_yuv = (uint8_t *)malloc(1024 * 1024);
	uint8_t *out_yuv = tmp_yuv;
	int height = 0;
	int width = 0;
	int i;
	if (decoderA == NULL && decoderB == NULL) {
		return 0;
	} else if (decoderA != NULL && decoderB != NULL) {
		width = decoderA->rect.w;
		height = decoderA->rect.h;

		yuv_data = decoderA->data[0];
		if (yuv_data == NULL)
			return len;

		for (i = 0; i < height; i++) {
			memcpy(out_yuv, yuv_data, width);
			len += width;
			yuv_data += decoderA->bufInfo.UsrData.sSystemBuffer.iStride[0];
			out_yuv += width;
		}
		yuv_data = decoderB->data[0];
		if (yuv_data == NULL)
			return len;

		for (i = 0; i < height; i++) {
			memcpy(out_yuv, yuv_data, width);
			len += width;
			yuv_data += decoderB->bufInfo.UsrData.sSystemBuffer.iStride[0];
			out_yuv += width;
		}

		width = decoderA->rect.w / 2;
		height = decoderA->rect.h / 2;

		yuv_data = decoderA->data[1];
		for (i = 0; i < height; i++) {
			memcpy(out_yuv, yuv_data, width);
			len += width;
			yuv_data += decoderA->bufInfo.UsrData.sSystemBuffer.iStride[1];
			out_yuv += width;
		}
		yuv_data = decoderB->data[1];
		for (i = 0; i < height; i++) {
			memcpy(out_yuv, yuv_data, width);
			len += width;
			yuv_data += decoderB->bufInfo.UsrData.sSystemBuffer.iStride[1];
			out_yuv += width;
		}

		yuv_data = decoderA->data[2];
		for (i = 0; i < height; i++) {
			memcpy(out_yuv, yuv_data, width);
			len += width;
			yuv_data += decoderA->bufInfo.UsrData.sSystemBuffer.iStride[2];
			out_yuv += width;
		}
		yuv_data = decoderB->data[2];
		for (i = 0; i < height; i++) {
			memcpy(out_yuv, yuv_data, width);
			len += width;
			yuv_data += decoderB->bufInfo.UsrData.sSystemBuffer.iStride[2];
			out_yuv += width;
		}

		//fp = fopen("test.yuv", "wb+");
		//fwrite(tmp_yuv, 1, len, fp);
		//fclose(fp);
		//YV12_to_RGB24(file_name, tmp_yuv, decoderA->rect.w, decoderA->rect.h * 2);
		//YV12ToBGR24_Native(file_name, tmp_yuv, decoderA->rect.w, decoderA->rect.h * 2);
	} else {
		decoder = decoderA != NULL ? decoderA : decoderB;
		width = decoder->rect.w;
		height = decoder->rect.h;

		yuv_data = decoder->data[0];
		if (yuv_data == NULL)
			return len;

		for (i = 0; i < height; i++) {
			memcpy(out_yuv, yuv_data, width);
			len += width;
			yuv_data += decoder->bufInfo.UsrData.sSystemBuffer.iStride[0];
			out_yuv += width;
		}

		width = decoder->rect.w / 2;
		height = decoder->rect.h / 2;

		yuv_data = decoder->data[1];
		for (i = 0; i < height; i++) {
			memcpy(out_yuv, yuv_data, width);
			len += width;
			yuv_data += decoder->bufInfo.UsrData.sSystemBuffer.iStride[1];
			out_yuv += width;
		}

		yuv_data = decoder->data[2];
		for (i = 0; i < height; i++) {
			memcpy(out_yuv, yuv_data, width);
			len += width;
			yuv_data += decoder->bufInfo.UsrData.sSystemBuffer.iStride[2];
			out_yuv += width;
		}
		//YV12_to_RGB24(file_name, tmp_yuv, decoder->rect.w, decoder->rect.h);
		YV12ToBGR24_Native(file_name, tmp_yuv, decoder->rect.w, decoder->rect.h);
	}
	free(tmp_yuv);

	return len;
}

H264Decoder *create_decoder() {
	SDecodingParam decParam;
	long rv;
	H264Decoder *decoder;

	decoder	= (H264Decoder *)malloc(sizeof(H264Decoder));
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

	rv = WelsCreateDecoder(&decoder->dec);
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
	}
#endif

	return 0;
}