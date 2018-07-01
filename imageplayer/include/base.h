#ifndef __BASE_H__
#define __BASE_H__

#include <linux/fb.h>
/***********************************************/
#define IMG_MAX_XRES		(4160)
#define IMG_MAX_YRES		(3120)
#define IMG_MAX_BPP			(24)
#define IMG_BUF_SIZE		(IMG_MAX_XRES*IMG_MAX_YRES*IMG_MAX_BPP/8)
#define MAX_NAME_LENTH			(256)

/*****************************ERROR NUMBER************************/
#define NO_ERROR			 0
#define COMMON_ERROR		-1
#define OPEN_ERROR			-2
#define MAP_ERROR			-3
#define FORK_ERROR			-4
#define IOCTRL_ERROR		-5
#define JUDGE_ERROR			-7
#define MALLOC_ERROR		-8
#define READ_ERROR			-9
#define WRITE_ERROR			-10

/*************************************TYPEDEF****************************************/
typedef unsigned long ul;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;


#define DEVICE_TOUCHSCREEN		"/dev/input/event2"
#define TOUCH_WIDTH				200				/* Width of area which is touched to switch pages */ 

/********************************************************************************/
typedef enum img_type
{

	IMG_TYPE_BMP,
	IMG_TYPE_JPG,
	IMG_TYPE_PNG,
	IMG_TYPE_UNKNOWN,
	
}img_type_e;



/* Structure to save image information */
typedef struct ImgInfo
{		
	char pathname[MAX_NAME_LENTH];
	img_type_e type;
	u8 *img_pData;			/* Pointer to image data */
	u32 img_xres;
	u32 img_yres;
	u32 img_bpp;
	u32 img_lsize;
	u32 img_size;
	u16 img_startX;		/* The starting point coordinate x of the picture on screen */
	u16 img_startY;		/* The starting point coordinate y of the picture on screen */
}ImgInfo;

/* framebuffer information */
typedef struct FbInfo	
{		
	u32 *pfb;
	struct fb_var_screeninfo *vscinfo;
	struct fb_fix_screeninfo *fscinfo;
}FbInfo;


int do_Stretch_Linear(ImgInfo *dst, const ImgInfo *src);


#endif


