#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pigpio.h>
#include <alsa/asoundlib.h>
#include "noteListen.h"

//char *filenames[4][3][8] = { "" };
//int keyPress[4][3][8] = { 0 };
//int keyPressHist[4][3][8] = { 0 };
//FILE *stream[4][3][8] = { 0 };
//float sustain[4][3][8] = { 0 };
struct keyInfo keys[88];
char *key[4][3][8];




/* Load wav file names into filenames
(< The path of the samples folder>)
Samples must be of the form A0v16.wav, where A is the key and 0 is the octave
*/
int loadNames(char *path)
{
		int i, j, column, chip, select;
		char prefix[50], postfix[50], filePath[50];


		for(i = 0; i < 88; i++){
				column = keys[i].column;
				chip = keys[i].chip;
				select = keys[i].select;
				prefix[0] = '\0';																		//Clear prefix and filePath
				filePath[0] = '\0';

				for(j = 0; j < 5; j++){
						strcpy(filePath, path);													//Use filePath to store path temporarily
						strcpy(prefix, key[column][chip][select]);			//Copy the key prefix <A0/A#1/B4...>
						sprintf(postfix, "v%d.wav", j+1);								//Assign "v <velocity> .wav for postfix"

						strcat(prefix, postfix);												//Append postfix to give name of file
						strcat(filePath, prefix);												//Append file name to path to get full path

						keys[i].filename[j] = malloc(strlen(filePath) + 1);//Assign space for the string
						strcpy(keys[i].filename[j], filePath);					//Save in filenames array
						printf("%s\n", keys[i].filename[j]);
				}
		}
		return(0);
}




/* Open each of the samples and save in stream

*/
int readNames()
{
		int i, j;

		printf("Opening sample files...\n");
		for(i = 0; i < 88; i++)
		{																															//Loop through keys
				for(j = 0; j < 5; j++){
						keys[i].stream[j] = fopen(keys[i].filename[j], "r");  //Open the file descriptor
						if(keys[i].stream[j] == NULL){												//Check if the file couldn't be opened
								printf("Couldn't read the file: %s", keys[i].filename[j]);
								return(-1);
						}
						fseek(keys[i].stream[j], 44, SEEK_SET);								//Skip the format header of the WAV file
				}
		}
		printf("Finished opening\n");
		return(0);
}




/* Read wave files
(<long to store the file size>
 <long to store the sample rate>
 <short to store the number of channels>
 <short to store bits per sample>)
Take a random file from stream and read the format of the samples. Reset the file pointer at the end. 
*/
int readFmt(unsigned long *fileSize, unsigned long *sampleRate,
              unsigned short *nChannels, unsigned short *bitsPerSample)
{
		char *buff_fmt;
    buff_fmt = (char *) malloc(44);                   				//Allocate memory for the WAV header

    printf("Reading format...\n");

    /* Read wav format */
    rewind(keys[0].stream[0]);
		fread(buff_fmt, 1, 4, keys[0].stream[0]);                 //Check if starts with "RIFF"
    if(*((unsigned long *)buff_fmt) != 0x46464952){
        printf("Err: file doesn't start with RIFF\n");
        return(-1);}

    fread(buff_fmt, 1, 4, keys[0].stream[0]);                 //File size minus 8 bytes for RIFF and these bytes
    *fileSize = *((unsigned long *)buff_fmt);
    printf("Filesize: %d\r\n", *fileSize);

    fread(buff_fmt, 1, 14, keys[0].stream[0]);                //Skip ahead to format bytes
    fread(buff_fmt, 1, 2, keys[0].stream[0]);                 //Number of channels
    *nChannels = *((unsigned short *)buff_fmt);
    printf("Channels: %d\r\n", *nChannels);

    fread(buff_fmt, 1, 4, keys[0].stream[0]);                 //Sample rate
    *sampleRate = *((unsigned long *)buff_fmt);
    printf("Sample rate: %d\r\n", *sampleRate);

    fread(buff_fmt, 1, 6, keys[0].stream[0]);                 //Bits per sample
    fread(buff_fmt, 1, 2, keys[0].stream[0]);
    *bitsPerSample = *((unsigned short *)buff_fmt);
    printf("Bits per sample: %d\r\n", *bitsPerSample);

    fread(buff_fmt, 1, 8, keys[0].stream[0]);                 //Skip last bits
    fseek(keys[0].stream[0], 44, SEEK_SET);										//Set read pointer back to beginning
		free(buff_fmt);

		return(0);
}




