/******************************************************************************
 * File Name:framebuffer.c
 * Description:This file includes some functions releted to picture management
 * Author:Alion
 * Date:2018.6.3
 *****************************************************************************/

/* Header Files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <image_manager.h>
#include <fb_bmp.h>
#include <fb_jpeg.h>
#include <fb_png.h> 
#include <fcntl.h>
#include <linux/input.h>

/* Debug Macro */
#define DEBUG
#ifdef DEBUG
#define Debug(fmt,...) printf("FILE: "__FILE__", LINE: %d: "fmt"", \
                        __LINE__, ##__VA_ARGS__)
#else
#define Debug(fmt,...)
#endif


ImgInfo images[MAX_IMG_CNT];	/* Array used to manage pictures*/

u32 image_index = 0;			

extern struct fb_var_screeninfo *vscinfo;

/*
 * Function Name:scan_image
 * Description:Recursively find all pictures under a specified folder
 * Input:The specified folder's pathname
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */
int scan_image(const char *baseDir)
{
	DIR *dir = NULL;
	struct dirent *ptr = NULL;
	struct stat buf = {0};
	char base[1000] = {0};

	/* Open specified director */
	if ((dir=opendir(baseDir)) == NULL)
	{			
		fprintf(stderr, "Open %s error....\n", baseDir);
		return OPEN_ERROR;
    }
	
	while ((ptr=readdir(dir)) != NULL)
	{			
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)/* The file read out is current director or parent director */
		{	  
			continue;
		}
		
		/* Process file name, get file attribute information */
		memset(base,'\0',sizeof(base));
		strcpy(base,baseDir);
		strcat(base,"/");
		strcat(base,ptr->d_name);
		lstat(base, &buf); 
		
		/* Determine the type of file */
		if (S_ISREG(buf.st_mode))		/* regular file */
		{		
			Debug("this file is a regular file.\n");
		
			/* Determine the format of picture */
			if (!is_bmp(base))
			{
				strcpy(images[image_index].pathname, base);
				images[image_index].type = IMG_TYPE_BMP;	
			}
			
			if (!is_jpg(base))
			{
				strcpy(images[image_index].pathname, base);
				images[image_index].type = IMG_TYPE_JPG;	
			}
			
			if (!is_png(base))
			{
				strcpy(images[image_index].pathname, base);
				images[image_index].type = IMG_TYPE_PNG;
			}
			image_index++;
		}
		else if(S_ISDIR(buf.st_mode))
		{
			Debug("this file is a directoy.\n");
			scan_image(base);
		}
		else
		{
			fprintf(stderr, "this file is not wanted.\n");
			return COMMON_ERROR;
		} 
	}
	closedir(dir);
	return NO_ERROR;
}

