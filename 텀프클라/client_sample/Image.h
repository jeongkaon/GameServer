#pragma once
#include <SFML/Graphics.hpp>

class Image
{
public:
	//�ʿ��� �̹��� �� ���⼭ �ε�����.
	sf::Texture* LoginTexture;
	sf::Texture* SelectTexture;
	sf::Texture* GameTexture;//�ʺ��� ������ �־���ϳ�?

	//ĳ���� �̹����� �迭�� �ؼ� 
	//ĳ���� ��ȣ�� �´°� ����.
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

