#ifndef _XL_record_h
#define _XL_record_h

#include "XL_decoder.h"

typedef struct _YUVFrame YUVFrame;
typedef struct _YUVFrame {
	char *yuv;
	int idle;
	YUVFrame *next;
};

typedef struct Record {
	int frame_rate;
	int width;
	int height;
	int is_full;
	int is_initialize;
	YUVFrame *head;
	YUVFrame *tail;
} Record;


extern Record *create_record();
extern void destroy_record(Record *record);
extern void cache_frame(Record *record, H264Decoder *decoder);
extern void save_file(Record *record, const char *file_name);

#endif