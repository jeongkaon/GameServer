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