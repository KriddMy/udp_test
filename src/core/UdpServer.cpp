#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <chrono>
#include <thread>

#include "UdpServer.h"

UdpServer::UdpServer(const uint16_t bitrate) : m_fdSocket(-1), m_bitrate(bitrate), m_port(0)
{
    
}

UdpServer::~UdpServer()
{
    close();
}

int UdpServer::open(const uint16_t port)
{
    m_port = port;
    return open();
}

void UdpServer::close()
{
    ::close(m_fdSocket);
    m_fdSocket = -1;
}

unsigned int UdpServer::getBitrate() const
{
    return m_bitrate;
}

void UdpServer::setBitrate(const uint16_t  bitrate)
{
    m_bitrate = bitrate;
}

ssize_t UdpServer::read(u_char* buffer, size_t length, sockaddr_in &client_addr)
{
    socklen_t add_length = sizeof(client_addr);
    return recvfrom(m_fdSocket, buffer, length, 0, (sockaddr*)&client_addr, &add_length);
}

ssize_t UdpServer::send(const u_char* buffer, size_t length, const sockaddr_in &addr)
{
    static uint16_t last_size = 0;
    static auto last_time = std::chrono::steady_clock::now();
    auto time_passed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_time);
    
    float current_bitrate = 0;
    if(time_passed.count())
        current_bitrate = 1000 * last_size / (time_passed.count());

    if(current_bitrate > m_bitrate) {
        auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::milliseconds(1000 * last_size / m_bitrate) - time_passed); 
        std::this_thread::sleep_for(delay);
    }

    last_size = length * 8;
    last_time = std::chrono::steady_clock::now();
    return sendto(m_fdSocket, buffer, length, 0, (sockaddr*)&addr, sizeof(addr));
}

unsigned int UdpServer::getPort()const
{
    return m_port;
}

int UdpServer::open()
{
    m_fdSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(m_fdSocket < 0) {
        close();
        return -1;
    }

    int sendbuf_size = 1024 * 1024; //1 MB
    if(setsockopt(m_fdSocket, SOL_SOCKET, SO_SNDBUF, &sendbuf_size, sizeof(sendbuf_size))) {
        close();
        return -1;
    }

    int recvbuf_size = 1024 * 1024; // 1 MB
    if(setsockopt(m_fdSocket, SOL_SOCKET, SO_RCVBUF, &recvbuf_size, sizeof(recvbuf_size))) {
        close();
        return -1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(m_port);

    if(bind(m_fdSocket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close();
        return -1;
    }

    return 0;
}

int UdpServer::getSocket() const
{
    return m_fdSocket;
}
