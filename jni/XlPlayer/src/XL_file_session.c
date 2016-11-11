#include "XL_file_session.h"

FileSession *create_file_session(int index, file_end_cb cb)
{
	FileSession *file_session = (FileSession *)malloc(sizeof(FileSession));
	memset(file_session, 0, sizeof(FileSession));
	file_session->index = index;
	file_session->decoder = create_decoder();
	file_session->cb = cb;
	
	return file_session;
}

void destroy_file_session(FileSession *file_session)
{
	YUVFrame *cur = file_session->head;
	while (cur) {
		YUVFrame *del = cur;
		cur = cur->next;
		if (del->y) {
			free(del->y);
		}
		if (del->u) {
			free(del->u);
		}
		if (del->v) {
			free(del->v);
		}
		free(del);
	}
	destroy_decoder(file_session->decoder);
}

void file_start(FileSession *file_session, const char *file_name)
{
	if (!file_session->is_initialize) {
		FILE *fp = fopen(file_name, "rb+");
		if (fp) {
			fread(&file_session->header, sizeof(char), sizeof(FileHeader), fp);
			file_session->decoder->rect.w = file_session->header.width;
			file_session->decoder->rect.h = file_session->header.height;
		}
		int frame_num = file_session->header.prev_time * file_session->header.frame_rate;
		file_session->interval = 1000 / file_session->header.frame_rate;
		file_session->decoder->bufInfo.UsrData.sSystemBuffer.iStride[0] = file_session->header.width;
		file_session->decoder->bufInfo.UsrData.sSystemBuffer.iStride[1] = file_session->header.width / 2;

		YUVFrame *frame = 0;
		int length = file_session->header.width * file_session->header.height;
		for (int i = 0; i < frame_num; i++) {
			frame = (YUVFrame *)malloc(sizeof(YUVFrame));
			if (fp) {
				frame->y = (char *)malloc(length);
				fread(frame->y, sizeof(char), length, fp);
				frame->u = (char *)malloc(length / 4);
				fread(frame->u, sizeof(char), length / 4, fp);
				frame->v = (char *)malloc(length / 4);
				fread(frame->v, sizeof(char), length / 4, fp);
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
		fclose(fp);
		file_session->is_initialize = 1;
	}
	file_session->start_time = GetTickCount();
	file_session->cursor = file_session->head;
}

void file_process(JoSurface *surface, FileSession *file_session)
{
	if (file_session->cursor) {
		int cur_time = GetTickCount();
		if (cur_time - file_session->start_time >= file_session->interval) {
			file_session->decoder->data[0] = file_session->cursor->y;
			file_session->decoder->data[1] = file_session->cursor->u;
			file_session->decoder->data[2] = file_session->cursor->v;
			render_frame(surface, file_session->decoder, file_session->index);
			file_session->start_time = cur_time;
			file_session->cursor = file_session->cursor->next;
		}
		if (file_session->cursor == file_session->tail) {
			file_session->cb(file_session->index);
			file_session->cursor = 0;
		}
	}
}

void file_stop(FileSession *file_session)
{
	file_session->cursor = 0;
}