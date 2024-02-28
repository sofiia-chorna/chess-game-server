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
                doWrite(length);
            }
        });
}

void Session::doWrite(std::size_t length)
{
    auto self(shared_from_this());
    boost::asio::async_write(
        socket_, boost::asio::buffer(data_, length),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
            {
                doRead();
            }
        });
}
