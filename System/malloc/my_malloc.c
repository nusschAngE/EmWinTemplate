
#include "stm32h7xx.h"
#include "my_malloc.h"

#if 1
__ALIGNED(4) uint8_t AXISRAM_BASE[AXISRAM_MALLOC_SIZE]; //default system SRAM
             uint32_t AXISRAM_MAP[AXISRAM_MAPTAB_SIZE];

#if 0//ARMCC 6.10
    /* ARMCC Link : DATA type  */
    /* base address */
    __ALIGNED(4) uint8_t AHBSRAM12_BASE[AHBSRAM12_MALLOC_SIZE] _SRAM12_RW;
                 uint32_t AHBSRAM12_MAP[AHBSRAM12_MAPTAB_SIZE] _SRAM12_RW;

    __ALIGNED(4) uint8_t AHBSRAM4_BASE[AHBSRAM4_MALLOC_SIZE] _SRAM4_RW;
    			 uint32_t AHBSRAM4_MAP[AHBSRAM4_MAPTAB_SIZE] _SRAM4_RW;

    __ALIGNED(4) uint8_t DTCM_BASE[DTCM_MALLOC_SIZE] _DTCM_RW;
    			 uint32_t DTCM_MAP[DTCM_MAPTAB_SIZE] _DTCM_RW;

    __ALIGNED(4) uint8_t ITCM_BASE[ITCM_MALLOC_SIZE] _ITCM_RW;
    			 uint32_t ITCM_MAP[ITCM_MAPTAB_SIZE] _ITCM_RW;

#if defined(ExtSDRAM_ENABLE)
    __ALIGNED(4) uint8_t SDRAM_BASE[SDRAM_MALLOC_SIZE] _EXTSDRAM_RW;
    			 uint32_t SDRAM_MAP[SDRAM_MAPTAB_SIZE] _EXTSDRAM_RW;
#endif //ExtSDRAM_ENABLE
			 
#endif

#if 0
    /* ARMCC Link : DATA type  */
    __ALIGNED(4) uint8_t AHBSRAM12_BASE[AHBSRAM12_MALLOC_SIZE] __attribute__((section(".ARM.__at_0x30000000")));
                 uint32_t AHBSRAM12_MAP[AHBSRAM12_MAPTAB_SIZE] __attribute__((section(".ARM.__at_0x3003C000")));

    __ALIGNED(4) uint8_t AHBSRAM4_BASE[AHBSRAM4_MALLOC_SIZE] __attribute__((section(".ARM.__at_0x38000000")));
    			 uint32_t AHBSRAM4_MAP[AHBSRAM4_MAPTAB_SIZE] __attribute__((section(".ARM.__at_0x3800F000")));

    __ALIGNED(4) uint8_t DTCM_BASE[DTCM_MALLOC_SIZE] __attribute__((section(".ARM.__at_0x20000000")));
    			 uint32_t DTCM_MAP[DTCM_MAPTAB_SIZE] __attribute__((section(".ARM.__at_0x2001E000")));

    __ALIGNED(4) uint8_t ITCM_BASE[ITCM_MALLOC_SIZE] __attribute__((section(".ARM.__at_0x00000000")));
    			 uint32_t ITCM_MAP[ITCM_MAPTAB_SIZE] __attribute__((section(".ARM.__at_0x0000F000")));

#if defined(ExtSDRAM_ENABLE)
    __ALIGNED(4) uint8_t SDRAM_BASE[SDRAM_MALLOC_SIZE] __attribute__((section(".ARM.__at_0xC0000000")));//30MB
                 uint32_t SDRAM_MAP[SDRAM_MAPTAB_SIZE] __attribute__((section(".ARM.__at_0xC1E00000")));
#endif //ExtSDRAM_ENABLE
    
#endif

