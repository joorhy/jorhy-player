#ifndef _XL_file_session_h
#define _XL_file_session_h

#include "XL_define.h"
#include "XL_decoder.h"
#include "XL_surface.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

typedef void(*file_end_cb)(int index);
typedef struct _FileSession FileSession;
typedef struct _FileSession {
	FileHeader header;
	int interval;
	int is_initialize;
	int start_time;
	int index;
	FileSession *next;
	YUVFrame *head;
	YUVFrame *cursor;
	YUVFrame *tail;
	H264Decoder *decoder;
	file_end_cb cb;
};

extern FileSession *create_file_session(int index, file_end_cb cb);
extern void destroy_file_session(FileSession *file_session);
extern void file_start(FileSession *file_session, const char *file_name);
extern void file_process(JoSurface *surface, FileSession *file_session);
extern void file_stop(FileSession *file_session);

#ifdef __cplusplus
}
#endif

#endif /* _XL_file_session_h */