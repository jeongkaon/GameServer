#include "Image.h"

Image::Image()
{
	LoginTexture= new sf::Texture;
	SelectTexture = new sf::Texture;
	GameTexture = new sf::Texture;

	//캐릭터 4가지 
	charType1 = new sf::Texture;
	charType2 = new sf::Texture;
	charType3 = new sf::Texture;
	charType4 = new sf::Texture;

	//npc
	//npcType1 = new sf::Texture;
	//npcType2 = new sf::Texture;
	//npcType3 = new sf::Texture;
	//npcType4 = new sf::Texture;


}

Image::~Image()
{
	delete LoginTexture;
	delete SelectTexture;
	delete GameTexture;

	delete	charType1;
	delete	charType2;
	delete	charType3;
	delete	charType4;

	delete	npcType1;
	delete	npcType2;
	delete	npcType3;
	delete	npcType4;
}

void Image::ImageSetting()
{

	LoginTexture->loadFromFile("loginwindow.png");
	SelectTexture->loadFromFile("select.png");
	GameTexture->loadFromFile("section1.png");



	charType1->loadFromFile("chartype1.png");
	charType2->loadFromFile("chartype2.png");
	charType3->loadFromFile("chartype2.png");
	charType4->loadFromFile("chartype2.png");


}
