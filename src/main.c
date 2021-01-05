#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <GL/glew.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear/nuklear.h"
#include "nuklear/demo/sdl_opengl3/nuklear_sdl_gl3.h"

#define MAX_VERT_BUF 512 * 1024
#define MAX_ELEM_BUF 128 * 1024
static struct nk_context *nkctx;

#define DEFAULT_WIDTH 1200
#define DEFAULT_HEIGHT 800
#define PI32 3.14159265358979323846f
#define GL_CALL_CASE(c) case c: printf(#c); break

#ifndef NDEBUG
#define GL_CALL(x) \
	x; \
	do { \
		GLenum err; \
		while ((err = glGetError())) { \
			printf("[OpenGL Error]("); \
			switch (err) { \
			GL_CALL_CASE(GL_INVALID_ENUM); \
			GL_CALL_CASE(GL_INVALID_VALUE); \
			GL_CALL_CASE(GL_INVALID_OPERATION); \
			GL_CALL_CASE(GL_STACK_OVERFLOW); \
			GL_CALL_CASE(GL_STACK_UNDERFLOW); \
			GL_CALL_CASE(GL_OUT_OF_MEMORY); \
			GL_CALL_CASE(GL_INVALID_FRAMEBUFFER_OPERATION); \
			default: printf("%d", err); break; \
			} \
			printf("): " #x " %s:%d\n", __FILE__, __LINE__); \
		} \
		fflush(stdout); \
	} while (0)
#else
#define GL_CALL(x) x
#endif

#include "smart-rockets.h"

#define die(...) fprintf(stderr, __VA_ARGS__), exit(EXIT_FAILURE)


int main(void)
{
	/* Platform */
	SDL_Window *window;
	SDL_GLContext context;
	int width = DEFAULT_WIDTH, height = DEFAULT_HEIGHT;
	struct nk_font_atlas *atlas;
	float time, lasttime = 0;
	int isdone = 0, isMouseDown = 0;


	/* SDL2 */
	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS) < 0)
		die("Unable to initialize SDL: %s", SDL_GetError());
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	window = SDL_CreateWindow(
			"smart-rockets",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			width, height,
			SDL_WINDOW_OPENGL |
			SDL_WINDOW_SHOWN |
			SDL_WINDOW_RESIZABLE);
	if (!window) /* Die if creation failed */
		die("Unable to create window: %s", SDL_GetError());

	context = SDL_GL_CreateContext(window);
	//SDL_GL_SetSwapInterval(0);

	/* OpenGL */
	GL_CALL(glViewport(0, 0, width, height));
	glewExperimental = 1;
	if (glewInit() != GLEW_OK) {
		die("Failed to setup GLEW\n");
	}
	GL_CALL(glEnable(GL_BLEND));
	GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	/* Nuklear */
	nkctx = nk_sdl_init(window);
	nk_sdl_font_stash_begin(&atlas);
	nk_sdl_font_stash_end();

	time = (float)clock() / CLOCKS_PER_SEC;
	sym_init();

	glClearColor(0.05f, 0.12f, 0.18f, 1.0f);
	while (!isdone) {
		float dt;

		/* Input */
		SDL_Event e;
		nk_input_begin(nkctx);
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT: isdone = 1; break;
			case SDL_MOUSEBUTTONDOWN:
				if (e.button.button == SDL_BUTTON_RIGHT)
					isMouseDown = 1;
				break;
			case SDL_MOUSEBUTTONUP:
				if (e.button.button == SDL_BUTTON_RIGHT)
					isMouseDown = 0;
				break;
			}
			nk_sdl_handle_event(&e);
		} nk_input_end(nkctx);

		time = (float)clock() / CLOCKS_PER_SEC;
		dt = time - lasttime;
		lasttime = time;
		sym_update(dt);

		/* Draw */
		glClear(GL_COLOR_BUFFER_BIT);
		sym_render(width, height, isMouseDown);

		SDL_GetWindowSize(window, &width, &height);
		GL_CALL(glViewport(0, 0, width, height));
		GL_CALL(glEnable(GL_BLEND));
		GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		SDL_GL_SwapWindow(window);
	}

	nk_sdl_shutdown();
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

#include "math.c"
#include "pop.c"
#include "sym.c"
#include "util.c"
