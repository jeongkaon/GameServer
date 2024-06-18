#include "stdafx.h"
#include "LuaFunction.h"
#include "Server.h"

int API_get_x(lua_State* L)
{
	int id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	auto server = Server::getInstance();
	int x = server->LuaGetX(id);
	lua_pushnumber(L, x);
	return 1;
}
int API_get_y(lua_State* L)
{
	int id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	auto server = Server::getInstance();
	int y = server->LuaGetY(id);
	lua_pushnumber(L, y);
	return 1;
}

int API_Active_Agro(lua_State* L)
{
	int agro_id = (int)lua_tointeger(L, -2);
	int target_id = (int)lua_tointeger(L, -1);

	//어그로몬스터 타켓을 받아오자
	std::cout << agro_id << "몬스터가 " << target_id << "를 타겟으로 삼음" << std::endl;

	lua_pop(L, 3);
	//clients[user_id].send_chat_packet(my_id, mess);
	return 0;
}

int API_Attck_To_Player(lua_State* L)
{
	return 0;

}

int API_SendMessage(lua_State* L)
{
	int my_id = (int)lua_tointeger(L, -3);
	int user_id = (int)lua_tointeger(L, -2);
	char* mess = (char*)lua_tostring(L, -1);
	std::cout << mess << std::endl;

	lua_pop(L, 4);
	//clients[user_id].send_chat_packet(my_id, mess);
	return 0;
}

int API_MoveTo(lua_State* L)
{
	int id = lua_tointeger(L, -3);
	int x = lua_tointeger(L, -2);
	int y = lua_tointeger(L, -1);
	std::cout << "Moving ID " << id << " to position (" << x << ", " << y << ")" << std::endl;
	lua_pop(L, 4);

	return 0;
}

