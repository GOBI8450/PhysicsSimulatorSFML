#pragma once
#include <unordered_map>;
#include <iostream> 
using namespace std;
#include "CircleBase.h";
#include "Rectangle.h"
#include <random>;  // For random number generation
#include <ctime>;   // For seeding with current time

class Grid //Using Hash Mapping
{
private:
	std::unordered_map<int, std::vector<BaseShape*>> gridMap; // Changed Circle* to BaseShape*
	std::mt19937 rnd; // random variable
	int ballCount = 0;
	std::vector<int> hashKeyVec;

	// Hash Function for unordered_map
	int hashFunction(int x, int y) const {
		return x + y * 1000000007; // hashing this, prime number to make y stand out from x so no hash collision
	}

public:
	Grid() {}; // Constructor

	// Insert the circle by getting the hash Key and putting it in the unordered map 
	void InsertObj(BaseShape* obj) { // Changed Circle* to BaseShape*
		sf::Vector2f pos = obj->GetPosition();
		float multiplier = 2.5; // !still need to work on the multiplier for best performance!
		int gridX;
		int gridY;
		if (Circle* newCircle = dynamic_cast<Circle*>(obj)) {
			gridX = static_cast<int>(pos.x / (newCircle->getRadius() * multiplier));
			gridY = static_cast<int>(pos.y / (newCircle->getRadius() * multiplier));
		}
		else if (Rectangle* rectangle = dynamic_cast<Rectangle*>(obj)) {
			gridX = static_cast<int>(pos.x / (rectangle->GetWidth() * multiplier));
			gridY = static_cast<int>(pos.y / (rectangle->GetHeight() * multiplier));
		}

		int hashKey = hashFunction(gridX, gridY);
		hashKeyVec.push_back(hashKey);
		gridMap[hashKey].push_back(obj);
	}

	// Clears the gridMap 
	void clear() {
		gridMap.clear();
	}

	// Returns the hash map size
	int GetHashMapSize() {
		return gridMap.size();
	}

	// Because hashing is non-reversible we give all the keys for external use to easy access to every gridMap vectors
	std::vector<int> GetAllHashKeys() {
		return hashKeyVec;
	}

	// Turn a hash keys vector to a vector that contains vectors that contains BaseShape*
	std::vector<std::vector<BaseShape*>> GetCircelsVectorOfVectorsFromKeyVectors(std::vector<int> hashKeysVec) {
		std::vector<std::vector<BaseShape*>> vectorsOfCircleBaseVector;
		for (size_t currentKey = 0; currentKey < hashKeysVec.size(); currentKey++) {
			vectorsOfCircleBaseVector.push_back(gridMap[hashKeysVec[currentKey]]);
		}
		return vectorsOfCircleBaseVector;
	}

	// This function returns us all the nearby cells to a circle, this way we can handle collisions only in grids near
	std::vector<BaseShape*> GetNerbyCellsObjects(BaseShape* obj) { // Changed Circle* to BaseShape*
		std::vector<BaseShape*> nerbyCellsVector;
		sf::Vector2f pos = obj->GetPosition();
		float multiplier = 2.5; // !still need to work on the multiplier for best performance!
		// Get us the place on the 2D dimension, like (2,2), (69,34)... like pos.x=1000 and gridSize=500 so gridX is the second cell 
		int gridX;
		int gridY;
		if (Circle* circle = dynamic_cast<Circle*>(obj)) {
			gridX = static_cast<int>(pos.x / (circle->getRadius() * multiplier));
			gridY = static_cast<int>(pos.y / (circle->getRadius() * multiplier));
		}
		else if(Rectangle* rectangle = dynamic_cast<Rectangle*>(obj))
		{
			gridX = static_cast<int>(pos.x / (rectangle->GetWidth() * multiplier));
			gridY = static_cast<int>(pos.y / (rectangle->GetHeight() * multiplier));
		}
		// We go over all the cells near the circle cell: the one up, the one down, and the one right and the one left, working like a two-dimensional array
		for (short int y = -1; y <= 1; y++) { // we do this because it is the cells near him. like if the cell cord is (3,3). so in x axis his neighbors are (4,3) and (2,3)
			for (short int x = -1; x <= 1; x++) { // we do this because it is the cells near him. like if the cell cord is (3,3). so in y axis his neighbors are (3,2) and (3,4)
				int hashKey = hashFunction(gridX + x, gridY + y); // we make the nearby cell to hashFunction to work with it
				if (gridMap.find(hashKey) != gridMap.end()) { // if not found will point to the end of the grid map, and if found it will show an iterator.
					nerbyCellsVector.insert(nerbyCellsVector.end(), gridMap[hashKey].begin(), gridMap[hashKey].end()); // Insert the ball at the end of the vector, because there might be a few circles in the same area, we take all of them, that's why gridMap[hashKey].begin() -> gridMap[hashKey].end() to take all the ones in the same cell
				}
			}
		}
		return nerbyCellsVector; // returns a vector of all the cells that have objects in them
	}

	sf::Vector2f Vector2iToVector2f(sf::Vector2i pointPos) {
		return sf::Vector2f(static_cast<float>(pointPos.x), static_cast<float>(pointPos.y));
	}

	//Finds if a point is landing on a specific circle. for mouse detection
	BaseShape* IsInSpecificGridRadius(sf::Vector2i pointPos, int hashKey) {
		std::vector<BaseShape*> objVec = gridMap[hashKey];
		for (auto& obj : objVec) {
			sf::Vector2f pointPosf = Vector2iToVector2f(pointPos);
			sf::Vector2f objPos = obj->GetPosition();
			double distance = std::sqrt(std::pow(objPos.x - pointPosf.x, 2) + std::pow(objPos.y - pointPosf.y, 2));
			if (Circle* circle = dynamic_cast<Circle*>(obj)) {
				if (distance <= circle->getRadius()) {
					return obj; // Return a pointer to the circle as a baseShape if the point is within the radius
				}
			}
			else if (Rectangle* rectangle = dynamic_cast<Rectangle*>(obj)) {
				if (rectangle->IsCollision(pointPosf)) {
					return obj; // Return a pointer to the circle as a baseShape if the point is within the radius
				}
			}
		}
		
		return nullptr; // Return nullptr if no ball contains the point
	}

	//Finds if a point is landing on a specific grid. for mouse detection
	BaseShape* IsInGridRadius(sf::Vector2i pointPos) {
		BaseShape* shapePointer;
		for (auto& keyAndCircle : gridMap) {
			int hashKey = keyAndCircle.first;
			 shapePointer = IsInSpecificGridRadius(pointPos, hashKey);
			if (shapePointer!=nullptr)
			{
				return shapePointer;
			}
		}
		return nullptr; // Return nullptr if no ball contains the point
	}
};
