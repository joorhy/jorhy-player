#include "XL_file_session.h"

FileSession *create_file_session(int index)
{
	FileSession *file_session = (FileSession *)malloc(sizeof(FileSession));
	file_session->head = 0;
	file_session->cursor = 0;
	file_session->tail = 0;
	file_session->is_initialize = 0;
	file_session->ndex = index;
	decoder = create_decoder();
	
	return file_session;
}

void destroy_file_session(FileSession *file_session)
{
	YUVFrame *cur = file_session->head;
	while (cur) {
		YUVFrame *del = cur;
		cur = cur->next;
		if (del->yuv) {
			free(del->yuv);
		}
		free(del);
	}
	destroy_decoder(file_session->decoder);
}

void file_start(FileSession *file_session, const char *file_name)
{
	if (!is_initialize) {
		FILE *fp = fopen(file_name, "rb+");
		if (fp) {
			fread(&file_session->header, sizeof(char), sizeof(FileHeader), fp);
		}
		int frame_num = prev_time * frame_rate;
		file_session->interval = file_session->prev_time * 1000 / frame_rate;
		file_session->decoder->bufInfo.UsrData.sSystemBuffer.iStride[0] = file_session->header.width;
		file_session->decoder->bufInfo.UsrData.sSystemBuffer.iStride[1] = file_session->header.width / 2;

		YUVFrame *frame = 0;
		for (int i = 0; i < frame_num; i++) {
			frame = (YUVFrame *)malloc(sizeof(YUVFrame));
			frame->yuv = (char *)malloc(width * height * 3 / 2);
			if (fp) {
				fread(frame->yuv, sizeof(char), width * height * 3 / 2, fp);
			}
			frame->next = 0;
			if (!file_session->head) {
				file_session->head = frame;
			}
			else {
				YUVFrame *cur = file_session->head;
				while (cur->next) {
					cur = cur->next;
				}
				cur->next = frame;
			}
		}
		file_session->tail = frame;
		fcolse(fp);
		file_session->is_initialize = 1;
	}
	start_time = GetTickCount();
	file_session->cursor = file_session->head;
}

void file_process(JoSurface *surface, FileSession *file_session)
{
	if (file_session->cursor) {
		int cur_time = GetTickCount();
		if (cur_time - start_time >= file_session->interval) {
			file_session->decoder->data[0] = file_session->yuv;
			file_session->decoder->data[1] = file_session->yuv + file_session->header.width;
			file_session->decoder->data[2] = file_session->yuv + file_session->header.width / 2;
			render_frame(surface, file_session->decoder, file_session->index);
			start_time = cur_time;
		}
	}
}

void file_stop(FileSession *file_session)
{
	file_session->cursor = 0;
}