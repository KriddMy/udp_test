#include <iostream>

#include "PacketReciever.h"

int main()
{
    try {
        std::cout << "Starting server on port: " << SERVER_PORT << "." << std::endl;
    
        PacketReciever server(1000); // 1 mb/s
        if(server.open(SERVER_PORT) < 0) {
            std::cerr << "Server: Failed to open connection.";
            return 1;
        }

        while(true) {
            server.acceptPacket(10); //10 sec
        }
    } catch(...) { std::cerr << "Server: Unknown Error Occured... Exiting"; }
    

    return 0;
}