/*
 * Function Name:print_images
 * Description:print all images 
 * Input:None
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
void print_images(void)
{
	int i;
	
	printf("iamge_index = %d.\n", image_index);
	
	for (i=0; i<image_index; i++)
	{
		printf("images[i].pathname = %s, type = %d.\n", images[i].pathname, images[i].type);
	}
}

/*
 * Function Name:display_image
 * Description:Play images in order
 * Input:None
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
void display_image(void)
{
	int i;

	for (i=0; i<image_index; i++)
	{
		switch (images[i].type)
		{
			case IMG_TYPE_BMP:
				Debug("IMG_TYPE_BMP.\n");
				display_bmp(&images[i]);		
				break;
			case IMG_TYPE_JPG:
				Debug("IMG_TYPE_JPG.\n");
				display_jpg(&images[i]);		
				break;
			case IMG_TYPE_PNG:
				Debug("IMG_TYPE_PNG.\n");
				display_png(&images[i]);		
				break;
			default:
				break;
		}
		sleep(2);
	}
}

/*
 * Function Name:show_image
 * Description:Play specified image according to index
 * Input:Index of array
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
void show_image(int index)
{
	Debug("index = %d.\n", index);
	switch (images[index].type)
	{
		case IMG_TYPE_BMP:
			display_bmp(&images[index]);		
			break;
		case IMG_TYPE_JPG:
			display_jpg(&images[index]);		
			break;
		case IMG_TYPE_PNG:
			display_png(&images[index]);		
			break;
		default:
			break;
	}	
}



/*
 * Function Name:ts_slide
 * Description:slide or touch screen to switch picture
 * Input:None
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */
int ts_slide(void)
{
	
	int fd = -1, ret = -1;
	struct input_event ev;
	u32 WIDTH = vscinfo->xres;
	u32 ev_value[10] = {0};
	int imgNum = 0;		/*Used to record the current picture's index in array*/
	int k = 0; 
	
	fd = open(DEVICE_TOUCHSCREEN, O_RDONLY);
	if (fd < 0)
	{
		perror("open");
		return OPEN_ERROR;
	}
	while (1)
	{
		/* Step1:Detect touch event */
		k = 0;
		memset(ev_value, 0, sizeof(ev_value));
		memset(&ev, 0, sizeof(struct input_event));
		
		/* Read input event before touch bounce */
		while (!(ev.type==EV_ABS && ev.code == ABS_PRESSURE && ev.value == 0))
		{ 	
			ret = read(fd, &ev, sizeof(struct input_event));
			if (ret != sizeof(struct input_event))
			{
				perror("read");
				close(fd);
				return READ_ERROR;
			}
			
			/* Parsing the event package to know what kind of input event occurred */
			Debug("-------------------------\n");
			Debug("type: %hd\n", ev.type);
			Debug("code: %hd\n", ev.code);
			Debug("value: %d\n", ev.value);
			Debug("\n");
			
			/* Step2:put valid data into array */
			if ((ev.type == EV_ABS)&&(ev.code == ABS_X))
			{
				Debug("–¥»Î ˝◊È.\n");
				if (k < 6)
				{ 
					ev_value[k] = ev.value;
					Debug("ev_value[%d] = %d.\n", k, ev_value[k]);
					k++;	
				}	
			}
		} 
		/*
		for (k=0; k<6; k++) {
			printf("ev_value[%d] = %d.\n", k, ev_value[k]);

		}
		*/
		if ((ev.type == EV_ABS)&&(ev.code == ABS_PRESSURE)&&(ev.value == 0))
		{
			if (ev_value[5])
			{
				Debug("ªÆ∆¡∑≠“≥.\n");
				if ((ev_value[0] > ev_value[5])&&((ev_value[0] - ev_value[5]) > 50))
				{
					Debug("…œ∑≠“≥.\n");
					/* switch pages forward */
					if (imgNum-- <= 1)
					{
						imgNum = image_index;
						Debug("imgNum=%d.\n", imgNum);
					}	
		 		}
				else if ((ev_value[0] < ev_value[5])&&((ev_value[5] - ev_value[0]) > 50))
		 		{
					Debug("œ¬∑≠“≥.\n");
					/* switch pages backward */			
					if (imgNum++ >= image_index)
					{
						imgNum= 1;
						Debug("imgNum=%d.\n", imgNum);
		 			}
				}
			}
			else if (!ev_value[5])
			{
				Debug("¥•√˛∑≠“≥.\n"); 
				if ((ev_value[0] >= 0) && (ev_value[0] < TOUCH_WIDTH))
				{
					Debug("…œ∑≠“≥.\n");
					/* switch pages forward */
					if (imgNum-- <= 1)
					{
						imgNum = image_index;
						Debug("imgNum=%d.\n", imgNum);
					}			
				}
				else if((ev_value[0] > (WIDTH - TOUCH_WIDTH)) && (ev_value[0] <= WIDTH))
				{
					Debug("œ¬∑≠“≥.\n");
					/* switch pages backward */		
					if (imgNum++ >= image_index)
					{
						imgNum= 1;
						Debug("imgNum=%d.\n", imgNum);
					} 	
				} 	
			} 
			show_image(imgNum - 1);
		} 	
	}
	close(fd);
	return NO_ERROR;
}



 /*
  * Function Name:ts_updown
  * Description:touch screen to switch picture
  * Input:None
  * Output:None
  * Return:ERROR NUMBER
  * Author:Alion
  * Date:2018.6.3
  */
int ts_updown(void)
{
	/* Step1:Detect touch event */
	int fd = -1, ret = -1;
	struct input_event ev;
	int imgNum = 0;							
	u32 WIDTH = vscinfo->xres;
	
	fd = open(DEVICE_TOUCHSCREEN, O_RDONLY);
	if (fd < 0)
	{
		perror("open");
		return OPEN_ERROR;
	}
		
	while (1)
	{
		memset(&ev, 0, sizeof(struct input_event));
		ret = read(fd, &ev, sizeof(struct input_event));
		if (ret != sizeof(struct input_event))
		{
			perror("read");
			close(fd);
			return READ_ERROR;
		}
		
		/* Parsing the event package to know what kind of input event occurred */
		Debug("-------------------------\n");
		Debug("type: %hd\n", ev.type);
		Debug("code: %hd\n", ev.code);
		Debug("value: %d\n", ev.value);
		Debug("\n");
		 
		/* Step2:Switch pages according to touch coordinate */
		if ((ev.type == EV_ABS) && (ev.code == ABS_X))	/* Is x coordinate? */
		{	
			if ((ev.value >= 0) && (ev.value < TOUCH_WIDTH))
			{
				/* Switch page forward */
				if (imgNum-- <= 1)
				{
					imgNum = image_index;
					Debug("imgNum=%d.\n", imgNum);
				}
			}
			else if ((ev.value > (WIDTH - TOUCH_WIDTH)) && (ev.value <= WIDTH))
			{
				/* Switch page backward */			
				if (imgNum++ >= image_index)
				{
					imgNum = 1;
					Debug("imgNum=%d.\n", imgNum);
				}
			}
			else
			{}
			show_image(imgNum - 1);
		} 	 
	}	
	close(fd);
	return NO_ERROR;	
} 