#if 1
    /* ARMCC Link : ZERO type  */
    __ALIGNED(4) uint8_t AHBSRAM12_BASE[AHBSRAM12_MALLOC_SIZE] __attribute__((at(0x30000000)));
                 uint32_t AHBSRAM12_MAP[AHBSRAM12_MAPTAB_SIZE] __attribute__((at(0x30000000+AHBSRAM12_MALLOC_SIZE)));

    __ALIGNED(4) uint8_t AHBSRAM4_BASE[AHBSRAM4_MALLOC_SIZE] __attribute__((at(0x38000000)));
    			 uint32_t AHBSRAM4_MAP[AHBSRAM4_MAPTAB_SIZE] __attribute__((at(0x38000000+AHBSRAM4_MALLOC_SIZE)));

    __ALIGNED(4) uint8_t DTCM_BASE[DTCM_MALLOC_SIZE] __attribute__((at(0x20000000)));
    			 uint32_t DTCM_MAP[DTCM_MAPTAB_SIZE] __attribute__((at(0x20000000+DTCM_MALLOC_SIZE)));

    __ALIGNED(4) uint8_t ITCM_BASE[ITCM_MALLOC_SIZE] __attribute__((at(0x00000000)));
    			 uint32_t ITCM_MAP[ITCM_MAPTAB_SIZE] __attribute__((at(0x00000000+ITCM_MALLOC_SIZE)));

#if defined(ExtSDRAM_ENABLE)
    __ALIGNED(4) uint8_t SDRAM_BASE[SDRAM_MALLOC_SIZE] __attribute__((at(0xC0000000)));//30MB
                 uint32_t SDRAM_MAP[SDRAM_MAPTAB_SIZE] __attribute__((at(0xC0000000+SDRAM_MALLOC_SIZE)));
#endif //ExtSDRAM_ENABLE

#endif

const uint32_t MallocSize[MLCSRC_NUM] = 
{
	AXISRAM_MALLOC_SIZE, 
	AHBSRAM12_MALLOC_SIZE, 
  	AHBSRAM4_MALLOC_SIZE,
 	DTCM_MALLOC_SIZE,
 	ITCM_MALLOC_SIZE
#if defined(ExtSDRAM_ENABLE)
	,SDRAM_MALLOC_SIZE 
#endif //ExtSDRAM_ENABLE
};

const uint32_t MallocBlockSize[MLCSRC_NUM] = 
{
	AXISRAM_BLOCK_SIZE, 
 	AHBSRAM12_BLOCK_SIZE, 
	AHBSRAM4_BLOCK_SIZE,
   	DTCM_BLOCK_SIZE,
  	ITCM_BLOCK_SIZE
#if defined(ExtSDRAM_ENABLE)
	,SDRAM_BLOCK_SIZE
#endif //ExtSDRAM_ENABLE
};

const uint32_t MallocBlockNbr[MLCSRC_NUM] = 
{
 	AXISRAM_MAPTAB_SIZE, 
   	AHBSRAM12_MAPTAB_SIZE, 
   	AHBSRAM4_MAPTAB_SIZE,
 	DTCM_MAPTAB_SIZE,
   	ITCM_MAPTAB_SIZE
#if defined(ExtSDRAM_ENABLE)
	,SDRAM_MAPTAB_SIZE
#endif //ExtSDRAM_ENABLE
};
                                                                                                   

static MallocSystem_t mallocSys = {
    myMalloc_Init,
    myMalloc_GetUsed,
#if defined(ExtSDRAM_ENABLE)
	{AXISRAM_BASE, AHBSRAM12_BASE, AHBSRAM4_BASE, DTCM_BASE, ITCM_BASE, SDRAM_BASE},
	{AXISRAM_MAP, AHBSRAM12_MAP, AHBSRAM4_MAP, DTCM_MAP, ITCM_MAP, SDRAM_MAP},
	{FALSE, FALSE, FALSE, FALSE, FALSE, FALSE}
#else
    {AXISRAM_BASE, AHBSRAM12_BASE, AHBSRAM4_BASE, DTCM_BASE, ITCM_BASE},
    {AXISRAM_MAP, AHBSRAM12_MAP, AHBSRAM4_MAP, DTCM_MAP, ITCM_MAP},
    {FALSE, FALSE, FALSE, FALSE, FALSE}
#endif //ExtSDRAM_ENABLE    
};

/******************** PUBLIC FUNCTION **********************/

void myMalloc_Init(uint8_t mlcSrc)
{
    myMemset(mallocSys.memMap[mlcSrc], 0, MallocBlockNbr[mlcSrc]);
    mallocSys.memRdy[mlcSrc] = TRUE;
}

//percentage * 1000;
uint16_t myMalloc_GetUsed(uint8_t mlcSrc)
{
    uint32_t used = 0, i = 0;

    for(i = 0; i < MallocBlockNbr[mlcSrc]; i++)
    {
        if(mallocSys.memMap[mlcSrc][i])
        {
            used++;
        }
    }

    return (uint16_t)((used * 1000)/MallocBlockNbr[mlcSrc]);
}

