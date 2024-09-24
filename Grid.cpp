#pragma once
#include <unordered_map>;
#include <iostream> using namespace std;
#include "CircleBase.h";
#include <random>;  // For random number generation
#include <ctime>;   // For seeding with current time

class Grid //Using Hash Maping
{
private:
	std::unordered_map<int, std::vector<CircleBase*>> gridMap;// int for Key, and std::vector<CircleBase*>> for type of object
	std::mt19937 rnd;//random variable
	int ballCount = 0;

	int hashFunction(int x, int y) const {
		return x + y * 1000000007; //hashing this, prime number to make y stand out from x so no hash collision
	}


public:
	Grid() {};

	void InsertCircle(CircleBase* circle) {
		sf::Vector2f pos = circle->getPosition();
		int gridX = static_cast<int>(pos.x / (circle->getRadius() * 2.5));
		int gridY = static_cast<int>(pos.y / (circle->getRadius() * 2.5));
		int hashKey = hashFunction(gridX, gridY);
		gridMap[hashKey].push_back(circle);
	}

	void clear() {
		gridMap.clear();
	}

	int GetHashMapSize() {
		return gridMap.size();
	}

	std::vector<CircleBase*> GetNerbyCellsObjects(CircleBase* circle) {
		std::vector<CircleBase*> nerbyCellsVector;
		sf::Vector2f pos = circle->getPosition();
		//Get us the place on the 2d dimension, like (2,2), (69,34)... like pos.x=1000 and gridSize=500 so gridX is the second cell 
		int gridX = static_cast<int>(pos.x / (circle->getRadius() * 2.5));
		int gridY = static_cast<int>(pos.y / (circle->getRadius() * 2.5));
		//we go over all the cells near the circle cell. the one up and the one down, and the one right and the one left. working like two dimensional array
		for (short int y = -1; y <= 1; y++) // we do this because it is the cells near him. like if the cell cord is (3,3). so in x axis his neighbores is (4,3) and (2,3)
		{
			for (short int x = -1; x <= 1; x++)// we do this because it is the cells near him. like if the cell cord is (3,3). so in y axis his neighbores is (3,2) and (3,4)
			{
				int hashKey = hashFunction(gridX + x, gridY + y);//we make the nerby cell to hashFunction to work with it
				if (gridMap.find(hashKey) != gridMap.end()) {//if not found will point to the end of the grid map. and if found it will show an iterator.
					nerbyCellsVector.insert(nerbyCellsVector.end(), gridMap[hashKey].begin(), gridMap[hashKey].end()); //Insert the ball at the end of the vector, because there might be few circles in the same are we take all of them, thats means gridMap[hashKey].begin() -> gridMap[hashKey].end() so it can take all the ones in the same cell
				}
			}
		}
		return nerbyCellsVector; //returns a vector of all the cells that have objects in
	}

	CircleBase* IsInSpecificGridRadius(sf::Vector2i pointPos, int hashKey) {
		std::vector<CircleBase*> circleVec = gridMap[hashKey];
		for (auto& circle : circleVec) {
			sf::Vector2f CirclePos = circle->getPosition();
			double distance = std::sqrt(std::pow(CirclePos.x - pointPos.x, 2) + std::pow(CirclePos.y - pointPos.y, 2));

			if (distance <= circle->getRadius()) {
				return circle; // Return a pointer to the ball if the point is within the radius
			}
		}
		
		return nullptr; // Return nullptr if no ball contains the point
	}

	CircleBase* IsInGridRadius(sf::Vector2i pointPos) {
		CircleBase* circlePointer;
		for (auto& keyAndCircle : gridMap) {
			int hashKey = keyAndCircle.first;
			 circlePointer = IsInSpecificGridRadius(pointPos, hashKey);
			if (circlePointer!=nullptr)
			{
				return circlePointer;
			}
		}
		return nullptr; // Return nullptr if no ball contains the point
	}
};
