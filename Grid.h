#pragma once
#include <unordered_map>
#include <iostream> 
#include "CircleBase.h"
#include "Rectangle.h"
#include <random>  // For random number generation
#include <ctime>   // For seeding with current time

class Grid {
protected:
	std::mt19937 rnd; // random variable
	int ballCount = 0;

public:
	virtual ~Grid() = default;

	Grid(){};

	// Insert the circle by getting the hash Key and putting it in the unordered map 
	virtual void InsertObj(BaseShape* obj) {}

	// Clears the gridMap 
	virtual void clear() {}

	// This function returns us all the nearby cells to a circle, this way we can handle collisions only in grids near
	virtual std::vector<BaseShape*> GetNerbyCellsObjects(BaseShape* obj) { // Changed Circle* to BaseShape*
		return std::vector<BaseShape*>();
	}

	virtual int GetGridColumn(BaseShape* obj) {
		int gridRow;
		return gridRow;
	}

	virtual int GetGridRow(BaseShape* obj) {
		int gridRow;
		return gridRow;
	}

	virtual sf::Vector2f Vector2iToVector2f(sf::Vector2i pointPos) {
		return sf::Vector2f(static_cast<float>(pointPos.x), static_cast<float>(pointPos.y));
	}

	//Finds if a point is landing on a specific grid. for mouse detection
	virtual BaseShape* IsInGridRadius(sf::Vector2f pointPos) {
		return nullptr; // Return nullptr if no ball contains the point
	}


	virtual sf::RectangleShape createGridVisually(const sf::Vector2f& size, const sf::Vector2f& position, float outlineThickness, sf::Color outlineColor) { return sf::RectangleShape(); }

	virtual void DrawGrids(sf::RenderWindow& window) {}
};

class GridUnorderd : public Grid //Using Hash Mapping
{
private:
	std::unordered_map<int, std::vector<BaseShape*>> gridMap; // Changed Circle* to BaseShape*
	std::vector<int> hashKeyVec;
	float multiplier = 2.5; // !still need to work on the multiplier for best performance!

	// Hash Function for unordered_map
	int hashFunction(int column, int row) const {
		return column + row * 1000000007; // hashing this, prime number to make y stand out from x so no hash collision
	}

public:
	GridUnorderd() : Grid() {}; // Constructor

	// Insert the circle by getting the hash Key and putting it in the unordered map 
	void InsertObj(BaseShape* obj) override {
		sf::Vector2f pos = obj->GetPosition();
		int gridColumn = GetGridColumn(obj);
		int gridRow = GetGridRow(obj);

		int hashKey = hashFunction(gridColumn, gridRow);
		hashKeyVec.push_back(hashKey);
		gridMap[hashKey].push_back(obj);
	}

