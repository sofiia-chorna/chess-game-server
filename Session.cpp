#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "Session.h"

using namespace boost::asio;
using ip::tcp;

Session::Session(tcp::socket socket) : socket_(std::move(socket)) {}

void Session::start()
{
    doRead();
}

void Session::doRead()
{
    auto self(shared_from_this());
    socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                std::cout << "Received message from client: " << data_ << "\n";
                handleRequest(data_, length); // Call handleRequest instead of doWrite
            }
        });
}

void Session::doWrite(const std::string &message)
{
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(message),
                             [this, self](boost::system::error_code ec, std::size_t /*length*/)
                             {
                                 if (ec)
                                 {
                                     std::cerr << "Error sending response: " << ec.message() << "\n";
                                 }
                             });
}

void Session::handleRequest(const char *data, std::size_t length)
{
    std::string request(data, length);
    // Respond with a simple HTTP message
    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
    doWrite(response);
    //        // Handle the request from the client
    //        std::string response = "Server received: " + request;
    //        doWrite(response);
}
