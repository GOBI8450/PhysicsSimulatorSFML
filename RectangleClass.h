#pragma once
#include <SFML/Graphics.hpp>
#include <iostream> 
#include "BaseShape.h"
#include "Circle.h"

class RectangleClass : public BaseShape, public sf::RectangleShape
{
private:
    float width;
    float height;
    sf::Vector2f velocity;

public:
    // Constructor with radius, color, gravity, mass
    RectangleClass(float width, float height, sf::Color color, float gravity, double mass)
        : BaseShape(color, gravity, mass), width(width), height(height)
    {
        setSize(sf::Vector2f(width, height));
        setFillColor(color);
        setOrigin(width / 2, height / 2);
        setPosition(width / 2, height / 2);
        oldPosition = sf::Vector2f(width, height);
        acceleration = sf::Vector2f(0, gravity * 100); //(x axis, y axis)
    }

    // Constructor with radius, color, gravity, mass, position
    RectangleClass(float width, float height, sf::Color color, sf::Vector2f pos, float gravity, double mass)
        : BaseShape(color, gravity, mass), width(width), height(height)
    {
        setSize(sf::Vector2f(width, height));
        setFillColor(color);
        setOrigin(width / 2, height / 2);
        setPosition(pos);
        oldPosition = pos;
        acceleration = sf::Vector2f(0, gravity * 100);//(x axis, y axis)
    }

    void SetVelocity(const sf::Vector2f& newVelocity) override {
        velocity = newVelocity;
    }

    void SetVelocity(float x, float y) override {
        velocity.x = x;
        velocity.y = y;
    }

    sf::Vector2f GetVelocity() const override {
        return velocity;
    }

    // Modify the updatePosition method:
    void updatePositionVerlet(float dt) override
    {
        sf::Vector2f currentPos = GetPosition();
        sf::Vector2f newPos = currentPos + velocity * dt + (acceleration * (dt * dt * 0.5f));
        oldPosition = currentPos;
        setPosition(newPos);

        // Update velocity for the next frame
        velocity = (newPos - currentPos) / dt;
    }

    //update the position based on euler integration.
    void updatePositionEuler(float dt) override
    {
        sf::Vector2f currentPos = GetPosition();
        sf::Vector2f newPos = currentPos + velocity * dt + (acceleration * (dt * dt * 0.5f));
        oldPosition = currentPos;
        setPosition(newPos);

        // Update velocity for the next frame
        velocity = (newPos - currentPos) / dt;
    }

    // Set shape color
    void setColor(sf::Color newColor) override
    {
        color = newColor;  // Update the member variable
        setFillColor(newColor);
    }

    // Function to draw the rectangle
    void draw(sf::RenderWindow& window)
    {
        window.draw(*this);
    }





    void handleWallCollision(sf::RenderWindow& window) {
        sf::Vector2f pos = GetPosition();
        sf::Vector2u windowSize = window.getSize();
        float energyLossFactor = 0;// If you wanna add energy loss
        //as the origin point is set to the center of the circle the point will be always radius far away from its edges
        if (pos.x - width / 2 < 0)
        {
            oldPosition.x = pos.x;
            pos.x = width / 2;
            oldPosition.x = pos.x + (pos.x - oldPosition.x) * energyLossFactor; // 0.9f Damping factor
        }
        else if (pos.x + width / 2 > windowSize.x)
        {
            oldPosition.x = pos.x;
            pos.x = windowSize.x - width / 2;
            oldPosition.x = pos.x + (pos.x - oldPosition.x) * energyLossFactor;
        }

        if (pos.y - height / 2 < 0)
        {
            oldPosition.y = pos.y;
            pos.y = height / 2;
            oldPosition.y = pos.y + (pos.y - oldPosition.y) * energyLossFactor;
        }
        else if (pos.y + height / 2 > windowSize.y)
        {
            oldPosition.y = pos.y;
            pos.y = windowSize.y - height / 2;
            oldPosition.y = pos.y + (pos.y - oldPosition.y) * energyLossFactor;
        }

        setPosition(pos);
    }



    double FindOverlap(RectangleClass* otherRec) {
        sf::Vector2f pos = GetPosition();
        sf::Vector2f otherPos = otherRec->GetPosition();

        float halfW1 = width / 2, halfH1 = height / 2;
        float halfW2 = otherRec->width / 2, halfH2 = otherRec->height / 2;

        // Calculate the distances between centers along each axis
        float distX = std::abs(pos.x - otherPos.x);
        float distY = std::abs(pos.y - otherPos.y);

        // Calculate the combined half-widths and half-heights
        float combinedHalfWidth = halfW1 + halfW2;
        float combinedHalfHeight = halfH1 + halfH2;

        // Calculate the overlap in each axis (if any)
        float overlapX = combinedHalfWidth - distX;
        float overlapY = combinedHalfHeight - distY;

        // If both overlaps are positive, rectangles are overlapping
        if (overlapX > 0 && overlapY > 0) {
            // Return the smaller overlap, which represents the minimum push required to separate the rectangles
            return std::min(overlapX, overlapY);
        }

        // No overlap
        return 0.0;
    }

