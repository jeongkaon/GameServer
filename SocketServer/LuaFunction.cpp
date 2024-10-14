#include "stdafx.h"
#include "LuaFunction.h"
#include "Server.h"

int API_get_x(lua_State* L)
{
	int id = (int)lua_tointeger(L, -1);
	if (id < 0) return 0;

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

	lua_pop(L, 3);
	return 0;
}

int API_Attck_To_Player(lua_State* L)
{
	return 0;

}

int API_AttackMessage(lua_State* L)
{
	int my_id = (int)lua_tointeger(L, -3);
	int user_id = (int)lua_tointeger(L, -2);
	char* mess = (char*)lua_tostring(L, -1);

	//TODO. 여기서 attack관련 일 처리해야한다
	std::cout << mess << std::endl;

	lua_pop(L, 4);
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

