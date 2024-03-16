#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <boost/asio.hpp>
#include "ChatRoom.h"

using boost::asio::ip::tcp;

class chat_server
{
public:
    chat_server(boost::asio::io_service &io_service, const tcp::endpoint &endpoint);

    void wait_for_connection();
    void handle_accept(const boost::system::error_code &error, chat_session_ptr new_session);

private:
    boost::asio::io_service &m_io_service;
    tcp::acceptor m_acceptor;
    chat_room_ptr m_room;
};

#endif // CHAT_SERVER_H
