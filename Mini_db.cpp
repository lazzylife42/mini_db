#include "Mini_db.hpp"

Mini_db::Mini_db(std::string path) : _path(path)
{
    loadBackup();
}

Mini_db::~Mini_db()
{
    backup();
}

int Mini_db::POST(std::string key, std::string val) {
    _db[key] = val; // Exemple de fonction POST
    return 0;
}

int Mini_db::GET(std::string key, std::string& response) {
    if (_db.find(key) != _db.end()) {
        response = _db[key];
        return 0;
    }
    return -1; // Erreur si la clé n'est pas trouvée
}

int Mini_db::DELETE(std::string key) {
    return _db.erase(key); // Retirer la clé et retourner le nombre d'éléments supprimés
}

void Mini_db::backup()
{
    // Utiliser un flux de sortie de fichier
    std::ofstream ofs(_path.c_str(), std::ios::binary);
    if (!ofs) {
        std::cerr << "Erreur: Impossible d'ouvrir le fichier de sauvegarde." << std::endl;
        return;
    }

    // Itération sur chaque paire clé-valeur dans la map
    for (std::map<std::string, std::string>::iterator it = _db.begin(); it != _db.end(); ++it) {
        std::string key = it->first;
        std::string value = it->second;

        size_t keyLength = key.size();
        size_t valueLength = value.size();

        // Écrire la taille de la clé et la clé elle-même
        ofs.write(reinterpret_cast<const char*>(&keyLength), sizeof(keyLength));
        ofs.write(key.c_str(), keyLength);

        // Écrire la taille de la valeur et la valeur elle-même
        ofs.write(reinterpret_cast<const char*>(&valueLength), sizeof(valueLength));
        ofs.write(value.c_str(), valueLength);
    }

    ofs.close();
}

void Mini_db::loadBackup()
{
    // Utiliser un flux d'entrée de fichier
    std::ifstream ifs(_path.c_str(), std::ios::binary);
    if (!ifs) {
        std::cerr << "Erreur: Impossible d'ouvrir le fichier de sauvegarde." << std::endl;
        return;
    }

    while (true) {
        size_t keyLength;
        ifs.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));
        if (ifs.eof()) break;

        char* keyBuffer = new char[keyLength + 1]; // +1 pour le caractère nul
        ifs.read(keyBuffer, keyLength);
        keyBuffer[keyLength] = '\0'; // Ajout du caractère nul

        std::string key(keyBuffer);
        delete[] keyBuffer; // Libération de la mémoire

        size_t valueLength;
        ifs.read(reinterpret_cast<char*>(&valueLength), sizeof(valueLength));
        char* valueBuffer = new char[valueLength + 1]; // +1 pour le caractère nul
        ifs.read(valueBuffer, valueLength);
        valueBuffer[valueLength] = '\0'; // Ajout du caractère nul

        std::string value(valueBuffer);
        delete[] valueBuffer; // Libération de la mémoire

        // Récupérer la paire clé-valeur dans la map
        _db[key] = value; 
    }

    ifs.close();
}

void Mini_db::print() const
{
    std::cout << "Contenu de la base de données :" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = _db.begin(); it != _db.end(); ++it) {
        std::cout << it->first << " : " << it->second << std::endl;
    }
}


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <path_to_db> <values...>" << std::endl;
        return 1;
    }

    Mini_db db(argv[1]);

    for (int i = 2; i < argc; ++i)
    {
        std::string key = "key <" + std::string(argv[i]) + ">";
        db.POST(key, argv[i]);
    }

    db.print();
    db.backup();

    Mini_db test(argv[1]);
    test.loadBackup();

    test.print();

    return 0;
}
