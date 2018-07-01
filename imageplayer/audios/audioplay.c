/******************************************************************************
 * File Name:Audio.c
 * Description:This file is related to operate audio equipment
 * The implemented function is to play background music when the picture is displayed
 * Use libmad library to decode mp3 format audio files and then play
 * Author:Alion
 * Date:2018.6.3
 *****************************************************************************/
 
/* Header Files */
#include <stdio.h>
#include <audio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

/* Debug Macro */
#define DEBUG
#ifdef DEBUG
#define Debug(fmt,...) printf("FILE: "__FILE__", LINE: %d: "fmt"", \
                        __LINE__, ##__VA_ARGS__)
#else
#define Debug(fmt,...)
#endif


/* Macro Definition */
#define FILE		"audios/music"
#define PATH		"audios/music/"

/* Global Variables */
extern struct audiolist *pHeader;
int shmid = -1;				/* ID of shared memory */
char *p_addr = NULL;		/* Adress ID of Shared memory in mapped process */
pid_t grdcldpid = -1;		/* child process of audio playing process */
pid_t c_pid = -1;			/* flag of child */
int play_flag = -1;			/* playing flag:1 means playing 0 means stop playing */


/*
 * Function Name:play
 * Description:play music 
 * Input:music name
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */
int play(const char *music_name)
{
	struct audiolist *p = pHeader;
	//char pathname[50] = PATH;
	pid_t pid = -1; 
	char *cp_addr = NULL;	/* Shared memory mapped address identifier in the current process */
	struct audiolist *current_music = NULL; 
	
	Debug("pathname = %s\n\n", music_name);
	
	/* Search node with file name */ 
	while(p->pNext)
	{
		p = p->pNext;
		if (!strcmp(p->music.pathname, music_name))
		{
			current_music = p;
			break;
		}
	}
	Debug("current_music = %p\n", current_music);
	Debug("current_music->pNext = %p\n", current_music->pNext);
	Debug("current_music->pPrev = %p\n", current_music->pPrev);
 
	while(current_music)
	{
		pid = fork();
	
		if (pid < 0)
		{
			fprintf(stderr, "fork child process error.\n");
			exit(FORK_ERROR);
		} 
		else if (pid == 0)
		{
			Debug("current process is child process.\n");
			/* play music */
			printf("Current playing music is %s.\n", current_music->music.pathname);
			
			if (play_mp3(current_music->music.pathname) < 0)
			{
				fprintf(stderr, "Play music %s error.\n", current_music->music.pathname);
				return COMMON_ERROR;
			}
			exit(NO_ERROR);
		}
		else
		{
			Debug("current process is parent process.\n");
			
			/* Map shared memory to current process */
			cp_addr = shmat(shmid, NULL,0);
			
			/* Pass child pid and current music node to shared memory */
			memcpy(cp_addr, &pid, sizeof(pid_t));
			memcpy(cp_addr+sizeof(pid_t)+1, &current_music, sizeof(struct audiolist*));
			
			if (wait(NULL) == pid)
			{
				current_music = current_music->pNext;
				printf("The next music is %s.\n", current_music->music.pathname); 
			}
		}
	} 
	return NO_ERROR;	
}


/*
 * Function Name:startplay
 * Description:startup play music 
 * Input:music name
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
void startplay(const char *music_name)  
{   
    pid_t pid;    
	
    pid = fork();
  
    if(pid > 0)
	{ 
        c_pid = pid;  
        play_flag = 1;  
        sleep(1);  	/* sleep to run child process */
		
        /* Pass the child process pid of the music player process to the current parent process */  
        memcpy(&grdcldpid,p_addr,sizeof(pid_t)); 
    }  
    else if(0 == pid)
    {
        play(music_name);
		exit(NO_ERROR);
    }  
}  


/*********************************************************


/*
 * Function Name:mypause
 * Description:Pause the current playing music
 * Input:None
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
void mypause(void)  
{  
    printf("=======================PAUSE!PRESS K1 TO CONTINUE===================\n");  
    kill(grdcldpid,SIGSTOP); /* Send SKGSTOP signal to grandchild process */ 
    play_flag = 0;  
}  
  

/*
* Function Name:mypause
* Description:Stop the current playing music
* Input:None
* Output:None
* Return:None
* Author:Alion
* Date:2018.6.3
*/
void stop(void)  
{  
    printf("========================stop playing music =========================\n");   
    kill(c_pid,SIGKILL);   		/* Send SKGSTOP signal to child process */  
    kill(grdcldpid,SIGKILL); 	/* Send SKGSTOP signal to grandchild process */ 
    //first_key=1;  
}  
  

/*
 * Function Name:continue_play
 * Description:Continue playing music from pause
 * Input:None
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */
void continue_play(void)  
{  
    printf("===============================CONTINUE PLAYING=============================\n");  
    kill(grdcldpid,SIGCONT); 		/* Send SKGSTOP signal to grandchild process */
    play_flag=1;  
}  
  
