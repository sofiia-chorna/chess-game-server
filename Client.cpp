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
#include "Client.h"
#include "Logger.h"

#define BUF_SIZE 1024
#define SERVER_PORT 5208
#define IP "127.0.0.1"

void Client::start()
{
    // Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1)
    {
        Logger::handleError("socket() failed!");
    }

    // Set up server address
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(IP);
    serverAddress.sin_port = htons(SERVER_PORT);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        Logger::handleError("connect() failed!");
    }

    // Send the client's name to the server
    std::string clientNameMessage = "#new client:" + this->name;
    send(sock, clientNameMessage.c_str(), clientNameMessage.length() + 1, 0);

    // Start sender and receiver threads
    std::thread senderThread(&Client::sendMessage, this, sock);
    std::thread receiverThread(&Client::receiveMessage, this, sock);

    // Wait for threads to finish
    senderThread.join();
    receiverThread.join();

    // Close the socket
    close(sock);
}

void Client::sendMessage(int sock)
{
    while (true)
    {
        getline(std::cin, this->msg);
        if (this->msg == "Quit" || this->msg == "quit")
        {
            close(sock);
            exit(0);
        }
        // Format the message with the client's name
        std::string messageWithUsername = "[" + this->name + "]: " + this->msg;
        send(sock, messageWithUsername.c_str(), messageWithUsername.length() + 1, 0);
    }
}

void Client::receiveMessage(int sock)
{
    char receivedMessage[BUF_SIZE + this->msg.length() + 1];
    while (true)
    {
        int strLen = recv(sock, receivedMessage, BUF_SIZE + this->msg.length() + 1, 0);
        if (strLen == -1)
        {
            exit(-1);
        }
        std::cout << std::string(receivedMessage) << std::endl;
    }
}
