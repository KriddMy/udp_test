#include <iostream>

#include "PacketReciever.h"

int main()
{
    std::cout << "Starting server on port: " << SERVER_PORT << "." << std::endl;
    
    PacketReciever server(1000); // 1 mb/s
    if(server.open(SERVER_PORT) < 0) {
        std::cerr << "Server: Failed to open connection.";
        return 1;
    }

    while(true) {
        server.acceptPacket(10); //10 sec
    }

    return 0;
}