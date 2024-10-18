#pragma once
#include "Circle.h"
#include "Rectangle.h"
#include "BaseShape.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include <random>
#include <ctime>
#include <functional>


// Definition of a generic node in the general tree 
template <typename T>
struct TreeNode {
    T data;  // Object of type T
    std::vector<TreeNode<T>*> children;  // A vector to store child nodes

    // Constructor
    explicit TreeNode(T val) : data(std::move(val)) {}

    // Destructor to free allocated memory for children
    ~TreeNode() {
        for (auto child : children) {
            delete child;  // Recursively delete all child nodes
        }
    }
};

// Function to add a child node to a parent node
template <typename T>
void addToTree(TreeNode<T>* parent, T childData) {
    TreeNode<T>* childNode = new TreeNode<T>(std::move(childData));
    parent->children.push_back(childNode);
}

// Function to search for an object in the tree
template <typename T>
TreeNode<T>* searchTree(TreeNode<T>* root, const T& target) {
    if (!root) return nullptr;  // Base case: if root is null, return nullptr
    if (root->data == target) {
        return root;  // Found the target, return the node
    }
    // Recur for each child
    for (TreeNode<T>* child : root->children) {
        TreeNode<T>* result = searchTree(child, target);
        if (result) {
            return result;  // If found in child, return the node
        }
    }
    return nullptr;  // If not found in this node or any children
}

// Function to combine two trees (tree2 becomes a child of tree1)
template <typename T>
void combineTrees(TreeNode<T>* tree1, TreeNode<T>* tree2) {
    tree1->children.push_back(tree2);
}

template <typename T>
void applyFuncToParentAndChildren(TreeNode<T>* node, std::function<void(T, T)>& func) {
    if (!node) {
        return;
    }

    for (auto child : node->children) {
        func(node->data, child->data);
        applyFuncToParentAndChildren(child, func);
    }
}



class LineLink
{
private:
    float lineLength;
    int objCount = 0;
    std::mt19937 rnd;
    std::vector<TreeNode<BaseShape*>*> objectLink;

public:

    LineLink(float lineLength) : lineLength(lineLength) {
        rnd.seed(static_cast<unsigned>(std::time(nullptr)));
    }

    ~LineLink() {
        for (auto node : objectLink) {
            delete node;
        }
    }
    void MakeNewLink(BaseShape* mainObj, BaseShape* targetObj) {
        if (targetObj->GetLinked() == -1)
        {
            TreeNode<BaseShape*>* mainObjTree;
            if (mainObj->GetLinked() == -1)
            {
                mainObjTree = new TreeNode<BaseShape*>(mainObj);  // Create a new TreeNode pointer
                objectLink.push_back(mainObjTree);
                mainObj->SetLinked(objectLink.size()-1);
                targetObj->SetLinked(objectLink.size()-1);
                addToTree(mainObjTree, targetObj);
            }
            else
            {
                TreeNode<BaseShape*>* root = objectLink[mainObj->GetLinked()];
                mainObjTree = searchTree(root, mainObj);
                targetObj->SetLinked(mainObj->GetLinked()-1);
                addToTree(mainObjTree, targetObj);
            }
        }
        else if (mainObj->GetLinked() == -1)
        {
            TreeNode<BaseShape*>* targetObjTree;
            if (targetObj->GetLinked() == -1)
            {
                targetObjTree = new TreeNode<BaseShape*>(targetObj);  // Create a new TreeNode pointer
                objectLink.push_back(targetObjTree);
                mainObj->SetLinked(objectLink.size()-1);
                targetObj->SetLinked(objectLink.size()-1);
                addToTree(targetObjTree, mainObj);
            }
            else
            {
                TreeNode<BaseShape*>* root = objectLink[targetObj->GetLinked()];
                targetObjTree = searchTree(root, targetObj);
                mainObj->SetLinked(targetObj->GetLinked()-1);
                addToTree(targetObjTree, mainObj);
            }
        }
        else
        {
            TreeNode<BaseShape*>* mainObjTree = searchTree(objectLink[mainObj->GetLinked() - 1], mainObj);
            TreeNode<BaseShape*>* targetObjTree = searchTree(objectLink[targetObj->GetLinked() - 1], targetObj);

            if (mainObjTree && targetObjTree) {
                combineTrees(mainObjTree, targetObjTree);
                // Update the linked index for all nodes in the target tree
                std::function<void(BaseShape*, BaseShape*)> updateLinkedIndex =
                    [this, mainObj](BaseShape* parent, BaseShape* child) {
                    child->SetLinked(mainObj->GetLinked()-1);
                };
                applyFuncToParentAndChildren(targetObjTree, updateLinkedIndex);
            }
        }
    }

    void ApplyLink(BaseShape* parent, BaseShape* child) {
        sf::Vector2f vector = child->GetPosition() - parent->GetPosition();
        float vectorLength = std::sqrt(vector.x * vector.x + vector.y * vector.y);
        sf::Vector2f normalizedVector = vector / vectorLength;

        float deltaLength = vectorLength - lineLength;
        sf::Vector2f correction = normalizedVector * (deltaLength * 0.5f);

        parent->SetPosition(parent->GetPosition() + correction);
        child->SetPosition(child->GetPosition() - correction);

        // Apply a small upward force to counteract gravity
        float upwardForce = 9.8f * 0.1f; // Adjust this value to change the behavior of the string
        parent->applyForce(sf::Vector2f(0, upwardForce));
        child->applyForce(sf::Vector2f(0, -upwardForce));

        //shape1->updatePosition(1.0f / fps);
        //shape2->updatePosition(1.0f / fps);
        //if (!objectLink.empty()) {
        //    objectLink.back()->data->applyForce(sf::Vector2f(0, 9.8f * 0.2f)); // apply extra downward force to the last shape
        //}
    }

    void ApplyAllLinks() {
        for (auto& node : objectLink) {
            std::function<void(BaseShape*, BaseShape*)> applyLinkFunc =
                [this](BaseShape* parent, BaseShape* child) {
                this->ApplyLink(parent, child);
            };
            applyFuncToParentAndChildren(node, applyLinkFunc);
        }
    }

    void Draw(sf::RenderWindow& window)
    {
        if (objectLink.size() > 1) {
            ApplyAllLinks();

            for (const auto& node : objectLink) {
                node->data->draw(window);
            }

            // Draw lines between shapes to visualize the string
            sf::VertexArray lines(sf::Lines, (objectLink.size() - 1) * 2);
            for (size_t i = 0; i < objectLink.size() - 1; ++i) {
                lines[i * 2].position = objectLink[i]->data->GetPosition();
                lines[i * 2 + 1].position = objectLink[i + 1]->data->GetPosition();
                lines[i * 2].color = sf::Color::White;
                lines[i * 2 + 1].color = sf::Color::White;
            }
            window.draw(lines);
        }
        
    }
};
