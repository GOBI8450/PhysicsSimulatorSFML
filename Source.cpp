#include <SFML/Graphics.hpp>;
#include <SFML/Window.hpp>
#include <sstream>
#include <iostream> using namespace std;
#include "LineLink.h"
#include "ObjectsList.h"
#include "Grid.h"
#include "Button.h"
#include "Rectangle.h"
#include <random>  // For random number generation
#include <ctime>   // For seeding with current time


bool hovering = false;

struct {
    sf::VideoMode desktopSize = sf::VideoMode::getDesktopMode();
    int window_height = desktopSize.height;
    int window_width = desktopSize.width;
    bool fullscreen = false;
    float gravity =9.8;
    double massLock = 0;
} options;

std::string Settings(sf::RenderWindow& window, sf::Color background_color, sf::RectangleShape headerText, std::vector<std::pair< Button, bool>> buttonVec) {
    window.setTitle("Settings");

    sf::Vector2u windowSize = window.getSize();
    sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosFloat = static_cast<sf::Vector2f>(currentMousePos);
    bool mouseClickFlag = false;
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed) { window.close(); }
        if (event.type == sf::Event::MouseButtonReleased) { mouseClickFlag = true; }
    }
    window.clear(background_color);
    if (mouseClickFlag)
    {
        for (auto& button : buttonVec)
        {
            if (button.first.IsInRadius(mousePosFloat))
            {
                //for better clicking experience
                /*button.first.SetScale(1.5);
                button.first.draw(window);
                button.first.SetScale(2/3);*/
                return button.first.GetName();
            }
        }
    }
    for (auto& button : buttonVec)
    {
        button.second = button.first.MouseHover(mousePosFloat, hovering);
    }
    window.draw(headerText);
    for (auto& button : buttonVec)
    {
        button.first.draw(window);
    }
    window.display();
    return "SETTINGS";
}

std::string MainMenu(sf::RenderWindow& window, sf::Color background_color, sf::RectangleShape headerText, std::vector<std::pair< Button, bool>> buttonVec) {
    window.setTitle("Main Menu");

    sf::Vector2u windowSize = window.getSize();
    sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosFloat = static_cast<sf::Vector2f>(currentMousePos);
    bool mouseClickFlag = false;
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed) { window.close(); }
        if (event.type == sf::Event::MouseButtonReleased) { mouseClickFlag = true; }
    }
    window.clear(background_color);
    if (mouseClickFlag)
    {
        for (auto& pair : buttonVec)
        {
            if (pair.first.IsInRadius(mousePosFloat))
            {
                return pair.first.GetName();
            }
        }
    }
    for (auto& button : buttonVec)
    {
        button.second = button.first.MouseHover(mousePosFloat, hovering);
    }
    window.draw(headerText);
    for (auto& button : buttonVec)
    {

        button.first.draw(window);
    }

    window.display();
    return "MAIN MENU";
}

