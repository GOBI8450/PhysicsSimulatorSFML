#pragma once
#include <SFML/Graphics.hpp>

class Button : public sf::RectangleShape  // Note: public inheritance
{
private:
    float width;
    float height;
    std::string name;
    bool isHovered = false;
    const float scaleAmount = 1.05f;
    sf::Vector2f originalScale;

public:
    Button(float width, float height, sf::Vector2f pos, std::string name)
        : width(width), height(height), name(name)
    {
        setSize(sf::Vector2f(width, height));
        setOrigin(width / 2, height / 2);
        setPosition(pos);
        originalScale = getScale();
    }

    void SetTexture(const sf::Texture& newTexture) {
        setTexture(&newTexture);
    }

    bool IsInRadius(sf::Vector2f mousePos) {
        sf::Vector2f pos = getPosition();
        float scaledWidth = width * getScale().x;
        float scaledHeight = height * getScale().y;

        return mousePos.x >= pos.x - scaledWidth / 2 &&
            mousePos.x <= pos.x + scaledWidth / 2 &&
            mousePos.y >= pos.y - scaledHeight / 2 &&
            mousePos.y <= pos.y + scaledHeight / 2;
    }

    bool MouseHover(sf::Vector2f mousePos, bool& hovering) {
        bool wasHovered = isHovered;
        isHovered = IsInRadius(mousePos);

        // Only change scale if hover state changed
        if (isHovered != wasHovered) {
            if (isHovered) {
                // Scale up
                setScale(originalScale.x * scaleAmount, originalScale.y * scaleAmount);
                hovering = true;
            }
            else {
                // Reset to original scale
                setScale(originalScale);
                hovering = false;
            }
        }

        return isHovered;
    }

    std::string GetName() {
        return name;
    }

    void Update(sf::Vector2f mousePos, bool& hovering) {
        MouseHover(mousePos, hovering);
    }

    void SetScale(float scale) {
        originalScale = sf::Vector2f(scale, scale);
        setScale(originalScale);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(*this);
    }
};
