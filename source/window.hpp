#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "../include/SDL2/SDL.h"
#include "../include/SDL2/SDL_image.h"
#include "point.hpp"

class Colour {
    public:
        int r = 255;
        int g = 255;
        int b = 255;
        int a = 255;

        Colour(int r, int g, int b, int a) {
            init(r, g, b, a);
        }

        Colour(Uint32 argb8888) {
            int a = (int)((Uint8)(argb8888 >> 24));
            int r = (int)((Uint8)(argb8888 >> 16));
            int g = (int)((Uint8)(argb8888 >> 8));
            int b = (int)((Uint8)argb8888);
            init(r, g, b, a);
        }

        void init(int r, int g, int b, int a) {
            this->r = std::min(std::max(r, 0), 255);
            this->g = std::min(std::max(g, 0), 255);
            this->b = std::min(std::max(b, 0), 255);
            this->a = std::min(std::max(a, 0), 255);
        }

        bool operator==(Colour& colour) {
            return this->ARGB8888() == colour.ARGB8888();
        }

        bool operator!=(Colour& colour) {
            return !(*this == colour);
        }

        Colour operator-(const Colour& colour) {
            return Colour(r - colour.r, g - colour.g, b - colour.b, a - colour.a);
        }

        Colour operator+(const Colour& colour) {
            return Colour(r + colour.r, g + colour.g, b + colour.b, a + colour.a);
        }

        Uint32 ARGB8888() {
            return (Uint32)((a << 24) + (r << 16) + (g << 8) + b);
        }
};

// can be accessed with Colours::colourName syntax
struct Colours {
    static inline Colour none = {0, 0, 0, 0};
    static inline Colour red = {255, 0, 0, 255};
    static inline Colour blue = {0, 0, 255, 255};
    static inline Colour green = {0, 255, 0, 255};
    static inline Colour white = {255, 255, 255, 255};
    static inline Colour black = {0, 0, 0, 255};
    static inline Colour grey = {128, 128, 128, 255};
    static inline Colour yellow = {255, 255, 0, 255};
    static inline Colour magenta = {255, 0, 255, 255};
    static inline Colour cyan = {0, 255, 255, 255};
    static inline Colour orange = {255, 132, 0, 255};
};

enum RenderMode {
    simpleRenderer,  // just the window renderer using simple shapes
    softwareRendering,  // uses surfaces and the window surface to blit with the CPU. Does not use renderer at all
    hardwareRendering,  // uses textures and window renderer to render with the GPU.
};

class Window {
    public:
        static inline const int screenWidth = 720;
        static inline const int screenHeight = 1280;
        std::string title = "";

// MARK: -- SETUP AND SHUTDOWN -------------------------------------------------------

        Window(RenderMode renderMode=RenderMode::simpleRenderer) {
            // NOTE: Either use the renderer for drawing to the screen and using textures
            // !OR! use the window screen surface to blit image surfaces. Can't use both!!
            this->renderMode = renderMode;
            bool failure = false;

            // 0 indicates success
            if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
                std::cerr << "Error initializing SDL.\n";  // out to standard error stream
                failure = true;
            }

            // create window
            if (!failure) {
                window = SDL_CreateWindow(
                    NULL,  // pass NULL for no title (like when using flag SDL_WINDOW_BORDERLESS
                    SDL_WINDOWPOS_CENTERED,  // otherwise provide x coord (pos on screen window appears)
                    SDL_WINDOWPOS_CENTERED,  // otherwise provide y coord
                    screenWidth,  // width px
                    screenHeight,  // height px
                    SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE  // window flags (i.e. SDL_WINDOW_BORDERLESS)
                );
                if (!window) {
                    std::cerr << "Error creating SDL window.\n";
                    failure = true;
                }
            }

            // window screen surface for software rendering (surface based)
            if (!failure && renderMode == RenderMode::softwareRendering) {
			    screenSurface = SDL_GetWindowSurface(window);
                if (!screenSurface) {
                    std::cerr << "Error getting window screen surface.\n";
                    failure = true;
                }
            }

            // create renderer
            if (!failure && (renderMode == RenderMode::simpleRenderer || renderMode == RenderMode::hardwareRendering)) {
                renderer = SDL_CreateRenderer(
                    window, 
                    -1, 
                    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
                if (!renderer) {
                    std::cerr << "Error creating SDL renderer.\n";
                    failure = true;
                }
                clear();
            }

            // init sdl_image (for texture based hardware rendering)
            if (!failure && renderMode == RenderMode::hardwareRendering) {
                if(!IMG_Init(IMG_INIT_PNG)) {
                    std::cerr << "SDL_image could not initialize! SDL_image Error.\n";
                    failure = true;
                }
                screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, screenWidth, screenHeight);
            }
        }

        void close() {
            // free all textures
            for (int i = 0; i < allocatedTextures.size(); i++) {
                if (allocatedTextures[i] != NULL) {
                    SDL_DestroyTexture(allocatedTextures[i]);
                }
            }

            // free all surfaces
            for (int i = 0; i < allocatedSurfaces.size(); i++) {
                if (allocatedSurfaces[i] != NULL) {
                    SDL_FreeSurface(allocatedSurfaces[i]);
                }
            }

            // remove in REVERSE order to creation :)
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            IMG_Quit();
            SDL_Quit();
        }

