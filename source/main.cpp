#include <iostream>
#include <unordered_map>
#include <string>

#include "../include/SDL2/SDL.h"
#include "window.hpp"
#include "input.hpp"
#include "scene.hpp"

int main(int argc, char* argv[]) {
    // get command line args
    int width, height, numBalls, ballSize, ballInterval, frameDuration;
    std::string imagePath;
    if (argc == 8) {
        width = atoi(argv[1]);
        height = atoi(argv[2]);
        numBalls = atoi(argv[3]);
        ballSize = atoi(argv[4]);
        ballInterval = atoi(argv[5]);
        frameDuration = atoi(argv[6]);
        imagePath = argv[7];
    } else {
        std::cerr << "Error: Incorrect number of command line arguments\n";
        throw argc;
    }
    
    const bool generateVideo = true;
    const int targetFps = 60;  // SDL auto caps at 60
    const int ticksPerFrame = 1000 / targetFps;  // a tick is a ms
    Window window = Window(width, height, RenderMode::simpleRenderer);

    // get scaled reveal image
    SDL_Surface* unscaledImg = SDL_ConvertSurfaceFormat(window.loadSurface(imagePath), SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_Rect unscaledRect {0, 0, unscaledImg->w, unscaledImg->h};
    Uint32 rmask, gmask, bmask, amask;
    if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
        bmask = 0xff000000;
        gmask = 0x00ff0000;
        rmask = 0x0000ff00;
        amask = 0x000000ff;
    } else {
        bmask = 0x000000ff;
        gmask = 0x0000ff00;
        rmask = 0x00ff0000;
        amask = 0xff000000;
    }
    SDL_Surface* revealImg = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, rmask, gmask, bmask, amask);
    SDL_Rect revealRect {0, 0, revealImg->w, revealImg->h};
    SDL_BlitScaled(unscaledImg, &unscaledRect, revealImg, &revealRect);
    SDL_FreeSurface(unscaledImg);

    Scene scene = Scene(numBalls, ballSize, ballInterval, frameDuration, revealImg);
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
            SDL_Rect screenRect = {0, 0, width, height};
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

    SDL_FreeSurface(revealImg);
    window.close();
    return 0;
}
