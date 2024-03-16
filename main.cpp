#include <iostream>
#include <boost/asio.hpp>
#include "ChatServer.h"

int main()
{
    try
    {
        // Create an io_context object
        boost::asio::io_context io_context;

        // Create a TCP endpoint for accepting connections on port 8080
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 8080);

        // Create a chat_server object and start accepting connections
        chat_server server(io_context, endpoint);

        // Run the io_context to start processing asynchronous events
        io_context.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
