#include "ChatRoom.h"
#include "ChatSession.h"
#include "ChatServer.h"

chat_room::chat_room(boost::asio::io_service& io_service)
    : m_io_service(io_service) // Initialize m_io_service in the constructor initialization list
{
    std::cout << "New room" << std::endl;
}


void chat_room::join(chat_session_ptr participant)
{
    m_participants.insert(participant);

    // Inform the sessions of the room // (1)
    chat_message e;
    e.m_type = chat_message::PERSON_CONNECTED;
    deliver(e);
}

void chat_room::leave(chat_session_ptr participant)
{
    // Inform the sessions of the room // (2)
    chat_message e;
    e.m_type = chat_message::PERSON_LEFT;
    deliver(e);

    m_participants.erase(participant);
}

void chat_room::deliver(const chat_message& msg)
{
    std::for_each(m_participants.begin(), m_participants.end(),
                  boost::bind(&chat_session::deliver, _1, boost::ref(msg)));
}
