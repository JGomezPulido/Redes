#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <unistd.h>

using namespace std;
int main(int argc ,char** argv)
{
    if(argc < 3){
        cerr << "No hay argumentos suficientes, especifique dominio o ip y puerto\n";
        return -1;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    struct addrinfo* res;

    int sc = getaddrinfo(argv[1], argv[2], &hints, &res);

    if(sc != 0){
        cerr << "[getaddrinfo]: " << gai_strerror(sc) << "\n";
        return -1;
    }

    int sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if(sd < 0){
        std::cout << "Error en la creacion del socket, cerrando el programa... Código de error: " << errno << "\n";
        return -1;
    }

    int err = bind(sd,(struct sockaddr *) res->ai_addr, res->ai_addrlen);

    if(err == 1){

        return -1;
    }

    char buffer[80];
    sockaddr cliente;
    socklen_t cliente_len = sizeof(struct sockaddr);
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    while(true){
        ssize_t bytes = recvfrom(sd, buffer, 80, 0, &cliente, &cliente_len);

        if(bytes == -1){
            cout << "ERROR\n";
            continue;
        }
        buffer[bytes] = '\0';

        getnameinfo(&cliente, cliente_len, host, NI_MAXHOST, 
            serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

        cout << bytes << " bytes recibidos de [" << host << ":" << serv << "]\n";

        sendto(sd, buffer, bytes, 0, &cliente, cliente_len);
    }
    return 0;
}
