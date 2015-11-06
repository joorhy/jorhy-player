#ifndef _XL_scheduler_h
#define _XL_scheduler_h

#include "XL_rtsp_session.h"
#include "XL_surface.h"
#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_mutex.h"

typedef struct Scheduler {
	RtspSession *list;
	SDL_mutex *lock;
	JoSurface *surface;
	fd_set readfds;
	struct timeval tv;
} Scheduler;

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

extern Scheduler *create_scheduler(void *native_windows);
extern void scheduler_process(Scheduler *schd);
extern void destroy_scheduler(Scheduler *schd);

extern void add_session(Scheduler *schd, RtspSession *session);
extern void del_session(Scheduler *schd, RtspSession *session);

#ifdef __cplusplus
}
#endif

#endif /* _XL_scheduler_h */