#pragma once
#include <random>
#include "Grid.h"
#include "CircleBase.h"
#include <iostream>
#include <thread>
#include <functional>
#include <atomic>
#include "ThreadPool.h" // Include the ThreadPool header


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

    ~ObjectsList() { // Deleter or else memory leak ):
        DeleteAll();
    }

    void DeleteAll() {
        for (auto ball : ballsList) {
            delete ball;
        }
        ballsList.clear();
        ballCount = 0;
    }

    void CreateNewCircle(float gravity, sf::Color color,sf::Vector2f pos) {
        std::uniform_int_distribution<int> radiusRange(5, 5); 
        std::uniform_int_distribution<int> massRange(1, 1);
        std::uniform_int_distribution<int> rndXRange(300, 500);  // Replace 920 with actual window width
        //std::uniform_int_distribution<int> rndYRange(50, 1280 - 50); // Replace 1280 with actual window height
        sf::Vector2f position(pos);

        CircleBase* ball = new CircleBase(radiusRange(rnd), color, position, gravity, massRange(rnd));
        ballsList.push_back(ball);
        ballCount += 1;
        //std::cout << "Creating ball at position: (" << position.x << ", " << position.y << ")\n";
    }

    //Gets a vector of -> circleBase Objects vectors, and handles the collisions in them
    void HandleCollisionsInRange(sf::RenderWindow& window, float elastic, std::vector<std::vector<CircleBase*>> vecOfVecObj) {
        if (elastic==0)//Varlet integration
        {
            for (auto& vecObj:vecOfVecObj)
            {
                for (auto& ball : vecObj) {
                    ball->handleWallCollision(window);
                    std::vector<CircleBase*> potentialCollisions = grid.GetNerbyCellsObjects(ball);
                    for (auto& otherBall : potentialCollisions) {
                        if (ball != otherBall) {
                            for (size_t i = 0; i < 1; i++)
                            {
                                ball->handleCollision(otherBall);
                            }
                        }
                    }
                }
            } 
        }
        else { //Euler integration
            for (auto& vecObj : vecOfVecObj)
            {
                for (auto& ball : vecObj) {
                    ball->handleWallCollision(window);
                    std::vector<CircleBase*> potentialCollisions = grid.GetNerbyCellsObjects(ball);
                    for (auto& otherBall : potentialCollisions) {
                        if (&ball != &otherBall) {
                            ball->handleCollisionElastic(otherBall, elastic);
                        }
                    }
                }
            }
        }

    }
    void HandleAllCollisions(sf::RenderWindow& window, float elastic) {
        if (elastic == 0)
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
                        ball->handleCollisionElastic(otherBall, elastic);
                    }
                }
            }
        }
    }


    //void MultiThreadChecks(sf::RenderWindow& window, float elastic) { //Get position negative problem
    //    int hashMapSize = grid.GetHashMapSize();
    //    std::vector<int> allHashKeys = grid.GetAllHashKeys();
    //    size_t threadsNum = std::thread::hardware_concurrency();//How much threads will be
    //    size_t eachThreadNeedTo = hashMapSize / threadsNum; // like if there is 10 grids, 5 threads -> each thread will handle 2.
    //    if (eachThreadNeedTo < 1)
    //    {
    //        threadsNum = hashMapSize;
    //    }
    //    std::vector<std::thread> threadsVec;
    //    int loopCount = 0;
    //    for (size_t i = 0; i < threadsNum; i++)
    //    {
    //        std::vector<int> currentKeysVec;
    //        size_t inner = static_cast<size_t>(hashMapSize / threadsNum);
    //        for (size_t j = 0; j < inner; j++)
    //        {
    //            if (loopCount <= hashMapSize)
    //            {
    //                currentKeysVec.push_back(allHashKeys[loopCount]);
    //                loopCount += 1;
    //            }
    //        }
    //        std::vector<std::vector<CircleBase*>> currentCircleVector = grid.GetCircelsVectorOfVectorsFromKeyVectors(currentKeysVec);
    //        std::thread tempThread (&ObjectsList::HandleCollisionsInRange,std::ref (window), elastic, currentCircleVector);
    //        threadsVec.push_back(tempThread);
    //    }
    //    std::vector<int> currentKeysVec;
    //    for (size_t i = 0; i < hashMapSize % threadsNum; i++)//all the remaining ones
    //    {
    //        if (loopCount <= hashMapSize)
    //        {
    //            currentKeysVec.push_back(allHashKeys[loopCount]);
    //            loopCount += 1;
    //        }
    //    }
    //    std::vector<std::vector<CircleBase*>> currentCircleVector = grid.GetCircelsVectorOfVectorsFromKeyVectors(currentKeysVec);
    //    HandleCollisionsInRange(window, elastic, currentCircleVector);
    //    for (size_t i = 0; i < threadsVec.size(); i++)
    //    {
    //        threadsVec[i].join();
    //    }
    //}

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
        HandleAllCollisions(window, elastic);
        for (auto& ball : ballsList)
        {

            ball->updatePosition(deltaTime);  // Ensure deltaTime is reasonable
            ball->draw(window);
        }
    }

};

