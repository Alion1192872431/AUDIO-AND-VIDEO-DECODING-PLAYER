/******************************************************************************
 * File Name:fb_bmp.c
 * Description:This file includes some functions releted to bmp format picture
 * Author:Alion
 * Date:2018.6.3
 *****************************************************************************/

/* Header Files */
#include <fb_bmp.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


/* Macro Definitions */


/* Debug Macro */
//#define DEBUG
#ifdef DEBUG
#define Debug(fmt,...) printf("FILE: "__FILE__", LINE: %d: "fmt"", \
                        __LINE__, ##__VA_ARGS__)
#else
#define Debug(fmt,...)
#endif

/* Global Variables */
extern ImgInfo *img_dst;
extern struct fb_fix_screeninfo *fscinfo;
extern struct fb_var_screeninfo *vscinfo;


/*
 * Function Name:is_bmp
 * Description:Whether the specified file is bmp format or not
 * Input:music file pathname
 * Output:None
 * Return:ERROR NUMBER 
 * Author:Alion
 * Date:2018.6.3
 */
int is_bmp(const char *pathname)
{
	int fd = -1;
	int retval = -1;
	unsigned char bmp_buf[2] = {0};

	fd = open(pathname, O_RDONLY);						
	if (fd < 0)
	{
		fprintf(stderr, "open FILE %s failed.\n", pathname);
		return OPEN_ERROR;
	}
	
	/* Read out the first two bytes  */
	retval = read(fd, bmp_buf, sizeof(bmp_buf));		
	if (retval < 0)
	{
		fprintf(stderr, "read FILE %s type failed.\n", pathname);
		goto err;
	}

	if (!((bmp_buf[0] == 'B') &&(bmp_buf[1] == 'M')))		/* Is bmp format? */
	{	
		fprintf(stderr, "this picture is not a bmp file.\n");
		goto err;
	}
	close(fd);
	return NO_ERROR;
	
err:
	close(fd);
	return COMMON_ERROR;
}

 
/*
 * Function Name:bmp_analysis
 * Description:Parsing file information(size,offset of valid data, resolution,etc.)
 * Input:struct ImgInfo
 * Output:None
 * Return:ERROR NUMBER 
 * Author:Alion
 * Date:2018.6.3
 */
static int bmp_analysis(struct ImgInfo *img_info)
{
	BFileHeader *fhdr = NULL;
	BInfoHeader *ihdr = NULL;
	int fd = -1;
	int retval = -1; 
	u8 *pDataBuf = NULL;
	
	/* Malloc memory to store valid data of picture*/
	pDataBuf = (unsigned char *)malloc(sizeof(unsigned char) * IMG_BUF_SIZE);
	if (NULL == pDataBuf)
	{
		fprintf(stderr, "malloc for DataBuf failed.\n");
		return MALLOC_ERROR;
	}
	
	/* Malloc memory to store header information of picture */
	fhdr = (BFileHeader *)malloc(sizeof(BFileHeader));
	if (NULL == fhdr)
	{
		fprintf(stderr, "malloc for fileheaderinfo failed.\n");
		return MALLOC_ERROR;
	}
	/* Malloc memory to store information header of picture */
	ihdr = (BInfoHeader *)malloc(sizeof(BInfoHeader));
	if (NULL == ihdr)
	{
		fprintf(stderr, "malloc memory for fileinfoheader failed.\n");
		return MALLOC_ERROR;
	}
	
	/* Clear memory malloced  */
	memset(pDataBuf, 0, sizeof(pDataBuf));
	memset(fhdr, 0, sizeof(fhdr));
	memset(ihdr, 0, sizeof(ihdr));

	fd = open(img_info->pathname, O_RDONLY);						
	if (fd < 0) 
	{
		fprintf(stderr, "open FILE %s failed.\n", img_info->pathname);
		goto openerr;
	}
	Debug("open %s success.\n", img_info->pathname);
	
	/* Jump over the first two bytes that represent file type */
	retval = lseek(fd, sizeof(short), SEEK_SET);			
	if (retval < 0)
	{
		fprintf(stderr, "lseek for headtype failed.\n");
		goto lseekerr;
	}

	/* Read out header information of file */
	retval = read(fd, fhdr, sizeof(BFileHeader));		
	if (retval < 0)
	{
		fprintf(stderr, "read FILE %s header failed.\n", img_info->pathname);
		goto readerr;
	}
	
	/* Read out information header */
	retval = read(fd, ihdr, sizeof(BInfoHeader));		
	if (retval < 0)
	{
		fprintf(stderr, "read FILE %s header failed.\n", img_info->pathname);
		goto readerr;
	}
	
	img_info->img_xres = ihdr->biWidth;
	img_info->img_yres = ihdr->biHeight;
	img_info->img_bpp  = ihdr->biBitCount;
	img_info->img_size = ihdr->biSizeImage;
	img_info->img_pData = pDataBuf;
	
	Debug("sizeof(BInfoHeader) = %d.\n", sizeof(BInfoHeader));
	Debug("iheader.biWidth = %ld, iheader.biHeight = %ld, iheader.biSizeImage = %ld.\n", ihdr->biWidth, ihdr->biHeight, ihdr->biSizeImage);
	Debug("bpp = %d.\n", img_info->img_bpp);

	/* Move file pointer to the position of valid data */
	retval = lseek(fd, fhdr->bfOffBits, SEEK_SET);		
	if (fhdr->bfOffBits != retval)
	{
		fprintf(stderr, "lseek to valid data failed.\n");
		goto lseekerr;
	}
	
	/* Read out valid data */
	retval = read(fd, pDataBuf, ihdr->biSizeImage);		
	if (retval < 0)
	{
		fprintf(stderr, "read data error.\n");
		goto rdataerr;
	} 
	free(fhdr);
	free(ihdr);
	close(fd);
	return NO_ERROR;
	
rdataerr:
readerr:
lseekerr:
	close(fd);
openerr:
	free(pDataBuf);
	free(fhdr);
	free(ihdr);
	return COMMON_ERROR;
}


/*
 * Function Name:draw_bmp_picture
 * Description:To draw bmp format picture on screen
 * Input:struct ImgInfo
 * Output:None
 * Return:None 
 * Author:Alion
 * Date:2018.6.3
 */

static void draw_bmp_picture(struct ImgInfo *img_info)
{
	__draw_bmp_picture(img_info->img_pData, img_info->img_xres, img_info->img_yres,  img_info->img_startX,  img_info->img_startX);
}



/*
 * Function Name:display_bmp
 * Description:To display bmp format picture on screen
 * Input:struct ImgInfo
 * Output:None
 * Return:ERROR NUMBER 
 * Author:Alion
 * Date:2018.6.3
 */

int display_bmp(ImgInfo *img_info )
{
	int retval = -1;
	
	/* Step1: Check picture format */
	retval = is_bmp(img_info->pathname);
	if (retval != 0)
	{
		return COMMON_ERROR;
	}

	/* Step2:Decode picture and get it's valid data */
	retval = bmp_analysis(img_info);
	if (retval < 0)
	{
		return COMMON_ERROR;
	}
	
	/* Step3:Zoom picture */
	img_dst->img_xres = vscinfo->xres;
	img_dst->img_yres = vscinfo->yres;
	img_dst->img_bpp = img_info->img_bpp;
	
	do_Stretch_Linear(img_dst, img_info);
	Debug("here is do_Stretch_Linear end.\n");
	
	/* Step4:Display picture */
	draw_bmp_picture(img_dst);
	
	free(img_info->img_pData);
	return NO_ERROR;
}

