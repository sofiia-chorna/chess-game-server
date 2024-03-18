#include "Client.h"

int main(int argc, const char **argv, const char **envp) {
    if (argc != 2) {
        std::cerr << "Usage : " << argv[0] << " <Name>" << std::endl;
        exit(1);
    }

    Client client(argv[1]);
    client.start();

    return 0;
}
