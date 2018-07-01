#ifndef __FB_BMP_H__
#define __FB_BMP_H__

#include <base.h>
#include <framebuffer.h>





typedef struct  
{  
    //unsigned short    bfType;  
    ul    bfSize;  
    u16    bfReserved1;  
    u16    bfReserved2;  
    ul    bfOffBits;  
} BFileHeader;

typedef struct  
{  
    ul  biSize;   
    long   biWidth;   
    long   biHeight;   
    u16   biPlanes;   
   	u16   biBitCount;  
    ul  biCompression;   
    ul  biSizeImage;   
    long   biXPelsPerMeter;   
    long   biYPelsPerMeter;   
    ul   biClrUsed;   
    ul   biClrImportant;   
} BInfoHeader;

int is_bmp(const char *pathname);

int display_bmp(ImgInfo *img_info);

#endif