/* Initialise the PCM device
(<Number of channels: 1 for mono / 2 for stereo>
 <Sample rate in hertz>)
*/
int initPCM(unsigned int nChannels, unsigned int rate, unsigned int pcm)
{
		/* Open the PCM device in playback mode */
    if (pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE,SND_PCM_STREAM_PLAYBACK, 0) < 0)
    {
        printf("ERROR: Can't open \"%s\" PCM device. %s\n", PCM_DEVICE, snd_strerror(pcm));
    }

    /* Allocate parameters object and fill it with default values*/
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);

    /* Set parameters */
    if (pcm = snd_pcm_hw_params_set_access(pcm_handle, params,SND_PCM_ACCESS_RW_INTERLEAVED) < 0){      //Setup RW interleaved memory
        printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(pcm)); }
    if (pcm = snd_pcm_hw_params_set_format(pcm_handle, params,SND_PCM_FORMAT_S16_LE) < 0){              //Setup format as signed 16 bit little endian
        printf("ERROR: Can't set format. %s\n", snd_strerror(pcm)); }
    if (pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, nChannels) < 0){                       //Setup number of specified channels
        printf("ERROR: Can't set channels number. %s\n", snd_strerror(pcm)); }
    if (pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0) < 0){                       //Setup the specified bitrate
        printf("ERROR: Can't set rate. %s\n", snd_strerror(pcm)); }

    /* Write parameters */
    if (pcm = snd_pcm_hw_params(pcm_handle, params) < 0){
        printf("ERROR: Can't set hardware parameters. %s\n", snd_strerror(pcm)); }

    /* Print PCM information */
    printf("PCM name: '%s'\r\n", snd_pcm_name(pcm_handle));
    printf("PCM state: %s\r\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));

    /* Allocate buffer to hold single period */
    snd_pcm_hw_params_get_period_size(params, &frames, 0);      //Save size to frames

		return(0);
}




/* Write a buffer into the PCM device buffer
(<The buffer to write to the PCM device>)
*/
int bufWrite(char *buff)
{
		unsigned int pcm;
		unsigned long buffCap;
		
		/* Wait for the buffer to be emptied */
		while(1){
				buffCap = snd_pcm_avail_update(pcm_handle);								//Read amount of empty space in buffer
				if(buffCap < 0){
						printf("PCM buffer ran empty");
						return(-1);}
				if(buffCap > (1024*16) - 2048){											//If there is less than 1024 bytes left, move on
						break;}
				else{ return(1); }
		}
		//printf("%d \n", snd_pcm_avail_update(pcm_handle));

		/* Write data */
    pcm = snd_pcm_writei(pcm_handle, buff, frames);

    if(pcm < 0){                                        		//Check for an error writing
        printf("Write error: %s\n", snd_strerror(pcm));
        exit(EXIT_FAILURE);
    }

		return(0);
}



