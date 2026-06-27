#include "scenaries.h"

void print_scens(const char *path)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;
    int nfile, ndir;

    res = f_opendir(&dir, path);
    if (res == FR_OK) 
    {
        nfile = ndir = 0;
        uint8_t x=20,y=20;
        for (;;) 
        {
            res = f_readdir(&dir, &fno);
            if (fno.fname[0] == 0) break;
            draw_text_cutted(x,y,fno.fname,0x0000,2);
            y+=30;
            nfile++;
        }
    }
    f_closedir(&dir);
}

