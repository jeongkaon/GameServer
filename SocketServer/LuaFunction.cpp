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
int API_SendMessage(lua_State* L)
{
	int my_id = (int)lua_tointeger(L, -3);
	int user_id = (int)lua_tointeger(L, -2);
	char* mess = (char*)lua_tostring(L, -1);

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