#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <unordered_map>
#include <Windows.h>

#include "../../SocketServer/protocol_2024.h"

#include "defines.h"
#include "Button.h"
#include "TextBox.h"
using namespace std;

void client_main();

sf::TcpSocket s_socket;


bool isLoginWindow1Closed = false;
bool isSelectWindow1Closed = false;
bool isGameWindow1Closed = false;

int g_left_x;
int g_top_y;
int g_myid;
int g_type;

sf::Clock g_clock;
sf::RenderWindow* g_window;
sf::Font g_font;
sf::View view;

sf::Texture* Visuals[4];
sf::Texture* slimes[4];
sf::Texture* backgroundTexture;
sf::Texture* backgroundTexture13;

sf::RectangleShape debugBox(sf::Vector2f((VIEW_RANGE+0.5f)*TILE_WIDTH*2, 2*(VIEW_RANGE+0.5f)*TILE_WIDTH)); 

float viewX;
float viewY;

class OBJECT {
private:
	bool m_showing;
	sf::Sprite m_sprite;
	sf::Text m_name;

public:
	int m_x, m_y;
	int direction;

	int spriteX;
	int sprtieY;

	int spriteWidth;
	int spriteHeight;

	int level;
	int exp;
	

	char name[NAME_SIZE];

	//선택된 캐릭터그거
	int visual;

	OBJECT(sf::Texture& t, int x, int y, int x2, int y2, int vis) {
		visual = vis;
		m_showing = true;
		m_sprite.setTexture(t);
		if (vis > 10) {
			spriteWidth = SPRITE_WIDTH;
			spriteHeight = SPRITE_MON_HEIGHT;

		}
		else {
			spriteWidth = SPRITE_WIDTH;
			spriteHeight = SPRITE_HEIGHT;


		}
		m_sprite.setTextureRect(sf::IntRect(x, y, x + spriteWidth, y + spriteHeight));

		spriteX = 0;
		sprtieY = 0;

	}
	OBJECT() {
		m_showing = false;
	}
	void init(sf::Texture& t, int x, int y, int x2, int y2, int vis)
	{
	
		m_sprite.setTexture(t);
		if (vis > 10) {
			spriteWidth = SPRITE_WIDTH;
			spriteHeight = SPRITE_MON_HEIGHT;

		}
		else {
			spriteWidth = SPRITE_WIDTH;
			spriteHeight = SPRITE_HEIGHT;


		}
		m_sprite.setTextureRect(sf::IntRect(x, y, x + spriteWidth, y + spriteHeight));

		spriteX = 0;
		sprtieY = 0;


	}
	void show()
	{
		m_showing = true;
	}
	void hide()
	{
		m_showing = false;
	}

	void a_move(int x, int y) {
		m_sprite.setPosition((float)x, (float)y);
	}

	void a_draw() {
		g_window->draw(m_sprite);
	}

	void move(int x, int y) {

		m_x = x;
		m_y = y;

	
		static unsigned int count = 1;
		switch (direction) {
		case 2: //<-
			sprtieY = 1* spriteHeight;
			spriteX = (count % 4) * (spriteWidth);			

			break;
		case 3://오른쪽
			sprtieY = 2 * spriteHeight;
			spriteX = (count % 4) * (spriteWidth);

			break;
		case 0: //위
			sprtieY = 3* spriteHeight;
			spriteX = (count % 4) * (spriteWidth);

			break;
		case 1: //아래
			sprtieY =0* spriteHeight;
			spriteX = (count % 4) * (spriteWidth);
			break;
		}
	
		count++;

	}
	void draw(float x, float y)
	{
		m_sprite.setPosition(x, y);
	}
	void set_name(const char str[]) {
		m_name.setFont(g_font);
		m_name.setString(str);
		m_name.setFillColor(sf::Color(255, 255, 0));
		m_name.setStyle(sf::Text::Bold);
	}


