#pragma once
#include <iostream>
#include <string>
#include <stdio.h>
#include <string.h>

namespace PacketManager {
    int packet_add_data(char Buffer[], const char DataName[], const int Value)
    {
        sprintf(Buffer, "%s %s=%d", Buffer, DataName, Value);
        return strlen(Buffer);
    }

    int packet_add_data(char Buffer[], const char DataName[], const char Value[])
    {
        sprintf(Buffer, "%s %s=\"%s\"", Buffer, DataName, Value);
        return strlen(Buffer);
    }

    int packet_parse_get_data(const char Packet[], const char DataName[], std::string& DataString)
    {
        const char* Str;

        if (NULL == (Str = strstr(Packet, DataName)))
        {
            return 0;
        }

        int Len = strlen(Str);
        char* Pos;
        for (int i = 0; i < Len; ++i)
        {
            if ('=' == *(Pos + i))
            {
                for (int j = (i + 1); j < Len; ++j)
                {
                    if ((' ' == *(Pos + j)) || ('\n' == *(Pos + j)) || ('\0' == *(Pos + j)))
                        break;
                    DataString.push_back(*(Pos + j));
                }
                return atoi(DataString.c_str());
            }
        }

        return 0;
    }

    int packet_parse_data(const char Packet[], const char DataName[])
    {
        std::string DataString;
        int ReturnLength;

        return packet_parse_get_data(Packet, DataName, DataString);
    }

    int packet_parse_data(const char Packet[], const char DataName[], char Buffer[], int& BufferSize)
    {
        std::string DataString;
        int ReturnLength;

        ReturnLength = packet_parse_get_data(Packet, DataName, DataString);
        strcpy(Buffer, DataString.c_str());
        BufferSize = ReturnLength;

        return ReturnLength;
    }

    int packet_encode(char Packet[], const int MaxBufferSize, const char PacketID[], const char PacketData[])
    {
        int PacketLength = strlen(PacketID) + strlen(PacketData) + 7;
        sprintf(Packet, "<%s %03d %s>", PacketID, PacketLength, PacketData);
        return PacketLength;
    }

    int packet_decode(const char Packet[], char PacketID[], char PacketData[])
    {
        int PacketDataLength;
        int PacketLength = strlen(Packet);
        int Pos = 1;

        for (int j = 0; Pos < PacketLength; ++Pos)
        {
            if (' ' == Packet[Pos])
            {
                PacketID[j] = '\0';
                ++Pos;
                break;
            }
            PacketID[j++] = Packet[Pos];
        }

        strcpy(PacketData, &Packet[Pos]);
        PacketDataLength = strlen(PacketData);

        return PacketDataLength;
    }
}