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
        Scene() {
            width = 800;
            height = 500;
            numBalls = 30;
            ballFrameInterval = 30;
        }

        Scene(const int& sceneWidth, const int& sceneHeight, const int& sceneNumBalls, const int& sceneBallFrameInterval) {
            width = sceneWidth;
            height = sceneHeight;
            numBalls = sceneNumBalls;
            ballFrameInterval = sceneBallFrameInterval;
        }

        void update(double dt) {
            // -- input --
            //getInput();

            // -- update --
            if (frameTimer % ballFrameInterval == 0 && balls.size() < numBalls) {
                balls.push_back(Ball(random, Point2D(width / 2, 5)));
            }
            for (Ball& ball : balls) {
                ball.update(width, height, balls);
            }
            frameTimer++;
        }

        void draw(Window& window) {
            for (Ball& ball : balls) {
                ball.draw(window);
            }

            window.renderLine(Point2D(0, 0), Point2D(width, 0));
            window.renderLine(Point2D(0, 0), Point2D(0, height));
            window.renderLine(Point2D(width, 0), Point2D(width, height));
            window.renderLine(Point2D(0, height), Point2D(width, height));
        }

    private:
        Random random;
        int width;
        int height;
        int numBalls;
        int ballFrameInterval;
        int frameTimer = 0;
        std::vector<Ball> balls;
    
        // void getInput() {
        //     std::unordered_map<SDL_Keycode, bool> keydowns = Input::getKeydowns();

        //     if (keydowns[SDLK_1]) {
        //         drawMode2D = !drawMode2D;
        //     }
        // }

};
