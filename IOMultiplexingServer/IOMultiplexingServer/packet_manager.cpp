#include "packet_manager.h"

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
        const char* Pos = Str;
        for (int i = 0; i < Len; ++i)
        {
            if ('=' == *(Pos + i))
            {
                if ('\"' == *(Pos + i + 1))
                {
                    for (int j = (i + 2); j < Len; ++j)
                    {
                        if (('\"' == *(Pos + j)))
                            break;

                        DataString.push_back(*(Pos + j));
                    }
                }
                else {
                    for (int j = (i + 1); j < Len; ++j)
                    {
                        if ((' ' == *(Pos + j)) || ('\n' == *(Pos + j)) || ('\0' == *(Pos + j) || ('\r' == *(Pos + j)) || ('>' == *(Pos + j))))
                            break;
                        DataString.push_back(*(Pos + j));
                    }
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

    bool decodable(const char Packet[])
    {
        return Packet[0] == '<';
    }
}

Packet::Packet() 
    : PacketID{ '\0' }
    , PacketData{ '\0' }
{
}

Packet::~Packet()
{
}

void Packet::Encode(const char* _packetID)
{
    char tempBuffer[1024];
    strcpy(tempBuffer, PacketData);
    memset(PacketData, '\0', 1024);
    PacketLength = PacketManager::packet_encode(PacketData, 1024, _packetID, tempBuffer);
}

void Packet::Decode()
{
    char tempBuffer[1024];
    strcpy(tempBuffer, PacketData);
    memset(PacketData, '\0', 1024);
    PacketManager::packet_decode(tempBuffer, PacketID, PacketData);
}

Packet Packet::operator<<(const char* data)
{
    sprintf(PacketData, "%s %s", PacketData, data);
    return Packet();
}

int Packet::operator>>(const char* dataName)
{
    return PacketManager::packet_parse_data(PacketData, dataName);
}