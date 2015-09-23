#include "XL_log.h"
#include "XL_scheduler.h"

#pragma comment (lib, "lib\\SDL2.lib")

Scheduler *create_scheduler()
{
	Scheduler *schd = (Scheduler *)malloc(sizeof(Scheduler));
	memset(schd, 0, sizeof(Scheduler));

	return schd;
}

void initialize_scheduler(Scheduler *schd, void *native_windows)
{
	/* Init SDL */
	LOGI("initialize_scheduler start");
	if (SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		LOGI("initialize_scheduler error, %s", SDL_GetError());
		return;
	}
	LOGI("initialize_scheduler success");
	
	schd->surface = create_surface();
	initialize_surface(schd->surface, native_windows);

	schd->lock = SDL_CreateMutex();
}

void destroy_scheduler(Scheduler *schd)
{
	destroy_surface(schd->surface);
	SDL_DestroyMutex(schd->lock);
	free(schd);

	/* Quit SDL */
	SDL_Quit();
}

void sock_fd_set(Scheduler *schd)
{
	RtspSession *session = schd->session;

	FD_ZERO(&schd->readfds);
	while (session)
	{
		session->is_set = 0;
		FD_SET(session->packet->sock, &schd->readfds);
		session = session->next;
	}
}


int sock_select(Scheduler *schd)
{
	int is_set_num = 0;
	RtspSession *session = schd->session;

	schd->tv.tv_sec = 0;
	schd->tv.tv_usec = 1000;
	if ((is_set_num = select(100, &schd->readfds, NULL, NULL, &schd->tv)) > 0)
	{
		while (session)
		{
			if (FD_ISSET(session->packet->sock, &schd->readfds))
			{
				session->is_set = 1;
			}
			session = session->next;
		}
	}

	return is_set_num;
}

int schedule_func(void *data)
{
	Scheduler *schd = (Scheduler *)data;
	RtspSession *session;

	while (schd->running)
	{
		if (schd->session)
		{
			sock_fd_set(schd);
			if (sock_select(schd) > 0)
			{
				session = schd->session;
				while (session)
				{
					if (session->is_set)
					{
						session_process(schd->surface, session);
						session->is_set = 0;
					}
					session = session->next;
				}
			}
		}
	}
	return 0;
}

void scheduler_start(Scheduler *schd)
{
	schd->running = 1;
	schd->thread = SDL_CreateThread(schedule_func, "", schd);
}

void scheduler_wait(Scheduler *schd)
{
	SDL_Event e;
	while (1)
	{
		if (SDL_PollEvent(&e)) 
		{
			if (e.type == SDL_QUIT) 
			{
				break;
			}
			else if (e.type == SDL_KEYDOWN)
			{
				break;
			}
			//LOGI("%d\n", e.type);
		}
	}
}

void scheduler_stop(Scheduler *schd)
{
	int lots;
	schd->running = 0;
	SDL_WaitThread(schd->thread, &lots);
}

void add_session(Scheduler *schd, RtspSession *session)
{
	RtspSession *oldfirst;

	SDL_LockMutex(schd->lock);
	/* enqueue the session to the list of scheduled sessions */
	oldfirst = schd->session;
	schd->session = session;
	session->next = oldfirst;

	SDL_UnlockMutex(schd->lock);
}

void del_session(Scheduler *schd, RtspSession *session)
{
	RtspSession *tmp;
	int cond = 1;
	SDL_LockMutex(schd->lock);
	tmp = schd->session;

	if (tmp == session)
	{
		schd->session = tmp->next;
		SDL_UnlockMutex(schd->lock);
		return;
	}

	/* go the position of session in the list */
	while (cond)
	{
		if (tmp != NULL)
		{
			if (tmp->next == session)
			{
				tmp->next = tmp->next->next;
				cond = 0;
			}
			else tmp = tmp->next;
		}
		else 
		{
			/* the session was not found ! */
			LOGI("rtp_scheduler_remove_session: the session was not found in the scheduler list!");
			cond = 0;
		}
	}

	SDL_UnlockMutex(schd->lock);
}