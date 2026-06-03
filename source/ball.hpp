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

            radius = 20;
            pos = position;
            startPos = Point2D(pos.x(), pos.y());
            vel = Point2D(0, 10);
            startVel = Point2D(vel.x(), vel.y());
            mass = radius;
        }

        double x() {
            return pos.x();
        }

        double y() {
            return pos.y();
        }

        int getRadius() {
            return radius;
        }

        int ID() {
            return id;
        }

        void reset() {
            pos = startPos;
            vel = startVel;
            hasReset = true;
        }

        void setColour(const Colour& col) {
            colour = col;
        }

        void update(const int& sceneWidth, const int& sceneHeight, std::unordered_map<int, std::vector<Ball>>& batches, const int batchGridSize) {
            // y vel set to gravity every frame (later removed by collisions)
            vel.set(vel.x(), vel.y() + gravity);
            Friction();
            pos.set(pos + vel);
            for (int i = 0; i < physicsRepeatsPerFrame; i++) {
                Collision(sceneWidth, sceneHeight, batches, batchGridSize);
            }
        }
        
        // https://physicshub.github.io/simulations/BouncingBall  <-- Incredible resource
        void Friction() {
            // Friction/air resistance proportional to velocity squared
            double speed = sqrt(vel.x() * vel.x() + vel.y() * vel.y());
            double frictionMagnitude = frictionCoefficient * speed * speed;
            // Direction opposite to velocity
            vel = vel + Point2D(-frictionMagnitude * vel.x() / speed, -frictionMagnitude * vel.x() / speed);
        }

        void Collision(const int& sceneWidth, const int& sceneHeight, std::unordered_map<int, std::vector<Ball>>& batches, const int batchGridSize) {
            BallCollision(sceneWidth, sceneHeight, batches, batchGridSize);
            SceneCollision(sceneWidth, sceneHeight);
        }

        void SceneCollision(const int& sceneWidth, const int& sceneHeight) {
            if (pos.x() - radius < 0) {
                pos.set(radius, (int)pos.y());
                vel.set(-vel.x() * bounceDampening, vel.y());
            }
            else if (pos.x() + radius >= sceneWidth) {
                pos.set(sceneWidth - 1 - radius, (int)pos.y());
                vel.set(-vel.x() * bounceDampening, vel.y());
            }
            if (pos.y() - radius < 0) {
                pos.set((int)pos.x(), radius);
                vel.set(vel.x(), -vel.y() * bounceDampening);
            }
            else if (pos.y() + radius >= sceneHeight) {
                pos.set((int)pos.x(), sceneHeight - 1 - radius);
                vel.set(vel.x(), -vel.y() * bounceDampening);
            }
        }

        // https://www.youtube.com/watch?v=LPzyNOHY3A4
        void BallCollision(const int& sceneWidth, const int& sceneHeight, std::unordered_map<int, std::vector<Ball>>& batches, const int batchGridSize) {
            // see scene::batchBalls comment for more informaiton about batching system
            // doesn't use Point2D neighbours as too expensive with instantiation and deletion of point instances
            
            // determine ball batch cell
            int ballKey = (int) (pos.x() / batchGridSize) * 1000 + (int) (pos.y() / batchGridSize);
            std::vector<std::pair<int, int>> neighbourOffsets = {{-1000, -1}, {0, -1}, {1000, -1}, {-1000, 0}, {1000, 0}, {-1000, 1}, {0, 1}, {1000, 1}, {0, 0}};
            
            // for all the neighbour cells and this cell, collide with balls
            for (std::pair<int, int> offset : neighbourOffsets) {
                int neighbourKey = ballKey + offset.first + offset.second;
                for (Ball& ball : batches[neighbourKey]) {
                    // not self
                    if (ball.id != id) {
                        double distance = pos.getDistance(ball.pos);
                        // colliding
                        if (distance < radius + ball.radius) {
                            // from this ball to other ball normalised
                            Point2D normalVector = Point2D((ball.pos.x() - pos.x()) / distance, (ball.pos.y() - pos.y()) / distance);
                            double overlap = radius + ball.radius - distance;

                            // -- static collision --
                            // pop balls forcibly out of each other
                            pos.set(pos.x() + overlap/2 * -normalVector.x(), 
                                    pos.y() + overlap/2 * -normalVector.y());
                            ball.pos.set(ball.pos.x() + overlap/2 * normalVector.x(),
                                        ball.pos.y() + overlap/2 * normalVector.y());
                            
                            ball.SceneCollision(sceneWidth, sceneHeight);
                            
                            // -- dynamic collision
                            // take care of vels
                            // updated normal now balls have been popped out
                            distance = pos.getDistance(ball.pos);
                            normalVector = Point2D((ball.pos.x() - pos.x()) / distance, (ball.pos.y() - pos.y()) / distance);
                            Point2D tangentVector = Point2D(-normalVector.y(), normalVector.x());

                            // tangent response (vel dot product with tangent)
                            // amount of vel aligned with tangent, so then mult tangent and dp scalar to get reaction along tangent
                            double dpTangentA = vel.dotProduct(tangentVector);
                            double dpTangentB = ball.vel.dotProduct(tangentVector);

                            // normal response (vel dot product with normal)
                            // same as tangent but along collision normal
                            double dpNormA = vel.dotProduct(normalVector);
                            double dpNormB = ball.vel.dotProduct(normalVector);

                            // conservation of momentum 1D
                            // (used to scale normal response based on mass)
                            double m1 = (dpNormA * (mass - ball.mass) + 2.0 * ball.mass * dpNormB) / (mass + ball.mass);
                            double m2 = (dpNormB * (ball.mass - mass) + 2.0 * mass * dpNormA) / (mass + ball.mass);

                            // set vel as (tangent * tangent response scalar) + (normal * conservation of mass scalar)
                            vel.set((tangentVector.x() * dpTangentA + normalVector.x() * m1) * reboundDampening,
                                    (tangentVector.y() * dpTangentA + normalVector.y() * m1) * reboundDampening);
                            ball.vel.set((tangentVector.x() * dpTangentB + normalVector.x() * m2) * reboundDampening, 
                                        (tangentVector.y() * dpTangentB + normalVector.y() * m2) * reboundDampening);
                        }
                    }
                }
            }
        }

        void draw(Window& window) {
            if (!hasReset) {
                window.renderCircle(pos, radius, colour);
            } else {
                window.renderFilledCircle(pos.x(), pos.y(), radius, colour);
            }
        }

    private:
        Random random;
        static int nextId;

        int id;
        int radius;
        double mass;
        Colour colour = Colours::white;
        bool hasReset = false;

        Point2D pos;
        Point2D startPos;
        Point2D vel;
        Point2D startVel;
        int physicsRepeatsPerFrame = 40;
        double gravity = 1;
        double reboundDampening = 0.997;
        double bounceDampening = 0.5;
        double frictionCoefficient = 0.001;
};

int Ball::nextId = 0;