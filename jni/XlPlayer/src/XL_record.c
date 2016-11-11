#include "XL_record.h"
#include "stdio.h"

Record *create_record() {
	Record *record = (Record *)malloc(sizeof(Record));
	record->is_initialize = 0;
	record->is_full = 0;
	record->head = 0;
	record->tail = 0;
	return record;
}

static void initialize(Record *record, int prev_time, int frame_rate, int width, int height) {
	int frame_num = prev_time * frame_rate;
	record->prev_time;
	record->frame_rate = frame_rate;
	record->width = width;
	record->height = height;

	YUVFrame *frame = 0;
	for (int i = 0; i < frame_num; i++) {
		frame = (YUVFrame *)malloc(sizeof(YUVFrame));
		frame->yuv = (char *)malloc(width * height * 3 / 2);
		frame->next = 0;
		frame->idle = 1;
		if (!record->head) {
			record->head = frame;
		}
		else {
			YUVFrame *cur = record->head;
			while (cur->next) {
				cur = cur->next;
			}
			cur->next = frame;
		}
	}
	record->tail = frame;
}

void destroy_record(Record *record) {
	YUVFrame *cur = record->head;
	while (cur) {
		YUVFrame *del = cur;
		cur = cur->next;
		if (del->yuv) {
			free(del->yuv);
		}
		free(del);
	}
}

static YUVFrame *get_free(Record *record) {
	YUVFrame *cur = record->head;
	if (!record->is_full) {
		while (cur != record->tail) {
			if (cur->idle) {
				return cur;
			}
			cur = cur->next;
		}
		if (record->tail->idle) {
			return record->tail;
		}
		record->is_full = 1;
	}
	if (record->is_full) {
		cur = record->head;
		record->head = cur->next;
		cur->idle = 1;
		cur->next = 0;
		record->is_full = 1;
	}
	return cur;
}

void cache_frame(Record *record, H264Decoder *decoder) {
	int i, offset = 0;
	YUVFrame *frame = 0;
	if (decoder->data[0] != NULL) {
		if (!record->is_initialize) {
			initialize(record, 5, 4, decoder->bufInfo.UsrData.sSystemBuffer.iWidth, decoder->bufInfo.UsrData.sSystemBuffer.iHeight);
			record->is_initialize = 1;
		}
		frame = get_free(record);
		for (i = 0; i < record->height; i++) {
			memcpy(frame->yuv + offset, decoder->data[0] + i * decoder->bufInfo.UsrData.sSystemBuffer.iStride[0], record->width);
			offset += record->width;
		}
		for (i = 0; i < record->height / 2; i++) {
			memcpy(frame->yuv + offset, decoder->data[1] + i * decoder->bufInfo.UsrData.sSystemBuffer.iStride[1], record->width / 2);
			offset += record->width / 2;
		}
		for (i = 0; i < record->height / 2; i++) {
			memcpy(frame->yuv + offset, decoder->data[2] + i * decoder->bufInfo.UsrData.sSystemBuffer.iStride[1], record->width / 2);
			offset += record->width / 2;
		}
		if (record->is_full) {
			record->tail->next = frame;
			record->tail = frame;
		}
		frame->idle = 0;
	}
}

void save_file(Record *record, const char *file_name) {
	FILE *fp = fopen(file_name, "wb+");
	//fwrite(record, sizeof(char), sizeof(int) * 3, fp);
	YUVFrame *cur = record->head;
	int yuv_len = record->width * record->height * 3 / 2;
	while (cur) {
		fwrite(cur->yuv, sizeof(char), yuv_len, fp);
		cur = cur->next;
	}
	fclose(fp);
}