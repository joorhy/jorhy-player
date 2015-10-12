#include "XL_log.h"
#include "XL_surface.h"

JoSurface *create_surface()
{
	JoSurface *surface = (JoSurface *)malloc(sizeof(JoSurface));
	memset(surface, 0, sizeof(JoSurface));
	return surface;
}

int initialize_surface(JoSurface *surface, void *native_windows)
{
	int width, height, i;
	//SDL_Init(SDL_INIT_VIDEO);

#ifndef __ANDROID__
	SDL_CreateWindowAndRenderer(352, 576, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN, &surface->screen, &surface->render);
#else
	if (native_windows == NULL)
	{
		surface->screen = SDL_CreateWindow("aaaa", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 480, 762, SDL_WINDOW_OPENGL/* | SDL_WINDOW_FULLSCREEN*/);
	}
	else
	{
		surface->screen = SDL_CreateWindowFrom(native_windows);
	}

	surface->render = SDL_CreateRenderer(surface->screen, -1, 0/*SDL_RENDERER_SOFTWARE*/);
#endif
	SDL_GetWindowSize(surface->screen, &width, &height);
	LOGI("initialize_surface w = %d, h = %d", width, height);
	for (i = 0; i < 4; i++)
	{
		surface->texture[i] = SDL_CreateTexture(surface->render, /*SDL_PIXELFORMAT_BGR888*/SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
		//SDL_SetRenderDrawColor(surface->render, 0xA0, 0xA0, 0xA0, 0xFF);
		//SDL_RenderClear(surface->render);
	}
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

static bool video_1 = 0;
static bool video_2 = 0;

void render_frame(JoSurface *surface, H264Decoder *decoder, int index)
{
#ifdef USE_FFMPEG
	SDL_UpdateYUVTexture(surface->texture, &decoder->rect, (const Uint8 *)decoder->picture->data[0], decoder->picture->linesize[0],
		(const Uint8 *)decoder->picture->data[1], decoder->picture->linesize[1], (const Uint8 *)decoder->picture->data[2], decoder->picture->linesize[2]);
#else
	if (index == 0)
	{
		video_1 = 1;
	}
	else
	{
		video_2 = 1;
	}
	SDL_UpdateYUVTexture(surface->texture[index], &decoder->rect, (const Uint8 *)decoder->data[0], decoder->bufInfo.UsrData.sSystemBuffer.iStride[0],
		(const Uint8 *)decoder->data[1], decoder->bufInfo.UsrData.sSystemBuffer.iStride[1], (const Uint8 *)decoder->data[2], decoder->bufInfo.UsrData.sSystemBuffer.iStride[1]);
	//SDL_UpdateTexture(surface->texture, &decoder->rect, decoder->yuv, decoder->rect.w);
	//SDL_UpdateTexture(surface->texture, &decoder->rect, decoder->rgb, decoder->rect.w * 3);
#endif
	//SDL_SetRenderDrawColor(surface->render, 0xA0, 0xA0, 0xA0, 0xFF);
	SDL_RenderClear(surface->render);
	//LOGI("x = %d, y = %d, w = %d, h = %d", decoder->rect.x, decoder->rect.y, decoder->rect.w, decoder->rect.h);
	LOGI("x = %d, y = %d, w = %d, h = %d", surface->video[index].x, surface->video[index].y, surface->video[index].w, surface->video[index].h);
	//SDL_RenderCopy(surface->render, surface->texture, &decoder->rect, &decoder->rect);
	if (video_1 == 1)
	{
		SDL_RenderCopy(surface->render, surface->texture[0], &decoder->rect, &surface->video[0]);
	}
	if (video_2 == 1)
	{
		SDL_RenderCopy(surface->render, surface->texture[1], &decoder->rect, &surface->video[1]);
	}
	SDL_RenderPresent(surface->render);
}

void destroy_surface(JoSurface *surface)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		SDL_DestroyTexture(surface->texture[i]);
	}
	SDL_DestroyWindow(surface->screen);
	SDL_DestroyRenderer(surface->render);
	free(surface);
}