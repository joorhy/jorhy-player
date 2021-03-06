#include "XL_log.h"
#include "XL_scheduler.h"

#ifdef WIN32
#pragma comment (lib, "lib\\SDL2.lib")
#endif

void sock_fd_set(Scheduler *schd) {
	RtspSession *session = schd->list;

	FD_ZERO(&schd->readfds);
	while (session) {
		session->is_set = 0;
		FD_SET(session->packet->sock, &schd->readfds);
		session = session->next;
	}
}

int sock_select(Scheduler *schd) {
	int is_set_num = 0;
	RtspSession *session = schd->list;

	schd->tv.tv_sec = 0;
	schd->tv.tv_usec = 1000;
	if ((is_set_num = select(100, &schd->readfds, NULL, NULL, &schd->tv)) > 0) {
		while (session) {
			if (FD_ISSET(session->packet->sock, &schd->readfds)) {
				session->is_set = 1;
			}
			session = session->next;
		}
	}

	return is_set_num;
}

Scheduler *create_scheduler(void *native_windows) {
	Scheduler *schd = (Scheduler *)malloc(sizeof(Scheduler));
	memset(schd, 0, sizeof(Scheduler));

	/* Init SDL */
	LOGI("initialize_scheduler start");
	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		LOGI("initialize_scheduler error, %s", SDL_GetError());
		return NULL;
	}
	LOGI("initialize_scheduler success");
	
	schd->surface = create_surface(native_windows);
	schd->lock = SDL_CreateMutex();

	return schd;
}

void destroy_scheduler(Scheduler *schd) {
	if (schd != NULL) {
		destroy_surface(schd->surface);
		SDL_DestroyMutex(schd->lock);
		free(schd);	
	}

	/* Quit SDL */
	SDL_Quit();
}

static process_rtsp_session(Scheduler *schd) {
	RtspSession *session = NULL;
	if (schd == NULL) {
		return;
	}

	if (schd->list) {
		sock_fd_set(schd);
		if (sock_select(schd) > 0) {
			session = schd->list;
			while (session) {
				if (session->is_set) {
					session_process(schd->surface, session);
					session->is_set = 0;
				}
				session = session->next;
			}
		}
	} else {
#ifdef WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}
}

void process_file_session(Scheduler *schd) {
	FileSession *session = NULL;
	if (schd == NULL) {
		return;
	}

	session = schd->list_2;
	if (schd->list_2) {
		session = schd->list_2;
		while (session) {
			file_process(schd->surface, session);
			session = session->next;
		}
	} else {
#ifdef WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}
}

void scheduler_process(Scheduler *schd) {
	if (schd->list) {
		process_rtsp_session(schd);
	} else if (schd->list_2) {
		process_file_session(schd);
	} else {
#ifdef WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}
}

void add_session(Scheduler *schd, RtspSession *session) {
	RtspSession *oldfirst;

	SDL_LockMutex(schd->lock);
	/* enqueue the session to the list of scheduled sessions */
	oldfirst = schd->list;
	schd->list = session;
	session->next = oldfirst;

	SDL_UnlockMutex(schd->lock);
}

void del_session(Scheduler *schd, RtspSession *session) {
	RtspSession *tmp;
	int cond = 1;
	SDL_LockMutex(schd->lock);
	tmp = schd->list;

	if (tmp != NULL && tmp == session) {
		schd->list = tmp->next;
		SDL_UnlockMutex(schd->lock);
		return;
	}

	/* go the position of session in the list */
	while (cond) {
		if (tmp != NULL) {
			if (tmp->next == session) {
				tmp->next = tmp->next->next;
				cond = 0;
			} else tmp = tmp->next;
		} else {
			/* the session was not found ! */
			LOGI("rtp_scheduler_remove_session: the session was not found in the scheduler list!");
			cond = 0;
		}
	}
	schd->list = 0;
	SDL_UnlockMutex(schd->lock);
}

void add_session_2(Scheduler *schd, FileSession *session) {
	FileSession *oldfirst;

	SDL_LockMutex(schd->lock);
	/* enqueue the session to the list of scheduled sessions */
	oldfirst = schd->list_2;
	schd->list_2 = session;
	session->next = oldfirst;

	SDL_UnlockMutex(schd->lock);
}

void del_session_2(Scheduler *schd, FileSession *session) {
	FileSession *tmp;
	int cond = 1;
	SDL_LockMutex(schd->lock);
	tmp = schd->list_2;

	if (tmp != NULL && tmp == session) {
		schd->list_2 = tmp->next;
		SDL_UnlockMutex(schd->lock);
		return;
	}

	/* go the position of session in the list */
	while (cond) {
		if (tmp != NULL) {
			if (tmp->next == session) {
				tmp->next = tmp->next->next;
				cond = 0;
			} else tmp = tmp->next;
		} else {
			/* the session was not found ! */
			LOGI("rtp_scheduler_remove_session: the session was not found in the scheduler list!");
			cond = 0;
		}
	}
	schd->list_2 = 0;
	SDL_UnlockMutex(schd->lock);
}