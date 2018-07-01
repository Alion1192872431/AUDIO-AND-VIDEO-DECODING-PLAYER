/******************************************************************************
 * File Name:main.c
 * Description: This file is the entry of the whole program
 * Author:Alion
 * Date:2018.6.3
 *****************************************************************************/
/* Header Files */
#include <stdio.h> 
#include <base.h>


/* Debug Macro */
#define DEBUG
#ifdef DEBUG
#define Debug(fmt,...) printf("FILE: "__FILE__", LINE: %d: "fmt"", \
                        __LINE__, ##__VA_ARGS__)
#else
#define Debug(fmt,...)
#endif

#if 0
/*参数为：
 *返回图片的宽度(w_Dest),
 *返回图片的高度(h_Dest),
 *返回图片的位深(bit_depth),
 *源图片的RGB数据(src),
 *源图片的宽度(w_Src),
 *源图片的高度(h_Src)
 */

//unsigned char* do_Stretch_Linear(int w_Dest,int h_Dest,int bit_depth,unsigned char *src,int w_Src,int h_Src)
#endif
/*
 * Function Name:do_Stretch_Linear
 * Description:This function is designed to zoom picture to specified resolution
 * Input:ImgInfo *dst, const ImgInfo *src
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.5.25
 */
int do_Stretch_Linear(ImgInfo *dst, const ImgInfo *src)
{	
	Debug("here is do_Stretch_Linear inner top.\n");
	
	int w_Dest = dst->img_xres;
	int h_Dest = dst->img_yres;
	int bit_depth = dst->img_bpp;
	int w_Src = src->img_xres;
	int h_Src = src->img_yres;
	unsigned char *pSrc = src->img_pData;
	
	int sw = w_Src-1, sh = h_Src-1, dw = w_Dest-1, dh = h_Dest-1;
	int B, N, x, y, i, j, k;
	int nPixelSize = bit_depth/8;
	unsigned char *pLinePrev,*pLineNext;
	unsigned char buf[w_Dest*h_Dest*bit_depth/8];
	unsigned char *pDest = buf; 
	unsigned char *tmp;
	unsigned char *pA,*pB,*pC,*pD;

	dst->img_pData = pDest;
	Debug("here is do_Stretch_Linear inner mid.\n");
	for(i=0;i<=dh;++i)
	{
		tmp =pDest + i*w_Dest*nPixelSize;
		y = i*sh/dh;
		N = dh - i*sh%dh;
		pLinePrev = pSrc + (y++)*w_Src*nPixelSize;
		//pLinePrev =(unsigned char *)aSrc->m_bitBuf+((y++)*aSrc->m_width*nPixelSize);
		pLineNext = (N==dh) ? pLinePrev : pSrc+y*w_Src*nPixelSize;
		//pLineNext = ( N == dh ) ? pLinePrev : (unsigned char *)aSrc->m_bitBuf+(y*aSrc->m_width*nPixelSize);
		for(j=0;j<=dw;++j)
		{
			x = j*sw/dw*nPixelSize;
			B = dw-j*sw%dw;
			pA = pLinePrev+x;
			pB = pA+nPixelSize;
			pC = pLineNext + x;
			pD = pC + nPixelSize;
			if(B == dw)
			{
				pB=pA;
				pD=pC;
			}
			for(k=0;k<nPixelSize;++k)
			{
				*tmp++ = ( unsigned char )( int )(
					( B * N * ( *pA++ - *pB - *pC + *pD ) + dw * N * *pB++
					+ dh * B * *pC++ + ( dw * dh - dh * B - dw * N ) * *pD++
					+ dw * dh / 2 ) / ( dw * dh ) );
			}
		}
	}
	Debug("here is do_Stretch_Linear inner botm.\n");
	return NO_ERROR;
}



