#include <iostream>
#include <cstring>
#include <thread>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "Server.h"
#include "Logger.h"

#define BUFFER_SIZE 1024
#define MAX_NAME_LENGTH 20
#define MAX_PRE_NAME_LENGTH 12
#define MAX_CLIENTS 256

Server::Server(int port)
{
    this->createAndBindServerSocket(port);
}

Server::~Server()
{
    close(this->serverSocket);
}

void Server::start()
{
    this->acceptClientConnections();
}

void Server::createAndBindServerSocket(int port)
{
    this->serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (this->serverSocket == -1)
    {
        Logger::handleError("Failed to create server socket!");
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);

    if (bind(this->serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        Logger::handleError("Failed to bind server socket!");
    }
    Logger::printFormattedOutput(stdout, "Server is running on port %d\n", port);

    if (listen(this->serverSocket, MAX_CLIENTS) == -1)
    {
        Logger::handleError("Failed to listen on server socket!");
    }
}

void Server::acceptClientConnections()
{
    while (1)
    {
        struct sockaddr_in client_address;
        socklen_t client_address_size = sizeof(client_address);
        int clientSocket = accept(this->serverSocket, (struct sockaddr *)&client_address, &client_address_size);
        if (clientSocket == -1)
        {
            Logger::handleError("Failed to accept client connection!");
        }

        mtx.lock();
        this->clientCount++;
        mtx.unlock();

        std::thread th([this, clientSocket]()
                       { this->handleClient(clientSocket); });
        th.detach();

        Logger::printFormattedOutput(stdout, "Connected client IP: %s \n", inet_ntoa(client_address.sin_addr));
    }
}

void Server::handleNewClient(int clientSocket, const char *msg)
{
    const char new_client_prefix[MAX_PRE_NAME_LENGTH + 1] = "#new client:";
    if (strlen(msg) <= MAX_PRE_NAME_LENGTH)
    {
        // Message does not contain the new client prefix, forward it as a regular message
        this->sendMessage(std::string(msg));
        return;
    }

    // Extract the prefix from the message
    char prefix[MAX_PRE_NAME_LENGTH + 1];
    strncpy(prefix, msg, MAX_PRE_NAME_LENGTH);
    prefix[MAX_PRE_NAME_LENGTH] = '\0';

    // Check if the message starts with the new client prefix
    if (strcmp(prefix, new_client_prefix) == 0)
    {
        // Extract the name from the message
        char name[MAX_NAME_LENGTH];
        strcpy(name, msg + MAX_PRE_NAME_LENGTH);

        // Check if the client name already exists
        if (this->clientSocketMap.find(name) == this->clientSocketMap.end())
        {
            // Associate the client name with the client socket
            Logger::printFormattedOutput(stdout, "Socket %d is associated with client name: %s\n", clientSocket, name);
            this->clientSocketMap[name] = clientSocket;
        }
        else
        {
            // Notify the client that the name already exists
            std::string error_msg = std::string(name) + " already exists. Please choose another name.";
            send(clientSocket, error_msg.c_str(), error_msg.length() + 1, 0);
            mtx.lock();
            this->clientCount--;
            mtx.unlock();
            return;
        }
    }

    // Forward the message to all clients
    this->sendMessage(std::string(msg));
}

void Server::handleClientLeave(int clientSocket)
{
    std::string leave_msg;
    std::string name;
    mtx.lock();
    for (auto it = this->clientSocketMap.begin(); it != this->clientSocketMap.end(); ++it)
    {
        if (it->second == clientSocket)
        {
            name = it->first;
            this->clientSocketMap.erase(it->first);
        }
    }
    this->clientCount--;
    mtx.unlock();
    leave_msg = "Client " + name + " has left the chat room";
    this->sendMessage(leave_msg);
    Logger::printFormattedOutput(stdout, "Client %s has left the chat room\n", name.c_str());
    close(clientSocket);
}

void Server::handleClient(int clientSocket)
{
    char msg[BUFFER_SIZE];
    while (recv(clientSocket, msg, sizeof(msg), 0) != 0)
    {
        this->handleNewClient(clientSocket, msg);
    }
    this->handleClientLeave(clientSocket);
}

void Server::sendToClient(const std::string &msg, int clientSocket)
{
    send(clientSocket, msg.c_str(), msg.length() + 1, 0);
}

void Server::sendErrorToSender(const std::string &receiver_name, const std::string &sender_name)
{
    std::string errorMsg = "[error] client named " + receiver_name + " does not exist.";
    this->sendToClient(errorMsg, this->clientSocketMap[sender_name]);
}

void Server::broadcastMessage(const std::string &msg)
{
    for (const auto &client : this->clientSocketMap)
    {
        this->sendToClient(msg, client.second);
    }
}

void Server::sendToReceiverAndSender(const std::string &msg, const std::string &receiver_name, const std::string &sender_name)
{
    auto receiver_iter = this->clientSocketMap.find(receiver_name);
    if (receiver_iter != this->clientSocketMap.end())
    {
        this->sendToClient(msg, receiver_iter->second);
        this->sendToClient(msg, this->clientSocketMap[sender_name]);
    }
    else
    {
        this->sendErrorToSender(receiver_name, sender_name);
    }
}

bool Server::hasSpecialPrefix(const std::string &msg, const std::string &prefix)
{
    size_t first_space = msg.find_first_of(" ");
    return (first_space != std::string::npos && msg.compare(0, first_space, prefix) == 0);
}

std::pair<std::string, std::string> Server::extractNames(const std::string &msg)
{
    size_t first_space = msg.find_first_of(" ");
    size_t second_space = msg.find_first_of(" ", first_space + 1);
    std::string sender_name = msg.substr(1, first_space - 1);
    std::string receiver_name = msg.substr(first_space + 2, second_space - first_space - 2);
    return {sender_name, receiver_name};
}

void Server::sendMessage(const std::string &msg)
{
    mtx.lock();

    std::string prefix = "@";
    if (this->hasSpecialPrefix(msg, prefix))
    {
        auto [sender_name, receiver_name] = this->extractNames(msg);
        this->sendToReceiverAndSender(msg, receiver_name, sender_name);
    }
    else
    {
        this->broadcastMessage(msg);
    }

    mtx.unlock();
}
