// Compile with:
//    emcc fullscreen.c -s USE_SDL=2 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1
// Run with:
//    emrun index_fullscreen.html

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

void full_screen_toggle(SDL_Window *window, bool full_screen_q) {

    int ret_val = 0;

    if(full_screen_q) {
        ret_val = SDL_SetWindowFullscreen(window, 0);
    } else {
        ret_val = SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }

    if (ret_val < 0) {
        printf("Full screen toggle error: %s\n",SDL_GetError());
    } else {
        printf("Full screen toggle successful\n");
    } 
}

int main(int argc, char *argv[])
{
    bool quit = false;
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Testing fullscreen in Emscripten",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int mouse_x = 0, mouse_y = 0;
    bool full_screen_q = false;

    while (!quit)
    {
        SDL_WaitEvent(&event);

        switch (event.type)
        {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_f:
                        printf("'f' pressed\n");
                        full_screen_toggle(window, full_screen_q);
                        full_screen_q = !full_screen_q; 
                        break;
                    case SDLK_q:
                        quit = true;
                        break;
                }
                break;
            case SDL_MOUSEMOTION:
                mouse_x = event.motion.x;
                mouse_y = event.motion.y;
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    printf("Left Mouse Button Up\n");
                    full_screen_toggle(window, full_screen_q);
                    full_screen_q = !full_screen_q; 
                }
                break;
        }

        SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer, 0, 0, mouse_x, mouse_y);
        SDL_RenderPresent(renderer);
    }


    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