    double FindOverlap(sf::CircleShape* circle) {
        sf::Vector2f circlePos = circle->getPosition();  // Circle's center
        sf::Vector2f rectPos = GetPosition();      // Rectangle's center

        float halfRectWidth = width / 2;
        float halfRectHeight = height / 2;

        // Calculate the closest point on the rectangle to the circle's center
        float closestX = std::max(rectPos.x - halfRectWidth, std::min(circlePos.x, rectPos.x + halfRectWidth));
        float closestY = std::max(rectPos.y - halfRectHeight, std::min(circlePos.y, rectPos.y + halfRectHeight));

        // Calculate the distance from the circle's center to this closest point
        float distanceX = circlePos.x - closestX;
        float distanceY = circlePos.y - closestY;
        float distanceSquared = distanceX * distanceX + distanceY * distanceY;

        float radius = circle->getRadius();

        // If the distance is less than the radius, they overlap
        if (distanceSquared <= radius * radius) {
            // Calculate overlap as the difference between radius and distance
            float distance = std::sqrt(distanceSquared);
            return radius - distance;
        }

        // No overlap
        return 0.0;
    }

    //Checks if there is any collision between two Rectangles
    bool IsCollision(RectangleClass* otherRec) {

        sf::Vector2f pos = GetPosition();
        sf::Vector2f otherPos = otherRec->GetPosition();
        float x1 = pos.x;
        float y1 = pos.y;
        float x2 = otherPos.x;
        float y2 = otherPos.y;
        if (x1 + width >= x2 &&    // r1 right edge past r2 left
            x1 <= x2 + otherRec->width &&    // r1 left edge past r2 right
            y1 + height >= y2 &&    // r1 top edge past r2 bottom
            y1 <= y2 + otherRec->height) {    // r1 bottom edge past r2 top
            return true;
        }
        return false;
    }

    //Check if a point intersects with a rectangle
    bool IsCollision(sf::Vector2f otherPos) {
        sf::Vector2f pos = getPosition();
        float x = pos.x;
        float y = pos.y;
        float xMouse = otherPos.x;
        float yMouse = otherPos.y;
        if (xMouse <= x + width / 2 && xMouse >= x - width / 2 && yMouse <= y + height / 2 && yMouse >= y - height / 2)
        {
            return true;
        }
        return false;
    }

    void HandleCollision(RectangleClass* otherRec) {
        if (IsCollision(otherRec))
        {
            // Get positions of both circles
            sf::Vector2f pos = GetPosition();
            sf::Vector2f posOther = otherRec->GetPosition();

            // Calculate the distance and overlap
            double distance = Distance(otherRec);
            double overlap = FindOverlap(otherRec);

            if (overlap > 0) {
                sf::Vector2f direction = pos - posOther;//The direction vector, between center points of the circles
                float length = sqrt(direction.x * direction.x + direction.y * direction.y); // The length between the circles in scalar
                if (length > 0) { //if there is any length between them we should devide the direction by the length so it will give only the direction, like you dont say go right 5km you say go right.
                    direction /= length; // Normalize the direction vector
                }
                float massRatio = mass / otherRec->mass;
                // Move circles apart based on the overlap so they will no longer be in contact
                sf::Vector2f displacement = direction * static_cast<float>(overlap / 2.0f); // Split overlap
                pos += displacement;  // Move this circle
                posOther -= displacement * massRatio; // Move the other circle

                // Update positions
                setPosition(pos);
                otherRec->setPosition(posOther);
            }
        }
    }

    void HandleCollision(Circle* circle) {
        if (isCollison(circle))
        {
            // Get positions of both circles
            sf::Vector2f pos = GetPosition();
            sf::Vector2f posOther = circle->GetPosition();

            // Calculate the distance and overlap
            double distance = Distance(circle);
            double overlap = FindOverlap(circle);

            if (overlap > 0) {
                sf::Vector2f direction = pos - posOther;//The direction vector, between center points of the circles
                float length = sqrt(direction.x * direction.x + direction.y * direction.y); // The length between the circles in scalar
                if (length > 0) { //if there is any length between them we should devide the direction by the length so it will give only the direction, like you dont say go right 5km you say go right.
                    direction /= length; // Normalize the direction vector
                }
                float massRatio = mass / circle->GetMass();
                // Move circles apart based on the overlap so they will no longer be in contact
                sf::Vector2f displacement = direction * static_cast<float>(overlap / 2.0f); // Split overlap
                pos += displacement;  // Move this circle
                posOther -= displacement * massRatio; // Move the other circle

                // Update positions
                setPosition(pos);
                circle->setPosition(posOther);
            }
        }
    }

    bool isCollison(Circle* circle) {
        sf::Vector2f pos = GetPosition();
        sf::Vector2f otherPos = circle->GetPosition();
        float xRec = pos.x;
        float yRec = pos.y;
        float xCir = otherPos.x;
        float yCir = otherPos.y;
        float testX = xCir;
        float testY = yCir;
        if (xCir < xRec) { testX = xRec; } // left edge
        else if (xCir > xRec + width) { testX = xRec + width; } // right edge
        if (yCir < yRec) { testY = yRec; } // top edge
        else if (yCir > yRec + height) { testY = yRec + height; } // bottom edge
        float distX = xCir - testX;
        float distY = yCir - testY;
        float distance = sqrt((distX * distX) + (distY * distY));
        if (distance <= circle->getRadius()) {
            return true;
        }
        return false;
    }

    void HandleCollisionElastic(RectangleClass* otherRec, float elastic) {}///!!!!!!!!!!//// fill

    float GetHeight() {
        return height;
    }

    float GetWidth() {
        return width;
    }

    void SetPosition(sf::Vector2f newPos) override
    {
        setPosition(newPos);
    }

    void SetSizeAndOrigin(float newWidth, float newHeight) {
        width = newWidth;
        height = newHeight;
        setOrigin(width, height);
    }

    void SetOutline(sf::Color color, float thickness) override {
        setOutlineThickness(thickness);
        setOutlineColor(color);
    }

    sf::Vector2f GetPosition() override {
        return getPosition();
    }
};

