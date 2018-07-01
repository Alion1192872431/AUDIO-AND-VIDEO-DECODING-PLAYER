/******************************************************************************
 * File Name:imageplay.c
 * Description:This file includes some functions releted to playing pictures
 * Author:Alion
 * Date:2018.6.3
 *****************************************************************************/

/* Header Files */
#include <stdio.h>
#include <framebuffer.h>
#include <fb_bmp.h>
#include <fb_jpeg.h>
#include <fb_png.h>
#include <string.h>
#include <stdlib.h>
#include <image_manager.h>
#include <unistd.h>

/* Debug Macro */
//#define DEBUG
#ifdef DEBUG
#define Debug(fmt,...) printf("FILE: "__FILE__", LINE: %d: "fmt"", \
                        __LINE__, ##__VA_ARGS__)
#else
#define Debug(fmt,...)
#endif



/* Macro Definition */
#define DEVNAME	"/dev/fb0"

/* Global Variables */
extern struct fb_fix_screeninfo *fscinfo;
extern struct fb_var_screeninfo *vscinfo;
extern unsigned int *pfb;
extern ImgInfo images[MAX_IMG_CNT];
extern unsigned int image_index;
ImgInfo *img_dst = NULL; 

ImgInfo coverImgInfo = {
	.pathname = "picture/cover/cover.jpg",
};

/*
 * Function Name:img_play
 * Description:The top function to play images
 * Input:None
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */
int img_play(void) 
{
	int retval = -1;
	int fb_fd = -1;
	unsigned int start_x = 0;
	unsigned int start_y = 0;

	img_dst = (ImgInfo *)malloc(sizeof(ImgInfo));
	if (NULL == img_dst)
	{
		fprintf(stderr, "malloc for img_dst failed.\n");
		return MALLOC_ERROR;
	}
	memset(img_dst, 0, sizeof(img_dst));
	
	/* Step1: Open device file */
	fb_fd = fb_open(DEVNAME);
	if (fb_fd < 0)
	{
		fprintf(stderr, "open %s failed.\n", DEVNAME);
		return OPEN_ERROR;
	}
	
	/* Step2:Get device information */	
	get_fixinfo(fb_fd);
	get_varinfo(fb_fd);
	
	/* Step3: Map framebuffer to application process */
	pfb = fb_mmap(fb_fd);
	if (NULL == pfb)
	{
		fprintf(stderr, "fb_mmap failed.\n");
		return COMMON_ERROR;
	}
	Debug("sizeof(long) = %d,  sizeof(int) = %d\n", sizeof(long), sizeof(int));
	 
	/* Step4: Draw background */
	draw_background(BLUE);
	
	/* Step5:Display cover images */
	display_jpg(&coverImgInfo); 
	scan_image("picture");
	print_images();
	display_image();	
	//ts_updown();		
	//ts_slide();  		
	/* Step6:unmap */
	fb_unmap();
	fb_close(fb_fd);

	return NO_ERROR;
}


