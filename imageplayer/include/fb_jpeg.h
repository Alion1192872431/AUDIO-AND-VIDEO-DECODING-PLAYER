#ifndef __FB_JPEG_H__
#define __FB_JPEG_H__
/**********Header Flies***********/
#include <base.h>
#include <jpeglib.h>
#include <jerror.h>
#include <setjmp.h>
#include <framebuffer.h>

struct my_error_mgr
{
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};
typedef struct my_error_mgr * my_error_ptr;

int is_jpg(const char *pathname);

int display_jpg(ImgInfo *img_info);

#endif

