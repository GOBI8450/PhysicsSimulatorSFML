#include <SFML/Graphics.hpp>;
#include <SFML/Window.hpp>
#include <sstream>
#include <iostream> using namespace std;
#include "CircleBase.h"
#include "LineLink.h"
#include "ObjectsList.h"
#include "Grid.h"
#include <random>  // For random number generation
#include <ctime>   // For seeding with current time



std::vector<sf::Color> generateGradient(sf::Color startColor, sf::Color endColor, int steps) {
    std::vector<sf::Color> gradient;

    // Calculate the difference between start and end colors
    float stepR = (endColor.r - startColor.r) / (float)(steps - 1);
    float stepG = (endColor.g - startColor.g) / (float)(steps - 1);
    float stepB = (endColor.b - startColor.b) / (float)(steps - 1);

    for (int i = 0; i < steps; ++i) {
        sf::Color col;
        col.r = startColor.r + stepR * i;
        col.g = startColor.g + stepG * i;
        col.b = startColor.b + stepB * i;
        gradient.push_back(col);
    }
    return gradient;
}

int main()
{
    //Configaration
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    int window_height = 920;
    int window_width = 1280;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "TomySim", sf::Style::Default, settings);
    float gridSize = 20;
    ObjectsList objectList = ObjectsList();

    sf::Clock clock;
    sf::Clock fpsClock;
    int frameCount = 0;
    float currentFPS = 0.0f;
    // Target frame time in seconds
    const float targetFrameTime = 1.0f / 60.0f; // 60 FPS
    // Text setup for displaying FPS
    sf::Font font;
    if (!font.loadFromFile("font.ttf")) { // Ensure you have a valid font file
        return -1;
    }
    sf::Text ballsCountText;
    sf::Text fpsText;
    fpsText.setFont(font);
    fpsText.setCharacterSize(24);
    fpsText.setFillColor(sf::Color::White);
    ballsCountText.setFont(font);
    ballsCountText.setCharacterSize(24);
    ballsCountText.setFillColor(sf::Color::White);


    //Variables
    float deltaTime = 1 / 60;
    float elastic = 0;
    int ballsCount = 0;
    float radius = 50;
    float gravity = 9.8;
    double mass = 1;
    bool scaleFlag = false;
    sf::Color ball_color = sf::Color(238, 238, 238);
    sf::Color ball_color2 = sf::Color(50, 5, 11);
    sf::Color bg_color = sf::Color(30, 30, 30);
    sf::Color bb = sf::Color(44, 55, 100);
    sf::Color explosion = sf::Color(205, 92, 8);
    sf::Color startColor = sf::Color(128, 0, 128);//purple
    sf::Color endColor = sf::Color(0, 0, 255);  //blue
    short int gradientStep = 0;
    short int gradientStepMax = 400;
    std::vector<sf::Color> gradient = generateGradient(startColor,endColor,gradientStepMax);
    sf::Vector2f window_centerP = sf::Vector2f(window_height / 2, window_width / 2);
    //random config
