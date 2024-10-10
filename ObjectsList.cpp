#include <vector>
#include <random>
#include "Grid.h"
#include "CircleBase.h"
#include "Rectangle.h"
#include "Planet.h"
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
    std::vector<Planet*> planetList;

public:
    std::vector<BaseShape*> ballsList;  // Changed to BaseShape* to store any shapes that derive from BaseShape

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

    void CreateNewCircle(float gravity, sf::Color color, sf::Vector2f pos) {
        std::uniform_int_distribution<int> radiusRange(15, 15);
        std::uniform_int_distribution<int> rndXRange(300, 500);  // Replace 920 with actual window width
        // std::uniform_int_distribution<int> rndYRange(50, 1280 - 50); // Replace 1280 with actual window height

        sf::Vector2f position(pos);
        int randomRadius = radiusRange(rnd);
        int mass = randomRadius*3;//no real meaning for the multiply
        BaseShape* ball = new Circle(randomRadius, color, position, gravity, mass);
        ballsList.push_back(ball); // Pushing back the BaseShape* into the vector
        ballCount += 1;

        // std::cout << "Creating ball at position: (" << position.x << ", " << position.y << ")\n";
    }

    void CreateNewPlanet(float innerGravity, sf::Color color, sf::Vector2f pos, float radius, float mass) {
        std::uniform_int_distribution<int> rndXRange(300, 500);
        float gravity = 0;
        Planet* planet = new Planet(radius, color, pos, gravity, mass, innerGravity);        //^^^^^^float radius, sf::Color color, sf::Vector2f pos, float gravity, double mass, float innerGravity^^^^
        Planet* planetCopy = new Planet(*planet);  // Correct way to create a copy        //^^^^^^float radius, sf::Color color, sf::Vector2f pos, float gravity, double mass, float innerGravity^^^^
        ballsList.push_back(planet); // Pushing back the BaseShape* into the vector
        planetList.push_back(planetCopy); // Pushing back the BaseShape* into the vector
        ballCount += 1;
    }

    void CreateNewRectangle(float gravity, sf::Color color, sf::Vector2f pos) {
        std::uniform_int_distribution<int> heightRange(50, 50);
        std::uniform_int_distribution<int> widthRange(50, 50);
        std::uniform_int_distribution<int> rndXRange(300, 500);  // Replace 920 with actual window width
        // std::uniform_int_distribution<int> rndYRange(50, 1280 - 50); // Replace 1280 with actual window height
        int randomHeight=heightRange(rnd);
        int randomWidth=widthRange(rnd);
        int mass = (randomWidth + randomHeight) * 2;//no real meaning for the multiply
        sf::Vector2f position(pos);

        BaseShape* ball = new Rectangle(randomWidth, randomHeight, color, position, gravity, mass);
        ballsList.push_back(ball); // Pushing back the BaseShape* into the vector
        ballCount += 1;

        // std::cout << "Creating ball at position: (" << position.x << ", " << position.y << ")\n";
    }

    void HandleCollisionsInRange(sf::RenderWindow& window, float elastic, std::vector<std::vector<BaseShape*>> vecOfVecObj) {
        if (elastic == 0) { // Verlet integration
            for (auto& vecObj : vecOfVecObj) {
                for (auto& obj : vecObj) {
                    // Check if obj is a Circle
                    if (Circle* circle = dynamic_cast<Circle*>(obj)) {
                        circle->handleWallCollision(window);
                    }
                    // Check if obj is a Rectangle
                    else if (Rectangle* rectangle = dynamic_cast<Rectangle*>(obj)) {
                        rectangle->handleWallCollision(window);
                    }

                    // Get nearby objects for collision handling
                    std::vector<BaseShape*> potentialCollisions = grid.GetNerbyCellsObjects(obj);

                    for (auto& otherObj : potentialCollisions) {
                        // Handle Circle to Circle collision
                        if (Circle* circle = dynamic_cast<Circle*>(obj)) {
                            if (Circle* otherCircle = dynamic_cast<Circle*>(otherObj)) {
                                if (circle != otherCircle) {
                                    circle->HandleCollision(otherCircle);
                                }
                            }
                        }
                        // Handle Rectangle to other object collision
                        else if (Rectangle* rectangle = dynamic_cast<Rectangle*>(obj)) {
                            if (Rectangle* otherRectangle = dynamic_cast<Rectangle*>(otherObj))
                            {
                                if (rectangle != otherRectangle) {
                                    rectangle->HandleCollision(otherRectangle); // Handle collision with any other shape
                                }
                            } 
                        }
                       if (Rectangle* rectangle = dynamic_cast<Rectangle*>(obj)) {
                            if (Circle* otherCircle = dynamic_cast<Circle*>(otherObj))
                            {
                                rectangle->HandleCollision(otherCircle); // Handle collision with any other shape
                            }
                            std::cout << "recCir";
                       }
                       else if (Circle* otherCircle = dynamic_cast<Circle*>(otherObj)) {
                           if (Rectangle* rectangle = dynamic_cast<Rectangle*>(obj))
                           {
                               rectangle->HandleCollision(otherCircle); // Handle collision with any other shape
                           }
                           std::cout << "recCir";
                       }
                    }
                }
            }
        }
        else { // Euler integration
            for (auto& vecObj : vecOfVecObj) {
                for (auto& obj : vecObj) {
                    // Check if obj is a Circle
                    if (Circle* circle = dynamic_cast<Circle*>(obj)) {
                        circle->handleWallCollision(window);
                    }
                    // Check if obj is a Rectangle
                    else if (Rectangle* rectangle = dynamic_cast<Rectangle*>(obj)) {
                        rectangle->handleWallCollision(window);
                    }

                    // Get nearby objects for collision handling
                    std::vector<BaseShape*> potentialCollisions = grid.GetNerbyCellsObjects(obj);

                    for (auto& otherObj : potentialCollisions) {
                        // Handle Circle to Circle collision
                        if (Circle* circle = dynamic_cast<Circle*>(obj)) {
                            if (Circle* otherCircle = dynamic_cast<Circle*>(otherObj)) {
                                if (circle != otherCircle) {
                                    circle->HandleCollisionElastic(otherCircle,elastic);
                                }
                            }
                        }
                        // Handle Rectangle to other object collision
                        else if (Rectangle* rectangle = dynamic_cast<Rectangle*>(obj)) {
                            if (Rectangle* otherRectangle = dynamic_cast<Rectangle*>(otherObj))
                            {
                                if (rectangle != otherRectangle) {
                                    rectangle->HandleCollisionElastic(otherRectangle,elastic); // Handle collision with any other shape
                                }
                            }

                        }
                    }
                }
            }
        }
    }

    void HandleAllCollisions(sf::RenderWindow& window, float elastic) {
        if (elastic == 0) { // Verlet integration
            for (auto& obj : ballsList) {
                // Check if obj is a Circle
                if (Circle* circle = dynamic_cast<Circle*>(obj)) {
                    circle->handleWallCollision(window);
                }
                // Check if obj is a Rectangle
                else if (Rectangle* rectangle = dynamic_cast<Rectangle*>(obj)) {
                    rectangle->handleWallCollision(window);
                }

                // Get nearby objects for collision handling
                std::vector<BaseShape*> potentialCollisions = grid.GetNerbyCellsObjects(obj);

                for (auto& otherObj : potentialCollisions) {
                    // Handle Circle to Circle collision
                    if (Circle* circle = dynamic_cast<Circle*>(obj)) {
                        if (Circle* otherCircle = dynamic_cast<Circle*>(otherObj)) {
                            if (circle != otherCircle) {
                                circle->HandleCollision(otherCircle);
                            }
                        }
                    }
                    // Handle Rectangle to other object collision
                    else if (Rectangle* rectangle = dynamic_cast<Rectangle*>(obj)) {
                        if (Rectangle* otherRectangle = dynamic_cast<Rectangle*>(otherObj))
                        {
                            if (rectangle != otherRectangle) {
                                rectangle->HandleCollision(otherRectangle); // Handle collision with any other shape
                            }
                        }
                    }
                    if (Rectangle* rectangle = dynamic_cast<Rectangle*>(obj)) {
                        if (Circle* otherCircle = dynamic_cast<Circle*>(otherObj))
                        {
                            rectangle->HandleCollision(otherCircle); // Handle collision with any other shape
                        }
                    }
                    else if (Circle* otherCircle = dynamic_cast<Circle*>(otherObj)) {
                        if (Rectangle* rectangle = dynamic_cast<Rectangle*>(obj))
                        {
                            rectangle->HandleCollision(otherCircle); // Handle collision with any other shape
                        }
                    }
                }
            }
        }
        else { // Euler integration
            for (auto& obj : ballsList) {
                // Check if obj is a Circle
                if (Circle* circle = dynamic_cast<Circle*>(obj)) {
                    circle->handleWallCollision(window);
                }
                // Check if obj is a Rectangle
                else if (Rectangle* rectangle = dynamic_cast<Rectangle*>(obj)) {
                    rectangle->handleWallCollision(window);
                }

                // Get nearby objects for collision handling
                std::vector<BaseShape*> potentialCollisions = grid.GetNerbyCellsObjects(obj);

                for (auto& otherObj : potentialCollisions) {
                    // Handle Circle to Circle collision
                    if (Circle* circle = dynamic_cast<Circle*>(obj)) {
                        if (Circle* otherCircle = dynamic_cast<Circle*>(otherObj)) {
                            if (circle != otherCircle) {
                                circle->HandleCollisionElastic(otherCircle, elastic);
                            }
                        }
                    }
                    // Handle Rectangle to other object collision
                    else if (Rectangle* rectangle = dynamic_cast<Rectangle*>(obj)) {
                        if (Rectangle* otherRectangle = dynamic_cast<Rectangle*>(otherObj))
                        {
                            if (rectangle != otherRectangle) {
                                rectangle->HandleCollisionElastic(otherRectangle,elastic); // Handle collision with any other shape
                            }
                        }

                    }
                }
            }
        }
    }

    BaseShape* IsInRadius(sf::Vector2i pointPos) {
        return grid.IsInGridRadius(pointPos); // Return nullptr if no ball contains the point
    }

    void handlePlanetGravity(sf::RenderWindow& window) {
        
    }

    void MoveAndDraw(sf::RenderWindow& window, float fps, float elastic) {
        grid.clear(); // Clear the grid

        for (auto& ball : ballsList) {
            grid.InsertObj(ball); // Inserting BaseShape* objects
        }

        if (fps <= 0) {
            fps = 60;
        }
        float deltaTime = 1 / fps; // Calculate deltaTime for movement
        HandleAllCollisions(window, elastic); // Handle all the collisions
        for (auto& planet : planetList)
        {
            for (auto& ball : ballsList) {
                std::cout << "balls";
                planet->Gravitate(ball);
            }
        }
        for (auto& ball : ballsList) {
            ball->updatePosition(deltaTime);
            ball->draw(window);
        }
    }
};
