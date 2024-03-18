#include <iostream>
#include <cstring>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <stdarg.h>

#define SERVER_PORT 5209
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 256
#define MAX_NAME_LENGTH 20
#define MAX_PRE_NAME_LENGTH 12

void handle_client(int client_socket);
void send_message(const std::string &msg);
int print_output(const char *format, ...);
int print_error(const char *format, ...);
void handle_error(const std::string &message);

int client_count = 0;
std::mutex mtx;

// Mapping of client names to their respective sockets
std::unordered_map<std::string, int> client_socket_map;

int main(int argc, const char **argv, const char **envp)
{
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_size;

    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1)
    {
        handle_error("Failed to create server socket!");
    }

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        handle_error("Failed to bind server socket!");
    }
    print_output("Server is running on port %d\n", SERVER_PORT);

    if (listen(server_socket, MAX_CLIENTS) == -1)
    {
        handle_error("Failed to listen on server socket!");
    }

    while (1)
    {
        client_address_size = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_size);
        if (client_socket == -1)
        {
            handle_error("Failed to accept client connection!");
        }

        mtx.lock();
        client_count++;
        mtx.unlock();

        std::thread th(handle_client, client_socket);
        th.detach();

        print_output("Connected client IP: %s \n", inet_ntoa(client_address.sin_addr));
    }
    close(server_socket);
    return 0;
}

// Function to handle the case when a new client joins
void handle_new_client(int client_socket, const char* msg) {
    const char new_client_prefix[MAX_PRE_NAME_LENGTH + 1] = "#new client:";
    if (strlen(msg) > MAX_PRE_NAME_LENGTH) {
        char prefix[MAX_PRE_NAME_LENGTH + 1];
        strncpy(prefix, msg, MAX_PRE_NAME_LENGTH);
        prefix[MAX_PRE_NAME_LENGTH] = '\0';
        if (strcmp(prefix, new_client_prefix) == 0) {
            char name[MAX_NAME_LENGTH];
            strcpy(name, msg + MAX_PRE_NAME_LENGTH);
            if (client_socket_map.find(name) == client_socket_map.end()) {
                print_output("Socket %d is associated with client name: %s\n", client_socket, name);
                client_socket_map[name] = client_socket;
            } else {
                std::string error_msg = std::string(name) + " already exists. Please choose another name.";
                send(client_socket, error_msg.c_str(), error_msg.length() + 1, 0);
                mtx.lock();
                client_count--;
                mtx.unlock();
                return;
            }
        }
    }
    send_message(std::string(msg));
}

// Function to handle the case when a client leaves
void handle_client_leave(int client_socket) {
    std::string leave_msg;
    std::string name;
    mtx.lock();
    for (auto it = client_socket_map.begin(); it != client_socket_map.end(); ++it) {
        if (it->second == client_socket) {
            name = it->first;
            client_socket_map.erase(it->first);
        }
    }
    client_count--;
    mtx.unlock();
    leave_msg = "Client " + name + " has left the chat room";
    send_message(leave_msg);
    print_output("Client %s has left the chat room\n", name.c_str());
    close(client_socket);
}

// Main function to handle client communication
void handle_client(int client_socket) {
    char msg[BUFFER_SIZE];
    while (recv(client_socket, msg, sizeof(msg), 0) != 0) {
        handle_new_client(client_socket, msg);
    }
    handle_client_leave(client_socket);
}

// Send a message to a specific client
void send_to_client(const std::string &msg, int client_socket) {
    send(client_socket, msg.c_str(), msg.length() + 1, 0);
}

// Send an error message to a specific client
void send_error_to_sender(const std::string &receiver_name, const std::string &sender_name) {
    std::string error_msg = "[error] client named " + receiver_name + " does not exist.";
    send_to_client(error_msg, client_socket_map[sender_name]);
}

// Send a message to all clients
void broadcast_message(const std::string &msg) {
    for (const auto &client : client_socket_map) {
        send_to_client(msg, client.second);
    }
}

// Send a message to a specific client and the sender
void send_to_receiver_and_sender(const std::string &msg, const std::string &receiver_name, const std::string &sender_name) {
    auto receiver_iter = client_socket_map.find(receiver_name);
    if (receiver_iter != client_socket_map.end()) {
        send_to_client(msg, receiver_iter->second);
        send_to_client(msg, client_socket_map[sender_name]);
    } else {
        send_error_to_sender(receiver_name, sender_name);
    }
}

void send_message(const std::string &msg) {
    mtx.lock();
    
    std::string prefix = "@";
    size_t first_space = msg.find_first_of(" ");
    if (first_space != std::string::npos && msg.compare(0, first_space, prefix) == 0) {
        size_t second_space = msg.find_first_of(" ", first_space + 1);
        if (second_space != std::string::npos) {
            std::string sender_name = msg.substr(1, first_space - 1);
            std::string receiver_name = msg.substr(first_space + 2, second_space - first_space - 2);
            send_to_receiver_and_sender(msg, receiver_name, sender_name);
        }
    } else {
        broadcast_message(msg);
    }
    
    mtx.unlock();
}

int print_output(const char *arg, ...)
{
    va_list ap;
    va_start(ap, arg);
    int result = vfprintf(stdout, arg, ap);
    va_end(ap);
    return result;
}

int print_error(const char *arg, ...)
{
    va_list ap;
    va_start(ap, arg);
    int result = vfprintf(stderr, arg, ap);
    va_end(ap);
    return result;
}

void handle_error(const std::string &message)
{
    std::cerr << message << std::endl;
    // Consider throwing an exception or returning an error code instead of exiting
    exit(1);
}
