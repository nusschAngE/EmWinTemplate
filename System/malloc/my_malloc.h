
#ifndef _MY_MALLOC_H
#define _MY_MALLOC_H

#include "public.h"

/* MALLOC BANK */
enum
{   
    MLCSRC_AXISRAM = 0, //internal ,Max 200Mhz
    MLCSRC_AHBSRAM12,   //internal 
    MLCSRC_AHBSRAM4,    //internal
    MLCSRC_DTCM,        //internal ,CPU core clock, CPU&DMA Only
    MLCSRC_ITCM,        //internal ,CPU core clock, CPU Instructment Only
#if defined(ExtSDRAM_ENABLE)
	MLCSRC_SDRAM,
#endif
    MLCSRC_NUM
};

/* AXI SRAM malloc source 
*   AXI SRAM : 512KB, use 280KB for malloc
*   Note : set as default SRAM
*/
#define AXISRAM_BLOCK_SIZE          (64)
#define AXISRAM_MALLOC_SIZE         ((uint32_t)(280 * 1024))//280Kbytes
#define AXISRAM_MAPTAB_SIZE         (AXISRAM_MALLOC_SIZE / AXISRAM_BLOCK_SIZE)

/* AHB SRAM12 malloc source
*   AHB SRAM12 : 256KB, use 240KB for malloc
*   Note : 
*/
#define AHBSRAM12_BLOCK_SIZE        (64)
#define AHBSRAM12_MALLOC_SIZE       ((uint32_t)(240 * 1024))//240Kbytes
#define AHBSRAM12_MAPTAB_SIZE       (AHBSRAM12_MALLOC_SIZE / AHBSRAM12_BLOCK_SIZE)

/* AHB SRAM4 malloc source
*   AHB SRAM4 : 64KB, use 60KB for malloc
*   Note : 
*/
#define AHBSRAM4_BLOCK_SIZE         (64)
#define AHBSRAM4_MALLOC_SIZE        ((uint32_t)(60 * 1024))//60Kbytes
#define AHBSRAM4_MAPTAB_SIZE        (AHBSRAM4_MALLOC_SIZE / AHBSRAM4_BLOCK_SIZE)

/* DTCM malloc source 
*   DTCM : 128KB, use 120KB for malloc
*   Note : do not set as default SRAM
*/
#define DTCM_BLOCK_SIZE             (64)
#define DTCM_MALLOC_SIZE            ((uint32_t)(120 * 1024))//120Kbytes
#define DTCM_MAPTAB_SIZE            (DTCM_MALLOC_SIZE / DTCM_BLOCK_SIZE)

/* ITCM malloc source 
*   ITCM : 64KB, use 60KB for malloc
*   Note : 
*/
#define ITCM_BLOCK_SIZE             (64)
#define ITCM_MALLOC_SIZE            ((uint32_t)(60 * 1024))//60Kbytes
#define ITCM_MAPTAB_SIZE            (ITCM_MALLOC_SIZE / ITCM_BLOCK_SIZE)

#if defined(ExtSDRAM_ENABLE)
/* SDRAM malloc source 
*   SDRAM : 32MB, use 30MB for malloc 
*   Note : 
*/
#define SDRAM_BLOCK_SIZE            (64)//memory block size : 64bytes
#define SDRAM_MALLOC_SIZE           ((uint32_t)(30 * 1024 * 1024))//30Mbytes
#define SDRAM_MAPTAB_SIZE           (SDRAM_MALLOC_SIZE / SDRAM_BLOCK_SIZE)
#endif //ExtSDRAM_ENABLE

/* Malloc manage */
typedef struct
{
    void (*init)(uint8_t mlcSrc);
    uint16_t (*used)(uint8_t mlcSrc);
    uint8_t *memBase[MLCSRC_NUM];
    uint32_t *memMap[MLCSRC_NUM];
    bool memRdy[MLCSRC_NUM];
}MallocSystem_t;


/******************** PUBLIC FUNCTION **********************/
extern void myMalloc_Init(uint8_t mlcSrc);
extern uint16_t myMalloc_GetUsed(uint8_t mlcSrc);
extern void *myMalloc(uint8_t mlcSrc, uint32_t size);
extern void *myRealloc(uint8_t mlcSrc, void *src, uint32_t size);
extern void myFree(uint8_t mlcSrc, void *src);
extern void *myMemcpy(void *dst, const void *src, uint32_t size);
extern void *myMemset(void *src, int ch, uint32_t size);
extern int myMemcmp(const void *src1, const void *src2, uint32_t size);

#endif

