#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "../include/SDL2/SDL.h"
#include "../include/SDL2/SDL_image.h"
#include "window.hpp"
#include "point.hpp"
#include "random.hpp"

class Ball {
    public:
        Ball(Random random, Point2D position) {
            this->random = random;
            id = nextId;
            nextId++;

            radius = random.between(5, 21);
            pos = position;
            vel = Point2D(random.between(-10, 10), 0);
        }

        void update(const int& sceneWidth, const int& sceneHeight, std::vector<Ball>& balls) {
            // y vel set to gravity every frame (later removed by collisions)
            vel.set(vel.x(), vel.y() + gravity);

            Point2D newPos = pos + vel;

            SceneCollision(sceneWidth, sceneHeight, newPos);
            BallCollision(balls, newPos);

            // damp velocity
            if (abs(vel.x()) < 0.01) {
                vel.set(0.0, vel.y());
            } else {
                vel.set(vel.x() * drag, vel.y());
            }
            if (vel.y() < gravity && vel.y() > -gravity * 3) {  // magic number to prevent infinite bouncing
                vel.set(vel.x(), 0.0);
            }

            pos.set(newPos);
        }

        void SceneCollision(const int& sceneWidth, const int& sceneHeight, Point2D& newPos) {
            if (newPos.x() - radius < 0) {
                newPos.set(radius, (int)newPos.y());
                vel.set(-vel.x() * bounce, vel.y());
            }
            if (newPos.x() + radius >= sceneWidth) {
                newPos.set(sceneWidth - 1 - radius, (int)newPos.y());
                vel.set(-vel.x() * bounce, vel.y());
            }
            if (newPos.y() - radius < 0) {
                newPos.set((int)newPos.x(), radius);
                vel.set(vel.x(), -vel.y() * bounce);
            }
            if (newPos.y() + radius >= sceneHeight) {
                newPos.set((int)newPos.x(), sceneHeight - 1 - radius);
                vel.set(vel.x(), -vel.y() * bounce);
            }
        }

        void BallCollision(std::vector<Ball>& balls, Point2D& newPos) {
            for (Ball& ball : balls) {
                if (&ball != this) {
                    
                }
            }
        }

        void draw(Window& window) {
            window.renderFilledCircle(pos, radius, colour);
        }

    private:
        Random random;
        static int nextId;

        int id;
        int radius;
        Colour colour = Colours::white;

        Point2D pos;
        Point2D vel;
        double gravity = 0.2;
        double bounce = 0.7;
        double drag = 0.995;

};

int Ball::nextId = 0;