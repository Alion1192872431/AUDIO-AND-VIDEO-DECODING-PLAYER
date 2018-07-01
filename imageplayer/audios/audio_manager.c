/******************************************************************************
 * File Name:Audio.c
 * Description:This file is related to manage audio files
 * Author:Alion
 * Date:2018.6.3
 *****************************************************************************/
 
/* Header Files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <audio.h>

/* Debug Macro */
#define DEBUG
#ifdef DEBUG
#define Debug(fmt,...) printf("FILE: "__FILE__", LINE: %d: "fmt"", \
                        __LINE__, ##__VA_ARGS__)
#else
#define Debug(fmt,...)
#endif


/* Global Variables */
extern struct fb_var_screeninfo *vscinfo;
struct audiolist *pHeader =	NULL;
struct audiolist *pCur = NULL;
struct audiolist *pPrev = NULL;
struct audiolist *pNext = NULL; 
struct input_event ev; 		
pthread_mutex_t mutex;






/*
 * Function Name:create_node
 * Description:Create node for new song
 * Input:music file pathname
 * Output:None
 * Return:Pointer to a new node 
 * Author:Alion
 * Date:2018.6.3
 */
static struct audiolist *create_node(const char *pathname)
{	
	struct audiolist *ptr = (struct audiolist *)malloc(sizeof(struct audiolist));
	if (NULL == ptr)
	{
		printf("malloc error.\n");
		return NULL;
	}
	
	strcpy(ptr->music.pathname, pathname);
	ptr->music.type = MP3;
	ptr->pPrev = NULL;			
	ptr->pNext = NULL;
	
	return ptr;
}


/*
 * Function Name:insert_tail
 * Description:Insert node to list tail
 * Input:Pointer to list, pointer to new node
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
static void insert_tail(struct audiolist *pH, struct audiolist *new)
{
	/* Step1: Go to the last node of the list */
	struct audiolist *p = pH;
	while (NULL != p->pNext)
	{
		p = p->pNext;			
	}
	
	/* Step 2: Insert the new node behind the original tail node */
	p->pNext = new;				
	new->pPrev = p;											
}

/*
 * Function Name:insert_head
 * Description:Insert node to list head
 * Input:Pointer to list, pointer to new node
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
static void insert_head(struct audiolist *pH, struct audiolist *new)
{
	new->pNext = pH->pNext;			
	
	if (NULL != pH->pNext)
	{		
		pH->pNext->pPrev = new;
	}
	pH->pNext = new;				
	new->pPrev = pH;				
}

/*
 * Function Name:traverse_backward
 * Description:Traverse a linked list backward
 * Input:Pointer to list
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
static void traverse_backward(struct audiolist *pH)
{
	struct audiolist *p = pH;
	
	while (NULL != p->pNext)
	{	 
		p = p->pNext; 
		Debug("p = %p\n", p);
		printf("pathname = %s.\n", p->music.pathname);
		printf("\n\n\n");
	}
}


/*
 * Function Name:traverse_forward
 * Description:Traverse a linked list forward
 * Input:Pointer to list tail node
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
static void traverse_forward(struct audiolist *pTail)
{
	struct audiolist *p = pTail;
	
	while (NULL != p->pPrev)
	{
		printf("pathname = %s.\n", p->music.pathname);
		p = p->pPrev;
	}
}

 
/*
 * Function Name:delete_node
 * Description:Delete a node from a list
 * Input:Pointer to list, music file's pathname
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */
static int delete_node(struct audiolist *pH, const char *pathname)
{
	struct audiolist *p = pH;
	
	if (NULL == p)
	{
		return COMMON_ERROR;
	}
	while (NULL != p->pNext)
	{
		p = p->pNext;
		
		/* whether the current node is the one need to be deleted or not */
		if (p->music.pathname == pathname)
		{
			if (NULL == p->pNext)	/* Current node is tail node */
			{
				p->pPrev->pNext = NULL;
				//p->pPrev = NULL;			
				// Ïú»Ùp½Úµã
				//free(p);
			}
			else
			{ 
				p->pPrev->pNext = p->pNext;										
				p->pNext->pPrev = p->pPrev;		
				//free(p);
			}
			free(p);
			return NO_ERROR;
		}
	}
	fprintf(stderr, "cannot get the target node.\n");
	return COMMON_ERROR;
}
 


/*
 * Function Name:scan_audio
 * Description:Recursively find all audio files under a specified folder
 * Input:The specified folder's pathname
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */
int scan_audio(const char *baseDir)
{	
	DIR *dir = NULL;
	struct dirent *ptr = NULL;
	struct stat buf = {0};
	char base[1000] = {0};
	struct audiolist *new = NULL;
	
	/* The list pointer is used to point to an empty node */
	/* because the function about the list operation  consider the first empty node */
	pHeader = create_node("NULL");
	
	if ((dir=opendir(baseDir)) == NULL)
	{	 
		fprintf(stderr, "Open %s error....\n", baseDir);
		return OPEN_ERROR;
	} 
	while ((ptr=readdir(dir)) != NULL)
	{		
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0) /* The file read out is current director or parent director */
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
		
			/* Whether the file read out is mp3 format or not */
			if (!is_mp3(base))
			{ 
				new = create_node(base);
				insert_tail(pHeader, new);	/* Put this mp3 file to list */
			} 
		}
		else if (S_ISDIR(buf.st_mode))		/* director */
		{
			Debug("this file is a directoy.\n");
			scan_audio(base);
		}
		else
		{
			fprintf(stderr, "this file is not wanted.\n");
			return COMMON_ERROR;
		} 
	}
	closedir(dir);
	return 0;
}

/*
 * Function Name:print_audios
 * Description:Print all audio files in list
 * Input:Pointer to list
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */
void print_audios(struct audiolist *pH)
{	
	traverse_backward(pH); 
}


/*
 * Function Name:audio_sequence
 * Description:Play music in order
 * Input:Pointer to list
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */
int audio_sequence(struct audiolist *pH)
{ 
	struct audiolist *p = pH;
	
	while (NULL != p->pNext)
	{
		p = p->pNext;
		printf("pathname = %s.\n", p->music.pathname);

		if (p->music.type == MP3)
		{
			if (play_mp3(p->music.pathname) < 0)
			{
				fprintf(stderr, "play music %s failed.\n", p->music.pathname);
				return COMMON_ERROR;
			}
			sleep(1);
		}	
	}
	return NO_ERROR;
}


/*
 * Function Name:audio_loop
 * Description:Loop play music
 * Input:Pointer to list
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */

int audio_loop(struct audiolist *pH)
{ 	
	while(1)
	{
		struct audiolist *p = pH;
		
		while (NULL != p->pNext)
		{
			p = p->pNext;
			printf("pathname = %s.\n", p->music.pathname);

			if (p->music.type == MP3)
			{	
				if (play_mp3(p->music.pathname) < 0)
				{
					fprintf(stderr, "play music %s failed.\n", p->music.pathname);
					return COMMON_ERROR;
				}
				sleep(1);
			}	
		}
	}
	return NO_ERROR;
}

/*
 * Function Name:play_audio
 * Description:Play music
 * Input:music pathname
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */
int play_audio(const char *pathname)
{
	printf("play music %s.\n", pathname);
	if (play_mp3(pathname) < 0)
	{
		fprintf(stderr, "play music %s error.\n", pathname);
		return COMMON_ERROR;
	}
	return NO_ERROR;
}


