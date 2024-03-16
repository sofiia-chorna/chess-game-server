#include "ChatServer.h"
#include "ChatSession.h"

chat_server::chat_server(boost::asio::io_service &io_service, const tcp::endpoint &endpoint)
    : m_io_service(io_service), m_acceptor(io_service, endpoint), m_room(new chat_room(io_service))
{
    wait_for_connection();
}

void chat_server::wait_for_connection()
{
    chat_session_ptr new_session(new chat_session(m_io_service, m_room)); // (2)
    m_acceptor.async_accept(new_session->socket(),                        // (3)
                            boost::bind(&chat_server::handle_accept, this,
                                        boost::asio::placeholders::error, new_session));
}

void chat_server::handle_accept(const boost::system::error_code &error, chat_session_ptr new_session) // (4)
{
    if (!error)
    {
        new_session->start(); // (5)
    }
    wait_for_connection(); // (6)
}
