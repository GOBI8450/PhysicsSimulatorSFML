#pragma once
#include <SFML/Graphics.hpp>
#include <iostream> 
class BaseShape
{
protected:
    sf::Vector2f oldPosition; //Vector for X axis and Y axis
    sf::Vector2f acceleration; //Vector for X axis and Y axis
    sf::Color color;
    float gravity; // float to not be too strong
    double mass;//be as fat as you want brotha
    double fps;//my worst enemy

public:
    BaseShape(sf::Color color, float gravity, double mass)
        : color(color), gravity(gravity), mass(mass)
    {
        acceleration = sf::Vector2f(0, gravity ); //(x axis, y axis)
    }

    //Updates the position following varlet integration. meaning we calculate the next position based on the previos one and with time
    virtual void updatePosition(float dt){}

    //If you wanna apply force to the circle:
    void applyForce(sf::Vector2f force)
    {
        acceleration += force / static_cast<float>(mass); //because m*a = F --> a = F/m by newton second law, Thank you Tomy
    }

    // Set shape color
    virtual void setColor(sf::Color newColor) {}

    //Set the mass of the circle
    void SetMass(double newMass) { mass = newMass; }

    //Get the mass of the circle. if I will use it on your mother I will get an out of bounderies error!
    double GetMass() { return mass; }

    // Function to draw the circle
    virtual void draw(sf::RenderWindow& window){}

    double DistanceOnly(BaseShape* otherShape) {
        sf::Vector2f pos = GetPosition();       // Position of this shape
        sf::Vector2f posOther = otherShape->GetPosition(); // Position of the other shape
        double x1 = pos.x;
        double y1 = pos.y;
        double x2 = posOther.x;
        double y2 = posOther.y;
        return std::sqrt(std::pow((y2 - y1), 2) + std::pow((x2 - x1), 2)); // Return distance
    }

    double DistanceTwoPoints(sf::Vector2f otherPos) {
        sf::Vector2f pos = GetPosition();// Position of this shape
        double x1 = pos.x;
        double y1 = pos.y;
        double x2 = otherPos.x;
        double y2 = otherPos.y;
        return std::sqrt(std::pow((y2 - y1), 2) + std::pow((x2 - x1), 2)); // Return distance
    }

    virtual sf::Vector2f GetPosition() { return sf::Vector2f(0, 0); }

    virtual void SetPosition(sf::Vector2f newPos) {}
};

