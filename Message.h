#ifndef CHAT_MESSAGE_H
#define CHAT_MESSAGE_H

#include <iostream>
#include <boost/asio.hpp>
#include <list>
#include <Message.enum.h>

using namespace boost::asio;
using ip::tcp;
using std::string;

class Message
{

public:
    MessageType messageType;
    static constexpr size_t max_content_length = 1024;
    std::string content;

    const char *data() const;
    size_t length() const;
    void reset();
};

#endif // CHAT_MESSAGE_H
