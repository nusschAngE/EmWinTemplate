

#ifndef _PUBLIC_H
#define _PUBLIC_H

#include "BuildOption.h"
#include "typedef.h"
//#include "my_malloc.h"


/**************************************/
extern int myStrcmp(const char *str1, const char *str2);
extern char *myStrcpy(char *dest, const char *src);
extern char *myStrcat(char *dest, const char *src);
extern char *myStrstr(char *str1, const char *str2);
extern char *myStrchr(const char* str, char ch);
extern uint32_t myStrlen(const char *src);

extern uint8_t char_upper(uint8_t ch);
extern char *int2str(int64_t num);




#endif

