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
#include <sys/types.h>  
#include <sys/stat.h>  
#include <stdlib.h> 
#include <string.h>
#include <fcntl.h>  
#include <sys/mman.h>
#include <sys/soundcard.h>  
#include <unistd.h> 
#include <sys/ioctl.h>
#include <mad.h>
#include <audio.h>

/* Debug Macro */
//#define DEBUG
#ifdef DEBUG
#define Debug(fmt,...) printf("FILE: "__FILE__", LINE: %d: "fmt"", \
                        __LINE__, ##__VA_ARGS__)
#else
#define Debug(fmt,...)
#endif


/******************************** Loop play control **************************/
//#define LOOPSWITCH_ON
#ifdef LOOPSWITCH_ON
#define LOOPSWITCH_START 	while(1) {
#define LOOPSWITCH_END		}
#else 
#define LOOPSWITCH_START	
#define LOOPSWITCH_END 		
#endif 
/******************************************************************************/

/* Global Variables */ 
static int soundfd;                 /*soundcard file*/ 
static unsigned int prerate = 0;    /*the pre sample rate*/ 
static int channels = 0;


/*
 * Function Name:writedsp
 * Description:write data into soundcard 
 * Input:data need to be written in.
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */

static int writedsp(int c) 
{ 
    return write(soundfd, (char *)&c, 1); 
}



/*
 * Function Name:set_dsp
 * Description:Configure the basic arguments of soundcard 
 * Input:None
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.5.25
 */
static int set_dsp(void) 
{   
   	int format = AFMT_S16_LE;
	
    /* Open soundcard file ,readonly*/ 
	soundfd =open("/dev/dsp",O_WRONLY);
	if(soundfd < 0 ) 
	{
		perror("openerror\n");  
		return OPEN_ERROR;
	} 
	
	/* Set soundcard reading format */ 
	if(ioctl(soundfd, SNDCTL_DSP_SETFMT, &format) < 0) 
	{
		perror("SNDCTL_DSP_SETFMT");  
		return IOCTRL_ERROR;  
	}
	
	/* Set channel for soundcard  */
	if(ioctl(soundfd,SNDCTL_DSP_CHANNELS,&channels) < 0) 
	{  
		perror("SNDCTL_DSP_CHANNELS");  
		return IOCTRL_ERROR;  
	}
	
	/* Set play speed for soundcard */    
	if(ioctl(soundfd, SNDCTL_DSP_SPEED, &prerate) < 0)
	{  
		 perror("SNDCTL_DSP_SPEED");  
		 return IOCTRL_ERROR;  
	} 
	
	return NO_ERROR;
}

/*   
 * This is perhaps the simplest example use of the MAD high-level API.   
 * Standard input is mapped into memory via mmap(), then the high-level API   
 * is invoked with three callbacks: input, output, and error. The output   
 * callback converts MAD's high-resolution PCM samples to 16 bits, then   
 * writes them to standard output in little-endian, stereo-interleaved
 * format. 
 */
static enum mad_flow input(void *data, struct mad_stream *stream) 
{
	mp3_file *mp3fp;   
	int ret_code;
	int unproc_data_size; /*the unprocessed data's size*/   
	int copy_size;
	mp3fp = (mp3_file *)data;
	if(mp3fp->fpos < mp3fp->flen) {
		unproc_data_size = stream->bufend - stream->next_frame;       
		memcpy(mp3fp->fbuf, mp3fp->fbuf+mp3fp->fbsize-unproc_data_size, unproc_data_size);
		copy_size = BUFSIZE - unproc_data_size; 
      	if(mp3fp->fpos + copy_size > mp3fp->flen) {
			copy_size = mp3fp->flen - mp3fp->fpos;
		} 
      	fread(mp3fp->fbuf+unproc_data_size, 1, copy_size, mp3fp->fp); 
      	mp3fp->fbsize = unproc_data_size + copy_size;       
		mp3fp->fpos  += copy_size; 
		/*Hand off the buffer to the mp3 input stream*/ 
      	mad_stream_buffer(stream, mp3fp->fbuf, mp3fp->fbsize);       
		ret_code = MAD_FLOW_CONTINUE;  
	}else { 
      	ret_code = MAD_FLOW_STOP;   
	}
	return ret_code; 
}

static inline signed int scale(mad_fixed_t sample) 
{ 
	
  	/* round */
  	sample += (1L << (MAD_F_FRACBITS - 16));

  	/* clip */
  	if (sample >= MAD_F_ONE)
    	sample = MAD_F_ONE - 1;
  	else if (sample < -MAD_F_ONE)
    	sample = -MAD_F_ONE;

  	/* quantize */
  	return sample >> (MAD_F_FRACBITS + 1 - 16); 
}
 
static enum mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm) 
{ 
	unsigned int nchannels, nsamples; 
  	unsigned int rate; 
  	mad_fixed_t const *left_ch, *right_ch;  
  	/* pcm->samplerate contains the sampling frequency */

	rate	  = pcm->samplerate;   
	nchannels = pcm->channels;   
	nsamples  = pcm->length;   
	left_ch   = pcm->samples[0]; 
  	right_ch  = pcm->samples[1];
	
