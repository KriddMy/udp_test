#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <random>
#include <cmath>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>

#include "FileSender.h"
#include <sys/select.h>

FileSender::FileSender(const uint16_t bitrate) : m_fdSocket(-1), m_port(0), m_addr(), m_bitrate(bitrate), m_ids(), m_packets()
{

}

FileSender::~FileSender()
{
    close(m_fdSocket);
}

int FileSender::open(const char* addr, const uint16_t port)
{
    if( (m_fdSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
        return -1;

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    m_addr.sin_addr.s_addr = inet_addr(addr);

    return 0;
}

int FileSender::add(const std::vector<std::string>& file_paths)
{
    m_packets.clear();

    try { for(auto file_path : file_paths) {
            std::ifstream file(file_path, std::ios::binary | std::ios::ate);
            if(!file.is_open())
                return -1;

            auto fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            TestPacket packet;
            packet.seq_number = 0;
            packet.seq_total = std::ceil(fileSize / MAX_DATA_SIZE) + 1;
            packet.type = TestPacket::PUT;
            packet.id = generate_unique_id();

            uint32_t i = 0;
            while(!file.eof()) {
                file.read((char*)&packet.data, MAX_DATA_SIZE);
                packet.data_size = file.gcount();
                packet.seq_number = i;
                m_packets.push_back(packet);
                i++;
            }

            file.close();
        }

        //randomize
        std::default_random_engine generator (10);
        std::shuffle(std::begin(m_packets), std::end(m_packets), generator);

        return 0;
    } catch(...) { return -1; }
}

int FileSender::send_random()
{
    if(m_fdSocket < 0)
        return -1;

    const auto packet = m_packets.back();

    uint32_t crc = 0;
    if(send_packet(packet, crc) < 0)
        return -1;

    if(verifyAck(packet, crc) < 0)
        return -1;

    m_packets.pop_back();
    return m_packets.size();
}

int FileSender::generate_random(int min, int max)
{
    if( min > max)
        std::swap(min, max);
    else if(min == max)
        return max;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(min, max);
    
    return distr(gen);
}

uint64_t FileSender::generate_unique_id()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> distr;

    uint64_t newId;
    do {
        newId = distr(gen);
    } while (m_ids.find(newId) != m_ids.end());

    m_ids.insert(newId);
    return newId;
}

uint32_t FileSender::crc32c(uint32_t crc, const unsigned char *buf, size_t len)
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

uint16_t FileSender::getPort() const
{
    return m_port;
}

int FileSender::send_packet(const TestPacket& packet, uint32_t& crc)
{
    //bitrate 
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

    size_t pos = 0;
    u_char sendBuffer[sizeof(TestPacket)];
    std::memcpy(&sendBuffer[pos], &packet.seq_number, sizeof(packet.seq_number));

    pos += sizeof(packet.seq_number);
    std::memcpy(&sendBuffer[pos], &packet.seq_total, sizeof(packet.seq_total));
    
    pos += sizeof(packet.seq_total);
    std::memcpy(&sendBuffer[pos], (uint8_t*)&packet.type, sizeof(packet.type));

    pos += sizeof(packet.type);
    std::memcpy(&sendBuffer[pos], &packet.id, sizeof(packet.id));

    pos += sizeof(packet.id);
    std::memcpy(&sendBuffer[pos], &packet.data, sizeof(packet.data_size));

    int n = sendto(m_fdSocket, sendBuffer, 17 + packet.data_size, 0, (const sockaddr*)&m_addr, sizeof(m_addr));
    if(n < 0)
        return -1;
    
    crc = crc32c(0, sendBuffer, 17 + packet.data_size);
    return 0;
}

int FileSender::verifyAck(const TestPacket& sentPacket, uint32_t crc)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(m_fdSocket, &fds);

    timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = RESPONCE_TIMEOUT_SEC;

    int activty = select(m_fdSocket + 1, &fds, NULL, NULL, &tv);
    if((activty < 0) && (errno != EINTR)) {
        return -1;
    }

    u_char recvBuffer[33];
    if(FD_ISSET(m_fdSocket, &fds)) {
        auto read_lenth = read(m_fdSocket, &recvBuffer, sizeof(recvBuffer));
        if( read_lenth < 0)
            return -1;
        
        uint32_t recvCrc = 0;
        TestPacket recvPacket;
        if(parseAck(recvPacket, recvCrc, recvBuffer, sizeof(recvBuffer)))
            return -1;

        if(recvPacket.id != sentPacket.id && recvPacket.seq_number != sentPacket.seq_number)
            return -1;
        
        if(crc != recvCrc)
            return -1;

        if(sentPacket.seq_total == recvPacket.seq_total)
            std::cout << "Client: File with id: " << recvPacket.id << " succesfully sent." << std::endl;

        return 0;
    }

    return -1;
}

int FileSender::parseAck(TestPacket& packet, uint32_t& crc, const u_char* buffer, size_t length)
{
    if(length < PACKET_MINIMUM_SIZE)
        return -1;

    const u_char* pos = buffer;
    std::memcpy(&packet.seq_number, pos, sizeof(packet.seq_number));

    pos += sizeof(packet.seq_number);
    std::memcpy(&packet.seq_total, pos, sizeof(packet.seq_total));

    pos += sizeof(packet.seq_total);
    std::memcpy(&packet.type, pos, sizeof(packet.type));

    pos += sizeof(packet.type);
    std::memcpy(&packet.id, pos, sizeof(packet.id));

    pos += sizeof(packet.id);
    std::memcpy(&crc, pos, sizeof(crc));

    if(packet.type != TestPacket::ACK)
        return -1;

   return 0;
}
