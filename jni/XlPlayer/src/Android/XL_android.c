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

void Java_com_jorhy_player_PlayerActivity_nativeInit(JNIEnv* env, jclass cls, jobject obj)
{
	/* This interface could expand with ABI negotiation, calbacks, etc. */
    SDL_Android_Init(env, cls);
	
	schd = create_scheduler();
	initialize_scheduler(schd, NULL);

	session_1 = create_session();
	initialize_session(session_1, "222.214.218.237", 6601, "1299880", 0);
	set_session_index(session_1, 0);
	add_session(schd, session_1);

	session_2 = create_session();
	initialize_session(session_2, "222.214.218.237", 6601, "1299880", 1);
	set_session_index(session_2, 1);
	add_session(schd, session_2);

	set_surface_mode(schd->surface, mode_2);

	scheduler_start(schd);
	session_start(session_1);
	session_start(session_2);

	scheduler_wait(schd);

	session_stop(session_1);
	session_stop(session_2);
	destroy_session(session_1);
	destroy_session(session_2);
}