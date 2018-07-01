/******************************************************************************
 * File Name:fb_png.c
 * Description:This file includes some functions releted to PNG format picture
 * Author:Alion
 * Date:2018.6.3
 *****************************************************************************/

/* Header Files */
#include <stdio.h>
#include <base.h>
#include <fb_png.h>
#include <string.h>
#include <png.h>
#include <stdlib.h>
#include <pngstruct.h>
#include <pnginfo.h>
#include <framebuffer.h>


/* Macro Definition */
#define PNG_BYTES_TO_CHECK 4

/* Global Variables */
extern ImgInfo *img_dst;
extern struct fb_fix_screeninfo *fscinfo;
extern struct fb_var_screeninfo *vscinfo;


/* Debug Macro */
//#define DEBUG
#ifdef DEBUG
#define Debug(fmt,...) printf("FILE: "__FILE__", LINE: %d: "fmt"", \
                        __LINE__, ##__VA_ARGS__)
#else
#define Debug(fmt,...)
#endif

/*
 * Function Name:is_png
 * Description:Whether the specified file is PNG format or not
 * Input:music file pathname
 * Output:None
 * Return:ERROR NUMBER 
 * Author:Alion
 * Date:2018.6.3
 */
int is_png(const char *pathname)
{
	FILE *file =NULL;
	char buf[PNG_BYTES_TO_CHECK] = {0};
	int retval = -1;
	
   if ((file = fopen(pathname, "rb")) == NULL)
   {
   		fprintf(stderr, "fopen FILE %s failed.\n", pathname);
		return OPEN_ERROR;
   }  
   //Debug("here is check image type 1.\n");

   if (fread(buf, 1, PNG_BYTES_TO_CHECK, file) != PNG_BYTES_TO_CHECK)
   {
   		fprintf(stderr, "fread FILE %s checkinfo error.\n", pathname);
		goto freaderr;
   }
	retval = (png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK));
   if (retval)
   {
   		fprintf(stderr, "this picture is not a png file.\n");
		goto checkerr;
   }
   	fclose(file);
	return NO_ERROR;


	
checkerr:	
freaderr:
	fclose(file);
	return COMMON_ERROR;
}


/*
 * Function Name:png_analysis
 * Description:Parsing file information(size,offset of valid data, resolution,etc.)
 * Input:struct ImgInfo
 * Output:None
 * Return:ERROR NUMBER 
 * Author:Alion
 * Date:2018.6.3
 */
static int png_analysis(struct ImgInfo *img_info)
{
	FILE *fp = NULL;
	png_structp png_ptr;   
	png_infop info_ptr;
	unsigned char color_type;
	png_bytep* row_pointers;
	unsigned long len = 0;
	int pos = 0;
	int i = 0, j = 0;
	u8 *pDataBuf = NULL;
	
	/* Malloc memory to store valid data of picture*/
	pDataBuf = (unsigned char *)malloc(sizeof(unsigned char) * IMG_BUF_SIZE);
	if (NULL == pDataBuf)
	{
		fprintf(stderr, "malloc for DataBuf failed.\n");
		return MALLOC_ERROR;
	}
	memset(pDataBuf, 0, sizeof(pDataBuf));

	/* open picture source file(readonly¡¢binary) */
	if ((fp = fopen(img_info->pathname, "rb")) == NULL)
	{	 
		fprintf(stderr, "can't open %s\n", img_info->pathname);    
		return OPEN_ERROR;	
	}

	/* Step1: Related data structure instantiation */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (png_ptr == 0)
	{
		fclose(fp);
		return COMMON_ERROR;
	}

	info_ptr = png_create_info_struct(png_ptr);
  	if (info_ptr == 0)
	{
   		png_destroy_read_struct(&png_ptr, 0, 0);
   		fclose(fp);
   		return COMMON_ERROR;
  	}

	/* Step2: Set error handler */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		fclose(fp);
		return COMMON_ERROR;
	}

	/* Step3: Bind the file pointer of the png image to be decoded with the png decoder */
	png_init_io(png_ptr, fp);

	/* Step4: Read png format picture information */
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_ALPHA, 0);

	color_type = info_ptr->color_type;
	Debug("color_type = %d\n", color_type);

	img_info->img_pData = pDataBuf;
	img_info->img_xres	= info_ptr->width;
	img_info->img_yres 	= info_ptr->height;
	img_info->img_bpp 	= info_ptr->pixel_depth;
	len = info_ptr->width * info_ptr->height * info_ptr->pixel_depth / 8;
	Debug("width = %u, height = %u, bpp = %u\n", img_info->img_xres, img_info->img_yres, img_info->img_bpp);

	/* Step6: read data */
	row_pointers = png_get_rows(png_ptr,info_ptr);

	/* Only deal with 24bit RGB true color picture for the moment*/
	/* Step7:save data into buffer */
	if(color_type == PNG_COLOR_TYPE_RGB) 
	{
   		//memcpy(DataBuf, row_pointers, len);
   		
		for(i=0; i<img_info->img_yres; i++)
		{
			
			for(j=0; j<3*img_info->img_xres; j+=3)
			{
				
				pDataBuf[pos++] = row_pointers[i][j+0];		//red
				pDataBuf[pos++] = row_pointers[i][j+1];		//green
				pDataBuf[pos++] = row_pointers[i][j+2];		//blue
			}
		}
  	}

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	fclose(fp);
	return NO_ERROR;
}

/*
* Function Name:draw_png_picture
* Description:To draw PNG format picture on screen
* Input:struct ImgInfo
* Output:None
* Return:None 
* Author:Alion
* Date:2018.6.3
*/
static void draw_png_picture(struct ImgInfo *img_info)
{
	__draw_png_picture(img_info->img_pData, img_info->img_xres, img_info->img_yres,  img_info->img_startX,  img_info->img_startX);
	
}


/*
 * Function Name:display_png
 * Description:To display PNG format picture on screen
 * Input:struct ImgInfo
 * Output:None
 * Return:ERROR NUMBER 
 * Author:Alion
 * Date:2018.6.3
 */
int display_png(ImgInfo *img_info)
{
	int retval = -1;
	
	/* Step1: Check picture format */
	retval = is_png(img_info->pathname);
	if (retval != 0)
	{
		return COMMON_ERROR;
	}

	/* Step2:Decode picture and get it's valid data */
	retval = png_analysis(img_info);
	if (retval < 0)
	{
		return COMMON_ERROR;
	}
	Debug("start_x = %d, start_y = %d.\n", img_info->img_startX, img_info->img_startY);
	
	/* Step3:Zoom picture */
	img_dst->img_xres = vscinfo->xres;
	img_dst->img_yres = vscinfo->yres;
	img_dst->img_bpp = img_info->img_bpp;
	
	do_Stretch_Linear(img_dst, img_info);
	Debug("here is do_Stretch_Linear end.\n");
	
	/* Step4:Display picture */
	draw_png_picture(img_dst);
	free(img_info->img_pData);
	return NO_ERROR;
}