/* Mix up to 20 WAV files into one output buffer
(<List of the files to mix>
 <Size of the buffer to read from each file>
 <A pointer to the output buffer>)
*/
int mixer(FILE *files[4][3][8], int buff_size, char *out)
{
		char *buffs[88];
		int i, j, k, l;
		int noSamples = 0;
		signed long *shortBuffs[88], *shortPtr; 
		signed long shortOut[buff_size];
		unsigned int pcm;
		char *buffsPtr;

		for(i = 0; i < buff_size; i++){
				shortOut[i] = 0;
		}

		/* Read files and assign temporary buffers */
		for(i = 0; i < 88; i++){																				//Loop through each of the files
				if(keys[i].keyState > 0){
						buffs[i] = (char *) malloc(buff_size*2);									//Assign a temporary buffer for the file
						shortBuffs[i] = (signed long *) malloc(buff_size*4);

						pcm = fread(buffs[i], 1, buff_size, keys[i].stream[4]);	//Read the file
						if(pcm == 0){																						//File has ended
								memset(buffs[i], 0, buff_size);
						}

						noSamples++;
				}
		}

		if(noSamples > 0){
				/* Convert to long buffers */
				for(i = 0; i < 88; i++){                               			//Loop through each of the files
						if(keys[i].keyState > 0){
								buffsPtr = buffs[i];
								shortPtr = shortBuffs[i];
								for(j = 0; j < (buff_size/2); j++){									//Loop through 2 byte segments
										*shortPtr = *((signed short *)buffsPtr); 				//Convert char buffer to long buffer for manipulation
										buffsPtr += 2;
										shortPtr += 1;
								}
						}
				}

				/* Mix audio into the output buffer */
				for(i = 0; i < 88; i++){                               			//Loop through each of the files
						if(keys[i].keyState > 0){
								shortPtr = shortBuffs[i];
								for(j = 0; j < buff_size/2; j++){										//Loop through each 2 byte segment
										// Sustain
										if(keys[i].sustain > 0){ 
												*shortPtr = *shortPtr * keys[i].sustain; 
												keys[i].sustain -= 0.00005;
												if(keys[i].sustain <= 0.01){
												keys[i].sustain = 0.01;}
										}
										// Mixing
										shortOut[j] = shortOut[j] + *shortPtr;				//Mix the files
										shortPtr += 1;
								}
    				}
				}
				// Volume
				for(j = 0; j < buff_size/2; j++){
						shortOut[j] = shortOut[j]/2;
						if(shortOut[j] >= 32767){ shortOut[j] = 32767; }    //Restrict maximum
						if(shortOut[j] <= -32767){ shortOut[j] = -32767; }  //Restrict minimum
				}

				/* Convert output buffer back to char */
				for(i = 0; i < buff_size/2; i++){														//Convert 2 byte buffer into char buffer
        		*out = shortOut[i] & 0xFF;
        		out += 1;
        		*out = shortOut[i] >> 8;
    				out += 1;
    		}

				/* Clean up */
				out -= buff_size;													//Reset output buffer pointer
				for(i = 0; i < 88; i++){                  //Loop through each of the files
						if(keys[i].keyState > 0){
								free(buffs[i]);
								free(shortBuffs[i]);
						}
				}
		}
		return(noSamples);
}




/* Scan keyboard for key press

*/
int scanKeys()
{
		struct timespec tim, tim2; // Structure for delay
      tim.tv_sec = 0;
      tim.tv_nsec = 1L;
    int rdF = 1;
		int rdS = 1;
		int i, j;

    /* Shuffle through chips */
    for(i = 0; i < 88; i++){
				int chip = keys[i].chip;
				int select = keys[i].select;

        //First chip select line
        int cur_chip = chip;
        cur_chip &= 1;
        gpioWrite(KDB3, cur_chip);
        //Second chip select line
        cur_chip = chip;
        cur_chip = cur_chip >> 1;
        gpioWrite(KDB4, cur_chip);

        /* Shuffle through select pins */
        //First pin select
        int cur_sel = select >> 2;
        gpioWrite(KDB2, cur_sel);
        //Second pin select
        cur_sel = select >> 1;
        cur_sel &= ~(2);
        gpioWrite(KDB1, cur_sel);
        //Third pin select
        cur_sel = select;
        cur_sel &= ~(6);
        gpioWrite(KDB0, cur_sel);

        //3 to 8 converters have a delay of 2ns to switch
        //nanosleep(&tim, &tim2);
				j=0;
        while(j<300){j++;}
        /* Read column 1 */
        //if(keys[i].column == 0){
						rdF = gpioRead(keys[i].colPinF);
						rdS = gpioRead(keys[i].colPinS);
						initKey(rdF, rdS, i);
						velocityCall(rdF, rdS, i);
				//}
				/* Read column 2*/
				/*if(keys[i].column == 1){
            rd = gpioRead(KS1);
            initKey(rd, i);
        }*/
				/* Read column 3 */
				/*if(keys[i].column == 2){
            rd = gpioRead(KS2);
            initKey(rd, i);
        }*/
				/* Read column 4 */
				/*if(keys[i].column == 3){
            rd = gpioRead(KS3);
            initKey(rd, i);
        }*/
				j=0;
        while(j<300){j++;}
        //nanosleep(&tim, &tim2);	//Delay for the multiplexers
		}
		return(0);
}




