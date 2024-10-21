#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <sstream>
#include <iostream>
#include <random>
#include <ctime>
#include "LineLink.h"
#include "ObjectsList.h"
#include "Grid.h"
#include "Button.h"
#include "Rectangle.h"

class PhysicsSimulation {
private:
    // Window and view settings
    sf::RenderWindow window;
    sf::View view;
    sf::ContextSettings settings;
    const float ZOOM_FACTOR = 1.5f;

    // Cursors
    sf::Cursor handCursor;
    sf::Cursor defaultCursor;

    // Application states
    struct Options {
        sf::VideoMode desktopSize = sf::VideoMode::getDesktopMode();
        int window_height = desktopSize.height;
        int window_width = desktopSize.width;
        bool fullscreen = false;
        float gravity = 0;
        double massLock = 0;
    } options;

    std::string screen = "START";
    bool hovering = false;
    bool connectingMode = false;
    bool planetMode = false;

    // Physics and simulation parameters
    float gridSize = 20;
    float lineLength = 150;
    ObjectsList objectList;
    float deltaTime = 1.0f / 60.0f;
    float elastic = 0;
    int objCount = 0;
    float radius = 50;

    // Mouse and interaction state
    sf::Vector2f* previousMousePos = nullptr;
    bool mouseFlagClick = false;
    bool mouseFlagScrollUp = false;
    bool mouseFlagScrollDown = false;
    int mouseScrollPower = 5;
    bool scaleFlag = false;
    bool TouchedOnce = false;
    float moveSpeedScreen = 15.f;

    // Object pointers for interaction
    BaseShape* thisBallPointer = nullptr;
    BaseShape* previousBallPointer = nullptr;

    // Visual settings
    sf::Color ball_color = sf::Color(238, 238, 238);
    sf::Color ball_color2 = sf::Color(50, 5, 11);
    sf::Color background_color = sf::Color(30, 30, 30);
    sf::Color mainMenuBackgroundColor = sf::Color(25, 25, 25);
    sf::Color buttonColor = sf::Color(55, 58, 64);
    sf::Color bb = sf::Color(44, 55, 100);
    sf::Color explosion = sf::Color(205, 92, 8);
    sf::Color outlineColor = sf::Color(255, 255, 255);
    sf::Color previousColor = sf::Color(0, 0, 0);

    // Gradient settings
    short int gradientStep = 0;
    short int gradientStepMax = 400;
    std::vector<sf::Color> gradient;

    // UI Elements
    sf::Font font;
    sf::Text ballsCountText;
    sf::Text fpsText;
    sf::Text linkingText;

    // Menu elements
    sf::RectangleShape headerText;
    std::vector<std::pair<Button, bool>> mainMenuButtonVec;
    std::vector<std::pair<Button, bool>> settingsButtonVec;

    // Performance tracking
    sf::Clock clock;
    sf::Clock fpsClock;
    int frameCount = 0;
    float currentFPS = 0.0f;

    // Object templates
    Circle* copyObjCir;
    RectangleClass* copyObjRec;

    // Spawn settings
    float posYStartingPoint = 200;
    int posXStartingPoint = radius;
    short int startingPointAdder = 31;
    sf::Vector2f spawnStartingPoint;
    sf::Vector2f initialVel = sf::Vector2f(4, 0);

    float textureResizer = 1.2;

public:
    PhysicsSimulation() :
        settings(8),  // antialiasing level
        objectList(lineLength),
        spawnStartingPoint(posXStartingPoint, posYStartingPoint) {
        initializeWindow();
        initializeCursors();
        loadResources();
        initializeUI();
        setupGradient();
        createCopyObjects();
    }

    void run() {
        while (window.isOpen()) {
            if (screen == "START") {
                runSimulation();
            }
            else if (screen == "MAIN MENU") {
                screen = handleMainMenu();
            }
            else if (screen == "SETTINGS") {
                screen = handleSettings();
            }

            else if (screen == "FULLSCREEN") {
                toggleFullscreen();
                screen = handleSettings();
            }
            else { 
                window.close();
            }
        }
    }

private:
    void initializeWindow() {
        window.create(
            sf::VideoMode(options.window_width, options.window_height),
            "TomySim",
            sf::Style::Default,
            settings
        );
        view = window.getDefaultView();
        window.setVerticalSyncEnabled(true);
        window.setFramerateLimit(60);
    }

