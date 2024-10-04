#include "Server.hpp"

int main() {
    int port = 8080;  // Port du serveur

    Server server(port);
    server.run();  // Lancer le serveur

    return 0;
}
