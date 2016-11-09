#ifndef _XL_record_h
#define _XL_record_h

typedef struct Record{
	int frame_type;
	int frame_len;
	int time_stamp;
} Record;

typedef struct FrameHeader{
	int session_type;
	int frame_type;
	int frame_len;
	int time_stamp;
} FrameHeader;



#endif