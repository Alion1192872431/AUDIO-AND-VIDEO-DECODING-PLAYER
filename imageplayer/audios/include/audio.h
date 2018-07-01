#ifndef __AUDIO_H__
#define __AUDIO_H__

/* Header Files */
#include <base.h>

/* Macro Definition */
#define MP3		1
#define BUFSIZE		8192
#define	DEVNAME		/dev/dsp
#define	DEVICE_KEY	"/dev/input/event0"

/*******************************Key****************************************/
#define EV_KEY			1
#define KEY_LEFT		105
#define KEY_RIGHT		106
#define KEY_UP			103
#define KEY_DOWN		108
#define KEY_MENU		30
#define KEY_BACK		48
/*****************************************************************************/

/* offsetof */
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/* container_of */
#define container_of(ptr, type, member) ({ \
     const typeof( ((type *)0)->member ) *__mptr = (ptr); \
     (type *)( (char *)__mptr - offsetof(type,member) );})

/****************************************************************************/
typedef struct buffer 
{
	FILE  *fp;                    /*file pointer*/ 
	unsigned int  flen;           /*file length*/ 
  	unsigned int  fpos;           /*current position*/
	unsigned char fbuf[BUFSIZE];  /*buffer*/ 
  	unsigned int  fbsize;         /*indeed size of buffer*/
}mp3_file;

typedef struct music_info
{
	char pathname[50];
	int type;
}musicInfo;

/* List to manage all the audio files */ 
struct audiolist 
{
	musicInfo music;
	struct audiolist *pPrev;
	struct audiolist *pNext;
};



/****************************************Function Declaration**************************************/
int is_mp3(const char *pathname);

int play_mp3(const char *pathname);
int scan_audio(const char *baseDir);
void print_audios(struct audiolist *pH);
int audio_sequence(struct audiolist *pH);
int audio_loop(struct audiolist *pH); 

#endif



