	void draw() {
		if (false == m_showing) return;
		int rx = (m_x) * 32;// +1;
		int ry = (m_y) * 32;// +1;

		if (visual <= 10) {
			m_sprite.setPosition(rx, ry - 16);
		}
		else {
			m_sprite.setPosition(rx, ry-3);

		}

		g_window->draw(m_sprite);
		auto size = m_name.getGlobalBounds();
		m_name.setPosition(rx + 16 - size.width / 2, ry - 5);
		g_window->draw(m_name);

		//스프라이트 바꾸기
		m_sprite.setTextureRect(sf::IntRect(spriteX, sprtieY, spriteWidth, spriteHeight));

	}
};

OBJECT avatar; 

unordered_map <int, OBJECT> players;

void client_finish()
{
	players.clear();
	delete[] Visuals;
}

void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[2])
	{
	case SC_LOGIN_INFO:
	{

		SC_LOGIN_INFO_PACKET * packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		g_myid = packet->id;

		avatar.m_x = packet->x;
		avatar.m_y = packet->y;
		avatar.visual = packet->visual;
		viewX = packet->x  * TILE_WIDTH;
		viewY = packet->y  * TILE_WIDTH;


		avatar.direction = DOWN;
		avatar.show();

		isLoginWindow1Closed = true;
		isSelectWindow1Closed = true;

		avatar.move(packet->x, packet->y);
	}
	break;

	case SC_ADD_OBJECT:
	{
		SC_ADD_OBJECT_PACKET* my_packet = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(ptr);
		int id = my_packet->id;

		if (id == g_myid) {

			avatar.move(my_packet->x, my_packet->y);
			avatar.show();
		}
		else if(id <MAX_USER) {

			int visual = my_packet->visual;
			players[id] = OBJECT{ *Visuals[visual-1], 0, 0, 32 * 4, 384/2,visual};
			players[id].visual = visual; 
			players[id].move(my_packet->x, my_packet->y);
			players[id].set_name(my_packet->name);
			players[id].show();

		}
		else if(id > 0 ){
			int visual = my_packet->visual;
			int idx = visual % 10 - 1;
			players[id] = OBJECT{ *slimes[idx], 0, 0, 64 * 3, 64 * 3,visual};
			players[id].spriteWidth = SPRITE_WIDTH;
			players[id].spriteHeight = SPRITE_MON_HEIGHT;
			players[id].visual = visual;
			players[id].move(my_packet->x, my_packet->y);
			//players[id].set_name(my_packet->name);
			players[id].show();

		}
	
		break;
	}
	case SC_MOVE_OBJECT:
	{
		SC_MOVE_OBJECT_PACKET* my_packet = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			viewX = my_packet->x * 32;
			viewY = my_packet->y * 32;
			view.setCenter(viewX, viewY);
			g_window->setView(view);
			avatar.move(my_packet->x, my_packet->y);


		}
		else {
			players[other_id].direction = my_packet->dir;
			players[other_id].move(my_packet->x, my_packet->y);

			
		}
		break;
	}

	case SC_REMOVE_OBJECT:
	{
		SC_REMOVE_OBJECT_PACKET* my_packet = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.hide();
		}
		else  {
			players.erase(other_id);
		}
		
		break;
	}
	case SC_CHOICE_CHARACTER:
	{
		printf("SC_CHOICE_CHARACTER PACKET type[% d]\n", ptr[1]);

		isLoginWindow1Closed = true;
		break;
	}
	case SC_CHAT:
	{
		SC_CHAT_PACKET* my_packet = reinterpret_cast<SC_CHAT_PACKET*>(ptr);
		std::cout <<"[채팅] "<< my_packet->mess << std::endl;

		
		break;
	}
	case SC_ATTACK_SUCCESS:
	{
		SC_SUC_ATTACK_PACKET* packet = reinterpret_cast<SC_SUC_ATTACK_PACKET*>(ptr);
		std::cout << "[전투] 몬스터[" << packet->npcId << "] 를 때려서 " << packet->damage << "의 데미지를 입혔습니다\n";

		break;
	}
	case SC_GET_EXP:
	{
		SC_GET_EXP_PACKET* packet = reinterpret_cast<SC_GET_EXP_PACKET*>(ptr);
		std::cout << "[전투] 몬스터[" << packet->npcId << "] 를 무찔러서 " << packet->exp << "의 경험치를 얻었습니다\n";
		
		//level도 처리해야한다
		if (avatar.level != packet->level) {
			std::cout << "[전투] " << packet->level << "로 레벨업을 하였습니다\n";
			avatar.level = packet->level;

		}
		break;
	}
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void process_data(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}
void send_packet(void* packet)
{
	unsigned char* p = reinterpret_cast<unsigned char*>(packet);
	size_t sent = 0;
	s_socket.send(packet, p[0], sent);
}

