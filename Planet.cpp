#pragma once
#include <SFML/Graphics.hpp>
#include <iostream> 
#include "CircleBase.h"

class Planet:public Circle
{
private:
	float innerGravity;

public:
	Planet(float radius, sf::Color color, sf::Vector2f pos, float gravity, double mass, float innerGravity) : Circle(radius, color, pos, gravity, mass), innerGravity(innerGravity) {
		setRadius(radius);
		setFillColor(color);
		setOrigin(radius, radius);
		setPosition(radius, radius);
		oldPosition = sf::Vector2f(radius, radius);
		acceleration = sf::Vector2f(0, gravity * 100); //(x axis, y axis)
	}

	// Copy constructor
	Planet(const Planet& other)
		: Circle(other), innerGravity(other.innerGravity) {
		// Copy SFML shape properties
		setRadius(other.getRadius());
		setFillColor(other.getFillColor());
		setOrigin(other.getOrigin());
		setPosition(other.getPosition());

		// Copy Circle class properties
		oldPosition = other.oldPosition;
		acceleration = other.acceleration;
		mass = other.mass;
		gravity = other.gravity;
	}



	void Gravitate(BaseShape* object ) {
		sf::Vector2f distanceVec = GetPosition() - object->GetPosition();
		float distanceLength = DistanceTwoPoints(object->GetPosition());
		sf::Vector2f normalVector = distanceVec / distanceLength;
		object->applyForce(normalVector*innerGravity);
	}

};

