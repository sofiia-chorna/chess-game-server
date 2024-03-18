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
int print_formatted_output(FILE *stream, const char *format, ...);
void handle_error(const std::string &message);

std::mutex mtx;
int client_count = 0;

// Mapping of client names to their respective sockets
std::unordered_map<std::string, int> client_socket_map;

// Function to create and bind the server socket
int create_and_bind_server_socket()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1)
    {
        handle_error("Failed to create server socket!");
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        handle_error("Failed to bind server socket!");
    }
    print_formatted_output(stdout, "Server is running on port %d\n", SERVER_PORT);

    if (listen(server_socket, MAX_CLIENTS) == -1)
    {
        handle_error("Failed to listen on server socket!");
    }
    return server_socket;
}

// Function to accept client connections
void accept_client_connections(int server_socket)
{
    while (1)
    {
        struct sockaddr_in client_address;
        socklen_t client_address_size = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_size);
        if (client_socket == -1)
        {
            handle_error("Failed to accept client connection!");
        }

        mtx.lock();
        client_count++;
        mtx.unlock();

        std::thread th(handle_client, client_socket);
        th.detach();

        print_formatted_output(stdout, "Connected client IP: %s \n", inet_ntoa(client_address.sin_addr));
    }
}

int main(int argc, const char **argv, const char **envp)
{
    int server_socket = create_and_bind_server_socket();
    accept_client_connections(server_socket);
    close(server_socket);
    return 0;
}

// Function to handle the case when a new client joins
void handle_new_client(int client_socket, const char *msg)
{
    const char new_client_prefix[MAX_PRE_NAME_LENGTH + 1] = "#new client:";
    if (strlen(msg) <= MAX_PRE_NAME_LENGTH)
    {
        // Message does not contain the new client prefix, forward it as a regular message
        send_message(std::string(msg));
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
        if (client_socket_map.find(name) == client_socket_map.end())
        {
            // Associate the client name with the client socket
            print_formatted_output(stdout, "Socket %d is associated with client name: %s\n", client_socket, name);
            client_socket_map[name] = client_socket;
        }
        else
        {
            // Notify the client that the name already exists
            std::string error_msg = std::string(name) + " already exists. Please choose another name.";
            send(client_socket, error_msg.c_str(), error_msg.length() + 1, 0);
            mtx.lock();
            client_count--;
            mtx.unlock();
            return;
        }
    }

    // Forward the message to all clients
    send_message(std::string(msg));
}

// Function to handle the case when a client leaves
void handle_client_leave(int client_socket)
{
    std::string leave_msg;
    std::string name;
    mtx.lock();
    for (auto it = client_socket_map.begin(); it != client_socket_map.end(); ++it)
    {
        if (it->second == client_socket)
        {
            name = it->first;
            client_socket_map.erase(it->first);
        }
    }
    client_count--;
    mtx.unlock();
    leave_msg = "Client " + name + " has left the chat room";
    send_message(leave_msg);
    print_formatted_output(stdout, "Client %s has left the chat room\n", name.c_str());
    close(client_socket);
}

// Main function to handle client communication
void handle_client(int client_socket)
{
    char msg[BUFFER_SIZE];
    while (recv(client_socket, msg, sizeof(msg), 0) != 0)
    {
        handle_new_client(client_socket, msg);
    }
    handle_client_leave(client_socket);
}

// Send a message to a specific client
void send_to_client(const std::string &msg, int client_socket)
{
    send(client_socket, msg.c_str(), msg.length() + 1, 0);
}

// Send an error message to a specific client
void send_error_to_sender(const std::string &receiver_name, const std::string &sender_name)
{
    std::string error_msg = "[error] client named " + receiver_name + " does not exist.";
    send_to_client(error_msg, client_socket_map[sender_name]);
}

// Send a message to all clients
void broadcast_message(const std::string &msg)
{
    for (const auto &client : client_socket_map)
    {
        send_to_client(msg, client.second);
    }
}

// Send a message to a specific client and the sender
void send_to_receiver_and_sender(const std::string &msg, const std::string &receiver_name, const std::string &sender_name)
{
    auto receiver_iter = client_socket_map.find(receiver_name);
    if (receiver_iter != client_socket_map.end())
    {
        send_to_client(msg, receiver_iter->second);
        send_to_client(msg, client_socket_map[sender_name]);
    }
    else
    {
        send_error_to_sender(receiver_name, sender_name);
    }
}

// Function to check if a message has a special prefix
bool has_special_prefix(const std::string &msg, const std::string &prefix)
{
    size_t first_space = msg.find_first_of(" ");
    return (first_space != std::string::npos && msg.compare(0, first_space, prefix) == 0);
}

// Function to extract sender and receiver names from a message
std::pair<std::string, std::string> extract_names(const std::string &msg)
{
    size_t first_space = msg.find_first_of(" ");
    size_t second_space = msg.find_first_of(" ", first_space + 1);
    std::string sender_name = msg.substr(1, first_space - 1);
    std::string receiver_name = msg.substr(first_space + 2, second_space - first_space - 2);
    return {sender_name, receiver_name};
}

// Main function to send a message
void send_message(const std::string &msg)
{
    mtx.lock();

    std::string prefix = "@";
    if (has_special_prefix(msg, prefix))
    {
        auto [sender_name, receiver_name] = extract_names(msg);
        send_to_receiver_and_sender(msg, receiver_name, sender_name);
    }
    else
    {
        broadcast_message(msg);
    }

    mtx.unlock();
}

int print_formatted_output(FILE *stream, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int result = vfprintf(stream, format, args);
    va_end(args);
    return result;
}

void handle_error(const std::string &message)
{
    std::cerr << message << std::endl;
    // Consider throwing an exception or returning an error code instead of exiting
    exit(1);
}
