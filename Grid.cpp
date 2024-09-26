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
	std::vector<int> hashKeyVec;

	//Hash Function for unordered_map
	int hashFunction(int x, int y) const {
		return x + y * 1000000007; //hashing this, prime number to make y stand out from x so no hash collision
	}


public:
	Grid() {}; //Constructor

	//Insert the circle by getting the hash Key and putting it in the unordered map 
	void InsertCircle(CircleBase* circle) {
		sf::Vector2f pos = circle->getPosition();
		float multiplier = 2.5;// !still need to work on the multipier for best performance!
		int gridX = static_cast<int>(pos.x / (circle->getRadius() * multiplier)); 
		int gridY = static_cast<int>(pos.y / (circle->getRadius() * multiplier));
		int hashKey = hashFunction(gridX, gridY);
		hashKeyVec.push_back(hashKey);
		gridMap[hashKey].push_back(circle);
	}
	
	//Clears the gridMap 
	void clear() {
		gridMap.clear();
	}

	//returns the hash map size
	int GetHashMapSize() {
		return gridMap.size();
	}

	//because hashing is non reversable we give all the keys for external use to easy accsess to every gridMap vectors
	std::vector<int> GetAllHashKeys() {
		return hashKeyVec;
	}

	//turn a hash keys vector to a vector that contains vectors that contains CircleBase
	std::vector<std::vector<CircleBase*>> GetCircelsVectorOfVectorsFromKeyVectors(std::vector<int> hashKeysVec) {
		std::vector<std::vector<CircleBase*>> vectorsOfCircleBaseVector;
		for (size_t currentKey = 0; currentKey < hashKeysVec.size(); currentKey++)
		{
			vectorsOfCircleBaseVector.push_back(gridMap[hashKeysVec[currentKey]]);
		}
		return vectorsOfCircleBaseVector;
	}

	//This function returns us all the nerby cells to a circle, this way we can handle collisons only in grids near
	std::vector<CircleBase*> GetNerbyCellsObjects(CircleBase* circle) {
		std::vector<CircleBase*> nerbyCellsVector;
		sf::Vector2f pos = circle->getPosition();
		float multiplier = 2.5;// !still need to work on the multipier for best performance!
		//Get us the place on the 2d dimension, like (2,2), (69,34)... like pos.x=1000 and gridSize=500 so gridX is the second cell 
		int gridX = static_cast<int>(pos.x / (circle->getRadius() * multiplier));
		int gridY = static_cast<int>(pos.y / (circle->getRadius() * multiplier));
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

	//Finds if a point is landing on a specific circle. for mouse detection
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

	//Finds if a point is landing on a specific grid. for mouse detection
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
