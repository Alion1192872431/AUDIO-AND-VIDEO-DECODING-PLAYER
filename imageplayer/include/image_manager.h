#ifndef __IMAGE_MANAGER_H__
#define __IMAGE_MANAGER_H__

#include <base.h>
/***********************************Macro Definition***************************************/
#define MAX_NAME_LENTH			(256)
#define MAX_IMG_CNT		(100)

/***********************************Macro***************************************/
#if 0
typedef enum img_type {

	IMG_TYPE_BMP,
	IMG_TYPE_JPG,
	IMG_TYPE_PNG,
	IMG_TYPE_UNKNOWN,
	
}img_type_e;

typedef struct img_info {

	char pathname[MAX_NAME_LENTH];
	
	img_type_e type;
	
}img_info_t;

#endif
/*************************************Function Declaration**********************************/

int scan_image(const char *baseDir);
int scan_image2(const char *path);
void print_images(void);

void display_image();

void show_image(int index);
int ts_updown(void);

#endif



