#include <thread>
#include <functional>
#include <signal.h>
#include "Chat.h"

int main(int argc, char **argv)
{
    if(argc < 4){
        std::cout << "Parametros requeridos : IP server, puerto server, nickname\n";
        return -1;
    }
    try{
        ChatClient ec(argv[1], argv[2], argv[3]);
        struct sigaction sig;
        sigaction(SIGTERM, &sig, NULL);
        std::thread net_thread([&ec](){ ec.net_thread(); });

        ec.login();

        ec.input_thread();
    }catch (std::string c){
        std::cout << c;
    }catch(std::exception e){
        std::cout << e.what();
    }
}

