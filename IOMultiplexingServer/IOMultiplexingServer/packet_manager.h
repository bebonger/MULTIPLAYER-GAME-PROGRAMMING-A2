#pragma once
#include <iostream>
#include <string>
#include <stdio.h>
#include <string.h>


#pragma warning(disable:4996)

class Packet {
public:

    char PacketID[256];
    char PacketData[1024];
    int  PacketLength;

    Packet();
    ~Packet();

    void Encode(const char* _packetID);
    void Decode();

    Packet operator<<(const char* data);
    int operator>>(const char* dataName);
};

namespace PacketManager {
    int packet_add_data(char Buffer[], const char DataName[], const int Value);
    int packet_add_data(char Buffer[], const char DataName[], const char Value[]);
    int packet_parse_get_data(const char Packet[], const char DataName[], std::string& DataString);
    int packet_parse_data(const char Packet[], const char DataName[]);
    int packet_parse_data(const char Packet[], const char DataName[], char Buffer[], int& BufferSize);
    int packet_encode(char Packet[], const int MaxBufferSize, const char PacketID[], const char PacketData[]);
    int packet_decode(const char Packet[], char PacketID[], char PacketData[]);

    bool decodable(const char Packet[]);
}