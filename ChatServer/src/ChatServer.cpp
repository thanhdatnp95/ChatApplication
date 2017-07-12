#include <iostream>
#include "ConnectionHandler.h"

using namespace std;

int main()
{
    ConnectionHandler& server = ConnectionHandler::getInstance();
    server.start();
    return 0;
}