#pragma once
#include "Circle.h"
#include "Rectangle.h"
#include "BaseShape.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <random>
#include <ctime>
#include <iostream>
#include <functional>


class LineLink
{
private:
    float lineLength;
    std::unordered_map<BaseShape*,std::vector<BaseShape*>> connections;
    std::vector<BaseShape*> allObjects;
    std::mt19937 rng; // Random number generator

public:

    LineLink(float lineLength) : lineLength(lineLength) {
        rng.seed(std::time(nullptr));
    }
    ~LineLink() {
        allObjects.clear();
    }
    void ClearLinks() {
        allObjects.clear();
        connections.clear();
    }

    void AddObject(BaseShape* obj) {
        if (std::find(allObjects.begin(), allObjects.end(), obj) == allObjects.end())  {//if no connections then add to the objects that need to be connected
            allObjects.push_back(obj);
        }
    }

    void MakeNewLink(BaseShape* obj1, BaseShape* obj2) {
        //check if objects already connected:
        AddObject(obj1);
        AddObject(obj2);
        //add each other as connected
        connections[obj1].push_back(obj2);
        connections[obj2].push_back(obj1);
    }

    void ConnectAll() {
        for (size_t i = 0; i < allObjects.size(); i++) {
            for (size_t j = i + 1; j < allObjects.size(); j++) {
                MakeNewLink(allObjects[i], allObjects[j]);
            }
        }
    }

    void ConnectChain() {
        for (size_t i = 0; i < allObjects.size() - 1; i++) {
            MakeNewLink(allObjects[i], allObjects[i + 1]);
        }
    }

    void ConnectStar() {
        if (!allObjects.empty()) {
            for (size_t i = 1; i < allObjects.size(); i++) {
                MakeNewLink(allObjects[0], allObjects[i]);
            }
        }
    }

    void ConnectRandom(int numConnections) {
        if (allObjects.size() < 2) return; // Need at least 2 objects to make connections

        std::uniform_int_distribution<int> dist(0, allObjects.size() - 1);

        for (int i = 0; i < numConnections; ++i) {
            int index1 = dist(rng);
            int index2;
            do {
                index2 = dist(rng);
            } while (index2 == index1); // Ensure we don't connect an object to itself

            BaseShape* obj1 = allObjects[index1];
            BaseShape* obj2 = allObjects[index2];

            // Check if this connection already exists
            if (std::find(connections[obj1].begin(), connections[obj1].end(), obj2) == connections[obj1].end()) {
                MakeNewLink(obj1, obj2);
            }
        }
    }

    void ApplyLink(BaseShape* obj1, BaseShape* obj2) {
        sf::Vector2f vector = obj2->GetPosition() - obj1->GetPosition();
        float vectorLength = std::sqrt(vector.x * vector.x + vector.y * vector.y);
        // Prevent division by zero
        if (vectorLength < 0.0001f) {
            return;
        }
        sf::Vector2f normalizedVector = vector / vectorLength;
        float deltaLength = vectorLength - lineLength;
        // Limit the maximum correction to prevent extreme movements
        float maxCorrection = 50.f; // Adjust this value as needed
        float correctionMagnitude = std::min(std::abs(deltaLength * 0.5f), maxCorrection);
        // Calculate correction vector component-wise
        sf::Vector2f correction(
            normalizedVector.x * correctionMagnitude * (deltaLength > 0 ? 1 : -1),
            normalizedVector.y * correctionMagnitude * (deltaLength > 0 ? 1 : -1)
        );
        // Apply correction with damping
        float damping = 0.8; // Adjust this value between 0 and 1
        obj1->SetPosition(obj1->GetPosition() + sf::Vector2f(correction.x * damping, correction.y * damping));
        obj2->SetPosition(obj2->GetPosition() - sf::Vector2f(correction.x * damping, correction.y * damping));
        // Apply a small force instead of directly modifying velocity
        float forceMagnitude = 0.1f; // Adjust this value as needed
        sf::Vector2f force(normalizedVector.x * forceMagnitude, normalizedVector.y * forceMagnitude);
        obj1->applyForce(sf::Vector2f(-force.x, -force.y));
        obj2->applyForce(force);
    }

    void ApplyAllLinks() {
        for (const auto& pair : connections) {
            BaseShape* obj1 = pair.first;
            for (BaseShape* obj2 : pair.second) {
                ApplyLink(obj1, obj2);
            }
        }
    }

    void Draw(sf::RenderWindow& window) {
        ApplyAllLinks();

        sf::VertexArray lines(sf::Lines);
        for (const auto& pair : connections) {
            BaseShape* obj1 = pair.first;
            for (BaseShape* obj2 : pair.second) {
                lines.append(sf::Vertex(obj1->GetPosition(), sf::Color::White));
                lines.append(sf::Vertex(obj2->GetPosition(), sf::Color::White));
            }
        }
        window.draw(lines);
    }

    void Clear() {
        connections.clear();
        allObjects.clear();
    }
};
