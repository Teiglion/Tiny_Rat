#include "ff.h"

DWORD get_fattime(void)
{
    return ((DWORD)(2026 - 1980) << 25)  // year
         | ((DWORD)6 << 21)              // month
         | ((DWORD)10 << 16)             // day
         | ((DWORD)12 << 11)             // hour
         | ((DWORD)0 << 5)               // minute
         | ((DWORD)(0 / 2));             // second / 2
}