	// Clears the gridMap 
	void clear() override {
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
	std::vector<BaseShape*> GetNerbyCellsObjects(BaseShape* obj) override { // Changed Circle* to BaseShape*
		std::vector<BaseShape*> nerbyCellsVector;
		sf::Vector2f pos = obj->GetPosition();
		// Get us the place on the 2D dimension, like (2,2), (69,34)... like pos.x=1000 and gridSize=500 so gridX is the second cell 
		int gridColumn = GetGridColumn(obj);
		int gridRow = GetGridRow(obj);
		// We go over all the cells near the circle cell: the one up, the one down, and the one right and the one left, working like a two-dimensional array
		for (short int otherRow = -1; otherRow <= 1; otherRow++) { // we do this because it is the cells near him. like if the cell cord is (3,3). so in x axis his neighbors are (4,3) and (2,3)
			for (short int otherColumn = -1; otherColumn <= 1; otherColumn++) { // we do this because it is the cells near him. like if the cell cord is (3,3). so in y axis his neighbors are (3,2) and (3,4)
				int hashKey = hashFunction(gridColumn + otherColumn, gridRow + otherRow); // we make the nearby cell to hashFunction to work with it
				if (gridMap.find(hashKey) != gridMap.end()) { // if not found will point to the end of the grid map, and if found it will show an iterator.
					nerbyCellsVector.insert(nerbyCellsVector.end(), gridMap[hashKey].begin(), gridMap[hashKey].end()); // Insert the ball at the end of the vector, because there might be a few circles in the same area, we take all of them, that's why gridMap[hashKey].begin() -> gridMap[hashKey].end() to take all the ones in the same cell
				}
			}
		}
		return nerbyCellsVector; // returns a vector of all the cells that have objects in them
	}

	int GetGridColumn(BaseShape* obj) override {
		int gridColumn;
		sf::Vector2f pos = obj->GetPosition();
		if (Circle* circle = dynamic_cast<Circle*>(obj)) {
			gridColumn = static_cast<int>(pos.x / (circle->getRadius() * multiplier));
		}
		else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj))
		{
			gridColumn = static_cast<int>(pos.x / (rectangle->GetWidth() * multiplier));
		}
		return gridColumn;
	}

	int GetGridRow(BaseShape* obj) override {
		int gridRow;
		sf::Vector2f pos = obj->GetPosition();
		if (Circle* circle = dynamic_cast<Circle*>(obj)) {
			gridRow = static_cast<int>(pos.y / (circle->getRadius() * multiplier));
		}
		else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj))
		{
			gridRow = static_cast<int>(pos.y / (rectangle->GetHeight() * multiplier));
		}
		return gridRow;
	}

	//sf::Vector2f GetGridSize(BaseShape* obj, sf::RenderWindow& window) {
	//	int gridColumn = GetGridColumn(obj);
	//	int gridRow = GetGridRow(obj);
	//	return sf::Vector2f(window.getSize().x/, window.getSize().y);
	//}

	//Finds if a point is landing on a specific object. for mouse detection
	BaseShape* IsInSpecificGridRadius(sf::Vector2f pointPosf, int hashKey) {
		std::vector<BaseShape*> objVec = gridMap[hashKey];
		for (auto& obj : objVec) {
			sf::Vector2f objPos = obj->GetPosition();
			double distance = std::sqrt(std::pow(objPos.x - pointPosf.x, 2) + std::pow(objPos.y - pointPosf.y, 2));
			if (Circle* circle = dynamic_cast<Circle*>(obj)) {
				if (distance <= circle->getRadius()) {
					return obj; // Return a pointer to the circle as a baseShape if the point is within the radius
				}
			}
			else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
				if (rectangle->IsCollision(pointPosf)) {
					return obj; // Return a pointer to the circle as a baseShape if the point is within the radius
				}
			}
		}

		return nullptr; // Return nullptr if no ball contains the point
	}

	//Finds if a point is landing on a specific grid. for mouse detection
	BaseShape* IsInGridRadius(sf::Vector2f pointPos) override {
		BaseShape* shapePointer;
		for (auto& keyAndCircle : gridMap) {
			int hashKey = keyAndCircle.first;
			shapePointer = IsInSpecificGridRadius(pointPos, hashKey);
			if (shapePointer != nullptr)
			{
				return shapePointer;
			}
		}
		return nullptr; // Return nullptr if no ball contains the point
	}

	sf::RectangleShape createGridVisually(const sf::Vector2f& size, const sf::Vector2f& position, float outlineThickness, sf::Color outlineColor) override {
		sf::RectangleShape rectangle(size); // Set size
		rectangle.setPosition(position);    // Set position
		rectangle.setOutlineThickness(outlineThickness); // Set outline thickness
		rectangle.setOutlineColor(outlineColor);         // Set outline color
		rectangle.setFillColor(sf::Color::Transparent);  // Make it transparent

		return rectangle;
	}

	void DrawGrids(sf::RenderWindow& window) override {
		for (auto& keyAndObject : gridMap) {
			int hashKey = keyAndObject.first;
			std::vector<BaseShape*> objVec = keyAndObject.second;
			BaseShape* obj = objVec.front();
			sf::RectangleShape gridRect;
			if (Circle* circle = dynamic_cast<Circle*>(obj)) {
				gridRect = createGridVisually(sf::Vector2f(circle->GetRadius()*2, circle->GetRadius()*2), obj->GetPosition(), 3.0, sf::Color(255, 0, 0));
				gridRect.setOrigin(circle->GetRadius(), circle->GetRadius());
			}
			else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
				gridRect = createGridVisually(sf::Vector2f(rectangle->getSize().x, rectangle->getSize().y), obj->GetPosition(), 3.0, sf::Color(255, 0, 0));
			}
			window.draw(gridRect);		
		}
	}
};