// Initialize random number generator with a seed based on the current time
    std::mt19937 rnd(static_cast<unsigned>(std::time(nullptr)));  // // Initialize random number generator with a seed based on the current time
    // Define the range [1, 100] for the random number
    std::uniform_int_distribution<int> rnd_range(radius, 920-radius);
    sf::Vector2f otherStartP = sf::Vector2f(rnd_range(rnd), rnd_range(rnd));
    float posYStartingPoint = 200;
    int posXStartingPoint = radius;
    short int startingPointAdder = 10;
    sf::Vector2f spawnStartingPoint = sf::Vector2f(posXStartingPoint, posYStartingPoint);

    sf::Vector2i previousMousePos = sf::Mouse::getPosition(window);
    sf::Vector2i mousePos;
    bool mouseFlagClick = false;
    bool mouseFlagScrollUp = false;
    bool mouseFlagScrollDown = false;
    int mouseScrollPower = 5;
    CircleBase* thisBallPointer=nullptr;


    //Config Ball
    int disLine = 30;
    //LineLink lineLink=LineLink(disLine);


    /*for (size_t i = 0; i < 15; i++)
    {
        lineLink.NewBall(gravity, ball_color);
    }*/

    while (window.isOpen())
    {
        sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosFloat = static_cast<sf::Vector2f>(currentMousePos);

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) { window.close(); }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    std::cout << "Escape key pressed!" << std::endl;
                    window.close(); // Close the window
                }
                if (event.key.code == sf::Keyboard::A) {
                    for (size_t i = 0; i < 10; i++)
                    {
                        objectList.CreateNewCircle(gravity, gradient[gradientStep],spawnStartingPoint);
                        ballsCount += 1;
                        gradientStep += 1;
                        spawnStartingPoint.x += startingPointAdder;
                        if (gradientStep == gradientStepMax) {
                            std::reverse(gradient.begin(), gradient.end());
                            gradientStep = 0;
                        }
                        if (spawnStartingPoint.x>=window_width-radius||spawnStartingPoint.x<=radius)
                        {
                            startingPointAdder *= -1;
                        }
                    }
                    //Linking shit@#$@@#$@#@##$$@#
                    //lineLink.NewBall(ballsList.ballsList[ballsList.ballsList.size()-1]);
                }

                if (event.key.code == sf::Keyboard::H) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    for (size_t i = 0; i < 3; i++)
                    {
                        objectList.CreateNewCircle(gravity, gradient[gradientStep], mousePosFloat);
                        ballsCount += 1;
                    }
                }
                if (event.key.code == sf::Keyboard::F) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    objectList.CreateNewCircle(gravity, explosion, sf::Vector2f(mousePosFloat.x+3,mousePosFloat.y+3));
                    for (size_t i = 0; i < 50; i++)
                    {
                        objectList.CreateNewCircle(gravity, explosion, mousePosFloat);
                        ballsCount += 1;
                    }
                }
                if (event.key.code == sf::Keyboard::R) {
                    objectList.DeleteAll();
                    ballsCount = 0;
                }

                if (mouseFlagClick&&event.key.code == sf::Keyboard::S)
                {
                    scaleFlag = true;
                }
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                mouseFlagClick = false;
                scaleFlag = false;
            }
            if (event.type==sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    if (event.mouseWheelScroll.delta > 0) {
                        mouseFlagScrollUp = true;
                    }
                    else
                    {
                        mouseFlagScrollDown = true;
                    }

                }
            }

        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)&&mouseFlagClick==false) {
            thisBallPointer = objectList.IsInRadius(currentMousePos); // Check if a circle is within radius

            if (thisBallPointer != nullptr) { // Check if a circle was found
                mouseFlagClick = true; // Set flag if circle found
            }
        }

        if (mouseFlagClick) { // Check if a circle is found
            sf::Vector2f mousePosFloat = static_cast<sf::Vector2f>(currentMousePos); // Use current mouse position
            thisBallPointer->setPosition(mousePosFloat); // Set position of the found ball
            if (scaleFlag&&mouseFlagScrollUp||mouseFlagScrollDown) {
                if (mouseFlagScrollDown&& thisBallPointer->getRadius()>0.0001) {
                    thisBallPointer->SetRadiusAndCenter(thisBallPointer->getRadius() - mouseScrollPower);
                    thisBallPointer->SetMass(thisBallPointer->GetMass() - mouseScrollPower*10);
                    mouseFlagScrollDown = false;
                }
                else {
                    thisBallPointer->SetRadiusAndCenter(thisBallPointer->getRadius() + mouseScrollPower);
                    thisBallPointer->SetMass(thisBallPointer->GetMass() + mouseScrollPower*10);
                    mouseFlagScrollUp = false;
                }
            }
        }

        // Calculate the mouse movement (speed)

        // Calculate FPS
        frameCount++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            currentFPS = frameCount;
            frameCount = 0;
            fpsClock.restart();
        }

        window.clear(bg_color);
        //lineLink.MakeLinks(window,60);
        objectList.MoveAndDraw(window, currentFPS, elastic);

        // Update the FPS text
        std::ostringstream oss;
        std::ostringstream ossCount;
        oss << "FPS: " << static_cast<int>(currentFPS);
        oss << "Balls Count: " << static_cast<int>(ballsCount);
        fpsText.setString(oss.str());
        window.draw(fpsText);
        ballsCountText.setString(ossCount.str());
        window.draw(ballsCountText);
        //lineLink.Draw(window);

        window.display();

        // Update the previous mouse position
        previousMousePos = currentMousePos;

        // Limit to 60 FPS
        sf::Time elapsed = clock.restart();
        if (elapsed.asSeconds() < targetFrameTime) {
            sf::sleep(sf::seconds(targetFrameTime - elapsed.asSeconds()));
        }
    }

    return 0;
}
