#pragma once
#include <random> 
#include "Grid.h"
#include "CircleBase.h";
#include <iostream> using namespace std;
#include <thread>
#include <functional>


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

    void HandleCollisionsInRange(sf::RenderWindow& window, float elastic, std::vector<CircleBase*> vecObj) {
        if (elastic==0)
        {
            for (auto& ball : vecObj) {
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
        MultiThreadChecks(window, elastic);
    }

    void MultiThreadChecks(sf::RenderWindow& window, float elastic) { //Get position negative problem
        int hashMapSize = grid.GetHashMapSize();
        int threadsNum = 4;//How much threads will be
        if (hashMapSize/threadsNum<1)
        {
            threadsNum = hashMapSize;
        }
        std::vector<std::thread> threadsVec;
        int loopCount = 0;
        for (size_t i = 0; i < threadsNum; i++)
        {
            std::vector<CircleBase*> objVec;
            size_t inner = static_cast<rsize_t>(hashMapSize / threadsNum);
            for (size_t j = 0; j < inner; j++)
            {
                objVec.push_back(ballsList[loopCount]);
                loopCount+=1;
            }
            threadsVec.push_back(std::thread(std::bind(&ObjectsList::HandleCollisionsInRange, this, std::ref(window), elastic, std::ref(objVec))));
        }
        std::vector<CircleBase*> objVec;
        for (size_t i = 0; i < hashMapSize%threadsNum; i++)//all the remaining ones
        {
            objVec.push_back(ballsList[loopCount]);
            loopCount += 1;
        }
        threadsVec.push_back(std::thread(std::bind(&ObjectsList::HandleCollisionsInRange, this, std::ref(window), elastic, std::ref(objVec))));
        for (size_t i = 0; i < threadsVec.size(); i++)
        {
            threadsVec[i].join();
        }
    }

};

