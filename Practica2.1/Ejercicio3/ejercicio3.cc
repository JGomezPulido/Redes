#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

using namespace std;
int main(int argc ,char** argv)
{
    if(argc < 4){
        cerr << "No hay argumentos suficientes, especifique dominio o ip y puerto, y el comando a enviar\n";
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
        freeaddrinfo(res);
        return -1;
    }

    int err = connect(sd,(struct sockaddr *) res->ai_addr, res->ai_addrlen);

    if(err == -1){
        freeaddrinfo(res);
        close(sd);
        return -1;
    }

    char msg[1024];
    strcpy(msg, argv[3]);
    msg[1023] = '\0';
  
    sendto(sd, msg, strlen(msg), 0, res->ai_addr, res->ai_addrlen);

    char buff[1024]; 
    ssize_t bytes = recvfrom(sd, buff, 1024, 0, res->ai_addr, &res->ai_addrlen);
    buff[bytes] = '\0';
    cout << buff << "\n";
    close(sd);
    freeaddrinfo(res);
    return 0;
}