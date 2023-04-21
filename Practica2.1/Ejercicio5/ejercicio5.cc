#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <string>

using namespace std;
int main(int argc ,char** argv)
{
    if(argc < 3){
        cerr << "No hay argumentos suficientes (" << argc << "), especifique dominio o ip y puerto\n";
        return -1;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    struct addrinfo* res;

    int sc = getaddrinfo(argv[1], argv[2], &hints, &res);

    if(sc != 0){
        cerr << "[getaddrinfo]: " << gai_strerror(sc) << "\n";
        return -1;
    }

    int sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if(sd < 0){
        cout << "Error en la creacion del socket, cerrando el programa... Código de error: " << errno << "\n";
        freeaddrinfo(res);
        return -1;
    }

    int cd = connect(sd, res->ai_addr, res->ai_addrlen);

    if(cd == -1){
        cout << "Error en listen, cerrando el programa... Código de error: " << errno << "\n";
        freeaddrinfo(res);
        close(sd);
        return -1;
    }
    string buffer;
    char received[80];
    
    sockaddr cliente;
    socklen_t cliente_len = sizeof(struct sockaddr);
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    while(true){
        cin >> buffer;

        if(buffer == "Q"){
            freeaddrinfo(res);
            close(cd);
            break;
        }
        
        ssize_t enviados = send(sd, buffer.c_str(), buffer.length(), 0);
       
        ssize_t bytes = recv(sd, received, 80, 0);
        if(bytes > -1 && bytes < 80) received[bytes] = '\0';
        cout << received << "\n";
    }
    return 0;
}