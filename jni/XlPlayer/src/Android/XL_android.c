#include "XL_log.h"
#include "XL_android.h"
#include "XL_rtsp_session.h"
#include "XL_scheduler.h"

/*******************************************************************************
 This file links the Java side of Android with libsdl
*******************************************************************************/
#include <jni.h>

/*******************************************************************************
                               Globals
*******************************************************************************/
static Scheduler *schd;
static RtspSession *session_1;
static RtspSession *session_2;

int main(int argc, char *argv[])
{
	schd = create_scheduler();
	LOGI("main create_scheduler success");
	
	initialize_scheduler(schd, NULL);
	LOGI("main initialize_scheduler success");

	session_1 = create_session();
	LOGI("main create_session_1 success");
	
	initialize_session(session_1, "222.214.218.237", 6601, "1299880", 0);
	LOGI("main initialize_session_1 success");
	
	set_session_index(session_1, 0);
	LOGI("main set_session_index_1 success");
	
	add_session(schd, session_1);
	LOGI("main add_session_1 success");

	session_2 = create_session();
	LOGI("main create_session_2 success");
	
	initialize_session(session_2, "222.214.218.237", 6601, "1299880", 1);
	LOGI("main initialize_session_2 success");
	
	set_session_index(session_2, 1);
	LOGI("main set_session_index_2 success");
	
	add_session(schd, session_2);
	LOGI("main add_session_2 success");

	set_surface_mode(schd->surface, mode_2);
	LOGI("main set_surface_mode success");

	scheduler_start(schd);
	session_start(session_1);
	session_start(session_2);

	scheduler_wait(schd);

	session_stop(session_1);
	session_stop(session_2);
	destroy_session(session_1);
	destroy_session(session_2);
	
	return 0;
}

/* Include the SDL main definition header */
#include "SDL_main.h"

/*******************************************************************************
                 Functions called by JNI
*******************************************************************************/

/* Called before SDL_main() to initialize JNI bindings in SDL library */
extern void SDL_Android_Init(JNIEnv* env, jclass cls);

void Java_com_jorhy_player_PlayerActivity_nativeInit(JNIEnv* env, jclass cls, jobject obj)
{
	 /* This interface could expand with ABI negotiation, calbacks, etc. */
    //SDL_Android_Init(env, cls);
	//LOGI("Java_com_jorhy_player_PlayerActivity_nativeInit SDL_Android_Init success");
	
    SDL_SetMainReady();
	LOGI("Java_com_jorhy_player_PlayerActivity_nativeInit SDL_SetMainReady success");

    /* Run the application code! */
    int status;
    char *argv[2];
    argv[0] = SDL_strdup("SDL_app");
    argv[1] = NULL;
    status = SDL_main(1, argv);

    /* Do not issue an exit or the whole application will terminate instead of just the SDL thread */
    /* exit(status); */
}