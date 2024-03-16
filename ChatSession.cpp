#include "ChatSession.h"

chat_session::chat_session(boost::asio::io_service &io_service, chat_room_ptr room)
    : m_socket(io_service), m_room(room), m_strand(io_service)
{
}

tcp::socket &chat_session::socket()
{
    return m_socket;
}

void chat_session::start()
{
    m_socket.async_read_some(boost::asio::mutable_buffer(const_cast<void *>(static_cast<const void *>(m_message.data())), chat_message::max_body_length),

                             m_strand.wrap(
                                 boost::bind(&chat_session::handle_read, shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred)));
}

void chat_session::deliver(const chat_message &msg)
{
    bool write_in_progress = !m_messages.empty(); // (6)
    m_messages.push_back(msg);
    if (!write_in_progress)
    {
        boost::asio::async_write(m_socket, // (7)
                                 boost::asio::buffer(m_messages.front().data(),
                                                     m_messages.front().length()),
                                 m_strand.wrap(
                                     boost::bind(&chat_session::handle_write, shared_from_this(),
                                                 boost::asio::placeholders::error)));
    }
}

void chat_session::handle_read(const boost::system::error_code &error, std::size_t bytes_transferred)
{
    if (!error)
    {
        m_room->deliver(m_message); // (3)
        start();                    // (4)
    }
}

void chat_session::handle_write(const boost::system::error_code &error)
{
    if (!error)
    {
        m_messages.pop_front(); // (8)
        if (!m_messages.empty())
        {
            boost::asio::async_write(m_socket, // (9)
                                     boost::asio::buffer(m_messages.front().data(),
                                                         m_messages.front().length()),
                                     m_strand.wrap(
                                         boost::bind(&chat_session::handle_write, shared_from_this(),
                                                     boost::asio::placeholders::error)));
        }
        else
        {
            std::cout << "Here" << std::endl;
        }
    }
}
