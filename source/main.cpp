#include <iostream>
#include <unordered_map>
#include <string>

#include "../include/SDL2/SDL.h"
#include "window.hpp"
#include "input.hpp"
#include "scene.hpp"

int main(int argc, char* argv[]) {
    const bool generateVideo = true;
    const int targetFps = 60;  // SDL auto caps at 60
    const int ticksPerFrame = 1000 / targetFps;  // a tick is a ms
    Window window = Window(RenderMode::simpleRenderer);
    SDL_Surface* revealImg = SDL_ConvertSurfaceFormat(window.loadSurface("./assets/image.png"), SDL_PIXELFORMAT_ARGB8888, 0);

    Scene scene = Scene(695, 2, revealImg);
    scene.draw(window);

    Uint64 frameTimer = SDL_GetTicks64();
    double dt = 1.0;
    bool run = true;

    while (run) {
        // get delta time
        dt = (double) (SDL_GetTicks64() - frameTimer) * targetFps / 1000;
        frameTimer = SDL_GetTicks64();

        // read input once to be accessed by any system
        Input::readEvents(window);
        std::unordered_map<SDL_Keycode, bool> press = Input::getPressed();
        run = !Input::getQuit() && !press[SDLK_COMMA];

        // update
        scene.update(dt);

        // render
        window.clear();
        scene.draw(window);

        // output window frame to cout for ffmpeg processing via pipe
        // must be output before present render
        if (scene.displayCompleteSimulation && generateVideo) {
            SDL_Rect screenRect = {0, 0, revealImg->w, revealImg->h};
            window.coutFrame(&screenRect);
        }

        window.presentRender();

        // delay to cap frame rate
        Uint64 frameTime = SDL_GetTicks64() - frameTimer;
        if (frameTime < ticksPerFrame) {
             SDL_Delay(ticksPerFrame - frameTime);
        }

        // display fps in window title
        frameTime = SDL_GetTicks64() - frameTimer;
        window.setTitle(std::to_string((double) 1000 / frameTime));

        run = scene.getRun();
    }

    window.close();
    return 0;
}
