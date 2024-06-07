#include "stdafx.h"
#include "Server.h"


int main()
{

    Server* mainServer = Server::getInstance();
    mainServer->Init();
    mainServer->Start();
    mainServer->Stop();


}
