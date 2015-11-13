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

static int write_png(const char *filename, unsigned char *data, int width, int height) {
	png_FILE_p fp = NULL;
	png_structp write_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep *row_pointers = NULL;
	png_colorp palette = NULL;
	int i, y, number_passes, pass;

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
		png_destroy_write_struct(&write_ptr, NULL);
		return -1;
	}
	if (setjmp(png_jmpbuf(write_ptr)))
	{
		fclose(fp);
		png_destroy_write_struct(&write_ptr, &info_ptr);
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
		row_pointers[i] = (png_bytep)data + width*i*3;

	png_write_image(write_ptr, row_pointers);
	//number_passes = png_set_interlace_handling(write_ptr);

	//for (pass = 0; pass < number_passes; pass++)
	//{
	//	/* 一次性写几行 */
	//	//png_write_rows(write_ptr, &row_pointers[first_row], number_of_rows);

	//	/* 如果你一次性只写一行像素，可以用下面的代码 */
	//	for (y = 0; y < height; y++)
	//		png_write_rows(write_ptr, &row_pointers[y], 1);
	//}

	png_write_end(write_ptr, info_ptr);
	png_free(write_ptr, palette);
	free(row_pointers);
	fclose(fp);
}

static bool YUV420_To_BGR24(const char *file_name, unsigned char *puc_yuv, int width_y, int height_y)
{
	//初始化变量  
	int baseSize = width_y * height_y;
	int rgbSize = baseSize * 3;
	unsigned char *puc_y = puc_yuv;
	unsigned char *puc_u = puc_yuv + baseSize;
	unsigned char *puc_v = puc_u + baseSize / 4;

	unsigned char *rgbData = (unsigned char *)malloc(rgbSize);
	unsigned char *puc_rgb = (unsigned char *)malloc(rgbSize);
	/* 变量声明 */
	int temp = 0, x, y;

	unsigned char *rData = rgbData;                  //r分量地址  
	unsigned char *gData = rgbData + baseSize;       //g分量地址  
	unsigned char *bData = gData + baseSize;         //b分量地址  

	int uvIndex = 0, yIndex = 0;
	//将R,G,B三个分量赋给img_data  
	int widthStep = width_y * 3;

	memset(rgbData, 0, rgbSize);
	for (y = 0; y < height_y; y++)
	{
		for (x = 0; x < width_y; x++)
		{
			uvIndex = (y >> 1) * (width_y >> 1) + (x >> 1);
			yIndex = y * width_y + x;

			/* b分量 */
			temp = (int)(puc_y[yIndex] + (puc_v[uvIndex] - 128) * 1.4075);
			bData[yIndex] = temp<0 ? 0 : (temp > 255 ? 255 : temp);

			/* g分量 */
			temp = (int)(puc_y[yIndex] + (puc_u[uvIndex] - 128) * (-0.3455) +
				(puc_v[uvIndex] - 128) * (-0.7169));
			gData[yIndex] = temp < 0 ? 0 : (temp > 255 ? 255 : temp);

			/* r分量 */
			temp = (int)(puc_y[yIndex] + (puc_u[uvIndex] - 128) * 1.779);
			rData[yIndex] = temp < 0 ? 0 : (temp > 255 ? 255 : temp);
		}
	}

	for (y = 0; y < height_y; y++)
	{
		for (x = 0; x < width_y; x++)
		{
			puc_rgb[y * widthStep + x * 3 + 2] = rData[y * width_y + x];   //R  
			puc_rgb[y * widthStep + x * 3 + 1] = gData[y * width_y + x];   //G  
			puc_rgb[y * widthStep + x * 3 + 0] = bData[y * width_y + x];   //B  
		}
	}
	free(rgbData);
	write_png(file_name, puc_rgb, width_y, height_y);
	free(puc_rgb);

	return 1;
}

int save_png(const char *file_name, H264Decoder *decoderA, H264Decoder *decoderB) {
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
		YUV420_To_BGR24(file_name, tmp_yuv, decoderA->rect.w, decoderA->rect.h * 2);
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
		YUV420_To_BGR24(file_name, tmp_yuv, decoder->rect.w, decoder->rect.h);
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