/******************************************************************************
 * File Name:framebuffer.c
 * Description:This file includes some functions releted to framebuffer
 * Author:Alion
 * Date:2018.6.3
 *****************************************************************************/

 
/* Header Files*/
#include <stdio.h>
#include <linux/fb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <framebuffer.h>


/* Global Variables */
struct fb_fix_screeninfo *fscinfo;
struct fb_var_screeninfo *vscinfo;
u32 *pfb = NULL;
extern ImgInfo *img_dst;


/*
 * Function Name:fb_open
 * Description:Open framebuffer device file
 * Input: file pathname
 * Output:None
 * Return:file descriptor  
 * Author:Alion
 * Date:2018.6.3
 */
int fb_open(char *fb_file)
{	
	return open(fb_file, O_RDWR);
}

/*
 * Function Name:fb_close
 * Description:Close framebuffer device file
 * Input: file descriptor 
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
void fb_close(int fd)
{
	free(fscinfo);
	free(vscinfo);
	free(img_dst);
	close(fd);
}


/*
 * Function Name:get_fixinfo
 * Description:Get fixed information of framebuffer
 * Input: file descriptor 
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */
int get_fixinfo(int fd)
{
	int retval = -1;

	fscinfo = (struct fb_fix_screeninfo *)malloc(sizeof(struct fb_fix_screeninfo));
	if (NULL == fscinfo)
	{
		printf("malloc for fixinfo failed.\n");
		return MALLOC_ERROR;
	}
	
	memset(fscinfo, 0, sizeof(struct fb_fix_screeninfo));
	retval = ioctl(fd, FBIOGET_FSCREENINFO, fscinfo);
	if (retval < 0)
	{
		perror("get fixed screeninfo");
		return IOCTRL_ERROR;
	}
	
	return NO_ERROR;
}


/*
 * Function Name:get_varinfo
 * Description:Get variable information of framebuffer
 * Input: file descriptor 
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */
int get_varinfo(int fd)
{
	int retval = -1;
	
	vscinfo = (struct fb_var_screeninfo *)malloc(sizeof(struct fb_var_screeninfo));
	if (NULL == vscinfo)
	{
		printf("malloc for varinfo failed.\n");
		return MALLOC_ERROR;
	}
	memset(vscinfo, 0, sizeof(struct fb_var_screeninfo));
	retval = ioctl(fd, FBIOGET_VSCREENINFO, vscinfo);
	if (retval < 0)
	{
		perror("get varible screeninfo");
		return IOCTRL_ERROR;
	}
	
	return NO_ERROR;
}


/*
 * Function Name:fb_mmap
 * Description:Map framebuffer to application process
 * Input: file descriptor 
 * Output:None
 * Return:Ponter to virtual framebuffer in application process
 * Author:Alion
 * Date:2018.6.3
 */