void NetWork()
{

	char net_buf[BUF_SIZE];
	size_t	received;

	auto recv_result = s_socket.receive(net_buf, BUF_SIZE, received);
	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		exit(-1);
	}
	if (recv_result == sf::Socket::Disconnected) {
		wcout << L"Disconnected\n";
		exit(-1);
	}
	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);
}

void client_main()
{
	char net_buf[BUF_SIZE];
	size_t	received;

	auto recv_result = s_socket.receive(net_buf, BUF_SIZE, received);
	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		exit(-1);
	}
	if (recv_result == sf::Socket::Disconnected) {
		wcout << L"Disconnected\n";
		exit(-1);
	}
	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);

	for (auto& pl : players) pl.second.draw();
	avatar.draw();

	//디버깅박스를 그린다
	//debugBox.setPosition((avatar.m_x- VIEW_RANGE )* TILE_WIDTH , (avatar.m_y- VIEW_RANGE)* TILE_WIDTH);
	//g_window->draw(debugBox);

	sf::Text text;
	text.setFont(g_font);
	char buf[100];
	sprintf_s(buf, "(%d, %d)", avatar.m_x, avatar.m_y);
	text.setString(buf);
	g_window->draw(text);
}

//캐릭터창 선택하는거
void SelectCharacterWindow(sf::RenderWindow& window1, bool& isWindow1Closed) 
{
	std::vector<CharacterSelectButton> buttons;
	buttons.emplace_back("Aurora", g_font, 30, 440);
	buttons.emplace_back("Moon", g_font, 230, 440);
	buttons.emplace_back("Aladin", g_font, 420, 440);
	buttons.emplace_back("Cinderella", g_font, 600, 440);


	sf::Texture backgroundTexture;
	if (!backgroundTexture.loadFromFile("select.png"))
	{
		std::cerr << "Error loading background image" << std::endl;
	}
	sf::Sprite backgroundSprite;
	backgroundSprite.setTexture(backgroundTexture);


	while (window1.isOpen()) {
		sf::Event event;
		while (window1.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window1.close();
				isWindow1Closed = true;
			}
			if (event.type == sf::Event::MouseButtonPressed) {
				sf::Vector2i mousePos = sf::Mouse::getPosition(window1);
				for (const auto& button : buttons) {
					if (button.isClicked(mousePos)) {

						CS_CHOICECHAR_PACKET packet;
						packet.size = sizeof(CS_CHOICECHAR_PACKET);
						packet.type = CS_CHOICE_CHARACTER;

						switch (button._buttonNumber)
						{
						case 1:
							//std::cout << "Character Selected Aurora" << std::endl;
							avatar.visual = 1;
							packet.visual = 1;
							send_packet(&packet);

							break;
						case 2:
							//std::cout << "Character Selected Moon" << std::endl;
							avatar.visual = 2;
							packet.visual = 2;
							send_packet(&packet);

							break;
						case 3:
							//std::cout << "Character Selected Aladin" << std::endl;
							avatar.visual = 3;
							packet.visual = 3;
							send_packet(&packet);


							break;
						case 4:
							//
							// std::cout << "Character Selected Cinderella" << std::endl;
							avatar.visual = 4;
							packet.visual = 4;
							send_packet(&packet);


							break;
						default:
							break;
						}

						isWindow1Closed = true;
						window1.close();

						break;
					}
				}
			}
		}
		if (isWindow1Closed) {

			window1.close();
			return;
		}

		window1.clear();
		window1.draw(backgroundSprite);

		for (const auto& button : buttons) {
			button.draw(window1);
		}
		NetWork();

		window1.display();
	}
}


