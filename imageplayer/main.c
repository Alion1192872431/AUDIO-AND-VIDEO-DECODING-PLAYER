/******************************************************************************
 * File Name:main.c
 * Description: This file is the entry of the whole program
 * Author:Alion
 * Date:2018.6.3
 *****************************************************************************/
/* Header Files */
#include <stdio.h>
#include <framebuffer.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fb_bmp.h>
#include <fb_jpeg.h>
#include <fb_png.h>
#include <string.h>
#include <stdlib.h>
#include <image_manager.h>
#include <unistd.h>

/* Debug Macro */
#define DEBUG
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
 * Description:To play images with multi-Progress 
 * Input:None
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.5.25
 */
int img_play(void) 
{
	int retval = -1;
	int fb_fd = -1;
	unsigned int start_x = 0;
	unsigned int start_y = 0;

	img_dst = (ImgInfo *)malloc(sizeof(ImgInfo));
	if(NULL == img_dst) 
	{
		fprintf(stderr, "malloc for img_dst failed.\n");
		return MALLOC_ERROR;
	}
	/* Clear the memory */
	memset(img_dst, 0, sizeof(img_dst));
	
	/* Step1:Open device file */
	fb_fd = fb_open(DEVNAME);
	if (fb_fd < 0) 
	{
		fprintf(stderr, "open %s failed.\n", DEVNAME);
		return OPEN_ERROR;
	}
	
	/* Step2:Get information about device */	
	get_fixinfo(fb_fd);					/* Get fixed information of screen */
	get_varinfo(fb_fd);					/* Get variable information of screen */
	
	/* Step3:Map the framebuffer memory area to the application process£¨mmap£©*/
	pfb = fb_mmap(fb_fd);
	if (NULL == pfb) {
		fprintf(stderr, "fb_mmap failed.\n");
		return MAP_ERROR;
	}
	Debug("sizeof(long) = %d,  sizeof(int) = %d\n", sizeof(long), sizeof(int));
	 
	/* Step4:Draw background */
	draw_background(BLUE);
	
	/* Step5:Display cover image */
	display_jpg(&coverImgInfo);
	
	/* Step6:Display images */
	scan_image("picture");			/*Scan images in specified folder*/
	print_images();
	//display_image();	
	//ts_updown();		/* Click on the screen to turn pages */
	ts_slide();  		/* Slide the screen to turn pages */
	
	/* Step6:Unmap framebuffer memory */
	fb_unmap();
	
	/* Close device file */
	fb_close(fb_fd);

	return NO_ERROR;
}



/*
 * Function Name:main
 * Description:The entry of the project 
 * Input:argc---the quantity of arguments  argv---pointer to the value array of arguments
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.5.25
 */
int main(int argc, char *argv[])
{
	pid_t pid = -1;
	int status = -1;
	char buf[20] = {0};
 
	/* Fork child process  */
	pid = fork();
	if (pid > 0) 			/* Parent */
	{
		img_play(); 
		waitpid(pid, &status, 0);
		return NO_ERROR;
	}
	else if(pid == 0)		/* Child */
	{
		//sprintf(buf,"audios/include/%s",argv[1]);
		//execl("audios/audioplayer", "audio", buf, NULL);
		Debug("===========================here is child====================.\n");
		execl("./audios/audioplayer", "audio", NULL);
		return NO_ERROR;
	}
	else 
	{
		fprintf(stderr, "fork error.\n");
		return FORK_ERROR;
	}
	return NO_ERROR;
}
 