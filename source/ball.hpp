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
            vel = Point2D(random.between(-50, 50), 0);
            mass = radius;
        }

        void update(const int& sceneWidth, const int& sceneHeight, std::vector<Ball>& balls) {
            // y vel set to gravity every frame (later removed by collisions)
            vel.set(vel.x(), vel.y() + gravity);
            AirResistance();

            Point2D newPos = pos + vel;

            BallCollision(balls, newPos);
            SceneCollision(sceneWidth, sceneHeight, newPos);

            pos.set(newPos);
        }
        
        // https://physicshub.github.io/simulations/BouncingBall  <-- Incredible resource
        void AirResistance() {
            // Air resistance proportional to velocity squared
            double speed = sqrt(vel.x() * vel.x() + vel.y() * vel.y());
            double dragMagnitude = dragCoefficient * speed * speed;
            // Direction opposite to velocity
            vel = vel + Point2D(-dragMagnitude * vel.x() / speed, -dragMagnitude * vel.x() / speed);
        }

        void SceneCollision(const int& sceneWidth, const int& sceneHeight, Point2D& newPos) {
            if (newPos.x() - radius < 0) {
                newPos.set(radius, (int)newPos.y());
                vel.set(-vel.x() * bounceDampening, vel.y());
            }
            else if (newPos.x() + radius >= sceneWidth) {
                newPos.set(sceneWidth - 1 - radius, (int)newPos.y());
                vel.set(-vel.x() * bounceDampening, vel.y());
            }
            if (newPos.y() - radius < 0) {
                newPos.set((int)newPos.x(), radius);
                vel.set(vel.x(), -vel.y() * bounceDampening);
            }
            else if (newPos.y() + radius >= sceneHeight) {
                newPos.set((int)newPos.x(), sceneHeight - 1 - radius);
                vel.set(vel.x(), -vel.y() * bounceDampening);
            }
        }

        void BallCollision(std::vector<Ball>& balls, Point2D& newPos) {
            for (Ball& ball : balls) {
                Point2D directionVector = Point2D(pos.x() - ball.pos.x(), pos.y() - ball.pos.y());
                double distanceSqrd = directionVector.x() * directionVector.x() + directionVector.y() * directionVector.y();
                double combinedRadiiSqrd = (radius + ball.radius) * (radius + ball.radius);
                // ball not self and is overlapping
                if (&ball != this && distanceSqrd < combinedRadiiSqrd) {
                    // https://gamedev.stackexchange.com/questions/121693/simulating-elastic-ball-collisions-quickly-escalates-to-disaster
                    // https://codepen.io/anon/pen/QNXObe?editors=1010
                    Point2D combinedVelocityVec = Point2D(ball.vel.x() - vel.x(), ball.vel.y() - vel.y());
                    double dotProduct = combinedVelocityVec.x() * directionVector.x() + combinedVelocityVec.y() * directionVector.y();
                    // 1. if dot product of two vectors is negative, then the angle between the vectors is greater 
                    //    than 90 degrees;
                    // 
                    // 2. if the velocity vector is pointing away from the direction of the collision, then the 
                    //    balls are moving away from each other;
                    //
                    // 3. if the balls are moving away from each other, then they are not colliding.
                    if (dotProduct >= 0) {
                        double totalMass = mass + ball.mass;
                        double massFactorOne = 2 * ball.mass / totalMass;
                        double massFactorTwo = 2 * mass / totalMass;
                        double scaledMomentum = dotProduct / distanceSqrd;
                        double scalarOne = massFactorOne * scaledMomentum;
                        double scalarTwo = massFactorTwo * scaledMomentum;
                        vel.set(vel.x() + (scalarOne * directionVector.x()),
                                vel.y() + (scalarOne * directionVector.y()));
                        ball.vel.set(ball.vel.x() - (scalarTwo * directionVector.x()),
                                     ball.vel.y() - (scalarTwo * directionVector.y()));
                    }
                }
            }
        }

        void draw(Window& window) {
            window.renderCircle(pos, radius, colour);
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
        double gravity = 1;
        double bounceDampening = 0.5;
        double dragCoefficient = 0.001;
};

int Ball::nextId = 0;