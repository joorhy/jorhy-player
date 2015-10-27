#include "XL_log.h"
#include "XL_player.h"
#include "XL_rtsp_session.h"
#include "XL_scheduler.h"

#include <string.h>

#ifdef WIN32
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#undef main
#endif 

static Scheduler *schd;
static RtspSession *sessionA;
static RtspSession *sessionB;
static char serverAddr[32];
static int serverPort;
static char vehA[256];
static char vehB[256];
static int channelA;
static int channelB;

extern void start_play();
extern void stop_play();

int main(int argc, char *argv[]) {
	static int isPaused = 1;
	static int isRunning = 1;
	
	SDL_Event e;
	schd = create_scheduler(NULL, modeB);

	if (argc < 5) {
		LOGI("Param error \n");
		return 1;
	}

	memset(serverAddr, 0, sizeof(serverAddr));
	strcpy(serverAddr, argv[1]);
	serverPort = atoi(argv[2]);
	memset (vehA, 0, sizeof(vehA));
	strcpy (vehA, argv[3]);
	channelA = atoi(argv[4]);
	if (argc == 7)
	{
		memset (vehB, 0, sizeof(vehB));
		strcpy (vehB, argv[5]);
		channelA = atoi(argv[6]);
	}
	

	while (isRunning) {
		if (SDL_PollEvent(&e))  {
			if (e.type == SDL_QUIT  || e.type == SDL_FINGERDOWN) {
				isRunning = 0;
				break;
			}

			if (isPaused) {
			#ifdef __ANDROID__
				if(e.type == SDL_APP_WILLENTERFOREGROUND || e.type == SDL_APP_DIDENTERFOREGROUND) {
			#else
				if(e.type == SDL_KEYDOWN &&  e.key.keysym.sym == SDLK_SPACE) {
			#endif
					isPaused = 0;
					start_play();
					LOGI("Resume \n");
				}
			} else {
			#ifdef __ANDROID__
				if(e.type == SDL_APP_WILLENTERBACKGROUND || e.type == SDL_APP_DIDENTERBACKGROUND) {
			#else
				if(e.type == SDL_KEYDOWN &&  e.key.keysym.sym == SDLK_SPACE) {
			#endif
					isPaused = 1;
					stop_play();
					LOGI("Pause \n");
				}
			}
		}
		scheduler_process(schd);
	}

	destroy_scheduler(schd);
	
	return 0;
}

void start_play() {
	sessionA = create_session(serverAddr, serverPort, vehA, channelA, 0);
	add_session(schd, sessionA);

	sessionB = create_session(serverAddr, serverPort, vehB, channelB, 1);
	add_session(schd, sessionB);

	session_start(sessionA);
	session_start(sessionB);
}

void stop_play() {
	session_stop(sessionA);
	session_stop(sessionA);

	del_session(schd, sessionA);
	del_session(schd, sessionA);
	
	destroy_session(sessionA);
	destroy_session(sessionA);
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
    LOGI("Java_com_jorhy_player_PlayerActivity_nativeInit SDL_Android_Init success");
	
    SDL_SetMainReady();
    LOGI("Java_com_jorhy_player_PlayerActivity_nativeInit SDL_SetMainReady success");

    /* Run the application code! */
    int status;
    char *argv[7];
    argv[0] = SDL_strdup("SDL_app");

    const char *argvStr = (*env)->GetStringUTFChars(env, parm, 0);
   // use your string
   int argc = 1;
   const char *sep = ",";
   char *p = strtok((char *)argvStr, sep);
   while (p) {
   	LOGI("Java_com_jorhy_player_PlayerActivity_nativeInit SDL_Android_Init param%d = %s", argc, p);
	argv[argc] = p;
	++argc;
   }
   (*env)->ReleaseStringUTFChars(env, parm, argvStr);
   
    status = SDL_main(argc, argv);

    /* Do not issue an exit or the whole application will terminate instead of just the SDL thread */
    /* exit(status); */
}
#endif