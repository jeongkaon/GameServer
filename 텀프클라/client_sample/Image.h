#pragma once
#include <SFML/Graphics.hpp>

class Image
{
public:
	//필요한 이미지 다 여기서 로딩하자.
	sf::Texture* LoginTexture;
	sf::Texture* SelectTexture;
	sf::Texture* GameTexture;//맵별로 가지고 있어야하나?

	//캐릭터 이미지는 배열로 해서 
	//캐릭터 번호랑 맞는거 하자.
	sf::Texture* charType1;
	sf::Texture* charType2;
	sf::Texture* charType3;
	sf::Texture* charType4;

	sf::Texture* npcType1;
	sf::Texture* npcType2;
	sf::Texture* npcType3;
	sf::Texture* npcType4;

public:
	Image();
	~Image();
	void ImageSetting();
};

