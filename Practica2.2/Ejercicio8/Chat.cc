#include "Chat.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    char* data_copy = _data;

    memcpy(data_copy, &type, sizeof(uint8_t));
    data_copy += sizeof(uint8_t);

    memcpy(data_copy, nick.c_str(), sizeof(char) * MAX_NICK_L);
    data_copy += MAX_NICK_L;

    memcpy(data_copy, message.c_str(), sizeof(char) * MAX_MESSAGE_L);
}

int ChatMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(_data, bobj, MESSAGE_SIZE);

    memcpy(&type, bobj, sizeof(uint8_t));
    bobj += sizeof(uint8_t);

    char a[MAX_NICK_L] = {};
    memcpy(a, bobj, MAX_NICK_L);
    nick = a;
    bobj += MAX_NICK_L;

    char mes[MAX_MESSAGE_L] = {};
    memcpy(mes, bobj, MAX_MESSAGE_L);
    message = mes;
    

    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatServer::do_messages()
{
    while (true)
    {
        /*
         * NOTA: los clientes están definidos con "smart pointers", es necesario
         * crear un unique_ptr con el objeto socket recibido y usar std::move
         * para añadirlo al vector
         */

        //Recibir Mensajes en y en función del tipo de mensaje
        // - LOGIN: Añadir al vector clients
        // - LOGOUT: Eliminar del vector clients
        // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)
        ChatMessage  msg;
        Socket* client;
        socket.recv(msg, client);

        if(msg.type == msg.LOGIN){
            std::unique_ptr<Socket> cl = std::unique_ptr<Socket>(client);
            clients.push_back(std::move(cl));
        }else if(msg.type == msg.LOGOUT){
            auto it = clients.begin();
            bool found = false;
            std::cout << msg.nick << " Logout\n";
            while(it!=clients.end() && !found){
                found = *(*it) == *client;
                if(!found) ++it;
                else it = clients.erase(it);
            }
            
            
        }else{
            for(auto & c : clients){
                if(!(*c == *client)){
                    socket.send(msg, *c);
                }
            }
        }

    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatClient::login()
{
    std::string msg;

    ChatMessage em(nick, msg);
    em.type = ChatMessage::LOGIN;

    socket.send(em, socket);
}

void ChatClient::logout()
{
    // Completar
    std::string msg;

    ChatMessage em(nick, msg);
    em.type = ChatMessage::LOGOUT;

    socket.send(em, socket);

}

void ChatClient::input_thread()
{
    while (true)
    {
        // Leer stdin con std::getline
        std::string msg;
        std::getline(std::cin, msg);

        ChatMessage em(nick, msg);
        em.type = ChatMessage::MESSAGE;
        if(msg == "/q" || msg == "logout"){
            em.type = ChatMessage::LOGOUT;
        }
        // Enviar al servidor usando socket
        socket.send(em, socket);
    }
}

void ChatClient::net_thread()
{
    while(true)
    {
        //Recibir Mensajes de red
        std::string msg;
        ChatMessage em(nick, msg);
        socket.recv(em);

        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
        std::cout << em.nick << ": " << em.message << "\n";
    }
}