    void initializeCursors() {
        if (!defaultCursor.loadFromSystem(sf::Cursor::Arrow) ||
            !handCursor.loadFromSystem(sf::Cursor::Hand)) {
            throw std::runtime_error("Failed to load cursors");
        }
        window.setMouseCursor(defaultCursor);
    }

    void loadResources() {
        if (!font.loadFromFile("font.ttf")) {
            throw std::runtime_error("Failed to load font");
        }
        loadTextures();
    }

    void initializeUI() {
        setupText();
        setupButtons();
        setupHeaders();
    }

    void setupGradient() {
        sf::Color startColor(128, 0, 128);  // purple
        sf::Color endColor(0, 0, 255);      // blue
        gradient = GenerateGradient(startColor, endColor, gradientStepMax);
    }

    void createCopyObjects() {
        copyObjCir = new Circle(ball_color2, 0, 0, 4);
        copyObjRec = new RectangleClass(3, 3, ball_color2, 0, 0);
    }

    void runSimulation() {
        handleSimulationEvents();
        updateSimulation();
        renderSimulation();
    }

    void handleEvent(sf::Event event) {
        sf::Vector2i currentMousePosInt = sf::Mouse::getPosition(window);
        sf::Vector2f currentMousePos = window.mapPixelToCoords(currentMousePosInt, view);
        if (event.type == sf::Event::Closed) { window.close(); }
        if (event.type == sf::Event::MouseWheelScrolled) {
            if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel && !scaleFlag) {
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
            }
            if (event.key.code == sf::Keyboard::A) {
                for (size_t i = 0; i < 10; i++)
                {
                    BaseShape* newObject = objectList.CreateNewCircle(options.gravity, gradient[gradientStep], spawnStartingPoint, initialVel);
                    objCount += 1;
                    gradientStep += 1;
                    spawnStartingPoint.x += startingPointAdder;
                    if (gradientStep == gradientStepMax) {
                        std::reverse(gradient.begin(), gradient.end());
                        gradientStep = 0;
                    }
                    if (spawnStartingPoint.x >= options.window_width - radius || spawnStartingPoint.x <= radius)
                    {
                        startingPointAdder *= -1;
                    }
                    objectList.connectedObjects.AddObject(newObject);
                }
            }
            if (event.key.code == sf::Keyboard::T) {
                for (size_t i = 0; i < 10; i++)
                {
                    objectList.CreateNewRectangle(options.gravity, gradient[gradientStep], spawnStartingPoint);
                    objCount += 1;
                    gradientStep += 1;
                    spawnStartingPoint.x += startingPointAdder;
                    if (gradientStep == gradientStepMax) {
                        std::reverse(gradient.begin(), gradient.end());
                        gradientStep = 0;
                    }
                    if (spawnStartingPoint.x >= options.window_width - radius || spawnStartingPoint.x <= radius)
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
                    linkingText.setString("ACTIVATED");
                    linkingText.setFillColor(sf::Color::Magenta);
                }
                else {
                    connectingMode = false;
                    linkingText.setString("DEACTIVATED");
                    linkingText.setFillColor(sf::Color::White);
                }
            }
            if (event.key.code == sf::Keyboard::BackSpace && mouseFlagClick) {
                objectList.DeleteThisObj(thisBallPointer);
            }

            if (event.key.code == sf::Keyboard::H) {
                for (size_t i = 0; i < 1; i++)
                {
                    BaseShape* newObject = objectList.CreateNewCircle(options.gravity, gradient[gradientStep], currentMousePos, initialVel);
                    objCount += 1;
                    objectList.connectedObjects.AddObject(newObject);
                    objectList.connectedObjects.ConnectRandom(10);
                }
            }
            if (event.key.code == sf::Keyboard::F11)
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
                objectList.CreateNewCircle(options.gravity, explosion, sf::Vector2f(currentMousePos.x + 3, currentMousePos.y + 3), initialVel);
                for (size_t i = 0; i < 50; i++)
                {
                    objectList.CreateNewCircle(options.gravity, explosion, currentMousePos, initialVel);
                    objCount += 1;
                }
            }
            if (event.key.code == sf::Keyboard::J) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                objectList.CreateNewRectangle(options.gravity, explosion, sf::Vector2f(currentMousePos.x + 3, currentMousePos.y + 3));
                for (size_t i = 0; i < 50; i++)
                {
                    objectList.CreateNewRectangle(options.gravity, explosion, currentMousePos);
                    objCount += 1;
                }
            }
            if (event.key.code == sf::Keyboard::R) {
                objectList.DeleteAll();
                objCount = 0;
            }

            if (event.key.code == sf::Keyboard::S && mouseFlagClick)
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
            if (thisBallPointer != nullptr)
            {
                window.setMouseCursor(defaultCursor);
                thisBallPointer->setColor(previousColor);
                thisBallPointer->SetOutline(outlineColor, 0);
            }
            TouchedOnce = false;
        }
        if (event.type == sf::Event::MouseWheelScrolled) {
            if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel && !scaleFlag) {
                //sf::Vector2f beforeZoom = window.mapPixelToCoords(sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y), view);

                if (event.mouseWheelScroll.delta > 0) {


                    view.zoom(1.f / ZOOM_FACTOR);
                    mouseFlagScrollUp = true;
                }

                else if (event.mouseWheelScroll.delta < 0) {
                    view.zoom(ZOOM_FACTOR);
                    mouseFlagScrollDown = true;
                }

                //sf::Vector2f afterZoom = window.mapPixelToCoords(sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y), view);
                //sf::Vector2f offset = beforeZoom - afterZoom;
                //view.move(offset);
            }
        }
    }

    

    void handleSimulationEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            handleEvent(event);
        }
        handleMouseInteraction();
    }

    void updateSimulation() {
        updateFPS();
        objectList.MoveAndDraw(window, currentFPS, elastic, planetMode);
    }

    void renderSimulation() {
        window.clear(background_color);
        window.setView(view);

        copyObjCir->draw(window);
        objectList.MoveAndDraw(window, currentFPS, elastic, planetMode);

        window.setView(window.getDefaultView());
        renderUI();

        window.display();

        limitFrameRate();
    }

    // Helper methods for generating gradients
    std::vector<sf::Color> GenerateGradient(sf::Color startColor, sf::Color endColor, int steps) {
        std::vector<sf::Color> gradient;
        float stepR = (endColor.r - startColor.r) / static_cast<float>(steps - 1);
        float stepG = (endColor.g - startColor.g) / static_cast<float>(steps - 1);
        float stepB = (endColor.b - startColor.b) / static_cast<float>(steps - 1);

        for (int i = 0; i < steps; ++i) {
            gradient.push_back(sf::Color(
                startColor.r + stepR * i,
                startColor.g + stepG * i,
                startColor.b + stepB * i
            ));
        }
        return gradient;
    }
    void scaleCircle(Circle* circle) {
        float currentRadius = circle->GetRadius();
        if (mouseFlagScrollUp && currentRadius < 100.0f) {
            circle->SetRadius(currentRadius + mouseScrollPower);
            mouseFlagScrollUp = false;
        }
        else if (mouseFlagScrollDown && currentRadius > 10.0f) {
            circle->SetRadius(currentRadius - mouseScrollPower);
            mouseFlagScrollDown = false;
        }
    }

    void scaleRectangle(RectangleClass* rectangle) {
        sf::Vector2f currentSize = rectangle->getSize();
        if (mouseFlagScrollUp && currentSize.x < 100.0f) {
            rectangle->setSize(currentSize + sf::Vector2f(mouseScrollPower, mouseScrollPower));
            mouseFlagScrollUp = false;
        }
        else if (mouseFlagScrollDown && currentSize.x > 10.0f) {
            rectangle->setSize(currentSize - sf::Vector2f(mouseScrollPower, mouseScrollPower));
            mouseFlagScrollDown = false;
        }
    }

    void loadTextures() {
        // Since no textures are used in the original code, this method is kept minimal
        // but provides a hook for future texture loading if needed
        try {
            // Currently no textures are loaded as the simulation uses basic shapes
            // If textures are needed in the future, they can be loaded here:
            /*
            sf::Texture texture;
            if (!texture.loadFromFile("path/to/texture.png")) {
                throw std::runtime_error("Failed to load texture");
            }
            */
        }
        catch (const std::exception& e) {
            std::cerr << "Texture loading error: " << e.what() << std::endl;
        }
    }

    void handleMouseInteraction() {
        sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
        sf::Vector2f worldPos = window.mapPixelToCoords(currentMousePos, view);

        if (previousMousePos == nullptr) {
            previousMousePos = new sf::Vector2f(worldPos);
        }

        updateObjectPosition(worldPos);
        *previousMousePos = worldPos;
    }

    void updateFPS() {
        frameCount++;
        float timeElapsed = fpsClock.getElapsedTime().asSeconds();

        if (timeElapsed >= 1.0f) {
            currentFPS = frameCount / timeElapsed;
            frameCount = 0;
            fpsClock.restart();
        }
    }

    void setupText() {
        // FPS Text
        fpsText.setFont(font);
        fpsText.setCharacterSize(20);
        fpsText.setFillColor(sf::Color::White);
        fpsText.setPosition(10, 10);

        // Balls Count Text
        ballsCountText.setFont(font);
        ballsCountText.setCharacterSize(20);
        ballsCountText.setFillColor(sf::Color::White);
        ballsCountText.setPosition(10, 40);

        // Linking Text
        linkingText.setFont(font);
        linkingText.setString("DEACTIVATED");
        linkingText.setCharacterSize(20);
        linkingText.setFillColor(sf::Color::White);
        linkingText.setPosition(10, 70);
    }

    void setupButtons() {
    // Main Menu Buttons
    mainMenuButtonVec.emplace_back(
        Button(534 / textureResizer, 274 / textureResizer, sf::Vector2f(options.window_width / 2, 450), "START"),
        false);

    mainMenuButtonVec.emplace_back(
        Button(534 / textureResizer, 274 / textureResizer, sf::Vector2f(options.window_width / 2, 675), "SETTINGS"),
        false);

    mainMenuButtonVec.emplace_back(
        Button(534 / textureResizer, 274 / textureResizer, sf::Vector2f(options.window_width / 2, 925), "EXIT"),
        false);

    // Settings Buttons
    settingsButtonVec.emplace_back(
        Button(534 / textureResizer, 274 / textureResizer, sf::Vector2f(options.window_width / 2, 350), "FULLSCREEN"),
        false);

    settingsButtonVec.emplace_back(
        Button(534 / textureResizer, 274 / textureResizer, sf::Vector2f(options.window_width / 2, 550), "MAIN MENU"),
        false);
}

    void setupHeaders() {
        headerText.setSize(sf::Vector2f(400.f, 100.f));
        headerText.setPosition(
            options.window_width / 2.f - headerText.getSize().x / 2.f,
            100.f
        );
        headerText.setFillColor(buttonColor);
    }

    void handleKeyPress(sf::Event::KeyEvent key) {
        switch (key.code) {
        case sf::Keyboard::Escape:
            screen = "MAIN MENU";
            objectList.DeleteAll();
            objCount = 0;
            break;

        case sf::Keyboard::A:
            for (size_t i = 0; i < 10; i++) {
                BaseShape* newObject = objectList.CreateNewCircle(options.gravity, gradient[gradientStep], spawnStartingPoint, initialVel);
                objCount++;
                gradientStep++;
                spawnStartingPoint.x += startingPointAdder;

                if (gradientStep == gradientStepMax) {
                    std::reverse(gradient.begin(), gradient.end());
                    gradientStep = 0;
                }

                if (spawnStartingPoint.x >= options.window_width - radius || spawnStartingPoint.x <= radius) {
                    startingPointAdder *= -1;
                }

                objectList.connectedObjects.AddObject(newObject);
            }
            break;

        case sf::Keyboard::T:
            for (size_t i = 0; i < 10; i++) {
                objectList.CreateNewRectangle(options.gravity, gradient[gradientStep], spawnStartingPoint);
                objCount++;
                gradientStep++;
                spawnStartingPoint.x += startingPointAdder;

                if (gradientStep == gradientStepMax) {
                    std::reverse(gradient.begin(), gradient.end());
                    gradientStep = 0;
                }

                if (spawnStartingPoint.x >= options.window_width - radius || spawnStartingPoint.x <= radius) {
                    startingPointAdder *= -1;
                }
            }
            break;

        case sf::Keyboard::C:
            toggleConnectingMode();
            break;

        case sf::Keyboard::BackSpace:
            if (mouseFlagClick) {
                objectList.DeleteThisObj(thisBallPointer);
            }
            break;

        case sf::Keyboard::H:
            createConnectedObjects();
            break;

        case sf::Keyboard::F11:
            toggleFullscreen();
            break;

        case sf::Keyboard::L:
            createPlanet();
            break;

        case sf::Keyboard::F:
            createExplosionCircles();
            break;

        case sf::Keyboard::J:
            createExplosionRectangles();
            break;

        case sf::Keyboard::R:
            objectList.DeleteAll();
            objCount = 0;
            break;

        case sf::Keyboard::S:
            if (mouseFlagClick) {
                scaleFlag = true;
            }
            break;

            // Arrow key movement
        case sf::Keyboard::Left:
            view.move(-moveSpeedScreen, 0.f);
            break;
        case sf::Keyboard::Right:
            view.move(moveSpeedScreen, 0.f);
            break;
        case sf::Keyboard::Up:
            view.move(0.f, -moveSpeedScreen);
            break;
        case sf::Keyboard::Down:
            view.move(0.f, moveSpeedScreen);
            break;
        }
    }

    void handleMouseButton(sf::Event::MouseButtonEvent button) {
        if (button.button == sf::Mouse::Left) {
            if (button.button == sf::Event::MouseButtonReleased) {
                mouseFlagClick = false;
                scaleFlag = false;
                if (thisBallPointer != nullptr) {
                    window.setMouseCursor(defaultCursor);
                    thisBallPointer->setColor(previousColor);
                    thisBallPointer->SetOutline(outlineColor, 0);
                }
                TouchedOnce = false;
            }
            else if (button.button == sf::Event::MouseButtonPressed && !mouseFlagClick) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, view);
                thisBallPointer = objectList.IsInRadius(worldPos);

                if (previousBallPointer == nullptr) {
                    previousBallPointer = thisBallPointer;
                }

                if (thisBallPointer != nullptr) {
                    mouseFlagClick = true;
                    window.setMouseCursor(handCursor);
                }
            }
        }
    }

    void handleMouseWheel(sf::Event::MouseWheelScrollEvent wheel) {
        if (wheel.wheel == sf::Mouse::VerticalWheel && !scaleFlag) {
            if (wheel.delta > 0) {
                view.zoom(1.f / ZOOM_FACTOR);
                mouseFlagScrollUp = true;
            }
            else if (wheel.delta < 0) {
                view.zoom(ZOOM_FACTOR);
                mouseFlagScrollDown = true;
            }
        }
    }

    void updateObjectPosition(const sf::Vector2f& position) {
        if (mouseFlagClick && thisBallPointer != nullptr) {
            thisBallPointer->SetPosition(position);

            if (!TouchedOnce) {
                thisBallPointer->SetOutline(outlineColor, 5);
                sf::Color currentColor = thisBallPointer->GetColor();
                previousColor = currentColor;

                currentColor.r = std::max(0, currentColor.r - 15);
                currentColor.g = std::max(0, currentColor.g - 15);
                currentColor.b = std::max(0, currentColor.b - 15);

                thisBallPointer->setColor(currentColor);
                TouchedOnce = true;
            }

            handleScaling();
            handleConnecting();
        }
    }

    void toggleFullscreen() {
        if (!options.fullscreen) {
            window.create(options.desktopSize, "Fullscreen Mode", sf::Style::Fullscreen);
            options.fullscreen = true;
        }
        else {
            window.create(sf::VideoMode(1920, 980), "Tomy Mode", sf::Style::Default);
            options.fullscreen = false;
        }
        window.setVerticalSyncEnabled(true);
    }

    std::string handleMainMenu() {
        window.setTitle("Main Menu");

        sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosFloat = static_cast<sf::Vector2f>(currentMousePos);
        bool mouseClickFlag = false;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                mouseClickFlag = true;
            }
        }

        window.clear(mainMenuBackgroundColor);

        if (mouseClickFlag) {
            for (auto& pair : mainMenuButtonVec) {
                if (pair.first.IsInRadius(mousePosFloat)) {
                    return pair.first.GetName();
                }
            }
        }

        for (auto& button : mainMenuButtonVec) {
            button.second = button.first.MouseHover(mousePosFloat, hovering);
        }

        window.draw(headerText);
        for (auto& button : mainMenuButtonVec) {
            button.first.draw(window);
        }

        window.display();
        return "MAIN MENU";
    }

    std::string handleSettings() {
        window.setTitle("Settings");

        sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosFloat = static_cast<sf::Vector2f>(currentMousePos);
        bool mouseClickFlag = false;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                mouseClickFlag = true;
            }
        }

        window.clear(mainMenuBackgroundColor);

        if (mouseClickFlag) {
            for (auto& button : settingsButtonVec) {
                if (button.first.IsInRadius(mousePosFloat)) {
                    return button.first.GetName();
                }
            }
        }

        for (auto& button : settingsButtonVec) {
            button.second = button.first.MouseHover(mousePosFloat, hovering);
        }

        window.draw(headerText);
        for (auto& button : settingsButtonVec) {
            button.first.draw(window);
        }

        window.display();
        return "SETTINGS";
    }

    void renderUI() {
        std::ostringstream fpsStream;
        std::ostringstream ballCountStream;

        fpsStream << "FPS: " << static_cast<int>(currentFPS);
        ballCountStream << "Balls Count: " << static_cast<int>(objCount);

        fpsText.setString(fpsStream.str());
        ballsCountText.setString(ballCountStream.str());

        window.draw(fpsText);
        window.draw(linkingText);
        window.draw(ballsCountText);
    }

    void limitFrameRate() {
        sf::Time elapsed = clock.restart();
        if (elapsed.asSeconds() < deltaTime) {
            sf::sleep(sf::seconds(deltaTime - elapsed.asSeconds()));
        }
    }

    // Helper methods for the main handlers
    void toggleConnectingMode() {
        connectingMode = !connectingMode;
        linkingText.setString(connectingMode ? "ACTIVATED" : "DEACTIVATED");
        linkingText.setFillColor(connectingMode ? sf::Color::Magenta : sf::Color::White);
    }

    void createConnectedObjects() {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, view);

        for (size_t i = 0; i < 1; i++) {
            BaseShape* newObject = objectList.CreateNewCircle(options.gravity, gradient[gradientStep], worldPos, initialVel);
            objCount++;
            objectList.connectedObjects.AddObject(newObject);
            objectList.connectedObjects.ConnectRandom(10);
        }
    }

    void createPlanet() {
        planetMode = true;
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, view);
        objectList.CreateNewPlanet(7000, ball_color, worldPos, 20, 5.9722 * pow(10, 15));
        objCount++;
    }

    void handleScaling() {
        if (scaleFlag && (mouseFlagScrollUp || mouseFlagScrollDown)) {
            if (Circle* circle = dynamic_cast<Circle*>(thisBallPointer)) {
                scaleCircle(circle);
            }
            else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(thisBallPointer)) {
                scaleRectangle(rectangle);
            }
        }
    }

    void handleConnecting() {
        if (connectingMode && thisBallPointer != previousBallPointer) {
            objectList.connectedObjects.MakeNewLink(previousBallPointer, thisBallPointer);
        }
    }

    void createExplosionCircles() {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, view);

        objectList.CreateNewCircle(options.gravity, explosion, sf::Vector2f(worldPos.x + 3, worldPos.y + 3), initialVel);
        for (size_t i = 0; i < 50; i++) {
            objectList.CreateNewCircle(options.gravity, explosion, worldPos, initialVel);
            objCount++;
        }
    }

    void createExplosionRectangles() {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, view);

        objectList.CreateNewRectangle(options.gravity, explosion, sf::Vector2f(worldPos.x + 3, worldPos.y + 3));
        for (size_t i = 0; i < 50; i++) {
            objectList.CreateNewRectangle(options.gravity, explosion, worldPos);
            objCount++;
        }
    }
};

// Main function
int main() {
    try {
        PhysicsSimulation simulation;
        simulation.run();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
