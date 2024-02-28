#include <iostream>
#include <memory>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(tcp::socket socket);

    void start();

private:
    void doRead();
    void doWrite(const std::string &message);
    void handleRequest(const char *data, std::size_t length);

    tcp::socket socket_;
    enum
    {
        max_length = 1024
    };
    char data_[max_length];
};
