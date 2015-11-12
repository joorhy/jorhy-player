#include "XL_log.h"
#include "XL_surface.h"

JoSurface *create_surface(void *native_windows) {
	int w, h, i;

	JoSurface *surface = (JoSurface *)malloc(sizeof(JoSurface));
	memset(surface, 0, sizeof(JoSurface));

#ifdef __ANDROID__
	if (native_windows == NULL) {
		SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_FOREIGN, &surface->screen, &surface->render);
	} else {
		surface->screen = SDL_CreateWindowFrom(native_windows);
		surface->render = SDL_CreateRenderer(surface->screen, -1, 0);
	}
	
#else
	if (native_windows == NULL) {
		surface->screen = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 352, 576, SDL_WINDOW_OPENGL);
	} else {
		surface->screen = SDL_CreateWindowFrom(native_windows);
	}

	surface->render = SDL_CreateRenderer(surface->screen, -1, SDL_RENDERER_SOFTWARE);
#endif
	SDL_GetWindowSize(surface->screen, &w, &h);
	LOGI("initialize_surface w = %d, h = %d", w, h);
	for (i = 0; i < 4; i++) {
		surface->texture[i] = SDL_CreateTexture(surface->render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, w, h);
	}
	surface->screen_mode = screenModeNone;

	return surface;
}

void set_surface_mode(JoSurface *surface, int mode) {
	int w, h;
	SDL_Surface *WindowScreen;
	SDL_GetWindowSize(surface->screen, &w, &h);
	
	//WindowScreen = SDL_GetWindowSurface(surface->screen);
	//SDL_FillRect(WindowScreen, NULL, SDL_MapRGB(WindowScreen->format, 96, 96, 96));
	SDL_SetRenderDrawColor(surface->render, 96, 96, 96, 255);
	//SDL_RenderSetScale(surface->render, 1, 1);
	surface->full_screen.x = 0; surface->full_screen.y = 0;
	surface->full_screen.w = w; surface->full_screen.h = h;
	switch (mode) {
	case modeA:
		surface->video[0].x = 0; surface->video[0].y = 0;
		surface->video[0].w = w; surface->video[0].h = h;
		//SDLTest_DrawString(surface->render, surface->video[0].w / 2 - 40, surface->video[0].h / 2, "loading...");
		//SDL_RenderDrawRect(surface->render, &surface->video[0]);
		break;
	case modeB:
		/*for first window*/
		surface->video[0].x = 0; surface->video[0].y = 0;
		surface->video[0].w = w; surface->video[0].h = h / 2;
		//SDLTest_DrawString(surface->render, surface->video[0].w / 2 - 40, surface->video[0].h / 2, "loading...");
		//SDL_RenderDrawRect(surface->render, &surface->video[0]);
		/*for second window*/
		surface->video[1].x = 0; surface->video[1].y = h / 2;
		surface->video[1].w = w; surface->video[1].h = h / 2;
		//SDLTest_DrawString(surface->render, surface->video[0].w / 2 - 40, surface->video[0].h / 2 * 3, "loading...");
		//SDL_RenderDrawRect(surface->render, &surface->video[1]);
		break;
	case modeC:
		/*for left up window*/
		surface->video[0].x = 0; surface->video[0].y = 0;
		surface->video[0].w = w / 2; surface->video[0].h = h / 2;
		//SDL_RenderDrawRect(surface->render, &surface->video[0]);
		/*for right up window*/
		surface->video[1].x = w / 2; surface->video[1].y = 0;
		surface->video[1].w = w / 2; surface->video[1].h = h / 2;
		//SDL_RenderDrawRect(surface->render, &surface->video[1]);
		/*for left down window*/
		surface->video[2].x = 0; surface->video[2].y = h / 2;
		surface->video[2].w = w / 2; surface->video[2].h = h / 2;
		//SDL_RenderDrawRect(surface->render, &surface->video[2]);
		/*for right down window*/
		surface->video[3].x = w / 2; surface->video[3].y = h / 2;
		surface->video[3].w = w / 2; surface->video[3].h = h / 2;
		//SDL_RenderDrawRect(surface->render, &surface->video[3]);
		break;
	}
	//SDL_UpdateWindowSurface(surface->screen);
	//SDL_FreeSurface(WindowScreen);
}

void set_full_mode(JoSurface *surface, float x, float y) {
	if (surface->screen_mode == screenModeNone) {
		if (y > 0 && y < 0.5) {
			surface->screen_mode = screenModeA;
		}
		else if (y > 0.5 && y < 1) {
			surface->screen_mode = screenModeB;
		}
	} else {
		surface->screen_mode = screenModeNone;
	}
}

void screen_capture(const char *file_name, JoSurface *surface, H264Decoder *decoderA, H264Decoder *decoderB) {
	if (surface->screen_mode == screenModeNone) {
		save_png(file_name, decoderA, decoderB);
	} else if (surface->screen_mode == screenModeA) {
		save_png(file_name, decoderA, NULL);
	} else if (surface->screen_mode == screenModeB) {
		save_png(file_name, NULL, decoderB);
	}
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
	if (surface->screen_mode == screenModeNone) {
		if (videoA == 1) {
			SDL_RenderCopy(surface->render, surface->texture[0], &decoder->rect, &surface->video[0]);
		}
		if (videoB == 1) {
			SDL_RenderCopy(surface->render, surface->texture[1], &decoder->rect, &surface->video[1]);
		}
	} else if (surface->screen_mode == screenModeA) {
		SDL_RenderCopy(surface->render, surface->texture[0], &decoder->rect, &surface->full_screen);
	} else if (surface->screen_mode == screenModeB) {
		SDL_RenderCopy(surface->render, surface->texture[1], &decoder->rect, &surface->full_screen);
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