void LoginWindow(sf::RenderWindow& window1, bool& isLoginWindow1Closed)
{
	sf::Texture backgroundTexture;
	if (!backgroundTexture.loadFromFile("loginwindow.png"))
	{
		std::cerr << "Error loading background image" << std::endl;
	}
	sf::Sprite backgroundSprite;
	backgroundSprite.setTexture(backgroundTexture);

	//회원가입버튼
	RegisterButton registButton{ "register",g_font, 170,500 };

	//로그인버튼
	LoginrButton loginButton{ "login",g_font, 333,500 };
	//종료버튼
	CloseButton closeButton{ "close",g_font, 500,500 };


	//텍스트박스 사용하기
	TextBox InputId(g_font, 360, 400, 240, 30);
	TextBox InputPW(g_font, 360, 430, 240, 30);
	TextBox* activeTextBox = nullptr;

	while (window1.isOpen()) {
		sf::Event event;
		while (window1.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window1.close();
				isLoginWindow1Closed = true;
			}
			if (event.type == sf::Event::MouseButtonPressed) {
				sf::Vector2i mousePos = sf::Mouse::getPosition(window1);
				if (InputId.isClicked(mousePos)) {
					activeTextBox = &InputId;
					InputId.setActive(true);
					InputPW.setActive(false);
				}
				else if(InputPW.isClicked(mousePos)) {
					activeTextBox = &InputPW;
					InputPW.setActive(true);
					InputId.setActive(false);
				}

				if (registButton.isClicked(mousePos)) {
					if (registButton.GetUserInfo(InputId.getText(),InputPW.getText())) {
						//TODO 회원가입
						CS_REGIST_PACKET p;

						p.size = sizeof(CS_REGIST_PACKET);
						p.type = CS_REGIST;
						std::string temp;
						temp = InputId.getText();
						strcpy_s(p.name, temp.c_str());

						temp = InputPW.getText();
						strcpy_s(p.password, temp.c_str());
						send_packet(&p);

						avatar.set_name(p.name);

					}

				}

				if (loginButton.isClicked(mousePos)) {
					if (loginButton.CheckId(InputId.getText(), InputPW.getText())) {
						//TODO 로그인패킷 보내기
						CS_LOGIN_PACKET p;
						p.size = sizeof(p);
						p.type = CS_LOGIN;

						//아이디? 그거 세팅하는거임
						string player_name= InputId.getText();

						strcpy_s(p.name, player_name.c_str());
						strcpy_s(avatar.name, player_name.c_str());

						//로그인패킷을 보낸다.
						avatar.set_name(p.name);
						send_packet(&p);
						
						
					}

				}
				if (closeButton.isClicked(mousePos)) {
					
					window1.close();
				}


			}
			if (event.type == sf::Event::TextEntered && activeTextBox != nullptr) {
				if (event.text.unicode == '\b') { // 백스페이스 처리
					activeTextBox->removeLastCharacter();
				}
				else if (event.text.unicode == '\r') { // 엔터 처리
					// 엔터 입력 시 동작
					
					//std::cout << "User Input: " << activeTextBox->getText() << std::endl;
					activeTextBox = &InputPW;
					InputPW.setActive(true);
					InputId.setActive(false);

				}
				else if (event.text.unicode == '\t') {
					//밑으로 넘어가기
					activeTextBox = &InputPW;
					InputPW.setActive(true);
					InputId.setActive(false);

				}
				else {
					activeTextBox->addCharacter(static_cast<char>(event.text.unicode));
				}
			}
		}
		if (isLoginWindow1Closed) {

			window1.close();
			return;
		}

		window1.clear();
		window1.draw(backgroundSprite);

		InputId.draw(window1);
		InputPW.draw(window1);
		registButton.draw(window1);
		loginButton.draw(window1);
		closeButton.draw(window1);

		NetWork();

		window1.display();
	}

}

