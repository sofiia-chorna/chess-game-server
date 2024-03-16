#ifndef CHAT_SESSION_H
#define CHAT_SESSION_H

#include <boost/asio.hpp>
#include "ChatRoom.h"
#include "ChatMessage.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <deque>

using boost::asio::ip::tcp;

class chat_session : public boost::enable_shared_from_this<chat_session>
{
public:
    chat_session(boost::asio::io_service &io_service, chat_room_ptr room);

    tcp::socket &socket();

    void start();

    void deliver(const chat_message &msg);

private:
    void handle_read(const boost::system::error_code &error, std::size_t bytes_transferred);

    void handle_write(const boost::system::error_code &error);

    boost::asio::io_service::strand m_strand;
    tcp::socket m_socket;
    chat_room_ptr m_room;
    chat_message m_message;
    std::deque<chat_message> m_messages; // Added m_messages member
};

typedef boost::shared_ptr<chat_session> chat_session_ptr;

#endif // CHAT_SESSION_H
