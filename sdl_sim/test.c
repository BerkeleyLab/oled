// demo the `psu_board_gui` on a PC using SDL2
#include <SDL2/SDL.h>
#include <time.h>
#include <math.h>
#include "frame_buffer.h"
#include "psu_board_gui.h"
#include "lv_font.h"

#define ZOOM 4

uint16_t reg_map[64] = {
    6,          // uptime (1 h)
    25 << 8,    // PSU_TEMP (25 degC)
    0,          // PSU_VCCINT (0 V)
    1000,       // PSU_VCCAUX  (1 V)
    1800,       // PSU_VCCBRAM  (1.8 V)
    12546,      // PSU_IN_VOLTAGE
    1234,       // PSU_IN_CURRENT
    5535,       // PSU_A_VOLTAGE
    385,        // PSU_A_CURRENT
    5522,       // PSU_B_VOLTAGE
    119,        // PSU_B_CURRENT
    5627,       // PSU_C_VOLTAGE
    110,        // PSU_C_CURRENT
    5124,       // PSU_D_VOLTAGE
    50,         // PSU_D_CURRENT
    30,         // PSU_E_VOLTAGE
    198,        // PSU_E_CURRENT
    125 * 0x100,// DC_A_TEMP (125 degC)
    1000,       // DC_A_VOLTAGE (1 V)
    -25 * 0x100,// DC_B_TEMP (-25 degC)
    5000,       // DC_B_VOLTAGE (5 V)
    0,          // INLK_A_FLAGS
    0,          // INLK_A_VAL
    12 * 0x100, // INLK_A_VAL_MAX
    -65 * 0x100,// INLK_A_VAL_MIN
    0,          // INLK_B_FLAGS
    0,          // INLK_B_VAL
    0,          // INLK_B_VAL_MAX
    0,          // INLK_B_VAL_MIN
};

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

                case SDL_KEYDOWN:
                    switch(e.key.keysym.sym) {
                        case SDLK_LEFT:
                            btns |= 1;
                            break;
                        case SDLK_RIGHT:
                            btns |= 2;
                            break;
                        case SDLK_UP:
                            btns |= 4;
                            break;
                    }
                    break;

            }
        }
        if (isExit)
            break;

        if (btns & 4)
            for (int i=0; i < sizeof(reg_map) / sizeof(reg_map[0]); i++)
                reg_map[i] += 1;

        draw_psu_gui(btns);

        send_fb();
        SDL_Delay(100);
    }

    SDL_DestroyRenderer(rr);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

