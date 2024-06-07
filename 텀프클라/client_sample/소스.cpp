#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <unordered_map>
#include <Windows.h>
using namespace std;
#include "..\..\SERVER\SERVER\protocol.h"

sf::TcpSocket s_socket;

constexpr auto SCREEN_WIDTH = 16;
constexpr auto SCREEN_HEIGHT = 16;

constexpr auto TILE_WIDTH = 65;
constexpr auto WINDOW_WIDTH = SCREEN_WIDTH * TILE_WIDTH;   // size of window
constexpr auto WINDOW_HEIGHT = SCREEN_WIDTH * TILE_WIDTH;
constexpr auto SPRITE_WIDTH = 64;
constexpr auto SPRITE_HEIGHT = 384 / 4;

int g_left_x;
int g_top_y;
int g_myid;

sf::RenderWindow* g_window;
sf::Font g_font;
sf::View view;
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

	char name[NAME_SIZE];
	OBJECT(sf::Texture& t, int x, int y, int x2, int y2) {
		m_showing = true;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x + SPRITE_WIDTH, y + SPRITE_HEIGHT));
		spriteX = 0;
		sprtieY = 0;
		direction = -1;
	}
	OBJECT() {
		m_showing = false;
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

		//여기서 좌표를 바꾸고, 지금은 픽셀단위로 움직이게 두고
		//리팩토링할때 프레임단위로 바꾸게하자
		m_x = x;
		m_y = y;

		//스프라이트 번호도 여기서 바꾸자 -> count를 안보내니까 스프라이트 동기화 발생..
		//일단 넘겨..접속은됨
		static int count = 1;
		switch (direction) {
		case 2: //<-
			sprtieY = 1 * SPRITE_HEIGHT;
			spriteX = (count % 4) * (SPRITE_WIDTH);
			break;
		case 3://오른쪽
			sprtieY = 2 * SPRITE_HEIGHT;
			spriteX = (count % 4) * (SPRITE_WIDTH);

			break;
		case 0: //위
			sprtieY = 3 * SPRITE_HEIGHT;
			spriteX = (count % 4) * (SPRITE_WIDTH);

			break;
		case 1: //아래
			sprtieY = 0 * SPRITE_HEIGHT;
			spriteX = (count % 4) * (SPRITE_WIDTH);
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
		float rx = (m_x) * 65.0f + 1;
		float ry = (m_y) * 65.0f + 1;

		m_sprite.setPosition(rx, ry - 32);

		g_window->draw(m_sprite);
		auto size = m_name.getGlobalBounds();
		m_name.setPosition(rx + 32 - size.width / 2, ry - 10);
		g_window->draw(m_name);

		//스프라이트 바꾸기
		m_sprite.setTextureRect(sf::IntRect(spriteX, sprtieY, SPRITE_WIDTH, SPRITE_HEIGHT));

	}
};

OBJECT avatar;

unordered_map <int, OBJECT> players;

sf::Texture* board;
sf::Texture* pieces;

void client_initialize()
{
	board = new sf::Texture;
	pieces = new sf::Texture;
	board->loadFromFile("chessmap.bmp");
	pieces->loadFromFile("hero.png");
	if (false == g_font.loadFromFile("cour.ttf")) {
		cout << "Font Loading Error!\n";
		exit(-1);
	}


	avatar = OBJECT{ *pieces, 0,0, 64 * 4, 384 };
}

void client_finish()
{
	players.clear();
	delete board;
	delete pieces;
}

void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		g_myid = packet->id;
		avatar.m_x = packet->x;
		avatar.m_y = packet->y;
		avatar.show();
	}
	break;

	case SC_ADD_PLAYER:
	{
		SC_ADD_PLAYER_PACKET* my_packet = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
		int id = my_packet->id;

		if (id == g_myid) {

			avatar.move(my_packet->x, my_packet->y);
			avatar.show();
		}
		else {
			players[id] = OBJECT{ *pieces, 0, 0, 64 * 4, 384 };
			players[id].move(my_packet->x, my_packet->y);
			players[id].set_name(my_packet->name);
			players[id].show();
		}

		break;
	}
	case SC_MOVE_PLAYER:
	{
		SC_MOVE_PLAYER_PACKET* my_packet = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.move(my_packet->x, my_packet->y);
			viewX = my_packet->x * TILE_WIDTH;
			viewY = my_packet->y * TILE_WIDTH;


		}
		else if (other_id < MAX_USER) {
			players[other_id].move(my_packet->x, my_packet->y);
		}
		else {
			players[other_id].move(my_packet->x, my_packet->y);
		}
		break;
	}

	case SC_REMOVE_PLAYER:
	{
		SC_REMOVE_PLAYER_PACKET* my_packet = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.hide();
		}
		else {
			players.erase(other_id);
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


	avatar.draw();
	for (auto& pl : players) pl.second.draw();
	sf::Text text;
	text.setFont(g_font);
	char buf[100];
	sprintf_s(buf, "(%d, %d)", avatar.m_x, avatar.m_y);
	text.setString(buf);
	g_window->draw(text);
}

void send_packet(void* packet)
{
	unsigned char* p = reinterpret_cast<unsigned char*>(packet);
	size_t sent = 0;
	s_socket.send(packet, p[0], sent);
}

int main()
{
	// 텍스처 로드
	sf::Texture backgroundTexture;
	if (!backgroundTexture.loadFromFile("section1.png"))
	{
		std::cerr << "Error loading background image" << std::endl;
		return -1;
	}

	// 스프라이트 생성 및 텍스처 설정
	sf::Sprite backgroundSprite;
	backgroundSprite.setTexture(backgroundTexture);




	wcout.imbue(locale("korean"));
	sf::Socket::Status status = s_socket.connect("127.0.0.1", PORT_NUM);
	s_socket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		exit(-1);
	}

	client_initialize();
	CS_LOGIN_PACKET p;
	p.size = sizeof(p);
	p.type = CS_LOGIN;

	string player_name{ "P" };
	player_name += to_string(GetCurrentProcessId());

	strcpy_s(p.name, player_name.c_str());
	send_packet(&p);
	avatar.set_name(p.name);

	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
	g_window = &window;

	//뷰설정
	view = window.getDefaultView();
	while (window.isOpen())
	{

		sf::Event event;
		while (window.pollEvent(event))
		{

			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				switch (event.key.code) {
				case sf::Keyboard::Left:
					avatar.direction = 2;

					break;
				case sf::Keyboard::Right:
					avatar.direction = 3;

					break;
				case sf::Keyboard::Up:
					avatar.direction = 0;
					break;
				case sf::Keyboard::Down:
					avatar.direction = 1;

					break;
				case sf::Keyboard::Escape:
					window.close();
					break;
				}

				if (-1 != avatar.direction) {
					CS_MOVE_PACKET p;
					p.size = sizeof(p);
					p.type = CS_MOVE;
					p.direction = avatar.direction;
					send_packet(&p);
				}

			}
		}
		view.setCenter(viewX, viewY);
		window.setView(view);
		window.clear();


		window.draw(backgroundSprite);
		client_main();
		window.display();
	}
	client_finish();

	return 0;
}