/* Handles of the key pressed logic
(<the state of the first switch>
 <the state of the second switch>
 <the number of the key>)
*/
int initKey(int rdF, int rdS, int keyNum)
{
	struct timeval now, pulse;
	int keyState = keys[keyNum].keyState;
	int keyHist = keys[keyNum].keyHist;

	/* Key pressed */
	if(rdS == 0){
			keyState = 1;
			/* If key pressed and it was previously sustained */
			if(keyHist == 2){
					fseek(keys[keyNum].stream[4], 44, SEEK_SET);}
			/* If key pressed and previously pressed */
			if(keyHist < 2){
					keyHist = 1;}
			//printf("%s \n", key[keys[keyNum].column][keys[keyNum].chip][keys[keyNum].select]);
	}

	/* Key not pressed */
	else if(rdS == 1){
			/* If key not pressed but was pressed */
			if(keyHist > 0){
					keyState = 2;
			}
			else{
					keyHist = 0;
					keyState = 0;
			}
	}

	/* Sustain stuff */
	// If key is sustained and it was pressed before 	(Set sustain)
	if(keyState == 2 && keyHist == 1){
			keyHist = 2;
			keys[keyNum].sustain = 1;
			//printf("Sustain set: %d\n", keyNum);
	}
	// If sustain was on but is now pressed 					(Reset sustain)
	else if(keyHist == 2 && keyState == 1){
			keyHist = 1;
			keys[keyNum].sustain = 0;
			//printf("Sustain reset\n");
	}
	// If sustain is on but is finished								(Sustain ended)
	else if(keyHist == 2 && keys[keyNum].sustain <= 0.01){
			keys[keyNum].sustain = 0;
			keyState = 0;
			keyHist = 0;
			fseek(keys[keyNum].stream[4], 44, SEEK_SET);
			//printf("Sustain ended\n");
	}

	keys[keyNum].keyState = keyState;
	keys[keyNum].keyHist = keyHist;
}




/* Calculates velocity of a key
(<state of the first switch>
 <state of the second switch>
 <the number of the key>)
*/
int velocityCall(int rdF, int rdS, int keyNum)
{
		struct timespec gettime_now;
		int keyState = keys[keyNum].keyState;
  	int keyHist = keys[keyNum].keyHist;

		//if(rdF == 0){ printf("rdF "); }
		//if(rdS == 0){ printf("rdS\n"); }
		//else if(rdF == 0){ printf("\n"); }

		if(keyHist == 0 || keyHist == 2 && rdF == 0 && keys[keyNum].start == 0 && rdS != 0){
				clock_gettime(CLOCK_REALTIME, &gettime_now);
				//gettimeofday(&now, NULL);
				keys[keyNum].start = gettime_now.tv_nsec;}

		if(keys[keyNum].start != 0 && rdS == 0){
				//gettimeofday(&now, NULL);
				clock_gettime(CLOCK_REALTIME, &gettime_now);
				keys[keyNum].velocity = gettime_now.tv_nsec - keys[keyNum].start;
				keys[keyNum].start = 0;
				//printf("Velocity: %d\n", keys[keyNum].velocity);
		}
}