inline u32 *fb_mmap(int fd)
{
	return mmap(NULL, fscinfo->smem_len, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
}

/*
 * Function Name:fb_unmap
 * Description:Umap framebuffer
 * Input:None
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */
inline int fb_unmap(void)
{
	 return munmap(pfb, fscinfo->smem_len);	
}

/*
 * Function Name:draw_pixel
 * Description:Draw pixels of screen 
 * Input:pixel coordinate(x,y) and pixel color
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
void draw_pixel(u32 color, u32 x, u32 y)
{
	u32 width = vscinfo->xres;
	*(pfb + y*width + x) = color;
}

/*
 * Function Name:draw_background
 * Description:Draw background color
 * Input:Background color
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
void draw_background(u32 color)
{
	u32 x, y;
	u32 width = vscinfo->xres;
	u32 height = vscinfo->yres;
	
	for (y=0; y<height; y++)
	{
		for (x=0; x<width; x++)
		{
			draw_pixel(color, x, y);
		}
	}
}

/*
 * Function Name:draw_line
 * Description:Draw line
 * Input:The structure containing coordinates of lines, line color
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
void draw_line(struct line_param *lparam, u32 color)
{
	u32 x1 = lparam->x1;
	u32 y1 = lparam->y1;
	u32 x2 = lparam->x2;
	u32 y2 = lparam->y2;
	
	int dx,dy,e;
	dx=x2-x1; 
	dy=y2-y1;
    
	if(dx>=0)
	{
		if(dy >= 0)
		{// dy>=0 
			if(dx>=dy)
			{// 1/8 octant
				e=dy-dx/2;  
				while(x1<=x2)
				{
					draw_pixel(color, x1, y1);
					if(e>0)
					{
						y1+=1;e-=dx;
					}	
					x1+=1;
					e+=dy;
				}
			}
			else 
			{// 2/8 octant
				e=dx-dy/2;
				while(y1<=y2)
				{
					draw_pixel(color, x1, y1);
					if(e>0)
					{
						x1+=1;e-=dy;
					}	
					y1+=1;
					e+=dx;
				}
			}
		}
		else 
		{// dy<0
			dy=-dy;   // dy=abs(dy)

			if(dx>=dy)
			{// 8/8 octant
				e=dy-dx/2;
				while(x1<=x2)
				{
					draw_pixel(color, x1, y1);
					if(e>0)
					{
						y1-=1;e-=dx;
					}	
					x1+=1;
					e+=dy;
				}
			}
			else
			{// 7/8 octant
				e=dx-dy/2;
				while(y1>=y2)
				{
					draw_pixel(color, x1, y1);
					if(e>0)
					{
						x1+=1;e-=dy;
					}	
					y1-=1;
					e+=dx;
				}
			}
		}	
	}
	else
	{//dx<0
		dx=-dx;		//dx=abs(dx)
		if(dy >= 0)
		{// dy>=0
			if(dx>=dy)
			{// 4/8 octant
				e=dy-dx/2;
				while(x1>=x2)
				{
					draw_pixel(color, x1, y1);
					if(e>0)
					{
						y1+=1;e-=dx;
					}	
					x1-=1;
					e+=dy;
				}
			}
			else
			{// 3/8 octant
				e=dx-dy/2;
				while(y1<=y2)
				{
					draw_pixel(color, x1, y1);
					if(e>0)
					{
						x1-=1;e-=dy;
					}	
					y1+=1;
					e+=dx;
				}
			}
		}
		else
		{// dy<0
			dy=-dy;   // dy=abs(dy)
			if(dx>=dy)
			{// 5/8 octant
				e=dy-dx/2;
				while(x1>=x2)
				{
					draw_pixel(color, x1, y1);
					if(e>0)
					{
						y1-=1;e-=dx;
					}	
					x1-=1;
					e+=dy;
				}
			}
			else
			{// 6/8 octant
				e=dx-dy/2;
				while(y1>=y2)
				{
					draw_pixel(color, x1, y1);
					if(e>0)
					{
						x1-=1;e-=dy;
					}	
					y1-=1;
					e+=dx;
				}
			}
		}	
	}
}


/*
 * Function Name:draw_circular
 * Description:Draw a circle
 * Input:The structure containing coordinates and radius of circle, circle color
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
void draw_circular(struct circular_param *cparam, u32 color)
{
	u32 centerX = cparam->centerX;
	u32 centerY = cparam->centerY;
	u32 radius = cparam->radius;
	u32 width = vscinfo->xres;
	int x,y ;
	int tempX,tempY;;
    int SquareOfR = radius*radius;

	for(y=0; y<width; y++)
	{
		for(x=0; x<width; x++)
		{
			if(y<=centerY && x<=centerX)
			{
				tempY=centerY-y;
				tempX=centerX-x;                        
			}
			else if(y<=centerY&& x>=centerX)
			{
				tempY=centerY-y;
				tempX=x-centerX;                        
			}
			else if(y>=centerY&& x<=centerX)
			{
				tempY=y-centerY;
				tempX=centerX-x;                        
			}
			else
			{
				tempY = y-centerY;
				tempX = x-centerX;
			}
			if ((tempY*tempY+tempX*tempX)<=SquareOfR)
			{
				draw_pixel(color, x, y);
			}
		}
	}
}




/*
 * Function Name:__draw_bmp_picture
 * Description:Draw bmp format picture.
 * Scanning Mode:Vertical,from LEFT to RIGHT,from BOTTOM to TOP
 * Input:Pointer to data, resolution, start coordinate
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
void __draw_bmp_picture(const u8 *pData, u32 img_xres, u32 img_yres, u32 start_x, u32 start_y)
{	
	u32 x, y, color, p = 0;
		for (y=img_yres+start_y; y>start_y; y--)
		{
			for (x=start_x; x<img_xres+start_x; x++)
			{
				if (y > vscinfo->yres)
				{		
					break;
				}
				
				if (x > vscinfo->xres)
				{		
					p += 3;
					continue;
				}
				
				color = ((pData[p+0] << 0) | (pData[p+1] << 8) | (pData[p+2] << 16));
				draw_pixel(color, x, y);
				p += 3;
		}
	}
}

/*
 * Function Name:__draw_jpg_picture
 * Description:Draw JPG format picture.
 * Scanning Mode:Vertical,from LEFT to RIGHT,from BOTTOM to TOP
 * Input:Pointer to data, resolution, start coordinate
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */

void __draw_jpg_picture(const u8 *pData, u32 img_xres, u32 img_yres, u32 start_x, u32 start_y)
{	
	u32 x, y, color, p = 0;
		for (y=start_y; y<img_yres+start_y; y++)
		{
			for (x=start_x; x<img_xres+start_x; x++)
			{
				if (y > vscinfo->yres)
				{		
					break;
				}
				
				if (x > vscinfo->xres)
				{		
					p += 3;
					continue;
				}
				
				color = ((pData[p+2] << 0) | (pData[p+1] << 8) | (pData[p+0] << 16));
				draw_pixel(color, x, y);
				p += 3;
		}
	}
}


/*
 * Function Name:__draw_png_picture
 * Description:Draw PNG format picture.
 * Scanning Mode:Vertical,from LEFT to RIGHT,from BOTTOM to TOP
 * Input:Pointer to data, resolution, start coordinate
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
void __draw_png_picture(const u8 *pData, u32 img_xres, u32 img_yres, u32 start_x, u32 start_y)
{	
	u32 x, y, color, p = 0;
		for (y=start_y; y<img_yres+start_y; y++)
		{
			for (x=start_x; x<img_xres+start_x; x++)
			{
				if (y > vscinfo->yres)
				{		
					break;
				}
				
				if (x > vscinfo->xres)
				{		
					p += 3;
					continue;
				}
					
				color = ((pData[p+2] << 0) | (pData[p+1] << 8) | (pData[p+0] << 16));
				draw_pixel(color, x, y);
				p += 3;
		}
	}
}










