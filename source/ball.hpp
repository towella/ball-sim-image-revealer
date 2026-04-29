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
            vel = Point2D(random.between(-10, 10), random.between(-10, 10));
            mass = radius;
        }

        void update(const int& sceneWidth, const int& sceneHeight, std::vector<Ball>& balls, Window& window) {
            // y vel set to gravity every frame (later removed by collisions)
            vel.set(vel.x(), vel.y() + gravity);

            Point2D newPos = pos + vel;

            BallCollision(balls, newPos, window);
            SceneCollision(sceneWidth, sceneHeight, newPos);

            // clamp velocity and apply drag
            if (abs(vel.x()) < 0.01) {
                vel.set(0.0, vel.y());
            } else {
                vel.set(vel.x() * drag, vel.y());
            }
            if (vel.y() < gravity && vel.y() > -gravity * 3) {  // magic number to prevent infinite bouncing
                vel.set(vel.x(), 0.0);
            } else {
                vel.set(vel.x(), vel.y() * drag);
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

        // https://ericleong.me/research/circle-circle/
        void BallCollision(std::vector<Ball>& balls, Point2D& newPos) {
            for (Ball& ball : balls) {
                double distanceSqrd = newPos.getDistanceNoSqrt(ball.pos);
                double combinedRadiiSqrd = (radius + ball.radius) * (radius + ball.radius);
                // ball not self and is overlapping
                if (&ball != this && distanceSqrd < combinedRadiiSqrd) {

                    // this ball collides with other ball 
                    double pushBack = sqrt(combinedRadiiSqrd - distanceSqrd);
                    double angle = newPos.getAngleToRad(pos);  // angle to push point back along move vector
                    newPos.set(ball.pos.x() + sin(angle) * pushBack,
                               ball.pos.y() + cos(angle) * pushBack);

                    // this ball has velocity change from collision
                    double distance = newPos.getDistance(ball.pos);
                    Point2D normVecBallToSelf = Point2D((ball.pos.x() - pos.x()) / distance,
                                                        (ball.pos.y() - pos.y()) / distance);
                    double p = 2.0 * (double)(vel.x() * normVecBallToSelf.x() + vel.y() * normVecBallToSelf.y() - ball.vel.x() * normVecBallToSelf.x() - ball.vel.y() * normVecBallToSelf.y()) / (double)(mass + ball.mass);
                    vel.set(vel.x() - p * mass * normVecBallToSelf.x(),
                            vel.y() - p * mass * normVecBallToSelf.y());
                    ball.vel.set(ball.vel.x() + p * ball.mass * normVecBallToSelf.x(),
                                 ball.vel.y() + p * ball.mass * normVecBallToSelf.y());
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
        double mass;
        Colour colour = Colours::white;

        Point2D pos;
        Point2D vel;
        double gravity = 0.2;
        double bounce = 0.7;
        double drag = 0.995;

};

int Ball::nextId = 0;