#pragma once

#include <unordered_map>
#include <map>

#include "UdpServer.h"
#include "common.h"

class PacketReciever : public UdpServer
{        
    //TODO: add time of initilaization to remove garbage packets on timeout
public:
    /// @brief Creates server instance for backets recieve.
    /// @param bitrate bitrates for response sending
    PacketReciever(const uint16_t bitrate);
    virtual ~PacketReciever();

    /// @brief wait for new packet to recive in a given timeout
    /// @param timeout timeout for packet recieval
    /// @return 1 - succeed; 0 - timout; -1 - failed
    int acceptPacket(const time_t timeout);

protected:
    u_char* getBuffer() const;
    size_t getBufferLength() const;

    uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len);
    int parsePacket(TestPacket& packet, const u_char* buffer, size_t length);
    ssize_t sendAck(const sockaddr_in& addr, const TestPacket& packet, uint32_t crc, uint32_t total); 

    virtual void onFileResceved(const std::map<uint32_t, TestPacket>& packets);

private:
    size_t m_bufferSize;
    u_char* m_buffer;

    std::unordered_map<uint64_t, std::map<uint32_t, TestPacket>> m_files;
};