void updateChatBoxPosition(float viewX, float viewY, sf::RectangleShape& chatBackground, sf::Text& chatInputText)
{
	float viewLeft = viewX - WINDOW_WIDTH / 2;
	float viewBottom = viewY + WINDOW_HEIGHT / 2;

	chatBackground.setPosition(viewLeft , viewBottom - CHATBOX_HEIGHT - CHATBOX_MARGIN);
}

void GameWindow() 
{
	//디버깅용박스
	// 디버깅 박스 설정
	//debugBox.setFillColor(sf::Color::Transparent); // 투명한 채우기 색
	//debugBox.setOutlineColor(sf::Color::Red); // 빨간색 테두리
	//debugBox.setOutlineThickness(2); // 테두리 두께 설정


	// 스프라이트 생성 및 텍스처 설정
	sf::Sprite backgroundSprite;
	backgroundSprite.setTexture(*backgroundTexture);

	sf::Sprite backgroundSprite13;
	backgroundSprite13.setTexture(*backgroundTexture13);
	backgroundSprite13.setPosition(backgroundTexture->getSize().x, 0);


	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "KAON'S GAME");
	g_window = &window;


	// 채팅 관련 변수들
	TextBox ChatInput(g_font, 0, WINDOW_HEIGHT-28, WINDOW_WIDTH*0.75,30);

		// 채팅 배경 박스 설정
	avatar.init(*Visuals[avatar.visual - 1], 0, 0, 64 * 4 / 2, 384 / 2, avatar.visual);

	//뷰설정
	view = g_window->getDefaultView();

	while (g_window->isOpen())
	{
	
		sf::Event event;
		while (g_window->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				g_window->close();

			if (event.type == sf::Event::MouseButtonPressed) {
				sf::Vector2i mousePos = sf::Mouse::getPosition(*g_window);
				sf::Vector2f worldPos = g_window->mapPixelToCoords(mousePos);

				if (ChatInput.isClicked(worldPos)) {
					ChatInput.setActive(true);
					

				}
				else {
					ChatInput.setActive(false);
					

				}
			}
			if (event.type == sf::Event::TextEntered && ChatInput.isActive == true)
			{
				
					if (event.text.unicode == '\b') { // 백스페이스 처리
						ChatInput.removeLastCharacter();
					}
					else if (event.text.unicode == '\r') { // 엔터 처리
						// 엔터 입력 시 동작
						//std::cout << "User Input: " << ChatInput.getText() << std::endl;
						string chatting = "[";
						chatting += avatar.name;
						chatting += "] ";
						chatting += ChatInput.getText();

						//네트워크로 보내자 
						CS_CHAT_PACKET packet{};
						memset(&packet, 0, sizeof(CS_CHAT_PACKET));
						packet.type = CS_CHAT;
						strcpy_s(packet.mess, chatting.c_str());
						packet.mess[strlen(chatting.c_str())] = '\0';
			
						packet.size = strlen(chatting.c_str()) + 1 + 3;
						
						//로그인패킷을 보낸다.

						send_packet(&packet);


						ChatInput.setActive(false);
						ChatInput.setText("");
					}

					else {
						ChatInput.addCharacter(static_cast<char>(event.text.unicode));
					}
				

			}

			if (event.type == sf::Event::KeyPressed) {

			
				switch (event.key.code) {
			
				

				case sf::Keyboard::Left: 
				{
					if (ChatInput.isActive == true) break;

					avatar.direction = LEFT;
					CS_MOVE_PACKET p;
					p.size = sizeof(p);
					p.type = CS_MOVE;
					p.direction = avatar.direction;
					send_packet(&p);

					break;
				}
				case sf::Keyboard::Right:
				{
					if (ChatInput.isActive == true) break;

					avatar.direction = RIGHT;
					CS_MOVE_PACKET p;
					p.size = sizeof(p);
					p.type = CS_MOVE; p.direction = avatar.direction;
					
					send_packet(&p);

				}
					break;
				case sf::Keyboard::Up:
				{
					if (ChatInput.isActive == true) break;

					avatar.direction = UP;
					CS_MOVE_PACKET p;
					p.size = sizeof(p);
					p.type = CS_MOVE;
					p.direction = avatar.direction;
					send_packet(&p);

					break;
				}
				case sf::Keyboard::Down:
				{
					if (ChatInput.isActive == true) break;

					avatar.direction = DOWN;
					CS_MOVE_PACKET p;
					p.size = sizeof(p);
					p.type = CS_MOVE;
					p.direction = avatar.direction;
					send_packet(&p);

					break;
				}
				case sf::Keyboard::Escape:
					g_window->close();
					break;


				case sf::Keyboard::A:
				{
					if (ChatInput.isActive == true) break;
					//std::cout << "공격키 누름\n";
					CS_ATTACK_PACKET p;
					p.size = sizeof(p);
					p.type = CS_ATTACK;
					p.dir = ALL;
					send_packet(&p);

					break;
				}
				case sf::Keyboard::S:
				{
					if (ChatInput.isActive == true) break;

					CS_ATTACK_PACKET p;
					p.size = sizeof(p);
					p.type = CS_ATTACK;

					//사방이 아니면 아바타가 보고있는방향이 공격방향이다
					p.dir = avatar.direction;
					send_packet(&p);


					break;
				}
				default:
					break;
				}


			}
		}
	
		
		if (viewX <= 320) viewX = 320;
		if (viewY <= 320) viewY = 320;


		g_window->clear();
		view.setCenter(viewX, viewY);
		g_window->setView(view);
		
		g_window->draw(backgroundSprite);
		g_window->draw(backgroundSprite13);

		client_main();

		ChatInput.ChangePosition(viewX, viewY);
		ChatInput.draw(*g_window);
		g_window->display();
	}
	client_finish();
}

