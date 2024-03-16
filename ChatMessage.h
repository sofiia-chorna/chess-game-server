#ifndef CHAT_MESSAGE_H
#define CHAT_MESSAGE_H

#include <iostream>
#include <boost/asio.hpp>
#include <list>

using namespace boost::asio;
using ip::tcp;
using std::cout;
using std::endl;
using std::string;

class chat_message
{

public:
    void reset()
    {
        m_list_string.clear();
        m_message.clear();
        m_login.clear();
    }

    int m_type; // (1) Type d'événement : NEW_MSG, etc.

    // Generic datas
    std::list<std::string> m_list_string; // (4)
    std::string m_message;                // (2)
    std::string m_login;                  // (3)

    // Define body() and max_body_length if they are used to access message body
    static constexpr size_t max_body_length = 1024; // Example value, adjust as needed
    char body[max_body_length];

    // Define data() and length() if they are used to access message data and its length
    const char *data() const { return body; }
    size_t length() const { return std::strlen(body); }

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & m_type & m_list_string & m_message & m_login;
    }

    enum
    {
        NEW_MSG = 0,          // Nouveau message
        PERSON_LEFT = 1,      // Information : personne ayant quittée la room
        PERSON_CONNECTED = 2, // Information : nouvelle personne connectée à la room
    };
};
#endif // CHAT_MESSAGE_H
