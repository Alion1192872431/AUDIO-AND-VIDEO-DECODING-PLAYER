#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__
/*********************************Header Files*************************************/
#include <base.h>


/*********************************Macro Definition************************************/
#define WHITE		0xFFFFFFFF
#define	BLACK		0xFF000000
#define RED			0xFFFF0000
#define GREEN		0xFF00FF00
#define BLUE		0xFF0000FF


/**********************************Structure*********************************/
struct line_param 
{
	u32 x1;
	u32 y1;
	u32 x2;
	u32 y2;
};

struct circular_param
{
	u32 centerX;
	u32 centerY;
	u32 radius;
};


/***********************************Function Declaration*************************************/
int fb_open(char *fb_file);

void fb_close(int fd);

int get_fixinfo(int fd);
int get_varinfo(int fd);
inline u32 *fb_mmap(int fd);
inline int fb_unmap(void);
void draw_pixel(u32 color, u32 x, u32 y);
void draw_background(u32 color);
void draw_line(struct line_param *lparam, u32 color);
void draw_circular(struct circular_param *cparam, u32 color);
void __draw_bmp_picture(const u8 *pData, u32 img_xres, u32 img_yres, u32 start_x, u32 start_y);
void __draw_jpg_picture(const u8 *pData, u32 img_xres, u32 img_yres, u32 start_x, u32 start_y);
void __draw_png_picture(const u8 *pData, u32 img_xres, u32 img_yres, u32 start_x, u32 start_y);


#endif

















