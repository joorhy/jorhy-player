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
#if 1
static Scheduler *schd;
static RtspSession *session_1;
static RtspSession *session_2;

int main(int argc, char *argv[])
{
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

	return 0;
}

#else
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "SDL.h"

typedef struct Sprite
{
	SDL_Texture* texture;
	Uint16 w;
	Uint16 h;
} Sprite;

/* Adapted from SDL's testspriteminimal.c */
Sprite LoadSprite(const char* file, SDL_Renderer* renderer)
{
	Sprite result;
	result.texture = NULL;
	result.w = 0;
	result.h = 0;
	
    SDL_Surface* temp;

    /* Load the sprite image */
    temp = SDL_LoadBMP(file);
    if (temp == NULL)
	{
        fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
        return result;
    }
    result.w = temp->w;
    result.h = temp->h;

    /* Create texture from the image */
    result.texture = SDL_CreateTextureFromSurface(renderer, temp);
    if (!result.texture) {
        fprintf(stderr, "Couldn't create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(temp);
        return result;
    }
    SDL_FreeSurface(temp);

    return result;
}

void draw(SDL_Window* window, SDL_Renderer* renderer, const Sprite sprite)
{
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	SDL_Rect destRect = {w/2 - sprite.w/2, h/2 - sprite.h/2, sprite.w, sprite.h};
	/* Blit the sprite onto the screen */
	SDL_RenderCopy(renderer, sprite.texture, NULL, &destRect);
}

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;

    if(SDL_CreateWindowAndRenderer(0, 0, 0, &window, &renderer) < 0)
        exit(2);

	Sprite sprite = LoadSprite("image.bmp", renderer);
    if(sprite.texture == NULL)
        exit(2);

    /* Main render loop */
    Uint8 done = 0;
    SDL_Event event;
    while(!done)
	{
        /* Check for events */
        while(SDL_PollEvent(&event))
		{
            if(event.type == SDL_QUIT || event.type == SDL_KEYDOWN || event.type == SDL_FINGERDOWN)
			{
                done = 1;
            }
        }
		
		
		/* Draw a gray background */
		SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
		SDL_RenderClear(renderer);
		
		draw(window, renderer, sprite);
	
		/* Update the screen! */
		SDL_RenderPresent(renderer);
		
		SDL_Delay(10);
    }

    exit(0);
}
#endif

/* Include the SDL main definition header */
#include "SDL_main.h"

/*******************************************************************************
                 Functions called by JNI
*******************************************************************************/

/* Called before SDL_main() to initialize JNI bindings in SDL library */
extern void SDL_Android_Init(JNIEnv* env, jclass cls);

void Java_org_libsdl_app_SDLActivity_nativeInit(JNIEnv* env, jclass cls, jobject obj)
{
	 /* This interface could expand with ABI negotiation, calbacks, etc. */
    SDL_Android_Init(env, cls);
	LOGI("Java_com_jorhy_player_PlayerActivity_nativeInit SDL_Android_Init success");
	
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