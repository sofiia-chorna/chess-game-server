#include "Server.h"

#define SERVER_PORT 5208

int main()
{
    Server server(SERVER_PORT);
    server.start();
    return 0;
}
