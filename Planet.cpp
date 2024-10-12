#pragma once
#include <SFML/Graphics.hpp>
#include <iostream> 
#include "Circle.h"

class Planet :public Circle
{
private:
	float innerGravity;

public:
	Planet(float radius, sf::Color color, sf::Vector2f pos, float gravity, double mass, float innerGravity) : Circle(radius, color, pos, gravity, mass,sf::Vector2f(0,0)), innerGravity(innerGravity) {

	}

	// Copy constructor
	Planet(Planet& other)
		: Circle(other) {
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



	//void Gravitate(BaseShape* object) {
	//	// Calculate the vector from this object to the other object
	//	sf::Vector2f distanceVec = GetPosition() - object->GetPosition();

	//	// Calculate the distance between the two objects
	//	float distanceLength = std::sqrt(distanceVec.x * distanceVec.x + distanceVec.y * distanceVec.y);

	//	// Avoid division by zero or extremely small distances
	//	if (distanceLength == 0) {
	//		return;  // If the objects are in the same position, no force is applied
	//	}

	//	// Normalize the distance vector to get the direction of the gravitational force
	//	sf::Vector2f normalVector = distanceVec / distanceLength;

	//	// Gravitational constant
	//	const float G = 6.67430e-11f;

	//	// Calculate the gravitational force magnitude using the formula:
	//	// F = G * (m1 * m2) / d^2
	//	float forceMagnitude = (G * GetMass() * object->GetMass()) / (distanceLength * distanceLength);
	//	// Apply the force in the direction of the normalized vector
	//	object->applyForce(normalVector * forceMagnitude);
	//}



	void Gravitate(BaseShape* object)
	{
		float distance_x = GetPosition().x - object->GetPosition().x;
		float distance_y = GetPosition().y - object->GetPosition().y;

		float distance = sqrt(distance_x * distance_x + distance_y * distance_y);

		float inverse_distance = 1.f / distance;

		float normalized_x = inverse_distance * distance_x;
		float normalized_y = inverse_distance * distance_y;

		float inverse_square_dropoff = inverse_distance * inverse_distance;

		float acceleration_x = normalized_x * innerGravity * inverse_square_dropoff;
		float acceleration_y = normalized_y * innerGravity * inverse_square_dropoff;

		object->SetVelocity((object->GetVelocity().x+acceleration_x), (object->GetVelocity().y + acceleration_y));
		
		object->SetPosition(object->GetPosition() + object->GetVelocity());
	}

};
