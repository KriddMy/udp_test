#include <sys/select.h>
#include <cstddef>
#include <errno.h>
#include <cstring>
#include <iostream>

#include "PacketReciever.h"

PacketReciever::PacketReciever(const uint16_t bitrate) : 
    UdpServer(bitrate),
    m_bufferSize(sizeof(TestPacket) + 1),
    m_buffer(new u_char[m_bufferSize])
{

}

PacketReciever::~PacketReciever()
{

}

int PacketReciever::acceptPacket(time_t timeout)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(getSocket(), &fds);

    timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = timeout;

    int activty = select(getSocket() + 1, &fds, NULL, NULL, &tv);
    if((activty < 0) && (errno != EINTR)) {
        return -1;
    }

    if(FD_ISSET(getSocket(), &fds)) {
        sockaddr_in client;

        auto read_lenth = read(getBuffer(), getBufferLength(), client);
        if( read_lenth < 0)
            return -1;
        
        auto crc = crc32c(0, getBuffer(), read_lenth);

        TestPacket packet;
        if(parsePacket(packet, getBuffer(), read_lenth))
            return -1;

        auto& file = m_files[packet.id];

        // compare total number of packets for the first element. 
        // If total number of packets ate not the same - we dont trust 
        // this packet. Just send ack and wait for another one.
        if(file.size() && file.begin()->second.seq_total != packet.seq_total) {
            sendAck(client, packet, crc, file.size());
            return -1;
        }
            
        file[packet.seq_number] = packet;
        sendAck(client, packet, crc, file.size());

        // File ready. Todo: wait for a moment if ack is not passed.
        if(file.size() == packet.seq_total) {
            onFileResceved(file);
        }
    }

    return activty;
}

size_t PacketReciever::getBufferLength() const
{
    return m_bufferSize;
}

int PacketReciever::parsePacket(TestPacket& packet, const u_char* buffer, size_t length)
{
    if(length < PACKET_MINIMUM_SIZE)
        return -1;

    const u_char* pos = buffer;
    std::memcpy(&packet.seq_number, pos, 4);

    pos += 4;
    std::memcpy(&packet.seq_total, pos, 4);

    pos += 4;
    std::memcpy(&packet.type, pos, 1);

    pos++;
    std::memcpy(&packet.id, pos, 8);

    pos += 8;
    packet.data_size = length - (PACKET_MINIMUM_SIZE - 1);
    std::memcpy(&packet.data, pos, packet.data_size);

    if(packet.type != TestPacket::PUT)
        return -1;

    return 0;
}

ssize_t PacketReciever::sendAck(const sockaddr_in& addr, const TestPacket& packet, uint32_t crc, uint32_t total)
{
    u_char sendBuffer[33];
    size_t pos = 0;
    std::memcpy(&sendBuffer[pos], &packet.seq_number, sizeof(packet.seq_number));

    pos += sizeof(packet.seq_number);
    std::memcpy(&sendBuffer[pos], &total, sizeof(total));
    
    pos += sizeof(total);
    auto type = TestPacket::ACK;
    std::memcpy(&sendBuffer[pos], (uint8_t*)&type, sizeof(type));

    pos += sizeof(type);
    std::memcpy(&sendBuffer[pos], &packet.id, sizeof(packet.id));

    pos += sizeof(packet.id);
    std::memcpy(&sendBuffer[pos], &crc, sizeof(crc));
    
    return send(sendBuffer, 33, addr);
}

void PacketReciever::onFileResceved(const std::map<uint32_t, TestPacket>& packets)
{
    if(!packets.size())
        return;

    std::cout << "Server: Packet with id: " << packets.begin()->second.id << " Recieved. OK." << std::endl;
}

u_char* PacketReciever::getBuffer() const
{
    return m_buffer;
}

uint32_t PacketReciever::crc32c(uint32_t crc, const unsigned char *buf, size_t len)
{
    int k;
    crc = ~crc;
    while(len--) {
        crc^= *buf++;
        for (k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
    }

    return ~crc;
}