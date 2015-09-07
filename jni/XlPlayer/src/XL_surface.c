#include "XL_surface.h"

JoSurface *create_surface()
{
	JoSurface *surface = (JoSurface *)malloc(sizeof(JoSurface));
	memset(surface, 0, sizeof(JoSurface));
	return surface;
}

int initialize_surface(JoSurface *surface, void *native_windows)
{
	int width, height;
	if (native_windows == NULL)
	{
		surface->screen = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 352, 576, /*SDL_WINDOW_FULLSCREEN |*/ SDL_WINDOW_OPENGL);
	}
	else
	{
		surface->screen = SDL_CreateWindowFrom(native_windows);
	}

	surface->render = SDL_CreateRenderer(surface->screen, -1, SDL_RENDERER_SOFTWARE);

	SDL_GetWindowSize(surface->screen, &width, &height);
	surface->texture = SDL_CreateTexture(surface->render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
}

void set_surface_mode(JoSurface *surface, int mode)
{
	int x, y, w, h;
	x = 0;
	y = 0;
	//SDL_GetWindowPosition(surface->screen, &x, &y);
	SDL_GetWindowSize(surface->screen, &w, &h);

	switch (mode)
	{
	case mode_1:
		surface->video[0].x = x;
		surface->video[0].y = y;
		surface->video[0].w = w;
		surface->video[0].h = h;
		break;
	case mode_2:
		/*for first window*/
		surface->video[0].x = x;
		surface->video[0].y = y;
		surface->video[0].w = w;
		surface->video[0].h = h / 2;
		/*for second window*/
		surface->video[1].x = x;
		surface->video[1].y = y + h / 2;
		surface->video[1].w = w;
		surface->video[1].h = h / 2;
		break;
	case mode_4:
		/*for left up window*/
		surface->video[0].x = x;
		surface->video[0].y = y;
		surface->video[0].w = w / 2;
		surface->video[0].h = h / 2;
		/*for right up window*/
		surface->video[1].x = x + w / 2;
		surface->video[1].y = y;
		surface->video[1].w = w / 2;
		surface->video[1].h = h / 2;
		/*for left down window*/
		surface->video[2].x = x;
		surface->video[2].y = y + h / 2;
		surface->video[2].w = w / 2;
		surface->video[2].h = h / 2;
		/*for right down window*/
		surface->video[3].x = x + w / 2;
		surface->video[3].y = y + h / 2;
		surface->video[3].w = w / 2;
		surface->video[3].h = h / 2;
		break;
	}
}

void render_frame(JoSurface *surface, H264Decoder *decoder, int index)
{
	SDL_UpdateYUVTexture(surface->texture, &decoder->rect, (const Uint8 *)decoder->picture->data[0], decoder->picture->linesize[0],
		(const Uint8 *)decoder->picture->data[1], decoder->picture->linesize[1], (const Uint8 *)decoder->picture->data[2], decoder->picture->linesize[2]);
	//SDL_RenderClear(surface->render);
	SDL_RenderCopy(surface->render, surface->texture, &decoder->rect, &surface->video[index]);
	SDL_RenderPresent(surface->render);
}

void destroy_surface(JoSurface *surface)
{
	SDL_DestroyWindow(surface->screen);
	SDL_DestroyRenderer(surface->render);
	SDL_DestroyTexture(surface->texture);

	free(surface);
}