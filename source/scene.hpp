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
        Scene(const int& sceneWidth, const int& sceneHeight, 
            const int& sceneNumBalls, const int& sceneBallFrameInterval,
            SDL_Surface* revealSurface) {
            width = sceneWidth;
            height = sceneHeight;
            numBalls = sceneNumBalls;
            ballFrameInterval = sceneBallFrameInterval;
            revealImg = revealSurface;

            int yMax = std::ceil(height / batchGridSize);
            int xMax = std::ceil(width / batchGridSize);
            for (int y = 0; y < yMax; y++) {
                for (int x = 0; x < xMax; x++) {
                    batches[Point2D(x, y)] = {};
                }
            }
        }

        void update(double dt) {
            // -- input --
            //getInput();

            // -- update --
            if (frameTimer < endFrame) {
                // add new ball
                if (frameTimer % ballFrameInterval == 0 && simBalls.size() < numBalls) {
                    simBalls.push_back(Ball(random, Point2D(0, 0)));//width / 2, 5)));
                }
                updateBalls(simBalls);

            } else if (frameTimer == endFrame) {
                // assign ball colours

            } else if (frameTimer <= endFrame * 2 - 1) {
                // add ball from simBalls to displayBalls, but reset first
                if (displayBalls.size() != numBalls && (frameTimer - endFrame) % ballFrameInterval == 0) {
                    simBalls[displayBalls.size()].reset();
                    simBalls[displayBalls.size()].setColour(Colours::red);
                    displayBalls.push_back(simBalls[displayBalls.size()]);
                }
                updateBalls(displayBalls);
            }

            frameTimer++;
        }

        void updateBalls(std::vector<Ball>& balls) {
            // requied for update no matter the phase of simulation
            batchBalls(balls);
            for (Ball& ball : balls) {
                ball.update(width, height, balls);
            }
        }

        void batchBalls(std::vector<Ball>& balls) {
            // clear batches
            for (auto it = batches.begin(); it != batches.end(); it++) {
                batches[it->first] = {};
            }

            for (Ball& ball : balls) {
                Point2D cell = Point2D((int) (ball.x() / batchGridSize),
                                      (int) (ball.y() / batchGridSize));
                batches[cell].push_back(ball);
            }
        }

        void draw(Window& window) {
            // decide which set of balls needs to be rendered based on sim phase
            std::vector<Ball> balls;
            if (displayBalls.size() == 0) {
                balls = simBalls;
            } else {
                balls = displayBalls;
            }

            for (Ball& ball : balls) {
                ball.draw(window);
            }

            window.renderLine(Point2D(0, 0), Point2D(width, 0));
            window.renderLine(Point2D(0, 0), Point2D(0, height));
            window.renderLine(Point2D(width, 0), Point2D(width, height));
            window.renderLine(Point2D(0, height), Point2D(width, height));

            //drawBatchGrid(window);
            
        }

        void drawBatchGrid(Window& window) {
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
        int width;
        int height;
        SDL_Surface* revealImg;
        int endFrame = 900;

        int numBalls;
        int ballFrameInterval;
        int frameTimer = 0;
        std::vector<Ball> simBalls;
        std::vector<Ball> displayBalls;

        int batchGridSize = 50;
        std::unordered_map<Point2D, std::vector<Ball>, PointHasher> batches;
    
        // void getInput() {
        //     std::unordered_map<SDL_Keycode, bool> keydowns = Input::getKeydowns();

        //     if (keydowns[SDLK_1]) {
        //         drawMode2D = !drawMode2D;
        //     }
        // }

};