/*
 * Function Name:next
 * Description:Play next music
 * Input:None
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */  
void next(void)  
{  
	char music_name[50] = {0}; 
    struct audiolist *current_music;  
  	struct audiolist *next_music = NULL;
	
    /* Get the node pointer to current song from the shared memory */  
    memcpy(&current_music,p_addr + sizeof(pid_t)+1,sizeof(struct audiolist *)); 
	Debug("NEXT current_music = %p\n", current_music);
	
    /* Point to the next song */  
    next_music = current_music->pNext; 
	if (NULL == next_music) /*If current music is the last one£¬warn it and replay the current music */
	{
		strncpy(music_name, current_music->music.pathname, sizeof(music_name));
		printf("Current music %s is the last one\n", music_name);
	}
	else
	{
		strncpy(music_name, next_music->music.pathname, sizeof(music_name));	
		printf("Next music is %s\n", music_name);
	} 
	
    /* Kill child and grandchild process to stop current playing */  
    kill(c_pid,SIGKILL);  
    kill(grdcldpid,SIGKILL);  
    wait(NULL);  
    startplay(music_name);  
}  
  
/*
 * Function Name:prev
 * Description:Play previous music
 * Input:None
 * Output:None
 * Return:None
 * Author:Alion
 * Date:2018.6.3
 */  
void prev(void)  
{  
	char music_name[50] = {0}; 
    struct audiolist *current_music;  
	struct audiolist *prev_music = NULL; 
	
    /* Get the node pointer to current song from the shared memory */   
    memcpy(&current_music,p_addr + sizeof(pid_t)+1,sizeof(struct audiolist *));
	Debug("PREV current_music = %p\n", current_music);
	
    /* Point to the previous song */  
    prev_music = current_music->pPrev; 
	Debug("prev_music->music.pathname is %s\n\n", prev_music->music.pathname);
	if (!strcmp("NULL", prev_music->music.pathname))/*If current music is the first one£¬warn it and replay the current music */
	{
		strncpy(music_name, current_music->music.pathname, sizeof(music_name));
		printf("Current music %s is the head one\n", music_name);
	}
	else
	{
		strncpy(music_name, prev_music->music.pathname, sizeof(music_name));
		printf("Previous music is %s************************\n", music_name);
	}

    /* Kill child and grandchild process to stop current playing */
	kill(c_pid,SIGKILL);
	kill(grdcldpid,SIGKILL);	/* Kill the child process and then kill the grandchild process, the order can not be disordered */
	wait(NULL);     
    startplay(music_name); 	
}  

  


/*
 * Function Name:get_key
 * Description:Respond to key
 * Input:None
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */  
int get_key(void)
{
	int fd = -1, retval = -1;
	struct input_event ev;
	
	/* Open key releated device file */
	fd = open(DEVICE_KEY, O_RDONLY);
	if (fd < 0)
	{
		fprintf(stderr, "open DEVICE_KEY error.\n");
		return OPEN_ERROR;
	}
	/* Loop reading key events */
	while(1)
	{
		memset(&ev, 0, sizeof(struct input_event));
		retval = read(fd, &ev, sizeof(struct input_event));
		if (retval != sizeof(struct input_event))
		{
			perror("read");
			close(fd);
			return READ_ERROR;
		}
		/* Parsing the event package so as to know what kind of input event occurred */
		printf("-------------------------\n");
		printf("type: %hd\n", ev.type);
		printf("code: %hd\n", ev.code);
		printf("value: %d\n", ev.value);
		printf("\n");  
		
		/* Parsing key information */
		if (ev.type == EV_KEY)
		{ 
			if ((ev.code == KEY_LEFT) && (ev.value == 0))
			{
				Debug("====================== prev =========================\n\n\n");
				/* switch to the previous song */
				prev();
			}
			if ((ev.code == KEY_RIGHT) && (ev.value == 0))
			{
				Debug("====================== next =========================\n\n\n");
				/* switch to the next song */
				next();
			}
			if ((ev.code == KEY_DOWN) && (ev.value == 0))
			{
				Debug("====================== mypause =========================\n\n\n");
				/* pause current playing song */
				mypause();
			}
			if ((ev.code == KEY_UP) && (ev.value == 0))
			{
				Debug("====================== continue_play =========================\n\n\n");
				/* Resume suspended songs */ 
				continue_play();
			}
			if ((ev.code == KEY_MENU) && (ev.value == 0))
			{
				Debug("====================== stop =========================\n\n\n");
				/* stop current playing song */
				stop(); 
			}
			if ((ev.code == KEY_BACK) && (ev.value == 0))
			{
				
			}
		} 
	}
	close(fd);
	return NO_ERROR;
}

/*
 * Function Name:main
 * Description:The entry of the process 
 * Input:argc---the quantity of arguments  argv---pointer to the value array of arguments
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.5.25
 */
int main(int argc, char **argv)  
{   
	char pathname[50] = PATH; 
	scan_audio(FILE);
	print_audios(pHeader);
	
  	/* Request shared memory: used to store child process ID, node pointer to current playing music */  
    if((shmid = shmget(IPC_PRIVATE,sizeof(struct audiolist*)+ 2, S_IRUSR|S_IWUSR))== -1)
	{
		exit(1);
	}
    /* Map the shared memory to current process */      
    p_addr = shmat(shmid,0,0);
	
	/* Clear shared memory */
    memset(p_addr,0,1024); 
 
	if (argc == 1)
	{
		strcat(pathname, "xiaoyongyuan.mp3");
		startplay(pathname);
	}
	else if (argc == 2)
	{
		strcat(pathname, argv[1]);
		startplay(pathname);
	}
	
	get_key();

    return NO_ERROR;  
} 
 













