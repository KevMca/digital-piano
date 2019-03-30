#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pigpio.h>
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include "noteListen.h"

char *key[4][3][8] =
    {
        {
        {"A0", "C#1", "F1", "A1", "C#2", "F2", "A3", "C#3"},
        {"F3", "A3", "C#4", "F4", "A4", "C#5", "F5", "A5"} ,
        {"C#6", "F6", "A6", "C#7", "F7", "A7", "", ""}
        },
        {
        {"A#0", "D1", "F#1", "A#1", "D2", "F#2", "A#2", "D3"},
        {"F#3", "A#3", "D4", "F#4", "A#4", "D5", "F#5", "A#5"} ,
        {"D6", "F#6", "A#6", "D7", "F#7", "A#7", "", ""}
        },
        {
        {"B0", "D#1", "G1", "B1", "D#2", "G2", "B2", "D#3"},
        {"G3", "B3", "D#4", "G4", "B4", "D#5", "G5", "B5"} ,
        {"D#6", "G6", "B6", "D#7", "G7", "B7", "", ""}
        },
        {
        {"C1", "E1", "G#1", "C2", "E2", "G#2", "C3", "E3"},
        {"G#3", "C4", "E4", "G#4", "C5", "E5", "G#5", "C6"} ,
        {"E6", "G#6", "C7", "E7", "G#7", "C8", "", ""}
        },
    };
char *path = "/home/pi/Samples/";
struct keyInfo keys[88];

unsigned int pcm;
int buff_size, i, j, k, l;

void keySetup()
{
		int column = 0;
		int chip = 0;
		int select = 0;
		for(i = 0; i < 88; i++){
				keys[i].select = select;			//Assign select pin
				keys[i].chip = chip;					//Assign chip number
				keys[i].column = column;			//Assign column
				select++;

				if(chip == 2 && select == 6){ //No key corresponding to select>6 when chip=2
						select = 8; 							//, so skip ahead
				}
				if(select == 8){ 							//If cycled through all select pins
						chip++;
						select = 0; 
				}
				if(chip == 3){								//If cycled through all chips
						column++;
						chip = 0;
				}

				if(column == 0){
						keys[i].colPinF = KF0;
						keys[i].colPinS = KS0;}
				if(column == 1){
            keys[i].colPinF = KF1;
            keys[i].colPinS = KS1;}
				if(column == 2){
            keys[i].colPinF = KF2;
            keys[i].colPinS = KS2;}
				if(column == 3){
            keys[i].colPinF = KF3;
            keys[i].colPinS = KS3;}

				keys[i].keyState = 0;					//Intialise keyState and sustain
				keys[i].keyHist = 0;
				keys[i].sustain = 0;
				keys[i].start = 0;
				keys[i].velocity = 0;
		}
}

void pinSetup()
{
    gpioInitialise();

    //Set mode of the GPIO
    gpioSetMode(KDB4, PI_OUTPUT);
    gpioSetMode(KDB0, PI_OUTPUT);
    gpioSetMode(KDB1, PI_OUTPUT);
    gpioSetMode(KDB2, PI_OUTPUT);
    gpioSetMode(KDB3, PI_OUTPUT);
    gpioSetMode(KF0, PI_INPUT);
    gpioSetMode(KS0, PI_INPUT);
    gpioSetMode(KF1, PI_INPUT);
    gpioSetMode(KS1, PI_INPUT);
    gpioSetMode(KF2, PI_INPUT);
    gpioSetMode(KS2, PI_INPUT);
    gpioSetMode(KF3, PI_INPUT);
    gpioSetMode(KS3, PI_INPUT);

    gpioSetPullUpDown(KF0, PI_PUD_UP);
    gpioSetPullUpDown(KS0, PI_PUD_UP);
    gpioSetPullUpDown(KF1, PI_PUD_UP);
    gpioSetPullUpDown(KS1, PI_PUD_UP);
    gpioSetPullUpDown(KF2, PI_PUD_UP);
    gpioSetPullUpDown(KS2, PI_PUD_UP);
    gpioSetPullUpDown(KF3, PI_PUD_UP);
    gpioSetPullUpDown(KS3, PI_PUD_UP);

		gpioGlitchFilter(KF0, 100);
		gpioGlitchFilter(KS0, 500);
		//gpioSetISRFunc(KF0, FALLING_EDGE)
}


void main()
{
		unsigned long fileSize, sampleRate;
		unsigned short nChannels, bitsPerSample;
		char *out;				//output buffer
		FILE *files[4][3][8];	//output files
		int mix;
		int write = 0;					//used to save number of keys to mix

		struct timeval now, pulse; 																	//For timing

		pinSetup();
		keySetup();	
		loadNames(path); 																						//Adjust path above to change folder
		readNames();
		readFmt(&fileSize, &sampleRate, &nChannels, &bitsPerSample);
		initPCM((unsigned int)(nChannels), (unsigned int)(sampleRate), pcm);

		buff_size = (frames * nChannels * 2);                       //Calculate buffer size
		out = (char *) malloc(buff_size);

		/*for(i = 0; i < 88; i++){
				buffs[i] = (char *) malloc(buff_size);                  //Assign a temporary buffer for the file
    		shortBuffs[i] = (signed long *) malloc(buff_size*4);
		}*/

		while(1)
		{
				gettimeofday(&now, NULL);
				// Scan the keyboard
				scanKeys();

				if(write == 0){			
						// Mix files into one output buffer 
						mix = mixer(files, buff_size, out);
				}
				if(mix > 0){
						write = bufWrite(out);																			//Write buffer
				} else {
						memset(out, 0, buff_size);													//Set buff to 0 if nothing played
						write = bufWrite(out);
				}
				gettimeofday(&pulse, NULL);
				//printf("Time: %d\n", pulse.tv_usec-now.tv_usec);

		}
	
		snd_pcm_drain(pcm_handle);
  	snd_pcm_close(pcm_handle);
		free(out);
}
