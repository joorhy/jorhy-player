#include "XL_log.h"
#include "XL_rtsp_session.h"
#include "XL_scheduler.h"

//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

static Scheduler *schd;
static RtspSession *session_1;
static RtspSession *session_2;
static bool b_paused = 1;
static bool b_run = 1;

void start()
{
	session_1 = create_session();
	initialize_session(session_1, "222.214.218.237", 6601, "14120", 0);
	set_session_index(session_1, 0);
	add_session(schd, session_1);

	session_2 = create_session();
	initialize_session(session_2, "222.214.218.237", 6601, "14120", 1);
	set_session_index(session_2, 1);
	add_session(schd, session_2);

	set_surface_mode(schd->surface, mode_2);

	scheduler_start(schd);
	session_start(session_1);
	session_start(session_2);
}

void stop()
{
	session_stop(session_1);
	session_stop(session_2);

	del_session(schd, session_1);
	del_session(schd, session_2);
	
	destroy_session(session_1);
	destroy_session(session_2);
}

#undef main
int main(int argc, char* argv[])
{
	SDL_Event e;
	schd = create_scheduler();
	initialize_scheduler(schd, NULL);

	while (b_run)
	{
		if (SDL_PollEvent(&e)) 
		{
			if (e.type == SDL_QUIT  || e.type == SDL_FINGERDOWN) 
			{
				b_run = 0;
				break;
			}
			else if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.sym == SDLK_SPACE)
				{
					if (b_paused)
					{
						b_paused = 0;
						start();
						LOGI("Resume \n");
					}
					else
					{

						b_paused = 1;
						stop();
						LOGI("Pause \n");
					}
				}
				
			}
			//LOGI("%d\n", e.type);
		}

		scheduler_wait(schd);
	}

	return 0;
}