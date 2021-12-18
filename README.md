# SDL full screen and Emscripten

**This issue has been sufficiently resolved.  For whatever reason, the fullscreen change can't occur as part of the `mouseup` event.  See [Issue 158](https://github.com/emscripten-ports/SDL2/issues/158).  Change `SDL_MOUSEBUTTONUP` to `SDL_MOUSEBUTTONDOWN`** 

For some reason, the way in which I invoke the SDL full screen mode in Emscripten behaves different if it comes because of a key press vs. a mouse
click in the main loop.  Here is a small example program which demonstrates the issue:

```C
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

    /* Slightly different behavior between the accelerated and software renderers.
     *   The actual full screen transistion still doesn't occur on the mouse click in 
     *   Emscripten, but the "new" origin (0,0) for the indeterminate "full screen" mode 
     *   is different between the two renderers.
     */
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    //SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

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
                        // This full screen toggle works as expected
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
                    // This full screen toggle from a mouse click is "odd"
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
```

In this example, a red line is drawn from the upper left corner to the mouse
position.  By pressing the "f" key, the screen changes from "window" mode to
["desktop fullscreen"](https://wiki.libsdl.org/SDL_SetWindowFullscreen) mode.
Clicking anywhere with the left mouse button is also supposed to do the very
same thing.  But there is a catch, when clicking the mouse button to go to the
"fullscreen" mode, the screen doesn't update correctly; instead the screen
appearance stays the same, but the now the actual mouse cursor and where the
line gets drawn are not the same, presuably because something in the SDL system
thinks the screen has changed in resolution, but browser actually hasn't
changed anything yet.  The actual screen change will then occur if you press
some other key (say "x" or the space bar).  Or if you do something that would
otherwise obscure part of the screen (try the browser scroll bar).

This example can be tried over at: [https://www.escriben.org/fullscreen/index_fullscreen.html](https://www.escriben.org/fullscreen/index_fullscreen.html)

...with the source files at:

[https://github.com/gregbuchholz/SDL_fullscreen_emscripten](https://github.com/gregbuchholz/SDL_fullscreen_emscripten)

...there is one interesting difference between the
["accelerated"](https://wiki.libsdl.org/SDL_RendererFlags) and "software"
renderers. The main behavior is the same, with the mouse click leaving the
system in some sort of "indeterminate" full screen state, but with the software
renderer, the new origin (from where the red line get drawn) remains in the
same place as it was in the "windowed" mode, whereas with the accelerated
renderer, the new origin is somewhere above the actual physical screen.  See
the [screenshot](https://github.com/gregbuchholz/SDL_fullscreen_emscripten/blob/main/accelerated_anomaly.png).

This behavior is present with the following version of Emscripten:

    $ emcc -v
    emcc (Emscripten gcc/clang-like replacement + linker emulating GNU ld) 3.0.0 (3fd52e107187b8a169bb04a02b9f982c8a075205)
    clang version 14.0.0 (https://github.com/llvm/llvm-project 4348cd42c385e71b63e5da7e492172cff6a79d7b)
    Target: wasm32-unknown-emscripten
    Thread model: posix
    InstalledDir: /home/greg/emsdk/upstream/bin

...and seems consistent between the following browsers:

* Firefox 95.0 on Lubuntu
* Firefox 95.0.1 on Windows 10
* Edge 96.0.1054.62 on Windows 10
* Firefox 78.15.0 on Raspbian
* Chromium 92.0.4515.92 on Raspbian 

Thanks!

