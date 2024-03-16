#ifndef CHAT_ROOM_H
#define CHAT_ROOM_H

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <set>
#include "ChatMessage.h"

using boost::asio::ip::tcp;

class chat_session;
class chat_server;
typedef boost::shared_ptr<chat_session> chat_session_ptr;

class chat_room
{
public:
    chat_room(boost::asio::io_service& io_service); // Constructor modified

    void join(chat_session_ptr participant);
    void leave(chat_session_ptr participant);
    void deliver(const chat_message& msg);

private:
    std::set<chat_session_ptr> m_participants;
    boost::asio::io_service& m_io_service; // Added io_service member
};

typedef boost::shared_ptr<chat_room> chat_room_ptr;
typedef boost::weak_ptr<chat_room> chat_room_wptr;

#endif // CHAT_ROOM_H
