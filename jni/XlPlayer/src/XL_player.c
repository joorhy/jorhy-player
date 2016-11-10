#include "XL_log.h"
#include "XL_player.h"
#include "XL_rtsp_session.h"
#include "XL_scheduler.h"

#include <string.h>

#ifdef WIN32
//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#undef main
#endif 

static Scheduler *schd = NULL;
static RtspSession *sessionA = NULL;
static RtspSession *sessionB = NULL;
static char serverAddr[32];
static int serverPort;
static char vehA[256];
static char vehB[256];
static int channelA;
static int channelB;

static int isPaused = 1;
static int isRunning = 0;
static int videoMode = modeA;

int main(int argc, char *argv[]) {
	SDL_Event e;
	int x, y;
	if (argc < 5) {
		LOGI("Param error \n");
		return 1;
	}

	videoMode = modeA;
	memset(serverAddr, 0, sizeof(serverAddr));
	memcpy(serverAddr, argv[1], strlen(argv[1]));
	serverPort = atoi(argv[2]);
	memset (vehA, 0, sizeof(vehA));
	memcpy (vehA, argv[3], strlen(argv[3]));
	channelA = atoi(argv[4]);
	if (argc == 7)
	{
		memset (vehB, 0, sizeof(vehB));
		memcpy (vehB, argv[5], strlen(argv[5]));
		channelB = atoi(argv[6]);
		videoMode = modeB;
	}

	schd = create_scheduler(NULL);
	isRunning = 1;
	while (isRunning) {
		if (SDL_PollEvent(&e))  {
			if (e.type == SDL_QUIT) {
				stop_play();
				isRunning = 0;
				break;
			}
#ifdef __ANDROID__
			/*if (e.type == SDL_FINGERDOWN) {
				SDL_Touch *t = SDL_GetTouch(e.tfinger.touchId);
				x = t.tfinger.x / t->xres,
				y = t.tfinger.y / t->yres;
			}*/
#else
			if (isPaused) {
				if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {

					isPaused = 0;
					start_play();
					LOGI("Resume \n");
				}
			} else {
				if(e.type == SDL_KEYDOWN) {
					if (e.key.keysym.sym == SDLK_SPACE) {
						isPaused = 1;
						stop_play();
						LOGI("Pause \n");
						//break;
					} else if (e.key.keysym.sym == SDLK_F1) {
						changeScreen((float)0.5, (float)0.6);
					} else if (e.key.keysym.sym == SDLK_F2) {
						snapshot("test.png");
					}
					else if (e.key.keysym.sym == SDLK_F3) {
						record("path");
					}
				}
			}
#endif
		}
		scheduler_process(schd);
	}
	destroy_scheduler(schd);

	return 0;
}

void start_play() {
	if (isRunning) {
		set_surface_mode(schd->surface, videoMode);
		sessionA = create_session(serverAddr, serverPort, vehA, channelA, 0);
		add_session(schd, sessionA);
		session_start(sessionA);

		if (videoMode == modeB) {
			sessionB = create_session(serverAddr, serverPort, vehB, channelB, 1);
			add_session(schd, sessionB);
			session_start(sessionB);
		}
	}
}

void stop_play() {
	if (isRunning) {
		if (sessionA != NULL) {
			session_stop(sessionA);
			del_session(schd, sessionA);
			destroy_session(sessionA);
			sessionA = NULL;
		}
		
		if (videoMode == modeB) {
			if (sessionB != NULL) {
				del_session(schd, sessionB); 
				session_stop(sessionB); 
				destroy_session(sessionB);
				sessionB = NULL;
			}
		}
	}
}

void changeScreen(float x, float y) {
	if (videoMode == modeB) {
		set_full_mode(schd->surface, x, y);
	}
}

void snapshot(const char *file_name) {
	screen_capture(file_name, schd->surface, sessionA->decoder, sessionB->decoder);
}

void record(const char *file_name) {
	save_file(sessionA->record, "session_1.yuv");
	save_file(sessionB->record, "session_2.yuv");
}

#ifdef __ANDROID__
/* Include the SDL main definition header */
#include <jni.h>
#include "SDL_main.h"

/* Called before SDL_main() to initialize JNI bindings in SDL library */
extern void SDL_Android_Init(JNIEnv* env, jclass cls);

void Java_org_libsdl_app_SDLActivity_nativeInit(JNIEnv* env, jclass cls, jstring parm) {
	/* This interface could expand with ABI negotiation, calbacks, etc. */
	SDL_Android_Init(env, cls);
	LOGI("Java_org_libsdl_app_SDLActivity_nativeInit SDL_Android_Init success");

	SDL_SetMainReady();
	LOGI("Java_org_libsdl_app_SDLActivity_nativeInit SDL_SetMainReady success");

	/* Run the application code! */
	int status;
	char *argv[7];
	argv[0] = SDL_strdup("SDL_app");

	const char *argvStr = (*env)->GetStringUTFChars(env, parm, 0);
	LOGI("Java_org_libsdl_app_SDLActivity_nativeInit argvStr = %s", argvStr);
	// use your string
	int argc = 1;
	const char *sep = ",";
	char *p = strtok((char *)argvStr, sep);
	while (p) {
		LOGI("Java_org_libsdl_app_SDLActivity_nativeInit param%d = %s", argc, p);
		argv[argc] = (char *)malloc(strlen(p) + 1);
		memset(argv[argc], 0, strlen(p) + 1);
		memcpy(argv[argc], p, strlen(p));
		++argc;
		p = strtok(NULL, sep);
	}
	(*env)->ReleaseStringUTFChars(env, parm, argvStr);

	status = SDL_main(argc, argv);

	/* Do not issue an exit or the whole application will terminate instead of just the SDL thread */
	/* exit(status); */
}

void Java_org_libsdl_app_SDLActivity_nativeStartPlay(JNIEnv* env, jclass cls) {
	LOGI("Java_org_libsdl_app_SDLActivity_startPlay \n");
	if (isRunning && isPaused) {
		isPaused = 0;
		start_play();
	}
}

void Java_org_libsdl_app_SDLActivity_nativeStopPlay(JNIEnv* env, jclass cls) {
	LOGI("Java_org_libsdl_app_SDLActivity_stopPlay \n");
	if (isRunning && !isPaused) {
		isPaused = 1;
		stop_play();
	}
}

void Java_org_libsdl_app_SDLActivity_nativeExitPlay(JNIEnv* env, jclass cls) {
	LOGI("Java_org_libsdl_app_SDLActivity_exitPlay \n");
	if (isRunning && !isPaused) {
		isPaused = 1;
		stop_play();
		LOGI("Java_org_libsdl_app_SDLActivity_stopPlay \n");
	}
	isRunning = 0;
}

void Java_org_libsdl_app_SDLActivity_nativeDoubleClick(JNIEnv* env, jclass cls,  jfloat x,  jfloat y) {
	LOGI("Java_org_libsdl_app_SDLActivity_nativeDoubleClick \n");
	if (isRunning && !isPaused) {
		changeScreen(x, y);
		LOGI("changeScreen \n");
	}
}

void Java_org_libsdl_app_SDLActivity_nativeSnapshot(JNIEnv* env, jclass cls, jstring parm) {
	LOGI("Java_org_libsdl_app_SDLActivity_nativeSnapshot \n");
	if (isRunning && !isPaused) {
		snapshot((*env)->GetStringUTFChars(env, parm, 0));
		LOGI("snapshot \n");
	}
}
#endif