// MARK: -- LOAD MEDIA ---------------------------------------------------------------

        // Requires relative path from project root
        SDL_Surface* loadSurface(std::string path) {
            SDL_Surface* newSurface = IMG_Load(path.c_str());
            if (newSurface == NULL) {
                std::cerr << "Error loading image file at " << path << '\n' << SDL_GetError() << '\n';
            }
            allocatedSurfaces.push_back(newSurface);
            return newSurface;
        }

        // Requires relative path from project root
        SDL_Texture* loadTexture(std::string path) {
            SDL_Texture* newTexture = NULL;

            SDL_Surface* loadedSurface = IMG_Load(path.c_str());
            if (loadedSurface == NULL) {
                std::cerr << "Unable to load image: " << path.c_str() << '\n' << SDL_GetError() << '\n';
            
            } else {
                newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
                if (newTexture == NULL)
                {
                    std::cerr << "Unable to create texture of image " << path << " SDL_image Error.\n";
                }
                SDL_FreeSurface(loadedSurface);
                allocatedTextures.push_back(newTexture);
            }

            return newTexture;
        }

// MARK: -- RENDERING ----------------------------------------------------------------

        void clear() {
            for (int i = 0; i < screenHeight * screenWidth; i++) {
                pixels[i] = Colours::black.ARGB8888();
            }
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        }

        // -- Simple Rendering --

        void renderRect(int x, int y, int w, int h, int r, int g, int b, int a) {
            SDL_Rect rect = {x, y, w, h};
            SDL_SetRenderDrawColor(renderer, r, g, b, a);
            SDL_RenderFillRect(renderer, &rect);
        }

        void renderRect(const struct SDL_Rect& rect, const Colour& colour=Colours::white) {
            SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
            SDL_RenderFillRect(renderer, &rect);
        }

        void renderCircle(const int& centreX, const int& centreY, int radius, const Colour& colour=Colours::white) {
            SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
            const int diameter = radius * 2;
            int x = (radius - 1);
            int y = 0;
            int tx = 1;
            int ty = 1;
            int error = (tx - diameter);
            while (x >= y) {
                //  Each of the following renders an octant of the circle
                SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
                SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
                SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
                SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
                SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
                SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
                SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
                SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

                // center, x and y axis

                if (error <= 0) {
                    ++y;
                    error += ty;
                    ty += 2;
                }

                if (error > 0) {
                    --x;
                    tx += 2;
                    error += (tx - diameter);
                }
            }
        }

        void renderCircle(const Point2D& point, const int& radius, const Colour& colour=Colours::white) {
            renderCircle(point.x(), point.y(), radius, colour);
        }

        void renderCircle(const struct SDL_Rect& rect, const Colour& colour=Colours::white) {
            int radius = std::min(rect.w, rect.h);
            Point2D center = Point2D(rect.x + rect.w/2, rect.y + rect.h/2);
            renderCircle(center, radius, colour);
        }

        // https://stackoverflow.com/questions/1201200/fast-algorithm-for-drawing-filled-circles
        void renderFilledCircle(const int& centreX, const int& centreY, int radius, const Colour& colour=Colours::white) {
            SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);

            // 12 fps
            for (int y = -radius; y <= radius; y++) {
                for (int x = -radius; x <= radius; x++) {
                    // x - radius and y - radius offsets built into loops
                    if(x*x + y*y <= radius * radius) {
                        SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
                    }
                }
            }

            // 12 - 13 fps
            // for (int x = -radius; x < radius ; x++) {
            //     int hh = (int)std::sqrt(radius * radius - x * x);
            //     int rx = centreX + x;
            //     int ph = centreY + hh;

            //     for (int y = centreY-hh; y < ph; y++) {
            //         SDL_RenderDrawPoint(renderer, rx, y);
            //     }
            // }

            // 11 - 12 fps
            // int r2 = radius * radius;
            // int area = r2 << 2;
            // int rr = radius << 1;

            // for (int i = 0; i < area; i++)
            // {
            //     int tx = (i % rr) - radius;
            //     int ty = (i / rr) - radius;

            //     if (tx * tx + ty * ty <= r2)
            //         SDL_RenderDrawPoint(renderer, centreX + tx, centreY + ty);
            // }
        }

        void renderFilledCircle(const Point2D& point, const int& radius, const Colour& colour=Colours::white) {
            renderFilledCircle(point.x(), point.y(), radius, colour);
        }

        void renderFilledCircle(const struct SDL_Rect& rect, const Colour& colour=Colours::white) {
            int radius = std::min(rect.w, rect.h);
            Point2D center = Point2D(rect.x + rect.w/2, rect.y + rect.h/2);
            renderFilledCircle(center, radius, colour);
        }

        void renderLine(int x1, int y1, int x2, int y2, int r, int g, int b, int a) {
            SDL_SetRenderDrawColor(renderer, r, g, b, a);
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }

        void renderLine(const Point2D& p1, const Point2D& p2, const Colour& colour=Colours::white) {
            SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
            SDL_RenderDrawLine(renderer, p1.x(), p1.y(), p2.x(), p2.y());
        }

        void renderPoint(int x, int y, Colour& colour=Colours::white) {
            SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
            SDL_RenderDrawPoint(renderer, x, y);
        }

        void renderPoint(const Point2D& p, Colour& colour=Colours::white) {
            renderPoint(p.x(), p.y(), colour);
        }

        // -- Software Rendering --

        void renderSurface(SDL_Surface* pSurface, Point2D topLeft) {
            SDL_Rect destinationRect = {(int) topLeft.x(), (int) topLeft.y(), pSurface->w, pSurface->h};
            SDL_BlitSurface(pSurface, NULL, screenSurface, &destinationRect);
        }

        void renderSurface(SDL_Surface* pSurface) {
            SDL_BlitSurface(pSurface, NULL, screenSurface, NULL);
        }

        void renderMaskedSurface(SDL_Surface* pSurface, Point2D topLeft, SDL_Rect sourceMask) {
            SDL_Rect destinationRect = {(int) topLeft.x(), (int) topLeft.y(), pSurface->w, pSurface->h};
            SDL_BlitSurface(pSurface, &sourceMask, screenSurface, &destinationRect);
        }

        void renderScaledSurface(SDL_Surface* pSurface, Point2D topLeft, double scaleX, double scaleY) {
            SDL_Rect scaleRect = {(int) topLeft.x(), (int) topLeft.y(), (int) (pSurface->w * scaleX), (int) (pSurface->h * scaleY)};
            SDL_BlitScaled(pSurface, NULL, screenSurface, &scaleRect);
        }

        void renderScaledSurface(SDL_Surface* pSurface, SDL_Rect scaleAndPositionRect) {
            SDL_BlitScaled(pSurface, NULL, screenSurface, &scaleAndPositionRect);
        }

        void renderSurfaceMaskedAndScaled(SDL_Surface* pSurface, SDL_Rect sourceMask, SDL_Rect scaleAndPositionRect) {
            SDL_BlitScaled(pSurface, &sourceMask, screenSurface, &scaleAndPositionRect);
        }

        void renderSurfaceFillScreen(SDL_Surface* pSurface) {
            SDL_BlitScaled(pSurface, NULL, screenSurface, NULL);
        }

        // -- Hardware Rendering --

        // - texture -

        void renderTexture(SDL_Texture* pTexture, SDL_Rect sourceMask, SDL_Rect destinationArea) {
            SDL_RenderCopy(renderer, pTexture, &sourceMask, &destinationArea);
        }

        void renderTexture(SDL_Texture* pTexture, SDL_Rect destinationArea) {
            SDL_RenderCopy(renderer, pTexture, NULL, &destinationArea);
        }

        void renderTextureFillScreen(SDL_Texture* pTexture) {
            SDL_RenderCopy(renderer, pTexture, NULL, NULL);
        }

        // - pixel buffer -
        // https://gigi.nullneuron.net/gigilabs/sdl2-pixel-drawing/

        void renderPixel(int x, int y, Uint32 colour) {
            if (x < screenWidth && y < screenHeight && x >= 0 && y >= 0) {
                pixels[y * screenWidth + x] = colour;
            }
        }

        void renderPixel(int x, int y, Colour colour) {
            renderPixel(x, y, colour.ARGB8888());
        }

        void renderPixel(Point2D pixel, Colour colour) {
            renderPixel(pixel.x(), pixel.y(), colour);
        }

        // -- General --

        void setTitle(std::string title) {
            this->title = title;
            SDL_SetWindowTitle(window, this->title.c_str());
        }

        void presentRender() {
            // render by renderer
            if (renderMode == RenderMode::simpleRenderer || renderMode == RenderMode::hardwareRendering) {
                if (renderMode == RenderMode::hardwareRendering) {
                    SDL_UpdateTexture(screenTexture, NULL, pixels, screenWidth * sizeof(Uint32));
                    renderTextureFillScreen(screenTexture);
                }
                SDL_RenderPresent(renderer);
            // render by window surface
            } else {
                SDL_UpdateWindowSurface(window);
            }

        }

    private:
        RenderMode renderMode;
        SDL_Window* window = NULL;
        SDL_Renderer* renderer = NULL;
        SDL_Surface* screenSurface = NULL;
        SDL_Texture * screenTexture = NULL;  // used for per pixel modification and rendering
        Uint32 pixels[screenHeight * screenWidth];  // pixel buffer that is then used to update texture
        std::vector<SDL_Texture*> allocatedTextures = {screenTexture};  // vector of textures for deallocation on close
        std::vector<SDL_Surface*> allocatedSurfaces = {screenSurface};  // vector of surface for deallocation on close
};