/********************  FUNCTION **********************/


void *myMalloc(uint8_t mlcSrc, uint32_t size)
{
    int64_t idx = 0;
    uint32_t i = 0;
    uint32_t BlocksFind = 0, BlocksNeed = 0; 

    /* size==0 */
    if((size == 0) || (mlcSrc >= MLCSRC_NUM))
    {
        return (NULL);
    }

    if(mallocSys.memRdy[mlcSrc] == FALSE)
    {
	    mallocSys.init(mlcSrc);
    }
		
    BlocksNeed = size / MallocBlockSize[mlcSrc];
    if(size % MallocBlockSize[mlcSrc]) 
    {
        BlocksNeed++;
    }

    for(idx = MallocBlockNbr[mlcSrc] - 1; idx >= 0; idx--)
    {
        if(mallocSys.memMap[mlcSrc][idx] == 0)//block free
        {
            BlocksFind++;
        }
        else //not continuous
        {
            BlocksFind = 0;
        }
        
        if(BlocksFind == BlocksNeed)
        {
            for(i = 0; i < BlocksNeed; i++)
            {
                mallocSys.memMap[mlcSrc][idx + i] = BlocksNeed;//mark it as used
            }
            /* update ret */
            return (void *)(mallocSys.memBase[mlcSrc] + (idx * MallocBlockSize[mlcSrc]));
        }
    }

    return (NULL);
}

void *myRealloc(uint8_t mlcSrc, void *src, uint32_t size)
{
    void *tmp = NULL;

    if((size == 0) || (src == NULL))
    {
        return (NULL);
    }

    if(mallocSys.memRdy[mlcSrc] == FALSE)
    {
	    mallocSys.init(mlcSrc);
    }

    tmp = myMalloc(mlcSrc, size);
    if(tmp != NULL)
    {
        myMemcpy(tmp, src, size);
        myFree(mlcSrc, src);
    }

    return (tmp);
}

void myFree(uint8_t mlcSrc, void *src)
{
    int64_t ofs = 0;//address offset
    uint32_t idx = 0, BlocksFree = 0, i = 0;

    if((src == NULL) || (mlcSrc >= MLCSRC_NUM))
    {
        return ;
    }

    ofs = (uint32_t)src - (uint32_t)mallocSys.memBase[mlcSrc];

    if((ofs) && (ofs < MallocSize[mlcSrc]))
    {        
        idx = ofs / MallocBlockSize[mlcSrc];
        BlocksFree = mallocSys.memMap[mlcSrc][idx];

        for(i = 0; i < BlocksFree; i++)
        {
            mallocSys.memMap[mlcSrc][idx + i] = 0;//block free
        }
        //clear
        src = NULL;
    }
}

void *myMemcpy(void *dst, const void *src, uint32_t size)
{
    uint8_t *d = dst;
    const uint8_t *s = src;

    while(size--)
    {
        *d++ = *s++;
    }

    return (dst);
}

void *myMemset(void *src, int ch, uint32_t size)
{
    uint8_t *s = src;

    while(size--)
    {
        *s++ = ch;
    }

    return (src);
}

//src1 == src2 ? 0 : -1
int myMemcmp(const void *src1, const void *src2, uint32_t size)
{
    const uint8_t *s1 = src1;
    const uint8_t *s2 = src2;

    while(size--)
    {
        if(*s1++ != *s2++)
        {
            return (-1);
        }
    }

    return (0);
}

#else
#include <string.h>
#include <stdlib.h>

/* current system heap size : 0x200
*/
void *myMalloc(uint8_t mlcSrc, uint32_t size)
{
    (void)mlcSrc;
    return malloc(size);
}

void *myRealloc(uint8_t mlcSrc, void *src, uint32_t size)
{
    (void)mlcSrc;
    return realloc(src, size);
}

void myFree(uint8_t mlcSrc, void *src)
{
    (void)mlcSrc;
    free(src);
}

void *myMemcpy(void *dst, const void *src, uint32_t size)
{
    return memcpy(dst, src, size);
}

void *myMemset(void *src, uint8_t ch, uint32_t size)
{
    return memset(src, ch, size);
}

int myMemcmp(const void *src1, const void *src2, uint32_t size)
{
    return memcmp(src1, src2, size);
}

#endif

