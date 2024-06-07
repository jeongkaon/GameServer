#pragma once
#include <SFML/Graphics.hpp>

// TextBox 클래스 정의
class TextBox {
public:
    TextBox(const sf::Font& font, float x, float y, float width, float height) {
        boxShape.setSize(sf::Vector2f(width, height));
        boxShape.setPosition(x, y);
        boxShape.setFillColor(sf::Color::White);
        boxShape.setOutlineThickness(2);
        boxShape.setOutlineColor(sf::Color::Black);

        text.setFont(font);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::Black);
        text.setPosition(x , y ); // 여백 추가

        cursor.setSize(sf::Vector2f(2, text.getCharacterSize()));
        cursor.setFillColor(sf::Color::Black);
        cursor.setPosition(text.getPosition().x + text.getLocalBounds().width + 5, text.getPosition().y);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(boxShape);
        window.draw(text);
        if (isActive && showCursor) {
            window.draw(cursor);
        }
    }

    void setText(const std::string& str) {
        userInput = str;
        text.setString(userInput);
        cursor.setPosition(text.getPosition().x + text.getLocalBounds().width + 5, text.getPosition().y);
    }

    std::string getText() const {
        return userInput;
    }

    bool isClicked(const sf::Vector2i& mousePos) const {
        return boxShape.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
    }

    void addCharacter(char c) {
        userInput += c;
        text.setString(userInput);
        cursor.setPosition(text.getPosition().x + text.getLocalBounds().width + 5, text.getPosition().y);
    }

    void removeLastCharacter() {
        if (!userInput.empty()) {
            userInput.pop_back();
            text.setString(userInput);
            cursor.setPosition(text.getPosition().x + text.getLocalBounds().width + 5, text.getPosition().y);
        }
    }

    void updateCursor() {
        cursor.setPosition(text.getPosition().x + text.getLocalBounds().width + 5, text.getPosition().y);
    }

    void setActive(bool active) {
        isActive = active;
        showCursor = active; // 활성화될 때 커서 표시
    }

    void toggleCursorVisibility() {
        if (isActive) {
            showCursor = !showCursor;
        }
    }

private:
    sf::RectangleShape boxShape;
    sf::Text text;
    sf::RectangleShape cursor;
    std::string userInput;
    bool isActive = false;
    bool showCursor = true;
};

