#include "ChessServer.h"

ChessServer::ChessServer(io_service &io_service, short port)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
    startAccept();
}

void ChessServer::startAccept()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                std::cout << "New client connected\n";
                std::make_shared<Session>(std::move(socket))->start();
            }
            startAccept();
        });
}
