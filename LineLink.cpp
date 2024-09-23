#pragma once
#include "CircleBase.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include <random>
#include <ctime>

class LineLink
{
private:
    float lineLength;
    int ballCount = 0;
    std::mt19937 rnd;

public:
    std::vector<CircleBase> circleLink;

    LineLink(float lineLength) : lineLength(lineLength) {
        rnd.seed(static_cast<unsigned>(std::time(nullptr)));
    }
    void NewBall(CircleBase ball) {
        std::uniform_int_distribution<int> rndXRange(50, 1280 - 50);
        sf::Vector2f position;
        if (circleLink.empty()) {
            position = sf::Vector2f(rndXRange(rnd), 200);
        }
        else {
            const auto& lastBall = circleLink.back();
            position = lastBall.getPosition() + sf::Vector2f(lineLength, 0);
        }
        ball.setPosition(position);
        circleLink.push_back(ball);
        ballCount += 1;
    }

    void NewBall(float gravity, sf::Color color) {
        std::uniform_int_distribution<int> radiusRange(10, 10);
        std::uniform_int_distribution<int> massRange(1, 1);
        std::uniform_int_distribution<int> rndXRange(50, 1280 - 50);

        sf::Vector2f position;
        if (circleLink.empty()) {
            position = sf::Vector2f(rndXRange(rnd), 200);
        }
        else {
            const auto& lastBall = circleLink.back();
            position = lastBall.getPosition() + sf::Vector2f(lineLength, 0);
        }

        CircleBase ball(radiusRange(rnd), color, position, gravity, massRange(rnd));
        circleLink.push_back(ball);
        ballCount += 1;
    }

    void MakeLinks(sf::RenderWindow& window, int fps) {
        HandleCollisionsInList(window);

        for (size_t i = 0; i < circleLink.size() - 1; i++) {
            CircleBase& circle1 = circleLink[i];
            CircleBase& circle2 = circleLink[i + 1];

            sf::Vector2f vector = circle2.getPosition() - circle1.getPosition();
            float vectorLength = std::sqrt(vector.x * vector.x + vector.y * vector.y);
            sf::Vector2f normalizedVector = vector / vectorLength;

            float deltaLength = vectorLength - lineLength;
            sf::Vector2f correction = normalizedVector * (deltaLength * 0.5f);

            circle1.setPosition(circle1.getPosition() + correction);
            circle2.setPosition(circle2.getPosition() - correction);

            // Apply a small upward force to counteract gravity
            float upwardForce =  9.8f * 0.1f; // Adjust this value to change the "stiffness" of the string
            circle1.applyForce(sf::Vector2f(0, upwardForce));
            circle2.applyForce(sf::Vector2f(0, -upwardForce));

            circle1.updatePosition(1.0f / fps);
            circle2.updatePosition(1.0f / fps);
        }

        // Apply additional constraints to the first and last balls
        if (!circleLink.empty()) {
            circleLink.front().setPosition(300, 300); // Fix the y-position of the first ball
            circleLink.back().applyForce(sf::Vector2f(0, 9.8f * 0.2f)); // Apply extra downward force to the last ball
        }
    }

    void HandleCollisionsInList(sf::RenderWindow& window) {
        for (size_t i = 0; i < circleLink.size()-1; i++)
        {
            circleLink[i].handleWallCollision(window);
            circleLink[i].handleCollision(circleLink[i+1]);
        }
    }

    void Draw(sf::RenderWindow& window) {
        for (const auto& circle : circleLink) {
            window.draw(circle);
        }

        // Draw lines between balls to visualize the string
        sf::VertexArray lines(sf::Lines, (circleLink.size() - 1) * 2);
        for (size_t i = 0; i < circleLink.size() - 1; ++i) {
            lines[i * 2].position = circleLink[i].getPosition();
            lines[i * 2 + 1].position = circleLink[i + 1].getPosition();
            lines[i * 2].color = sf::Color::White;
            lines[i * 2 + 1].color = sf::Color::White;
        }
        window.draw(lines);
    }
};
