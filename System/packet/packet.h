
#ifndef _PACKET_H
#define _PACKET_H

#include "public.h"

/* packet size */
#define PACKET_SIZE             ((uint16_t)1024)

typedef struct
{
    uint32_t storeAddr;
    uint32_t storeSize;

    uint16_t packetSize;
    uint16_t packetNbr;

    uint8_t checksum;
}PKT_InfoTypedef;

/***** PUBLIC FUNCTION *****/
bool PKT_Packet(PKT_InfoTypedef *info, uint8_t *packet, uint16_t size, uint16_t index, uint8_t chksum);
bool PKT_Unpacket(PKT_InfoTypedef *info, uint8_t *packet, uint16_t *size, uint16_t index, uint8_t *chksum);
uint8_t PKT_GetChecksum(uint8_t *pData, uint32_t size);
uint8_t *PKT_GetStored(PKT_InfoTypedef *info);

#endif

