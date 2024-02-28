#include "Session.h"

class ChessServer
{
public:
    ChessServer(io_service &io_service, short port);

private:
    void startAccept();

    tcp::acceptor acceptor_;
};
