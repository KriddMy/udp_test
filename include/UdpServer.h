#pragma once

#include <sys/types.h>
#include <netinet/in.h>
#include <stdint.h>

class UdpServer
{
public:
    /// @brief Creates UDP server instance
    /// @param bitrate bitrate to send data
    UdpServer(const uint16_t bitrate);
    virtual ~UdpServer();
    
    /// @brief Opens connection for server to recieve data
    /// @param port Port of server
    /// @return 1 - succeed; -1 - failed
    int open(const uint16_t port);

    /// @brief closes connection
    void close();

    unsigned int getBitrate() const;
    void setBitrate(const uint16_t  bitrate);

    ssize_t read(u_char* buffer, size_t length, sockaddr_in &client_addr);
    ssize_t send(const u_char* buffer, size_t length, const sockaddr_in &addr);

    unsigned int getPort()const;

protected:
    int open();
    int getSocket()const;

private:
    int m_fdSocket;
    uint16_t m_bitrate;
    uint16_t m_port;
};