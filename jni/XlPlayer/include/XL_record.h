#ifndef _XL_record_h
#define _XL_record_h

#include "XL_define.h"
#include "XL_decoder.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct Record {
	FileHeader header;
	int is_full;
	int is_initialize;
	YUVFrame *head;
	YUVFrame *tail;
} Record;


extern Record *create_record();
extern void destroy_record(Record *record);
extern void cache_frame(Record *record, H264Decoder *decoder);
extern void save_file(Record *record, const char *file_name);

#ifdef __cplusplus
}
#endif

#endif /* _XL_record_h */