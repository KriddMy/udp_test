#pragma once
#include <cstdint>
#include <stddef.h>

#define PACKET_MINIMUM_SIZE 18
#define MAX_DATA_SIZE 1472

#define SERVER_PORT 50050

struct TestPacket
{
    enum Types : uint8_t {
        ACK = 0,
        PUT
    } type;

    uint32_t seq_number;
    uint32_t seq_total;
    uint64_t id;

    uint8_t data[MAX_DATA_SIZE];
    size_t data_size;
};