class GridFixed :public Grid {
private:
	std::vector<std::vector<std::vector<BaseShape*>>> grids;
	int gridSize = 3;

public:
	GridFixed(): Grid() {}

	void InsertObj(BaseShape* obj) override {
		int gridColumn = GetGridColumn(obj);
		int gridRow = GetGridRow(obj);
		grids[gridRow][gridColumn].push_back(obj);
		ballCount += 1;
	}

	int GetGridColumn(BaseShape* obj) override {
		int gridColumn;
		sf::Vector2f pos = obj->GetPosition();
		if (Circle* circle = dynamic_cast<Circle*>(obj)) {
			gridColumn = static_cast<int>(pos.x / (circle->getRadius()));
		}
		else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj))
		{
			gridColumn = static_cast<int>(pos.x / (rectangle->GetWidth()));
		}
		return gridColumn;
	}

	int GetGridRow(BaseShape* obj) override {
		int gridRow;
		sf::Vector2f pos = obj->GetPosition();
		if (Circle* circle = dynamic_cast<Circle*>(obj)) {
			gridRow = static_cast<int>(pos.y / (circle->getRadius()));
		}
		else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj))
		{
			gridRow = static_cast<int>(pos.y / (rectangle->GetHeight()));
		}
		return gridRow;
	}

	std::vector<BaseShape*> GetNerbyCellsObjects(BaseShape* obj) override { // Changed Circle* to BaseShape*
		std::vector<BaseShape*> nerbyCellsVector;
		sf::Vector2f pos = obj->GetPosition();
		int gridColumn = GetGridColumn(obj);
		int gridRow = GetGridRow(obj);
		// We go over all the cells near the circle cell: the one up, the one down, and the one right and the one left, working like a two-dimensional array
		for (short int thisRow = gridRow - 1; thisRow <= gridRow + 1; thisRow++) { // we do this because it is the cells near him. like if the cell cord is (3,3). so in x axis his neighbors are (4,3) and (2,3)
			for (short int thisColumn = gridColumn - 1; thisColumn <= gridColumn + 1; thisColumn++) { // we do this because it is the cells near him. like if the cell cord is (3,3). so in y axis his neighbors are (3,2) and (3,4)
				if (thisRow >= 0 && thisColumn >= 0)
				{
					nerbyCellsVector.insert(nerbyCellsVector.end(), grids[thisRow, thisColumn].begin(), grids[thisRow, thisColumn].end());
				}
			}
		}
		return nerbyCellsVector; // returns a vector of all the cells that have objects in them
	}

	//Finds if a point is landing on a specific object. for mouse detection
	BaseShape* IsInGridRadius(sf::Vector2f pointPosf) override {
		int gridColumn = pointPosf.x/ gridSize;
		int gridRow = pointPosf.y / gridSize;
		std::vector<BaseShape*> objVec = grids[gridRow][gridColumn];
		for (auto& obj : objVec) {
			sf::Vector2f objPos = obj->GetPosition();
			double distance = std::sqrt(std::pow(objPos.x - pointPosf.x, 2) + std::pow(objPos.y - pointPosf.y, 2));
			if (Circle* circle = dynamic_cast<Circle*>(obj)) {
				if (distance <= circle->getRadius()) {
					return obj; // Return a pointer to the circle as a baseShape if the point is within the radius
				}
			}
			else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
				if (rectangle->IsCollision(pointPosf)) {
					return obj; // Return a pointer to the circle as a baseShape if the point is within the radius
				}
			}
		}

		return nullptr; // Return nullptr if no ball contains the point
	}

	void clear() override {
		grids.clear();
	}
};
