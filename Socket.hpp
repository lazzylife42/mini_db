#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <iostream>

class Socket {
private:
    int sockfd;
    int port;

public:
    Socket(int port) : port(port), sockfd(-1) {}

    ~Socket() {
        if (sockfd >= 0) {
            close(sockfd);
        }
    }

    int getSockFd() const {
        return sockfd;
    }

    bool bindAndListen() {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            std::cerr << "Erreur: Impossible de créer le socket." << std::endl;
            return false;
        }

        // Option SO_REUSEADDR pour éviter les problèmes de "binding"
        int opt = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "Erreur: Impossible de définir l'option SO_REUSEADDR." << std::endl;
            return false;
        }

        struct sockaddr_in serverAddr;
        std::memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Erreur: Impossible de binder le socket au port " << port << std::endl;
            return false;
        }

        if (listen(sockfd, 10) < 0) {
            std::cerr << "Erreur: Impossible de mettre le socket en écoute." << std::endl;
            return false;
        }

        return true;
    }

    int accepte(struct sockaddr_in& clientAddr) {
        socklen_t clientLen = sizeof(clientAddr);
        int clientSock = accept(sockfd, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSock < 0) {
            std::cerr << "Erreur: Impossible d'accepter la connexion." << std::endl;
            return -1;
        }
        return clientSock;
    }
};

#endif // SOCKET_HPP