  	/* update the sample rate of soundcard */   
	if(rate != prerate) {
		if (ioctl(soundfd, SNDCTL_DSP_SPEED, &rate) < 0) {
			perror("SNDCTL_DSP_SPEED");
			return -1;
		}        
		prerate = rate;   
	}
	//Debug("channels = %d.\n", nchannels);
	/* update channel of soundcard */
	if (nchannels != channels) {  
		if(ioctl(soundfd, SNDCTL_DSP_CHANNELS, &nchannels) < 0) {  
			perror("SNDCTL_DSP_CHANNELS");  
			return -1;  
		}
		channels = nchannels;
	}
	
	while (nsamples--) { 
    	signed int sample;  
    	/* output sample(s) in 16-bit signed little-endian PCM */  
    	sample = scale(*left_ch++);     
		writedsp((sample >> 0) & 0xff);     
		writedsp((sample >> 8) & 0xff);
		if (nchannels == 2) {
      		sample = scale(*right_ch++);       
			writedsp((sample >> 0) & 0xff);       
			writedsp((sample >> 8) & 0xff);     
		}    
	} 
	return MAD_FLOW_CONTINUE;
}
 
static enum mad_flow error(void *data, struct mad_stream *stream, struct mad_frame *frame) 
{ 
	mp3_file *mp3fp = data;
	fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n", stream->error, mad_stream_errorstr(stream), stream->this_frame - mp3fp->fbuf);  
  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */  
  	return MAD_FLOW_CONTINUE; 
}
 

static int decode(mp3_file *mp3fp) 
{	Debug("here is decode start.\n");
	struct mad_decoder decoder;   
	int result;  
  	/* configure input, output, and error functions */ 
	Debug("here is mad_decoder_init start.\n");
	mad_decoder_init(&decoder, mp3fp, input, 0 /* header */, 0 /* filter */, output, error, 0 /* message */);  
  	/* start decoding */
	Debug("here is mad_decoder_run start.\n");
  	result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);  
  	/* release the decoder */ 
	Debug("here is mad_decoder_finish start.\n");
	mad_decoder_finish(&decoder);
	return result; 
}


/*
 * Function Name:is_mp3
 * Description:If an audio file is an mp3 format file or not.
 * Note that the judgment method used here is not particularly rigorous 
 * Input:audio file's pathname
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */
int is_mp3(const char *pathname)
{
	int fd = -1;
	int retval = -1;
	unsigned char bmp_buf[3] = {0};

	fd = open(pathname, O_RDONLY);					
	if (fd < 0) 
	{
		fprintf(stderr, "open FILE %s failed.\n", pathname);
		return OPEN_ERROR;
	}
	
	retval = read(fd, bmp_buf, sizeof(bmp_buf));		/* read content */
	if (retval < 0)
	{
		fprintf(stderr, "read FILE %s type failed.\n", pathname);
		goto err;
	}

	/* Whether this audio file is format mp3 or not */
	if (!((bmp_buf[0] == 'I') &&(bmp_buf[1] == 'D') && (bmp_buf[2] == '3')))
	{	
		fprintf(stderr, "this file is not a MP3 file.\n");
		goto err;
	}
	close(fd);
	return NO_ERROR;
	
err:
	close(fd);
	return JUDGE_ERROR;
}

 
 /*
 * Function Name:play_mp3
 * Description:play mp3 audio file
 * Input:audio file's pathname
 * Output:None
 * Return:ERROR NUMBER
 * Author:Alion
 * Date:2018.6.3
 */
int play_mp3(const char *pathname) 
{
 	long flen, fsta, fend;    
	mp3_file *mp3fp = NULL;

	LOOPSWITCH_START 
	mp3fp = (mp3_file *)malloc(sizeof(mp3_file));
	if(NULL == mp3fp)
	{
		fprintf(stderr, "malloc for mp3_file failed.\n");
		return MALLOC_ERROR;
	}
	
	if((mp3fp->fp = fopen(pathname, "r+")) == NULL)
	{ 
		fprintf(stderr, "can't open source file.\n");
		goto fopenerr;   
	}
	
	Debug("here is fopen file end.\n");
	fsta = ftell(mp3fp->fp);
	fseek(mp3fp->fp, 0, SEEK_END);   
	fend = ftell(mp3fp->fp);	
	flen = fend - fsta;
	fseek(mp3fp->fp, 0, SEEK_SET);
	
	fread(mp3fp->fbuf, 1, BUFSIZE, mp3fp->fp);
	mp3fp->fbsize = BUFSIZE;
	mp3fp->fpos   = BUFSIZE;
	mp3fp->flen   = flen;

	/* Set soundcard */
	if (set_dsp() < 0)
	{
		fprintf(stderr, "set dsp error.\n");
		goto setdsperr;
	}
	Debug("here is set_dsp end.\n");

	/* Decode audio file */
	if (decode(mp3fp) < 0) 
	{
		fprintf(stderr, "decode error.\n");
		goto decoderr;
	}
	Debug("here is decode end.\n");
	close(soundfd);	 					/* Close soundcard */
	fclose(mp3fp->fp); 				 	/* Close audio file */
	free(mp3fp);
	
	LOOPSWITCH_END 
		
	return NO_ERROR;
	 
decoderr:
	close(soundfd);
setdsperr:
	fclose(mp3fp->fp);	 
fopenerr:
	free(mp3fp);
	return COMMON_ERROR;
} 		  





