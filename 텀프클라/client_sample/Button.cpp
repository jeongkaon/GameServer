#include "Button.h"


Button::Button()
{
    buttonShape.setSize(sf::Vector2f(200, 50));
    buttonShape.setPosition(0,0);
   // buttonShape.setFillColor(sf::Color::White);

    //buttonText.setFont(font);
    buttonText.setString("default");
    buttonText.setCharacterSize(24);
    buttonText.setFillColor(sf::Color::Black);
    buttonText.setPosition(
        +(200 - buttonText.getLocalBounds().width) / 2,
        +(50 - buttonText.getLocalBounds().height) / 2
    );
}

Button::Button(const std::string& text, const sf::Font& font, float x, float y)
{
    buttonShape.setSize(sf::Vector2f(150, 30));
    buttonShape.setPosition(x, y);
   // buttonShape.setFillColor(sf::Color::White);

    buttonText.setFont(font);
    buttonText.setString(text);
    buttonText.setCharacterSize(24);
    buttonText.setFillColor(sf::Color::Black);
    buttonText.setPosition(
        x + (150 - buttonText.getLocalBounds().width) / 2, y);


}

int CharacterSelectButton::totalButtonNumber = 1;

CharacterSelectButton::CharacterSelectButton()
{
    _buttonNumber = totalButtonNumber++;

}

CharacterSelectButton::CharacterSelectButton(const std::string& text, const sf::Font& font, float x, float y)
{
    _buttonNumber = totalButtonNumber++;
    buttonShape.setSize(sf::Vector2f(150, 30));
    buttonShape.setPosition(x, y);
   // buttonShape.setFillColor(sf::Color::White);

    buttonText.setFont(font);
    buttonText.setString(text);
    buttonText.setCharacterSize(24);
    buttonText.setFillColor(sf::Color::Black);
    buttonText.setPosition(
        x + (150 - buttonText.getLocalBounds().width) / 2, y);

}


RegisterButton::RegisterButton()
{
    buttonShape.setSize(sf::Vector2f(130, 50));
    buttonShape.setPosition(0, 0);
    buttonShape.setFillColor(sf::Color::Transparent);

    //buttonText.setFont(font);
    buttonText.setString("default");
    buttonText.setCharacterSize(24);
    buttonText.setFillColor(sf::Color::Transparent);
    buttonText.setPosition(
        +(200 - buttonText.getLocalBounds().width) / 2,
        +(50 - buttonText.getLocalBounds().height) / 2);

}

RegisterButton::RegisterButton(const std::string& text, const sf::Font& font, float x, float y)
{
    buttonShape.setSize(sf::Vector2f(130, 50));
    buttonShape.setPosition(x, y);
    buttonShape.setFillColor(sf::Color::Transparent);

    buttonText.setFont(font);
    buttonText.setString(text);
    buttonText.setCharacterSize(24);
    buttonText.setFillColor(sf::Color::Transparent);
    buttonText.setPosition(
        x + (150 - buttonText.getLocalBounds().width) / 2, y);

}

bool RegisterButton::GetUserInfo(std::string id, std::string pw)
{
    //여기서 아이디 패스워드 조건검사해서 막아버리자.

    if (id.length() == 0 || pw.length() == 0) {
        std::cout << "register user fail!" << std::endl;

        return false;
    }
    _id = id;
    _pw = pw;


    
    std::cout << "id - " << _id << " pw - " << _pw;
    return true;

}

LoginrButton::LoginrButton()
{
    buttonShape.setSize(sf::Vector2f(130, 50));
    buttonShape.setPosition(0, 0);
    buttonShape.setFillColor(sf::Color::Transparent);

    //buttonText.setFont(font);
    buttonText.setString("default");
    buttonText.setCharacterSize(24);
    buttonText.setFillColor(sf::Color::Transparent);
    buttonText.setPosition(
        +(200 - buttonText.getLocalBounds().width) / 2,
        +(50 - buttonText.getLocalBounds().height) / 2);

}

LoginrButton::LoginrButton(const std::string& text, const sf::Font& font, float x, float y)
{
    buttonShape.setSize(sf::Vector2f(130, 50));
    buttonShape.setPosition(x, y);
    buttonShape.setFillColor(sf::Color::Transparent);

    buttonText.setFont(font);
    buttonText.setString(text);
    buttonText.setCharacterSize(24);
    buttonText.setFillColor(sf::Color::Transparent);
    buttonText.setPosition(
        x + (150 - buttonText.getLocalBounds().width) / 2, y);

}

bool LoginrButton::CheckId(std::string id, std::string pw)
{

    if (id.length() == 0 || pw.length() == 0) {
        std::cout << "register user fail!" << std::endl;

        return false;
    }
    _id = id;
    _pw = pw;



    std::cout << "id - " << _id << " pw - " << _pw;
    return true;
}

CloseButton::CloseButton()
{
    buttonShape.setSize(sf::Vector2f(130, 50));
    buttonShape.setPosition(0, 0);
    buttonShape.setFillColor(sf::Color::Transparent);

    //buttonText.setFont(font);
    buttonText.setString("default");
    buttonText.setCharacterSize(24);
    buttonText.setFillColor(sf::Color::Black);
    buttonText.setPosition(
        +(200 - buttonText.getLocalBounds().width) / 2,
        +(50 - buttonText.getLocalBounds().height) / 2);

}

CloseButton::CloseButton(const std::string& text, const sf::Font& font, float x, float y)
{
    buttonShape.setSize(sf::Vector2f(130, 50));
    buttonShape.setPosition(x, y);
    buttonShape.setFillColor(sf::Color::Transparent);

    buttonText.setFont(font);
    buttonText.setString(text);
    buttonText.setCharacterSize(24);
    buttonText.setFillColor(sf::Color::Transparent);
    buttonText.setPosition(
        x + (150 - buttonText.getLocalBounds().width) / 2, y);

}

bool CloseButton::CheckId(std::string id, std::string pw)
{
    return false;
}
