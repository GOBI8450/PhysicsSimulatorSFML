#pragma once
#include <random> 
#include "Grid.h"
#include "CircleBase.h";
#include <iostream> using namespace std;
#include <thread>


class ObjectsList
{
private:
    int ballCount = 0;
    std::mt19937 rnd;
    Grid grid;

public:
    std::vector<CircleBase*> ballsList;

    ObjectsList() { // Adjust cell size as needed
        rnd.seed(static_cast<unsigned>(std::time(nullptr)));
    }
    ~ObjectsList() {
        for (auto ball : ballsList) {
            delete ball;
        }
    }
    
    void DeleteAll() {
        for (size_t i = 0; i < ballsList.size(); i++)
        {
            ballsList.clear();
        }
    }

    void CreateNewCircle(float gravity, sf::Color color,sf::Vector2f pos) {
        std::uniform_int_distribution<int> radiusRange(2, 2); 
        std::uniform_int_distribution<int> massRange(1, 1);
        std::uniform_int_distribution<int> rndXRange(300, 500);  // Replace 920 with actual window width
        //std::uniform_int_distribution<int> rndYRange(50, 1280 - 50); // Replace 1280 with actual window height
        sf::Vector2f position(pos);

        CircleBase* ball = new CircleBase(radiusRange(rnd), color, position, gravity, massRange(rnd));
        ballsList.push_back(ball);
        ballCount += 1;
        //std::cout << "Creating ball at position: (" << position.x << ", " << position.y << ")\n";
    }

    void HandleAllCollisions(sf::RenderWindow& window, float elastic) {
        if (elastic==0)
        {
            for (auto& ball : ballsList) {
                for (size_t i = 0; i < 2; i++)
                {
                    ball->handleWallCollision(window);
                }
                std::vector<CircleBase*> potentialCollisions = grid.GetNerbyCellsObjects(ball);
                for (auto& otherBall : potentialCollisions) {
                    if (&ball != &otherBall) {
                        for (size_t i = 0; i < 1; i++)
                        {
                            ball->handleCollision(otherBall);
                        }
                    }
                }
            }
        }
        else {
            for (auto& ball : ballsList) {
                ball->handleWallCollision(window);
                std::vector<CircleBase*> potentialCollisions = grid.GetNerbyCellsObjects(ball);
                for (auto& otherBall : potentialCollisions) {
                    if (&ball != &otherBall) {
                        ball->handleCollisionElastic(otherBall,elastic);
                    }
                }
            }
        }

    }


    CircleBase* IsInRadius(sf::Vector2i pointPos) {
        return grid.IsInGridRadius(pointPos); // Return nullptr if no ball contains the point
    }

    void MoveAndDraw(sf::RenderWindow& window, float fps, float elastic)
    {
        grid.clear();
        for (auto& ball : ballsList) {
            grid.InsertCircle(ball);
        }
        if (fps <= 0) {
            fps = 60;
        }  // 60 FPS if the frame rate is undefined
        float deltaTime = 1 / fps;

        for (auto& ball : ballsList)
        {

            ball->updatePosition(deltaTime);  // Ensure deltaTime is reasonable
            ball->draw(window);
        }
        for (size_t i = 0; i < 1; i++)
        {
            HandleAllCollisions(window,elastic);
        }
    }

    void MultiThreadChecks(sf::RenderWindow& window, float elastic) {
        int size = grid.GetHashMapSize();
        std::vector<std::thread> threadsVec(size);
        int threadsNum = 8;//How much threads will be
        
        for (size_t i = 0; i < threadsNum; i++)
        {
            for (size_t j = 0; j < size/threadsNum; j++)
            {
                threadsVec[j] = std::thread(HandleAllCollisions,window,elastic);
            }
        }
        for (size_t i = 0; i < size%threadsNum; i++)//all the remaining ones
        {
            threadsVec[i] = std::thread(HandleAllCollisions, window, elastic);
        }

        for (size_t i = 0; i < size; i++)
        {
            threadsVec[i].join();
        }
    }

};

