
#include "packet.h"

/******** PRIVATE FUNCTION **********/





/******** PUBLIC FUNCTION *****************/

/*
*   @param packet: received packet 
*   @param size  : received packet size
*   @param index : 0-error, start from '1'.
*   @param chksum: received packet's checksum
*/
bool PKT_Packet(PKT_InfoTypedef *info, uint8_t *packet, uint16_t size, uint16_t index, uint8_t chksum)
{
    uint8_t checksum = 0;
    uint8_t *pAddr = NULL;
    uint16_t i = 0;

    if(!info) {return (FALSE);}
    if(!info->storeAddr || !packet) {return (FALSE);}
    if(index > info->packetNbr) {return (FALSE);}

    /* checksum */
    checksum = PKT_GetChecksum(packet, size);
    if(checksum != chksum) {return (FALSE);}
    /* write data */
    pAddr = (uint8_t *)(info->storeAddr + info->packetSize*(index-1));
    for(i=0; i<size; i++)
    {
        pAddr[i] = packet[i];
    }

    return (TRUE);
}

/*
*   @param packet: send packet 
*   @param size  : send packet size
*   @param index : 0-error, start from '1'.
*   @param chksum: send packet's checksum
*/
bool PKT_Unpacket(PKT_InfoTypedef *info, uint8_t *packet, uint16_t *size, uint16_t index, uint8_t *chksum)
{
    uint8_t *pAddr = NULL;
    uint16_t i = 0;
    uint16_t pSize = 0;

    if(!info) {return (FALSE);}
    if(!info->storeAddr || !packet) {return (FALSE);}
    if(index > info->packetNbr) {return (FALSE);}
    /* current packet size */
    *size = pSize = (index == info->packetNbr)/* last packet? */
                        ?(info->storeSize - ((info->packetNbr-1)* info->packetSize))
                        :(info->packetSize);
    /* data region */                                    
    pAddr = (uint8_t *)(info->storeAddr + info->packetSize*(index-1));
    /* read data */
    for(i=0; i<pSize; i++)
    {
        packet[i] = pAddr[i];
    }
    /* get checksum */
    *chksum = PKT_GetChecksum(packet, pSize);

    return (TRUE);
}

/* ^ */
uint8_t PKT_GetChecksum(uint8_t *pData, uint32_t size)
{
    uint8_t checksum = 0;

    if(!pData || !size) {return (0);}

    while(size--)
    {
        checksum ^= *pData++;
    }

    return (checksum);
}

uint8_t *PKT_GetStored(PKT_InfoTypedef *info)
{
    if(!info) {return (NULL);}

    return (uint8_t *)(info->storeAddr);
}

