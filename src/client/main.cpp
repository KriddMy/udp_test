#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include"FileSender.h"

int main(int argc, char* argv[])
{
    if( argc < 2) {
        std::cerr << "Client: Usage: " << argv[0] << " <file_path1> <file_path2> ..." << std::endl;
        return 1;
    }

    std::cout << "Starting client app. Begining to sent files..." << std::endl;

    FileSender fileSender(1000); // 1 mb/s
    if(fileSender.open("127.0.0.1", SERVER_PORT) < 0) {
        std::cerr << "Client: Socket creation failed" << std::endl;
        return 1;
    }

    std::vector<std::string> files;
    for(int i = 1; i < argc; ++i) {
        std::istringstream iss(argv[i]);
        std::string file;
        while(iss >> file) {
            files.push_back(file);
        }
    }

    if(fileSender.add(files) < 0) 
        std::cerr << "Client: Error occured while opening file." << std::endl;

    while(fileSender.send_random()) {}

    std::cout << "Client: All files sent. Exiting client app." << std::endl;
    return 0;
}