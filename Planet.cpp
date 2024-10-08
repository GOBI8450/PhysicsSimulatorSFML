#pragma once
#include <SFML/Graphics.hpp>
#include <iostream> 
#include "CircleShape.h"
#include "BaseShape.h"


// Base_Shape class inheriting from sf::Transformable
class Planet : public CircleShape
{

private:
    float innerGravity; //float to not be too big


public:
   Planet(sf::Color color, float gravity, double mass, float radius, float innerGravity)
        : Circle(color, gravity, mass,radius), innerGravity(innerGravity)
    {
        setRadius(radius);
        setFillColor(color);
        setOrigin(radius, radius);
        setPosition(radius,radius);
        oldPosition = sf::Vector2f(radius,radius);
        acceleration = sf::Vector2f(0, gravity * 100); //(x axis, y axis);
    }

    void ActiveGravityToObj(BaseShape* obj){
      float gravityForce=obj.mass * 6.6743 * pow(10,11);
      applyForce(gravityForce);      
    }

    void UpdateGravity(BaseShape* obj){
      
    }
}


