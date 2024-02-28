#include "ChessServer.h"

int main()
{
    try
    {
        boost::asio::io_service io_service;
        ChessServer server(io_service, 1234);
        io_service.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
