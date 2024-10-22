#pragma once

#include <stddef.h>
#include <cstdint>
#include <string>
#include <vector>
#include <list>
#include <unordered_set>
#include <netinet/in.h>

#include "common.h"

#define RESPONCE_TIMEOUT_SEC 3

class FileSender
{
public:
    FileSender(const uint16_t bitrate);
    virtual ~FileSender();

    /// @brief Creates socket that will send packets to a given server address and port
    /// @param addr Address of Server
    /// @param port Port of Server
    /// @return 0 - succeed; -1 - failed
    int open(const char* addr, const uint16_t port);
    
    /// @brief Breaks files into packets and shuffles its order in the queue
    /// @param file_paths paths of files to be sent
    /// @return 0 - success; -1 - failed
    int add(const std::vector<std::string>& file_paths);

    /// @brief Sends random packet from the files added by "add" function
    /// @return Size of packets left to send if succeed, otherwise -1
    int send_random();
    
    uint16_t getPort() const;

protected:
    int generate_random(int min, int max);
    uint64_t generate_unique_id();
    uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len);

private:
    int send_packet(const TestPacket& packet, uint32_t& crc);
    int verifyAck(const TestPacket& sentPacket, uint32_t crc);
    int parseAck(TestPacket& packet, uint32_t& crc, const u_char* buffer, size_t length);

    int m_fdSocket;
    const uint16_t m_port;
    sockaddr_in m_addr;
    uint16_t m_bitrate;

    std::unordered_set<uint64_t> m_ids;
    std::vector<TestPacket> m_packets;
};