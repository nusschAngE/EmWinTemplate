
#ifndef _BUILD_OPTION_H
#define _BUILD_OPTION_H

/* external SDRAM */
#ifndef ExtSDRAM_ENABLE
#define ExtSDRAM_ENABLE
#endif //!ExtSDRAM_ENABLE

/* IAP & Bootloader 
*   Must replace the "*.sct" file with "xx_iap.sct"
*/
#ifndef IAP_FUNCTION_ENABLE
//#define IAP_FUNCTION_ENABLE
#endif //IAP_FUNCTION_ENABLE

#endif //_BUILD_OPTION_H

