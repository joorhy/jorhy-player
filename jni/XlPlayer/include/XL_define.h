#ifndef _XL_define_h
#define _XL_define_h

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _YUVFrame YUVFrame;
typedef struct _YUVFrame {
	char *yuv;
	int idle;
	YUVFrame *next;
};

typedef struct FileHeader {
	int prev_time;
	int frame_rate;
	int width;
	int height;
} FileHeader;

#ifdef __cplusplus
}
#endif

#endif /* _XL_define_h */