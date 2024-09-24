#pragma once
#include <SFML/Graphics.hpp>
#include <iostream> 

// Base_Shape class inheriting from sf::Transformable
class CircleBase : public sf::CircleShape
{
private:
    sf::Vector2f oldPosition; //Vector for X axis and Y axis
    sf::Vector2f acceleration; //Vector for X axis and Y axis
    float radius; //float to not be too big
    sf::Color color; 
    float gravity; // float to not be too strong
    double mass;//be as fat as you want brotha
    double fps;//my worst enemy


public:
    // Constructor with radius, color, gravity, mass
    CircleBase(float radius, sf::Color color, float gravity, double mass)
        : radius(radius), color(color), gravity(gravity), mass(mass)
    {
        setRadius(radius);
        setFillColor(color);
        setOrigin(radius, radius);
        setPosition(radius,radius);
        oldPosition = sf::Vector2f(radius,radius);
        acceleration = sf::Vector2f(0, gravity*100); //(x axis, y axis)
    }
    // Constructor with radius, color, gravity, mass, position
    CircleBase(float radius, sf::Color color, sf::Vector2f pos, float gravity, double mass)
        : radius(radius), color(color), gravity(gravity), mass(mass)
    {
        setRadius(radius);
        setFillColor(color);
        setOrigin(radius, radius);
        setPosition(pos);
        oldPosition = pos;
        acceleration = sf::Vector2f(0, gravity*100);//(x axis, y axis)
    }

    void updatePosition(float dt)
    {
        sf::Vector2f currentPos = getPosition();
        sf::Vector2f velocity = currentPos - oldPosition;
        oldPosition = currentPos;// updating the current position to the old one
        sf::Vector2f newPos = currentPos + velocity + (acceleration * (dt * dt)); // Like tomy teached -> x=x0+vt+1/2*a*t^2, thanks Tomy
        setPosition(newPos);
    }

    //if you wanna apply force to the circle:
    void applyForce(sf::Vector2f force)
    {
        acceleration += force / static_cast<float>(mass); //because m*a = F --> a = F/m by newton second law
    }

    // Set shape color
    void setColor(sf::Color newColor)
    {
        color = newColor;  // Update the member variable
        setFillColor(newColor);
    }

    void handleWallCollision(sf::RenderWindow& window)
    {
        sf::Vector2f pos = getPosition();
        sf::Vector2u windowSize = window.getSize();
        float energyLossFactor = 1;// If you wanna add energy loss
        if (pos.x - radius < 0)
        {
            oldPosition.x = pos.x;
            pos.x = radius;
            //oldPosition.x = pos.x + (pos.x - oldPosition.x) * energyLossFactor; // 0.9f Damping factor
        }
        else if (pos.x + radius > windowSize.x)
        {
            oldPosition.x = pos.x;
            pos.x = windowSize.x - radius;
            //oldPosition.x = pos.x + (pos.x - oldPosition.x) * energyLossFactor;
        }

        if (pos.y - radius < 0)
        {
            oldPosition.y = pos.y;
            pos.y = radius;
            //oldPosition.y = pos.y + (pos.y - oldPosition.y) * energyLossFactor;
        }
        else if (pos.y + radius > windowSize.y)
        {
            oldPosition.y = pos.y;
            pos.y = windowSize.y - radius;
            //oldPosition.y = pos.y + (pos.y - oldPosition.y) * energyLossFactor;
        }

        setPosition(pos);
    }

    double DistanceOnly(CircleBase* otherCir) {
        sf::Vector2f pos = getPosition();
        sf::Vector2f posOther = otherCir->getPosition();  // Correct this line
        double x1 = pos.x;
        double y1 = pos.y;
        double x2 = posOther.x;
        double y2 = posOther.y;
        return sqrt(pow((y2 - y1), 2) + pow((x2 - x1), 2));
    }

