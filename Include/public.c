
#include "public.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <memory.h>

/**********************************/
int myStrcmp(const char *str1, const char *str2)
{
    return strcmp(str1, str2);
}

char *myStrcpy(char* dest, const char *src)
{
    return strcpy(dest, src);
}

char *myStrcat(char *dest, const char *src)
{
    return strcat(dest, src);
}

char *myStrstr(char *str1, const char *str2)
{
    return strstr(str1, str2);
}

char *myStrchr(const char* str, char ch)
{
    return strchr(str, ch);
}

uint32_t myStrlen(const char *src)
{
    return strlen(src);
}

/* char process */
uint8_t char_upper(uint8_t ch)
{
    if(ch >= 'a' && ch <= 'z')
    {
        return (ch - 0x20);
    }

    return ch;
}

char *int2str(int64_t num)
{
    static char tmp[32];

    memset(tmp, 0, 32);
    sprintf(tmp, "%lld", num);

    return tmp;
}


//char *float2str(float num)
//{    
//}



