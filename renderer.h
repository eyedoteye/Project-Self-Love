#include <SDL2/SDL_video.h>

#define LOAD_RENDERER(name) void name(void* AllocatedMemory, SDL_Window *Window)
typedef LOAD_RENDERER(load_renderer);

#define RELOAD_RENDERER(name) void name(void* AllocatedMemory, SDL_Window *Window)
typedef RELOAD_RENDERER(reload_renderer);

#define RENDER_GAME(name) void name(void* AllocatedMemory)
typedef RENDER_GAME(render_game);