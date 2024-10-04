#ifndef SERVER_HPP
#define SERVER_HPP

#include "Socket.hpp"
#include "mini_db.hpp"

#include <iostream>
#include <vector>
#include <sys/select.h> // Pour la fonction select()
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h> // Pour rendre les sockets non bloquants
#include <csignal> // Pour gérer les signaux (Ctrl+C)

class Server
{
private:
    int port;
    Socket _listeningSocket;        // Socket d'écoute
    std::vector<int> clientSockets; // Liste des sockets des clients connectés
    std::vector<int> clientIDs;     // Liste des IDs des clients
    int clientCounter;              // Compteur pour attribuer un ID unique à chaque client
    bool running;                   // Indicateur pour savoir si le serveur tourne

    // Pointeur vers l'instance du serveur pour le gestionnaire de signaux
    static Server *instance;

    // Méthode pour rendre un socket non bloquant
    void makeSocketNonBlocking(int sockfd)
    {
        int flags = fcntl(sockfd, F_GETFL, 0);
        if (flags == -1)
        {
            std::cerr << "Erreur: Impossible de récupérer les flags du socket." << std::endl;
            return;
        }
        if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            std::cerr << "Erreur: Impossible de définir le socket comme non-bloquant." << std::endl;
        }
    }

    // Méthode pour fermer proprement le serveur et les clients
    void closeServer()
    {
        std::cout << "\nFermeture du serveur..." << std::endl;
        for (std::vector<int>::iterator it = clientSockets.begin(); it != clientSockets.end(); ++it)
        {
            close(*it);
        }
        clientSockets.clear();
        clientIDs.clear();
        running = false;  // Mettez running à false pour sortir de la boucle dans run()
        close(_listeningSocket.getSockFd());
        std::cout << "Serveur fermé." << std::endl;
    }

    // Gestion du signal SIGINT
    static void signalHandler(int signum)
    {
        std::cout << "\nSignal (" << signum << ") reçu. Fermeture du serveur..." << std::endl;
        if (instance)
        {
            instance->closeServer();   // Appel à l'instance
        }
    }


public:
    // Constructeur qui prend un port
    Server(int port) : port(port), _listeningSocket(port), clientCounter(1), running(true)
    {
        instance = this; // Initialiser le pointeur d'instance
    }

    // Méthode pour démarrer le serveur
    void run()
    {
        // Bind et écoute via _listeningSocket
        if (!_listeningSocket.bindAndListen())
        {
            std::cerr << "Erreur lors du démarrage du serveur." << std::endl;
            return;
        }

        // Rendre le socket d'écoute non bloquant
        makeSocketNonBlocking(_listeningSocket.getSockFd());

        // Capturer le signal SIGINT (Ctrl+C)
        std::signal(SIGINT, signalHandler);

        fd_set readfds;
        int max_fd = _listeningSocket.getSockFd(); // Le descripteur le plus grand
        struct sockaddr_in clientAddr;

        std::cout << "Server running on 127.0.0.1 on port [" << port << "]" << std::endl;

        while (running)
        {
            FD_ZERO(&readfds);                              // Remet à zéro le set
            FD_SET(_listeningSocket.getSockFd(), &readfds); // Ajouter le socket d'écoute

            // Ajouter les clients existants aux set des descripteurs surveillés
            for (size_t i = 0; i < clientSockets.size(); ++i)
            {
                FD_SET(clientSockets[i], &readfds);
                if (clientSockets[i] > max_fd)
                {
                    max_fd = clientSockets[i]; // Garder le max pour `select()`
                }
            }

            // Utiliser select pour surveiller l'activité sur les sockets
            int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);

            if (activity < 0 && running)
            {
                std::cerr << "Erreur: `select()` a échoué." << std::endl;
                continue;
            }

            // Vérifier si une nouvelle connexion est en attente
            if (FD_ISSET(_listeningSocket.getSockFd(), &readfds))
            {
                if (!running)
                    break;

                socklen_t clientLen = sizeof(clientAddr);
                int newClientSock = _listeningSocket.accepte(clientAddr);
                if (newClientSock >= 0)
                {
                    std::cout << "Client " << clientCounter << " connecté." << std::endl;

                    // Ajouter le nouveau socket client à la liste
                    makeSocketNonBlocking(newClientSock);
                    clientSockets.push_back(newClientSock);
                    clientIDs.push_back(clientCounter); // Associer un ID unique au client

                    // Incrémenter le compteur pour le prochain client
                    clientCounter++;
                }
            }

            // Vérifier l'activité sur les sockets des clients existants
            for (size_t i = 0; i < clientSockets.size(); ++i)
            {
                int clientSock = clientSockets[i];
                if (FD_ISSET(clientSock, &readfds))
                {
                    char buffer[1024];
                    ssize_t bytesRead = read(clientSock, buffer, sizeof(buffer) - 1);
                    if (bytesRead <= 0)
                    {
                        // Si aucune donnée n'est reçue ou en cas de déconnexion
                        std::cout << "Client " << clientIDs[i] << " déconnecté." << std::endl;
                        close(clientSock);
                        clientSockets.erase(clientSockets.begin() + i);
                        clientIDs.erase(clientIDs.begin() + i);
                        --i; // Corriger l'indice après suppression
                    }
                    else
                    {
                        // Echo des données reçues
                        buffer[bytesRead] = '\0';
                        std::cout << "Client " << clientIDs[i] << " | " << buffer;
                    }
                }
            }
        }

        // // Fermer proprement le serveur
        // closeServer();
    }
};

// Initialisation du pointeur statique
Server *Server::instance = 0;

#endif // SERVER_HPP
