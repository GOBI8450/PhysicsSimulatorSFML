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
    int linked;

public:
    BaseShape(sf::Color color, float gravity, double mass)
        : color(color), gravity(gravity), mass(mass)
    {
        linked = -1;
        acceleration = sf::Vector2f(0, gravity ); //(x axis, y axis)
    }
    // Copy constructor
    BaseShape(const BaseShape& other)
        : oldPosition(other.oldPosition),
        acceleration(other.acceleration),
        color(other.color),
        gravity(other.gravity),
        mass(other.mass),
        fps(other.fps),
        linked(other.linked)
    {}

    ~BaseShape() {
        //will add if needed now not i guess
    }


    //Updates the position following varlet integration. meaning we calculate the next position based on the previos one and with time
    virtual void updatePositionVerlet(float dt){}
    virtual void updatePositionEuler(float dt) {}

    //If you wanna apply force to the circle:
    virtual void applyForce(sf::Vector2f force)
    {}

    // Set shape color
    virtual void setColor(sf::Color newColor) {}

    //Set the mass of the circle
    void SetMass(double newMass) { mass = newMass; }

    //Get the mass of the circle. if I will use it on your mother I will get an out of bounderies error!
    double GetMass() { return mass; }

    // Function to draw the circle
    virtual void draw(sf::RenderWindow& window){}

    double Distance(BaseShape* otherShape) {
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

    virtual void SetVelocity(const sf::Vector2f& newVelocity){}
    virtual void SetVelocity(float x, float y) {}
    virtual sf::Vector2f GetVelocity() const { return sf::Vector2f(); }

    void SetAcceleration(sf::Vector2f newAcceleration) {
        acceleration = newAcceleration;
    }

    sf::Vector2f GetAcceleration() {
        return acceleration;
    }
    // Getter for oldPosition
    sf::Vector2f GetOldPosition() const {
        return oldPosition;
    }

    // Setter for oldPosition
    void SetOldPosition(const sf::Vector2f& newOldPosition) {
        oldPosition = newOldPosition;
    }

    void SetLinked(bool isLinked) { linked = isLinked; }

    int GetLinked() { return linked; }

    virtual void SetOutline(sf::Color color, float thickness) {}


    sf::Color GetColor() { return color; }

};

