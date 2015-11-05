#include "XL_log.h"
#include "XL_surface.h"

JoSurface *create_surface(void *native_windows, int mode) {
	int w, h, i;

	JoSurface *surface = (JoSurface *)malloc(sizeof(JoSurface));
	memset(surface, 0, sizeof(JoSurface));

#ifndef __ANDROID__
	SDL_CreateWindowAndRenderer(352, 576, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN, &surface->screen, &surface->render);
#else
	if (native_windows == NULL) {
		surface->screen = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 480, 762, SDL_WINDOW_OPENGL);
	} else {
		surface->screen = SDL_CreateWindowFrom(native_windows);
	}

	surface->render = SDL_CreateRenderer(surface->screen, -1, 0);
#endif
	SDL_GetWindowSize(surface->screen, &w, &h);
	LOGI("initialize_surface w = %d, h = %d", w, h);
	for (i = 0; i < 4; i++) {
		surface->texture[i] = SDL_CreateTexture(surface->render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, w, h);
	}

	switch (mode) {
	case modeA:
		surface->video[0].x = 0; surface->video[0].y = 0;
		surface->video[0].w = w; surface->video[0].h = h;
		break;
	case modeB:
		/*for first window*/
		surface->video[0].x = 0; surface->video[0].y = 0;
		surface->video[0].w = w; surface->video[0].h = h / 2;
		/*for second window*/
		surface->video[1].x = 0; surface->video[1].y = h / 2;
		surface->video[1].w = w; surface->video[1].h = h / 2;
		break;
	case modeC:
		/*for left up window*/
		surface->video[0].x = 0; surface->video[0].y = 0;
		surface->video[0].w = w / 2; surface->video[0].h = h / 2;
		/*for right up window*/
		surface->video[1].x = w / 2; surface->video[1].y = 0;
		surface->video[1].w = w / 2; surface->video[1].h = h / 2;
		/*for left down window*/
		surface->video[2].x = 0; surface->video[2].y = h / 2;
		surface->video[2].w = w / 2; surface->video[2].h = h / 2;
		/*for right down window*/
		surface->video[3].x = w / 2; surface->video[3].y = h / 2;
		surface->video[3].w = w / 2; surface->video[3].h = h / 2;
		break;
	}
	
	return surface;
}

void render_frame(JoSurface *surface, H264Decoder *decoder, int index) {
	static int videoA = 0;
	static int videoB = 0;
#ifdef USE_FFMPEG
	SDL_UpdateYUVTexture(surface->texture, &decoder->rect, (const Uint8 *)decoder->picture->data[0], decoder->picture->linesize[0],
		(const Uint8 *)decoder->picture->data[1], decoder->picture->linesize[1], (const Uint8 *)decoder->picture->data[2], decoder->picture->linesize[2]);
#else
	if (index == 0) {
		videoA = 1;
	} else {
		videoB = 1;
	}
	SDL_UpdateYUVTexture(surface->texture[index], &decoder->rect, (const Uint8 *)decoder->data[0], decoder->bufInfo.UsrData.sSystemBuffer.iStride[0],
		(const Uint8 *)decoder->data[1], decoder->bufInfo.UsrData.sSystemBuffer.iStride[1], (const Uint8 *)decoder->data[2], decoder->bufInfo.UsrData.sSystemBuffer.iStride[1]);
#endif
	SDL_RenderClear(surface->render);
	LOGI("x = %d, y = %d, w = %d, h = %d", surface->video[index].x, surface->video[index].y, surface->video[index].w, surface->video[index].h);
	if (videoA == 1) {
		SDL_RenderCopy(surface->render, surface->texture[0], &decoder->rect, &surface->video[0]);
	}
	if (videoB == 1) {
		SDL_RenderCopy(surface->render, surface->texture[1], &decoder->rect, &surface->video[1]);
	}
	SDL_RenderPresent(surface->render);
}

void destroy_surface(JoSurface *surface) {
	int i;
	for (i = 0; i < 4; i++) {
		SDL_DestroyTexture(surface->texture[i]);
	}
	SDL_DestroyWindow(surface->screen);
	SDL_DestroyRenderer(surface->render);
	free(surface);
}