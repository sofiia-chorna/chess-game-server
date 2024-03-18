#ifndef SERVER_H
#define SERVER_H

#include <mutex>
#include <unordered_map>

class Server
{
public:
    Server(int port);
    ~Server();
    void start();

private:
    int serverSocket;
    std::mutex mtx;
    int clientCount;
    std::unordered_map<std::string, int> clientSocketMap;
    void createAndBindServerSocket(int server_socket);
    void acceptClientConnections();
    void handleNewClient(int clientSocket, const char *msg);
    void handleClientLeave(int clientSocket);

    // Main function to handle client communication
    void handleClient(int clientSocket);
    void sendToClient(const std::string &msg, int clientSocket);
    void sendErrorToSender(const std::string &receiverName, const std::string &senderName);

    // Send a message to all clients
    void broadcastMessage(const std::string &msg);
    void sendToReceiverAndSender(const std::string &msg, const std::string &receiverName, const std::string &senderName);
    bool hasSpecialPrefix(const std::string &msg, const std::string &prefix);

    // Function to extract sender and receiver names from a message
    std::pair<std::string, std::string> extractNames(const std::string &msg);
    void sendMessage(const std::string &msg);
};

#endif // SERVER_H
