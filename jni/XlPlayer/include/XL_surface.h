#ifndef _XL_surface_h
#define _XL_surface_h
#include "SDL.h"
#include "XL_decoder.h"

enum SurfaceMode
{
	mode_1 = 1,		/* 1x1 */
	mode_2 = 2,		/* 1x2 */
	mode_4 = 4,		/* 2x2 */
};

typedef struct JoSurface
{
	SDL_Window * screen;
	SDL_Renderer* render;
	SDL_Texture* texture[4];
	/*max video screen max 4*/
	SDL_Rect video[4];
} JoSurface;
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

extern JoSurface *create_surface();
extern int initialize_surface(JoSurface *surface, void *native_windows);
extern void set_surface_mode(JoSurface *surface, int mode);
extern void render_frame(JoSurface *surface, H264Decoder *decoder, int index);
extern void destroy_surface(JoSurface *surface);

#ifdef __cplusplus
}
#endif

#endif /* _XL_surface_h */