std::vector<sf::Color> GenerateGradient(sf::Color startColor, sf::Color endColor, int steps) {
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
    sf::View view;
    const float ZOOM_FACTOR = 1.1f;
    unsigned int window_height = options.window_height;
    unsigned int window_width = options.window_width;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "TomySim", sf::Style::Default, settings);
    view = window.getDefaultView();
    sf::Cursor handCursor;
    sf::Cursor defaultCursor;
    if (!defaultCursor.loadFromSystem(sf::Cursor::Arrow)) {
        return -1; // Error loading default cursor
    }

    // Load the wait cursor from the system
    if (!handCursor.loadFromSystem(sf::Cursor::Hand)) {
        return -1; // Error loading hand cursor
    }
    window.setMouseCursor(defaultCursor);
    float gridSize = 20;
    float lineLength = 150;
    ObjectsList objectList = ObjectsList(lineLength);
    window.setFramerateLimit(60);
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
    int objCount = 0;
    float radius = 50;
    float gravity = options.gravity;
    double massLock = options.massLock;
    bool scaleFlag = false;
    sf::Color ball_color = sf::Color(238, 238, 238);
    sf::Color ball_color2 = sf::Color(50, 5, 11);
    sf::Color background_color = sf::Color(30, 30, 30);
    sf::Color mainMenuBackgroundColor = sf::Color(25, 25, 25);
    sf::Color buttonColor = sf::Color(55, 58, 64);
    sf::Color bb = sf::Color(44, 55, 100);
    sf::Color explosion = sf::Color(205, 92, 8);
    sf::Color startColor = sf::Color(128, 0, 128);//purple
    sf::Color endColor = sf::Color(0, 0, 255);  //blue
    short int gradientStep = 0;
    short int gradientStepMax = 400;
    std::vector<sf::Color> gradient = GenerateGradient(startColor, endColor, gradientStepMax);
    sf::Vector2f window_centerP = sf::Vector2f(window_height / 2, window_width / 2);
    //random config
    // Initialize random number generator with a seed based on the current time
    std::mt19937 rnd(static_cast<unsigned>(std::time(nullptr)));  // // Initialize random number generator with a seed based on the current time
    // Define the range [1, 100] for the random number
    std::uniform_int_distribution<int> rnd_range(radius, 920 - radius);
    sf::Vector2f otherStartP = sf::Vector2f(rnd_range(rnd), rnd_range(rnd));
    float posYStartingPoint = 200;
    int posXStartingPoint = radius;
    short int startingPointAdder = 31;
    sf::Vector2f spawnStartingPoint = sf::Vector2f(posXStartingPoint, posYStartingPoint);

    sf::Vector2i mousePos;
    bool mouseFlagClick = false;
    bool mouseFlagScrollUp = false;
    bool mouseFlagScrollDown = false;
    int mouseScrollPower = 5;
    BaseShape* thisBallPointer = nullptr;
    BaseShape* previousBallPointer = nullptr;
    float moveSpeedScreen = 15.f;


    std::string screen = "START";

    float headerTextureResizer = 1.2;
    sf::RectangleShape headerText = sf::RectangleShape(sf::Vector2f(1212 / headerTextureResizer, 80 / headerTextureResizer));
    headerText.setOrigin(headerText.getSize().x / 2, headerText.getSize().y / 2);
    headerText.setPosition(window_width / 2, window_height / 2 - 350);
    sf::Texture startButtonTexture;
    sf::Texture exitButtonTexture;
    sf::Texture settingsButtonTexture;
    sf::Texture mainMenuHeaderTexture;
    if (!startButtonTexture.loadFromFile("START.png")) { return -1; }
    if (!exitButtonTexture.loadFromFile("EXIT.png")) { return -1; }
    if (!settingsButtonTexture.loadFromFile("SETTINGS.png")) { return -1; }
    if (!mainMenuHeaderTexture.loadFromFile("Header.png")) { return -1; }
    startButtonTexture.setSmooth(true);
    exitButtonTexture.setSmooth(true);
    settingsButtonTexture.setSmooth(true);
    mainMenuHeaderTexture.setSmooth(true);
    float textureResizer = 1.8;
    Button startButtonMainMenu = Button(534 / textureResizer, 274 / textureResizer, sf::Vector2f(window_width / 2, 450), "START");
    Button settingsButtonMainMenu = Button(534 / textureResizer, 274 / textureResizer, sf::Vector2f(window_width / 2, 675), "SETTINGS");
    Button exitButtonMainMenu = Button(534 / textureResizer, 274 / textureResizer, sf::Vector2f(window_width / 2, 925), "EXIT");
    startButtonMainMenu.SetTexture(startButtonTexture);
    settingsButtonMainMenu.SetTexture(settingsButtonTexture);
    exitButtonMainMenu.SetTexture(exitButtonTexture);
    headerText.setTexture(&mainMenuHeaderTexture);
    std::vector<std::pair< Button, bool>> mainMenuButtonVec;
    mainMenuButtonVec.push_back(std::make_pair(startButtonMainMenu, false));
    mainMenuButtonVec.push_back(std::make_pair(settingsButtonMainMenu, false));
    mainMenuButtonVec.push_back(std::make_pair(exitButtonMainMenu, false));

    //settings
    sf::RectangleShape settingsHeaderText = sf::RectangleShape(sf::Vector2f(555 / headerTextureResizer, 79 / headerTextureResizer));
    settingsHeaderText.setOrigin(settingsHeaderText.getSize().x / 2, settingsHeaderText.getSize().y / 2);
    settingsHeaderText.setPosition(window_width / 2, window_height / 2 - 350);
    sf::Texture SettingsHeaderTexture;
    sf::Texture fullscreenButtonTexture;
    if (!fullscreenButtonTexture.loadFromFile("FULLSCREEN.png")) { return -1; }
    if (!SettingsHeaderTexture.loadFromFile("SETTINGS_HEADER.png")) { return -1; }
    fullscreenButtonTexture.setSmooth(true);;
    SettingsHeaderTexture.setSmooth(true);;
    Button fullscreenButton = Button(534 / textureResizer, 274 / textureResizer, sf::Vector2f(window_width / 2, 350), "FULLSCREEN");
    Button exitButtonSettings = Button(534 / textureResizer, 274 / textureResizer, sf::Vector2f(window_width / 2, 550), "MAIN MENU");
    fullscreenButton.SetTexture(fullscreenButtonTexture);
    settingsHeaderText.setTexture(&SettingsHeaderTexture);
    exitButtonSettings.SetTexture(exitButtonTexture);
    std::vector<std::pair< Button, bool>> settingsButtonVec;
    settingsButtonVec.push_back(std::make_pair(fullscreenButton, false));
    settingsButtonVec.push_back(std::make_pair(exitButtonSettings, false));

    //PLANET::::!!!!
    bool planetMode = false;
    sf::Vector2f initialVel = sf::Vector2f(4, 0);
    //float innerGravity, sf::Color color, sf::Vector2f pos, float radius, float mass
    //objectList.CreateNewPlanet(7000, ball_color, sf::Vector2f(1980/2, 600), 20, 5.9722 * pow(10,1));
    //objectList.CreateNewPlanet(7000, ball_color, sf::Vector2f(1980/2-300, 600), 20, 5.9722 * pow(10,24));

    //for starting with balls:
    /*for (size_t i = 0; i < 15; i++)
    {
        objectList.CreateNewCircle(gravity, gradient[gradientStep], spawnStartingPoint);
    }*/

    //lineLink:
    sf::Vector2f* previousMousePos=nullptr;
    bool connectingMode = false;


    while (window.isOpen())
    {
        if (screen == "START")
        {
            sf::Vector2i currentMousePosInt = sf::Mouse::getPosition(window);
            sf::Vector2f currentMousePos = static_cast<sf::Vector2f>(currentMousePosInt);

            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed) { window.close(); }
                if (event.type == sf::Event::MouseWheelScrolled) {
                    if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                        if (event.mouseWheelScroll.delta > 0)
                            view.zoom(1.f / ZOOM_FACTOR);
                        else if (event.mouseWheelScroll.delta < 0)
                            view.zoom(ZOOM_FACTOR);
                    }
                }
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Escape) {
                        screen = "MAIN MENU";
                        objectList.DeleteAll();
                        objCount = 0;
                        break;
                    }
                    if (event.key.code == sf::Keyboard::A) {
                        for (size_t i = 0; i < 10; i++)
                        {
                            BaseShape* newObject=objectList.CreateNewCircle(gravity, gradient[gradientStep], spawnStartingPoint, initialVel);
                            objCount += 1;
                            gradientStep += 1;
                            spawnStartingPoint.x += startingPointAdder;
                            if (gradientStep == gradientStepMax) {
                                std::reverse(gradient.begin(), gradient.end());
                                gradientStep = 0;
                            }
                            if (spawnStartingPoint.x >= window_width - radius || spawnStartingPoint.x <= radius)
                            {
                                startingPointAdder *= -1;
                            }
                            objectList.connectedObjects.AddObject(newObject);
                        }
                    }
                    if (event.key.code == sf::Keyboard::T) {
                        for (size_t i = 0; i < 10; i++)
                        {
                            objectList.CreateNewRectangle(gravity, gradient[gradientStep], spawnStartingPoint);
                            objCount += 1;
                            gradientStep += 1;
                            spawnStartingPoint.x += startingPointAdder;
                            if (gradientStep == gradientStepMax) {
                                std::reverse(gradient.begin(), gradient.end());
                                gradientStep = 0;
                            }
                            if (spawnStartingPoint.x >= window_width - radius || spawnStartingPoint.x <= radius)
                            {
                                startingPointAdder *= -1;
                            }
                        }
                    }
                    if (event.key.code == sf::Keyboard::C) {
                        if (!connectingMode)
                        {
                            connectingMode = true;
                            previousBallPointer = thisBallPointer;
                        }
                        else {
                            connectingMode = false;
                        }
                    }

                    if (event.key.code == sf::Keyboard::H) {
                        for (size_t i = 0; i < 1; i++)
                        {
                            BaseShape* newObject = objectList.CreateNewCircle(gravity, gradient[gradientStep], currentMousePos, initialVel);
                            objCount += 1;
                            objectList.connectedObjects.AddObject(newObject);
                            objectList.connectedObjects.ConnectRandom(10);
                        }
                    }
                    if (event.key.code==sf::Keyboard::F11)
                    {
                        if (!options.fullscreen)
                        {
                            window.create(options.desktopSize, "Fullscreen Mode", sf::Style::Fullscreen);
                            options.fullscreen = true;
                        }
                        else {
                            window.create(sf::VideoMode(1920, 980), "Tomy Mode", sf::Style::Default);
                            options.fullscreen = false;
                        }
                    }
                    if (event.key.code == sf::Keyboard::L) {
                        planetMode = true;
                        objectList.CreateNewPlanet(7000, ball_color, currentMousePos, 20, 5.9722 * pow(10, 15));;
                        objCount += 1;
                    }
                    if (event.key.code == sf::Keyboard::F) {
                        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                        objectList.CreateNewCircle(gravity, explosion, sf::Vector2f(currentMousePos.x + 3, currentMousePos.y + 3), initialVel);
                        for (size_t i = 0; i < 50; i++)
                        {
                            objectList.CreateNewCircle(gravity, explosion, currentMousePos, initialVel);
                            objCount += 1;
                        }
                    }
                    if (event.key.code == sf::Keyboard::J) {
                        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                        objectList.CreateNewRectangle(gravity, explosion, sf::Vector2f(currentMousePos.x + 3, currentMousePos.y + 3));
                        for (size_t i = 0; i < 50; i++)
                        {
                            objectList.CreateNewRectangle(gravity, explosion, currentMousePos);
                            objCount += 1;
                        }
                    }
                    if (event.key.code == sf::Keyboard::R) {
                        objectList.DeleteAll();
                        objCount = 0;
                    }

                    if (mouseFlagClick && event.key.code == sf::Keyboard::S)
                    {
                        scaleFlag = true;
                    }

                    if (event.key.code == sf::Keyboard::Left)
                        view.move(-moveSpeedScreen, 0.f);
                    if (event.key.code == sf::Keyboard::Right)
                        view.move(moveSpeedScreen, 0.f);
                    if (event.key.code == sf::Keyboard::Up)
                        view.move(0.f, -moveSpeedScreen);
                    if (event.key.code == sf::Keyboard::Down)
                        view.move(0.f, moveSpeedScreen);

                }
                if (event.type == sf::Event::MouseButtonReleased) {
                    mouseFlagClick = false;
                    scaleFlag = false;
                    window.setMouseCursor(defaultCursor);
                }
                if (event.type == sf::Event::MouseWheelScrolled) {
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

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && mouseFlagClick == false) {
                thisBallPointer = objectList.IsInRadius(currentMousePosInt); // Check if a circle is within radius
                if (previousBallPointer==nullptr)
                {
                    previousBallPointer = thisBallPointer;
                }
                if (thisBallPointer != nullptr) { // Check if a circle was found
                    mouseFlagClick = true; // Set flag if circle found
                    window.setMouseCursor(handCursor);
                }
            }

            if (mouseFlagClick) { // Check if a circle is found
                sf::Vector2f mousePosFloat = static_cast<sf::Vector2f>(currentMousePosInt); // Use current mouse position
                thisBallPointer->SetPosition(mousePosFloat); // Set position of the found ball
                if (scaleFlag && mouseFlagScrollUp || mouseFlagScrollDown) {
                    if (Circle* circle = dynamic_cast<Circle*>(thisBallPointer)) {
                        if (mouseFlagScrollDown && circle->getRadius() > 0.0001) {
                            circle->SetRadiusAndCenter(circle->getRadius() - mouseScrollPower);
                            circle->SetMass(circle->GetMass() - mouseScrollPower * 10);
                            std::cout << circle->getRadius();
                            mouseFlagScrollDown = false;
                        }
                        else {
                            circle->SetRadiusAndCenter(circle->getRadius() + mouseScrollPower);
                            circle->SetMass(circle->GetMass() + mouseScrollPower * 10);
                            mouseFlagScrollUp = false;
                        }
                    }
                    else if (Rectangle* rectangle = dynamic_cast<Rectangle*>(thisBallPointer))
                    {
                        if (mouseFlagScrollDown && rectangle->GetHeight() > 0.0001 && rectangle->GetWidth() > 0.0001) {
                            rectangle->SetSizeAndOrigin(rectangle->GetWidth() - mouseScrollPower, rectangle->GetHeight() - mouseScrollPower);
                            rectangle->SetMass(rectangle->GetMass() - mouseScrollPower * 10);
                            mouseFlagScrollDown = false;
                        }
                        else {
                            rectangle->SetSizeAndOrigin(rectangle->GetWidth() + mouseScrollPower, rectangle->GetHeight() + mouseScrollPower);
                            rectangle->SetMass(rectangle->GetMass() + mouseScrollPower * 10);
                            mouseFlagScrollUp = false;
                        }
                    }
                }
                else if (connectingMode&& thisBallPointer != previousBallPointer)
                {
                    objectList.connectedObjects.MakeNewLink(previousBallPointer, thisBallPointer);
                }
            }


            // Calculate FPS
            frameCount++;
            if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
                currentFPS = frameCount;
                frameCount = 0;
                fpsClock.restart();
            }

            window.clear(background_color);
            window.setView(view);
            //lineLink.MakeLinks(window,60);
            objectList.MoveAndDraw(window, currentFPS, elastic,planetMode);
            window.setView(window.getDefaultView());

            // Update the FPS text
            std::ostringstream oss;
            std::ostringstream ossCount;
            oss << "FPS: " << static_cast<int>(currentFPS);
            oss << "Balls Count: " << static_cast<int>(objCount);
            fpsText.setString(oss.str());
            window.draw(fpsText);
            ballsCountText.setString(ossCount.str());
            window.draw(ballsCountText);


            //lineLink.Draw(window);

            window.display();

            // Update the previous mouse position

            // Limit to 60 FPS
            sf::Time elapsed = clock.restart();
            if (elapsed.asSeconds() < targetFrameTime) {
                sf::sleep(sf::seconds(targetFrameTime - elapsed.asSeconds()));
            }
        }
        else if (screen == "MAIN MENU")
        {

            screen = MainMenu(window, mainMenuBackgroundColor, headerText, mainMenuButtonVec);

        }
        else if (screen == "SETTINGS") {
            screen = Settings(window, mainMenuBackgroundColor, settingsHeaderText, settingsButtonVec);
        }
        else if (screen == "FULLSCREEN") {
            if (!options.fullscreen)
            {
                window.create(options.desktopSize, "Fullscreen Mode", sf::Style::Fullscreen);
                options.fullscreen = true;
            }
            else {
                window.create(sf::VideoMode(1920, 980), "Tomy Mode", sf::Style::Default);
                options.fullscreen = false;
            }
            screen = Settings(window, mainMenuBackgroundColor, settingsHeaderText, settingsButtonVec);
        }
        else
        {
            window.close();
        }
    }

    return 0;
}
