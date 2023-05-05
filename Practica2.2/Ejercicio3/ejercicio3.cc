#include "../Serializable.h"
#include <iostream>
#include <string>
#include <cstring>

#include "fcntl.h"
#include <stdio.h>
#include <unistd.h>

class Player : public Serializable
{
public:

    Player(const char * _n, int16_t _x, int16_t _y):Serializable(), pos_x(_x),pos_y(_y)
    {
        strncpy(name, _n, MAX_NAME);
    };
    ~Player()
    {
        //delete name;
    };

    void to_bin() override {

        int totalSize = sizeof(int16_t) * 2 + sizeof(char) * MAX_NAME;
        alloc_data(totalSize);

        char* data_copy = _data;

        memcpy(data_copy, &pos_x, sizeof(int16_t));
        data_copy += sizeof(int16_t);

        memcpy(data_copy, &pos_y, sizeof(int16_t));
        data_copy += sizeof(int16_t);

        memcpy(data_copy, name, sizeof(char) * MAX_NAME);

    };
    int from_bin(char * data) override {
        int totalSize = sizeof(int16_t) * 2 + sizeof(char) * MAX_NAME;

        memcpy(&pos_x, data, sizeof(int16_t));
        data += sizeof(int16_t);

        memcpy(&pos_y, data, sizeof(int16_t));
        data += sizeof(int16_t);

        memcpy(name, data, sizeof(char) * MAX_NAME);
    };

    int16_t getX()
    {
        return pos_x;
    }

    int16_t getY()
    {
        return pos_y;
    }

    const char* getName()
    {
        return name;
    }
    
private:
    static const size_t MAX_NAME = 20;

    int16_t pos_x;
    int16_t pos_y;
  
    char name[MAX_NAME];
};

// El comando od permite abrir los archivos de texto y leerlos como si utilizara caracteres hexadecimales, en octal en ascii, etc
// La salida del comando si lo mostramos en ascii podemos ver el nombre que le hayamos puesto

int main()
{
    Player p1 = Player("juan", 200, 30);

    p1.to_bin();

    char* buf = new char[p1.size()];

    int file = open("player", O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
    write(file, p1.data(), p1.size());
    lseek(file, 0, SEEK_SET);
    read(file, buf, p1.size());
    close(file);

    Player result = Player("", 0, 0);
    result.from_bin(buf);

    std::cout << result.getName() << " " << result.getX() << " " << result.getY() << "\n";

    return 0;
}