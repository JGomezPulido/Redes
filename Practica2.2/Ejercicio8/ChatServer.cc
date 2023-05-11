#include "Chat.h"

int main(int argc, char **argv)
{
    if(argc < 3){
        std::cout << "Parametros requeridos : IP server, puerto server\n";
        return -1;
    }
    ChatServer es(argv[1], argv[2]);

    es.do_messages();

    return 0;
}

