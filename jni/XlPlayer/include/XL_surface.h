#ifndef _XL_surface_h
#define _XL_surface_h
#include "SDL.h"
#include "XL_decoder.h"

enum SurfaceMode {
	modeA = 1,		/* 1x1 */
	modeB = 2,		/* 1x2 */
	modeC = 4,		/* 2x2 */
};

enum ScreenMode {
	screenModeNone = 0,
	screenModeA,
	screenModeB,
	screenModeC
};

typedef struct JoSurface {
	SDL_Window * screen;
	SDL_Renderer* render;
	SDL_Texture* texture[4];
	/*max video screen max 4*/
	SDL_Rect video[4];
	/*for full screen*/
	int screen_mode;
	SDL_Rect full_screen;
} JoSurface;
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

extern JoSurface *create_surface(void *native_windows);
extern void set_surface_mode(JoSurface *surface, int mode);
extern void set_full_mode(JoSurface *surface, float x, float y);
extern void screen_capture(const char *file_name, JoSurface *surface, H264Decoder *decoderA, H264Decoder *decoderB);
extern void render_frame(JoSurface *surface, H264Decoder *decoder, int index);
extern void destroy_surface(JoSurface *surface);

#ifdef __cplusplus
}
#endif

#endif /* _XL_surface_h */