int main()
{

	//이미지 로딩 먼저 해놓자
	//캐릭터사진 초기화
	for (int i = 0; i < 4; ++i) {
		Visuals[i] = new sf::Texture;
	}
	Visuals[0]->loadFromFile("charType2.png");
	Visuals[1]->loadFromFile("charType1.png");
	Visuals[2]->loadFromFile("charType3.png");
	Visuals[3]->loadFromFile("charType4.png");

	for (int i = 0; i < 4; ++i) {
		slimes[i] = new sf::Texture;
	}
	slimes[0]->loadFromFile("slime_monster1.png");
	slimes[1]->loadFromFile("slime_monster2.png");
	slimes[2]->loadFromFile("slime_monster3.png");
	slimes[3]->loadFromFile("slime_monster4.png");




	backgroundTexture = new sf::Texture;
	backgroundTexture13 = new sf::Texture;

	//이거 로딩이 너무 느림.... 백그라운드로 빼고시푼디...
	if (!backgroundTexture->loadFromFile("section123.jpg"))
	{
		std::cerr << "Error loading background image" << std::endl;
	}

	if (!backgroundTexture13->loadFromFile("section1314.jpg"))
	{
		std::cerr << "Error loading background image" << std::endl;
	}
	wcout.imbue(locale("korean"));
	sf::Socket::Status status = s_socket.connect("127.0.0.1", PORT_NUM);
	s_socket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		exit(-1);
	}

	if (!g_font.loadFromFile("cour.ttf")) {
		return -1;
	}


	sf::RenderWindow window1(sf::VideoMode(800, 600), "Window 1");

	LoginWindow(window1, isLoginWindow1Closed);


	// 첫 번째 창이 닫혔는지 확인 후 두 번째 창 실행
	if (isLoginWindow1Closed) {
		sf::RenderWindow window2(sf::VideoMode(800, 600), "Window 2");
		SelectCharacterWindow(window2, isSelectWindow1Closed);
	}

	if (isSelectWindow1Closed) {
		GameWindow();
	}

	return 0;
}


