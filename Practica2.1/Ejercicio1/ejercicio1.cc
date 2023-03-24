#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <unistd.h>


int main(int argc ,char** argv)
{
    if(argc < 2){
        std::cerr << "No hay argumentos suficientes, especifique dominio o ip\n";
        return -1;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    struct addrinfo* res;

    int sc = getaddrinfo(argv[1], NULL, &hints, &res);

    if(sc != 0){
        std::cerr << "[getaddrinfo]: " << gai_strerror(sc) << "\n";
        return -1;
    }

    struct addrinfo* it;
    for(it = res; it != NULL; it = it->ai_next){
        char host[NI_MAXHOST];
        char serv[NI_MAXSERV];
        int code = getnameinfo(it->ai_addr, it->ai_addrlen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST);
        if(code != 0){
            std::cerr << "[getnameinfo]: " << gai_strerror(code) << "\n";
            continue;    
        }
        std::cout << host << " " << it->ai_family << " " << it->ai_socktype <<  "\n";
    }
    freeaddrinfo(res);
    return 0;
}