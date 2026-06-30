#pragma once

#include <iostream>
#include <functional>
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>

#include "random.hpp"
#include "utilities.hpp"
#include "window.hpp"
#include "ray.hpp"
#include "point.hpp"
#include "input.hpp"
#include "ball.hpp"
#include "../include/SDL2/SDL.h"


class Scene {
    public:
        bool displayCompleteSimulation = false;

        Scene(const int& sceneNumBalls, const int& sceneBallSize, const int& sceneBallFrameInterval, const int& frameDuration, SDL_Surface* revealSurface) {
            width = revealSurface->w;
            height = revealSurface->h;
            numBalls = sceneNumBalls;
            ballSize = sceneBallSize;
            ballFrameInterval = sceneBallFrameInterval;
            endFrame = frameDuration;
            revealImg = revealSurface;

            int yMax = std::ceil(height / batchGridSize);
            int xMax = std::ceil(width / batchGridSize);
            for (int y = 0; y < yMax; y++) {
                for (int x = 0; x < xMax; x++) {
                    batches[x * 1000 + y] = {};
                }
            }
        }

        bool getRun() {
            return run;
        }

        void update(double dt) {
            // -- input --
            //getInput();

            // -- update --
            // - sim -
            if (frameTimer < endFrame) {
                updateSpawner();
                // add new ball
                if (frameTimer % ballFrameInterval == 0 && simBalls.size() < numBalls) {
                    simBalls.push_back(Ball(random, spawner, ballSize));
                }
                updateBalls(simBalls);

            // - colours -
            } else if (frameTimer == endFrame) {
                colourBalls();

            // - display -
            } else if (frameTimer <= endFrame * 2 - 1) {
                displayCompleteSimulation = true;
                // add ball from simBalls to displayBalls, but reset first
                if (displayBalls.size() != numBalls && (frameTimer - endFrame) % ballFrameInterval == 0) {
                    simBalls[displayBalls.size()].reset();
                    displayBalls.push_back(simBalls[displayBalls.size()]);
                }
                updateBalls(displayBalls);
            } else {
                run = false;
            }

            frameTimer++;
        }

        void updateSpawner() {
            spawner = spawner + Point2D(spawnerSpeed * spawnerDirection, 0);
            if ((spawnerDirection > 0 && spawner.x() >= revealImg->w - spawnerMargin) ||
                (spawnerDirection < 0 && spawner.x() <= spawnerMargin)) {
                    spawnerDirection *= -1;
            }
        }

        void updateBalls(std::vector<Ball>& balls) {
            // requied for update no matter the phase of simulation
            batchBalls(balls);
            for (Ball& ball : balls) {
                ball.update(width, height, batches, batchGridSize);
            }
        }

        // batches have keys consisting of int representing x and y values added together
        // to avoid duplicates like 2x + 3y = 2y + 3x = 5, multiply x by 1000 (should never be 1000 cells on x)
        // creates unique integer keys
        void batchBalls(std::vector<Ball>& balls) {
            // clear batches
            for (auto it = batches.begin(); it != batches.end(); it++) {
                batches[it->first] = {};
            }

            for (Ball& ball : balls) {
                int key = (int) (ball.x() / batchGridSize) * 1000 + (int) (ball.y() / batchGridSize);
                batches[key].push_back(ball);
            }
        }

        void colourBalls() {
            Uint32* revealPixels = (Uint32*)revealImg->pixels;

            for (Ball& ball : simBalls) {
                int r = 0;
                int g = 0;
                int b = 0;
                int radius = ball.getRadius();
                int numPixels = 0;

                for (int y = ball.y() - radius; y <= ball.y() + radius; y++) {
                    for (int x = ball.x() - radius; x <= ball.x() + radius; x++) {
                        Uint32 pixelColour = *(revealPixels + y * revealImg->pitch/4 + x);
                        r += (int)((Uint8)(pixelColour >> 16));
                        g += (int)((Uint8)(pixelColour >> 8));
                        b += (int)((Uint8)pixelColour);
                        numPixels++;
                    }
                }

                ball.setColour(Colour(r / numPixels, g / numPixels, b / numPixels, 255));
            }
        }

        void draw(Window& window) {

            if (frameTimer < endFrame) {
                for (Ball& ball : simBalls) {
                    ball.draw(window);
                }
            }

            for (Ball& ball : displayBalls) {
                ball.draw(window);
            }

            //drawBatchGrid(window);
        }

        void drawBatchGrid(Window& window) {
            window.renderLine(Point2D(0, 0), Point2D(width, 0));
            window.renderLine(Point2D(0, 0), Point2D(0, height));
            window.renderLine(Point2D(width-1, 0), Point2D(width-1, height-1));
            window.renderLine(Point2D(0, height-1), Point2D(width-1, height-1));

            for (int y = 0; y < std::ceil(height/batchGridSize); y++) {
                for (int x = 0; x < std::ceil(width/batchGridSize); x++) {
                    window.renderLine(Point2D(0, y * batchGridSize), Point2D(width, y * batchGridSize));
                }
            }
            for (int x = 0; x < std::ceil(width/batchGridSize); x++) {
                for (int y = 0; y < std::ceil(height/batchGridSize); y++) {
                    window.renderLine(Point2D(x * batchGridSize, 0), Point2D(x * batchGridSize, height));
                }
            }
        }

    private:
        Random random;
        bool run = true;
        int width;
        int height;
        SDL_Surface* revealImg;
        int endFrame;

        int numBalls;
        int ballSize;
        int ballFrameInterval;
        int frameTimer = 0;
        std::vector<Ball> simBalls;
        std::vector<Ball> displayBalls;
        
        int spawnerMargin = 50;
        int spawnerDirection = 1;
        int spawnerSpeed = 8;
        Point2D spawner = Point2D(0 + spawnerMargin, 0);

        int batchGridSize = 50;
        std::unordered_map<int, std::vector<Ball>> batches;
    
        // void getInput() {
        //     std::unordered_map<SDL_Keycode, bool> keydowns = Input::getKeydowns();

        //     if (keydowns[SDLK_1]) {
        //         drawMode2D = !drawMode2D;
        //     }
        // }

};
