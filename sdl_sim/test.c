// demo the `psu_board_gui` on a PC using SDL2
#include <SDL2/SDL.h>
#include <time.h>
#include <math.h>
#include "frame_buffer.h"
#include "lv_font.h"
#include "demo.h"

#define ZOOM 4

SDL_Renderer *rr = NULL;
SDL_Window* window = NULL;

// Send framebuffer to SDL2
static void send_fb()
{
    for (unsigned int y=0; y<DISPLAY_HEIGHT; y++) {
        for (int x=0; x<DISPLAY_WIDTH; x++) {
            SDL_SetRenderDrawColor(
                rr,
                0,
                0,
                getPixel(x, y) << 4,
                0xFF
            );
            SDL_RenderDrawPoint(rr, x, y);
        }
    }
    SDL_RenderPresent(rr);
}

static void init_sdl()
{
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
        return;
    }

    if (SDL_CreateWindowAndRenderer(
        DISPLAY_WIDTH, DISPLAY_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &rr
    )) {
        fprintf(stderr, "could not create window: %s\n", SDL_GetError());
        return;
    };

    SDL_RenderSetScale(rr, ZOOM, ZOOM);

}

void setLed(uint8_t val)
{
    ;
}

int main(int argc, char* args[])
{
    init_sdl();

    while (1) {
        SDL_Event e;
        bool isExit = false;
        unsigned btns=0;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    isExit = true;
                    break;
            }
        }
        if (isExit)
            break;

        demo(0);

        send_fb();
        SDL_Delay(20);
    }

    SDL_DestroyRenderer(rr);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

