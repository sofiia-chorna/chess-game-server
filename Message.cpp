#include <iostream>
#include <boost/asio.hpp>
#include <list>
#include <Message.h>

using namespace boost::asio;
using ip::tcp;
using std::string;

void Message::reset()
{
    this->content.clear();
}

const char *Message::data() const
{
    return this->content;
}

size_t length() const
{
    return std::strlen(this->content);
}
