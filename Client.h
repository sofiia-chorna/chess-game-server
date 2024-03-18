#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdarg.h>

#define BUF_SIZE 1024
#define SERVER_PORT 5209
#define IP "127.0.0.1"

class Client
{
public:
    Client(const std::string &name) : name(name) {}
    void start();

private:
    std::string name;
    std::string msg;

    void sendMessage(int sock);
    void receiveMessage(int sock);
    int output(const char *arg, ...);
    int errorOutput(const char *arg, ...);
    void errorHandling(const std::string &message);
};
