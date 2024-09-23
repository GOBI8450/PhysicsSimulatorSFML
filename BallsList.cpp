#pragma once
#include <SFML/Graphics.hpp>;
#include <iostream> using namespace std;
#include <vector>;
#include "CircleBase.h";
#include <random>;  // For random number generation
#include <ctime>;   // For seeding with current time


class BallsList
{
private:
    int ballCount = 0;
    std::mt19937 rnd;//random variable

public:
    std::vector<CircleBase> ballsList;//New empty list of circles

    BallsList() {
        rnd.seed(static_cast<unsigned>(std::time(nullptr)));//for random better
    }
    void NewBall(float radius, sf::Color color, sf::Vector2f pos, float gravity, double mass)
    {
        CircleBase ball(radius, color, pos, gravity, mass);
        ballCount += 1;
        ballsList.push_back(ball);
    }

    void NewBall(const CircleBase& ball) {
        ballsList.push_back(ball);
        ballCount += 1;
    }

    void NewBall(float gravity,sf::Color color) {
        std::uniform_int_distribution<int> radiusRange(10, 10);
        std::uniform_int_distribution<int> massRange(1, 1);
        std::uniform_int_distribution<int> rndXRange(50, 1280 - 50);  // Replace 920 with actual window width
        //std::uniform_int_distribution<int> rndYRange(50, 1280 - 50); // Replace 1280 with actual window height

        sf::Vector2f position(rndXRange(rnd), 200);

        CircleBase ball = CircleBase(radiusRange(rnd), color, position, gravity, massRange(rnd));
        ballsList.push_back(ball);
        ballCount += 1;
        //std::cout << "Creating ball at position: (" << position.x << ", " << position.y << ")\n";
    }

    //bool isFarEnough(const sf::Vector2f& position, float minDistance) {
    //    for (const auto& ball : ballsList) {
    //        if (std::sqrt(std::pow(position.x - ball.getPosition().x, 2) + std::pow(position.y - ball.getPosition().y, 2)) < minDistance) {
    //            return false; // Too close to another ball
    //        }
    //    }
    //    return true;
    //}




    std::vector<CircleBase> CombineBallsLists(const std::vector<CircleBase>& otherBallsList) {
        std::vector<CircleBase> newList = ballsList;
        newList.reserve(ballsList.size() + otherBallsList.size()); // preallocate memory
        newList.insert(newList.end(), otherBallsList.begin(), otherBallsList.end());
        return newList;  // Return the combined list
    }

    void HandleCollisonsInList(sf::RenderWindow& window) {
        const size_t size = ballsList.size();
        for (size_t i = 0; i < size; ++i) {
            ballsList[i].handleWallCollision(window);
            for (size_t j = i + 1; j < size; ++j) {
                ballsList[i].handleCollision(ballsList[j]);
            }
        }
    }
    void HandleCollisonsInListElastic(sf::RenderWindow & window, float elastic) {
        const size_t size = ballsList.size();
        for (size_t i = 0; i < size; ++i) {
            ballsList[i].handleWallCollision(window);
            for (size_t j = i + 1; j < size; ++j) {
                ballsList[i].handleCollisionElastic(ballsList[j],elastic);
            }
        }
    }
 

    void MoveAndDraw(sf::RenderWindow& window, float fps,float elastic)
    {
        if (fps <= 0) {
            fps = 60;
        }  // 60 FPS if the frame rate is undefined
        float deltaTime = 1 / fps;
        for (auto& ball : ballsList)
        {
            
            ball.updatePosition(deltaTime);  // Ensure deltaTime is reasonable
            ball.draw(window);
        }
        if (elastic == 0) {
            for (size_t i = 0; i < 2; i++)
            {
                HandleCollisonsInList(window);
            }
        }
        else {
            if (elastic == 0) {
                for (size_t i = 0; i < 1; i++)
                {
                    HandleCollisonsInListElastic(window, elastic);
                }
            }
        }
    }
    
    CircleBase* IsInRadius(sf::Vector2i pointPos) {
        for (auto& ball : ballsList) {
            sf::Vector2f ballPos = ball.getPosition();
            double distance = std::sqrt(std::pow(ballPos.x - pointPos.x, 2) + std::pow(ballPos.y - pointPos.y, 2));

            if (distance <= ball.getRadius()) {
                return &ball; // Return a pointer to the ball if the point is within the radius
            }
        }
        return nullptr; // Return nullptr if no ball contains the point
    }


};
