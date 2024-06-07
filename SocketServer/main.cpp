#include "stdafx.h"
#include "Server.h"
int main()
{

	Server* mainServer = new Server();
	mainServer->Init();
	mainServer->Start();
	mainServer->Stop();




}