#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main()
{
    try
    {
        boost::asio::io_context io_context;

        // Create a socket and connect to the chat server
        tcp::socket socket(io_context);
        tcp::resolver resolver(io_context);
        boost::asio::connect(socket, resolver.resolve("localhost", "8080"));

        // Perform chat operations here...
        while (true)
        {
            // Send message to the server
            std::cout << "Enter message: ";
            std::string message;
            std::getline(std::cin, message);
            boost::asio::write(socket, boost::asio::buffer(message + "\n"));

            // Receive message from the server
            boost::asio::streambuf receive_buffer;
            boost::asio::read_until(socket, receive_buffer, "\n");
            std::string received_message(boost::asio::buffers_begin(receive_buffer.data()),
                                         boost::asio::buffers_end(receive_buffer.data()));
            std::cout << "Received: " << received_message << std::endl;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
