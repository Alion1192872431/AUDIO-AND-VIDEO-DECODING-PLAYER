/******************************************************************************
 * File Name:fb_jpeg.c
 * Description:This file includes some functions releted to JPEG format picture
 * Author:Alion
 * Date:2018.6.3
 *****************************************************************************/

/* Header Files */
#include <stdio.h>
#include <fb_jpeg.h>
#include <string.h>

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
 * Function Name:is_jpg
 * Description:Whether the specified file is JPEG format or not
 * Input:music file pathname
 * Output:None
 * Return:ERROR NUMBER 
 * Author:Alion
 * Date:2018.6.3
 */
int is_jpg(const char *pathname)
{
	FILE *file = NULL;
	u8 jpg_buf[2] = {0};

	if (NULL ==(file = fopen(pathname, "rb"))) 
	{
		fprintf(stderr, "open FILE %s failed.\n", pathname);
		return OPEN_ERROR;
	}
	
	/* Read out the first two bytes  */
	if (fread(jpg_buf, 1, 2, file) < 0)
	{	
		fprintf(stderr, "read FILE %s header failed.\n", pathname);
		goto freaderr;
	}
	Debug("%x %x.\n", jpg_buf[0], jpg_buf[1]);
	
	/* Is bmp format? */
	if (jpg_buf[0] != 0xff || jpg_buf[1] != 0xd8)	/* Whether the first two bytes is 0xFFD8 or not */
	{			
		fclose(file);
		goto checkerr; 
	}
	if (fseek(file, -2, SEEK_END) < 0)
	{		
		fprintf(stderr, "fseek to FILE %s tail failed.\n", pathname);
		goto fseekerr;
	}
	/* Read out the last two bytes */
	if (fread(jpg_buf, 1, 2, file) < 0)
	{		
		fprintf(stderr, "read FILE %s tail failed.\n", pathname);
		goto freaderr;
	}
	Debug("%x %x.\n", jpg_buf[0], jpg_buf[1]);
	
	if (jpg_buf[0] != 0xff || jpg_buf[1] != 0xd9)	/* Whether the last two bytes is 0xFFD9 or not */
	{	
		fprintf(stderr, "this picture is not a jpg file.\n");
		goto checkerr; 
	}
	fclose(file);
	return NO_ERROR;

	
checkerr:	
fseekerr:
freaderr:
	fclose(file);
	return COMMON_ERROR;	
}



/*
 * Description:Error callback function realized by us
 */
METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}


/*
 * Function Name:jpg_analysis
 * Description:Parsing file information(size,offset of valid data, resolution,etc.)
 * Input:struct ImgInfo
 * Output:None
 * Return:ERROR NUMBER 
 * Author:Alion
 * Date:2018.6.3
 */
static int jpg_analysis(struct ImgInfo *img_info)
{
	struct jpeg_decompress_struct cinfo;		/* 结构体包含解压缩参数和指向fopen打开图片后的动态内存空间的指针 */
	struct my_error_mgr jerr;					/* 结构体中包含我们定义的一个错误处理回调函数，使用这个结构体时 */
	 											/* 必须保证定义了struct jpeg_decompress_struct cinfo;以防止野指针的问题 */
	FILE * infile;								
  	unsigned char *buffer = NULL;				/* Buffer used to store the decoded one line data of image */
  	int row_stride;								/* buffer的行宽度(字节为单位)=图像的xres*bpp/8 */
	u8 *pDataBuf = NULL;

	
	/* Malloc memory to store valid data of picture*/
	pDataBuf = (unsigned char *)malloc(sizeof(unsigned char) * IMG_BUF_SIZE);
	if (NULL == pDataBuf)
	{
		fprintf(stderr, "malloc for DataBuf failed.\n");
		return MALLOC_ERROR;
	}
	memset(pDataBuf, 0, sizeof(pDataBuf));
	
	/* Step1:open picture source file(readonly、binary) */
	infile = fopen(img_info->pathname, "rb");
	if (NULL == infile)
	{
	    fprintf(stderr, "can't open %s\n", img_info->pathname);
	    return OPEN_ERROR;
	}
	Debug("open %s success.\n", img_info->pathname);
	
	/* Step2:Malloc for struct cinfo and initialize this structure */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;		/* Bind the error handler function to our own design error callback function */
	if (setjmp(jerr.setjmp_buffer))				/* Judge whether the decoding is wrong */
	{			
	    jpeg_destroy_decompress(&cinfo);		/* clear jpeg object ,close source file and return */
	    fclose(infile);
	    return COMMON_ERROR;
	}
	jpeg_create_decompress(&cinfo);				/* Initialize structure cinfo */

	/* Step3: specify data source */
  	jpeg_stdio_src(&cinfo, infile);
	
	/* Step4:Read out file parameters from file header */
	(void) jpeg_read_header(&cinfo, TRUE);
	
	/* Step5:start decoder up  */
	(void) jpeg_start_decompress(&cinfo);
	
	Debug("resolution is %d * %d, bpp = %d.\n", 	\
		cinfo.output_width, cinfo.output_height, cinfo.output_components*8);
	
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (unsigned char*)malloc(row_stride);
	
	/* Step6:Decode picture data line by line*/ 
	while (cinfo.output_scanline < cinfo.output_height)
	{
	   (void) jpeg_read_scanlines(&cinfo, &buffer, 1);			/* 以行为单位解码图片数据并放置到行缓存里 */
		memcpy(pDataBuf+(cinfo.output_scanline-1)*row_stride, buffer, row_stride);			/* 将解码出来的一行一行的数据放置到一个数据缓冲区 */
	}
	/* Step7:fill structure img_info */
	img_info->img_pData = pDataBuf;
	img_info->img_xres  = cinfo.output_width;
	img_info->img_yres  = cinfo.output_height;
	img_info->img_bpp	= cinfo.output_components*8;
	
	Debug("here is put_scanline_someplace end.\n");
	
	/* Step8:stop decoder */
	(void) jpeg_finish_decompress(&cinfo);
	
	/* Step9:Release system resource */
	jpeg_destroy_decompress(&cinfo);
	Debug("here is jpeg_destroy_decompress end.\n");
	
	/* Step10: Close picture source file */
	fclose(infile);
	
	return NO_ERROR;
}

 
 /*
 * Function Name:draw_jpg_picture
 * Description:To draw JPG format picture on screen
 * Input:struct ImgInfo
 * Output:None
 * Return:None 
 * Author:Alion
 * Date:2018.6.3
 */
static void draw_jpg_picture(struct ImgInfo *img_info)
{
	__draw_jpg_picture(img_info->img_pData, img_info->img_xres, img_info->img_yres, img_info->img_startX, img_info->img_startX);
	
}


/*
 * Function Name:display_jpg
 * Description:To display JPG format picture on screen
 * Input:struct ImgInfo
 * Output:None
 * Return:ERROR NUMBER 
 * Author:Alion
 * Date:2018.6.3
 */
int display_jpg(ImgInfo *img_info)
{
	int retval = -1;
	
	/* Step1: Check picture format */
	retval = is_jpg(img_info->pathname);
	if (retval != 0)
	{
		return COMMON_ERROR;
	}

	/* Step2:Decode picture and get it's valid data */
	retval = jpg_analysis(img_info);
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
	draw_jpg_picture(img_dst);
	Debug("here is draw_jpg_picture end.\n");
	free(img_info->img_pData);
	return NO_ERROR;
}


