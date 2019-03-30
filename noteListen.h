#ifndef _NOTELISTEN_H_
#define _NOTELISTEN_H_

//#include "linkedlist.h"

#define PCM_DEVICE "default"
#define KDB4 26
#define KDB0 19 //A0
#define KDB1 13 //A1
#define KDB2 6 //A2
#define KDB3 5
#define KF0 2
#define KS0 3
#define KF1 4
#define KS1 17
#define KF2 27
#define KS2 22
#define KF3 20
#define KS3 21

#define MAX_POLYPHONY 20
#define VOLUME 2

snd_pcm_t *pcm_handle;                            //Handle for PCM device to be added to each function
snd_pcm_hw_params_t *params;                      //Handle for PCM parameters
snd_pcm_uframes_t frames; 

struct keyInfo
{
    char *filename[5];
		unsigned long start, velocity;
    int column, chip, select, colPinF, colPinS;
    int keyState, keyHist;
    FILE *stream[5];
    float sustain;
};
struct keyInfo keys[88];
extern char *key[4][3][8];

int loadNames(char *path);
int readNames();
int readFmt(unsigned long *fileSize, unsigned long *sampleRate, unsigned short *nChannels, unsigned short *bitsPerSample);
int initPCM(unsigned int nChannels, unsigned int rate, unsigned int pcm);
int bufWrite(char *buff);
int mixer(FILE *files[4][3][8], int buff_size, char *out);
int scanKeys();
int initKey(int rdF, int rdS, int keyNum);
int velocityCall(int rdF, int rdS, int keyNum);

#endif
