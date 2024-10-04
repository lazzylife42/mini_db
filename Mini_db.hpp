#ifndef MINI_DB_HPP
#define MINI_DB_HPP

// #include "Server.hpp"
// #include "Socket.hpp"

#include <map>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <fstream>

class Mini_db
{
    private:
        std::string _path;
        std::map<std::string, std::string> _db;

    public:
        Mini_db(std::string path);
        ~Mini_db();

        int POST(std::string key, std::string val);
        int GET(std::string key, std::string& response);
        int DELETE(std::string key);

        void backup();
        void loadBackup();
        void print() const;
        
};

#endif