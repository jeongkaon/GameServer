#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>



class Button {
public:
    sf::RectangleShape buttonShape;
    sf::Text buttonText;

public:
    Button();
    Button(const std::string& text, const sf::Font& font, float x, float y);

    void draw(sf::RenderWindow& window) const {
        window.draw(buttonShape);
        window.draw(buttonText);
    }

    bool isClicked(const sf::Vector2i& mousePos) const {
        return buttonShape.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
    }

    std::string getText() const {
        return buttonText.getString();
    }


};

class CharacterSelectButton : public Button {
public:

    int _buttonNumber;

private:
    static int totalButtonNumber;

public:
    CharacterSelectButton();
    CharacterSelectButton(const std::string& text, const sf::Font& font, float x, float y);

};


class RegisterButton :public Button 
{
    std::string _id;
    std::string _pw;

public:
    RegisterButton();
    RegisterButton(const std::string& text, const sf::Font& font, float x, float y);

    bool GetUserInfo(std::string id, std::string pw);


};


class LoginrButton :public Button
{
    std::string _id;
    std::string _pw;

public:
    LoginrButton();
    LoginrButton(const std::string& text, const sf::Font& font, float x, float y);

    bool CheckId(std::string id, std::string pw);


};

class CloseButton :public Button
{
  
public:
    CloseButton();
    CloseButton(const std::string& text, const sf::Font& font, float x, float y);

    bool CheckId(std::string id, std::string pw);


};