    bool IsCollision(CircleBase* otherCir) {
        double distance = DistanceOnly(otherCir);
        if (distance <= radius + otherCir->radius) {
            return true;
        }
        return false;
    }

    //The collision handeling is done by seperating the two circles with a vector between the two of them, and the overlap of them. the speed that will be created is done by verlet integretion
    void handleCollision(CircleBase* otherCir) {
        if (IsCollision(otherCir)) {
            // Get positions of both circles
            sf::Vector2f pos = getPosition();
            sf::Vector2f posOther = otherCir->getPosition();

            // Calculate the distance and overlap
            double distance = DistanceOnly(otherCir);
            double overlap = (radius + otherCir->radius) - distance;

            if (overlap > 0) {
                sf::Vector2f direction = pos - posOther;//The direction vector, between center points of the circles
                float length = sqrt(direction.x * direction.x + direction.y * direction.y); // The length between the circles in scalar
                if (length > 0) { //if there is any length between them we should devide the direction by the length so it will give only the direction, like you dont say go right 5km you say go right.
                    direction /= length; // Normalize the direction vector
                }
                float massRatio = mass/otherCir->mass;
                // Move circles apart based on the overlap so they will no longer be in contact
                sf::Vector2f displacement = direction * static_cast<float>(overlap / 2.0f); // Split overlap
                pos += displacement;  // Move this circle
                posOther -= displacement * massRatio; // Move the other circle

                // Update positions
                setPosition(pos);
                otherCir->setPosition(posOther);
            }
        }
    }

    //
    void handleCollisionElastic(CircleBase* otherCir, float elastic) {
        if (IsCollision(otherCir)) {
            // Current positions
            sf::Vector2f pos = getPosition();
            sf::Vector2f posOther = otherCir->getPosition();

            // Calculate the normal vector
            sf::Vector2f normal = pos - posOther;
            float distance = std::sqrt(normal.x * normal.x + normal.y * normal.y);
            normal /= distance;  // Normalize

            // Calculate relative velocity using position difference
            sf::Vector2f velocity = pos - oldPosition;
            sf::Vector2f velocityOther = posOther - otherCir->oldPosition;
            sf::Vector2f relativeVelocity = velocity - velocityOther;

            // Calculate impulse scalar
            // Coefficient of restitution (0.8 = slightly bouncy)
            float impulseScalar = -(1 + elastic) * (relativeVelocity.x * normal.x + relativeVelocity.y * normal.y) /
                (1 / mass + 1 / otherCir->mass);// This is like friction, if this is 0 means tottaly elastic. if more than 0 than the friction will be more and more noticable.

            // Apply impulse
            sf::Vector2f impulse = normal * impulseScalar;
            sf::Vector2f newVelocity = velocity + impulse / static_cast<float>(mass);
            sf::Vector2f newVelocityOther = velocityOther - impulse / static_cast<float>(otherCir->mass);

            // Update positions and oldPositions
            oldPosition = pos;
            otherCir->oldPosition = posOther;
            setPosition(pos + newVelocity);//!This is the part that do the hit physicly accurate, we add the new velocity to the position like euler integration!
            otherCir->setPosition(posOther + newVelocityOther);//!This is the part that do the hit physicly accurate, we add the new velocity to the position like euler integration!

            // Separate circles to prevent sticking
            float overlap = (radius + otherCir->radius) - distance;
            sf::Vector2f separation = normal * (overlap / 2.0f);
            setPosition(getPosition() + separation);
            otherCir->setPosition(otherCir->getPosition() - separation);
        }
    }

    void SetRadiusAndCenter(int newRadius) {
        setRadius(newRadius);
        radius = newRadius;
        setOrigin(sf::Vector2f(radius, radius));
    }

    void SetMass(double newMass) {mass = newMass;}

    double GetMass() {return mass;}

    // Function to draw the shape, passing the render window
    void draw(sf::RenderWindow& window)
    {
        window.draw(*this);
    }

};
