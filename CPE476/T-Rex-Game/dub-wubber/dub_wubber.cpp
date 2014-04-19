
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "dub_wubber.h"

#define CANCEL_NUM_BUFFS 10000
#define CANCEL_BUFF_COUNT 50   // these last 20 of the buffer will be used to cancel
#define CANCEL_POLL_DELAY 100
#define POWER_DIFFERENCE 0
#define POWER_THRESHOLD 0.75

static float *timeBuff;
static int playing = TRUE;
static PaStream *stream;

static void paerror(PaError err) {
   printf("PortAudio error: %s\n", Pa_GetErrorText(err));
   exit(err);
}

static int paCallback( const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags, void *userData) {
   
   float *out = (float*)outputBuffer;
   // memcpy(timeBuff, inputBuffer, framesPerBuffer);

   if (playing) {
      for (int i = 0; i < framesPerBuffer; i++) {
         Sample noteAmp = getNoteAmp();
         *out++ = noteAmp.channel[0];
         *out++ = noteAmp.channel[1];
      }
   }
   else {
      for (int i = 0; i < framesPerBuffer; i++) {
         *out++ = 0.0f;
         *out++ = 0.0f;
      }
   }
   return 0;
}

void setup() {
   // timeBuff = (float *) fftwf_malloc(sizeof(float) * FRAMES_PER_BUFFER);

   PaError err = Pa_Initialize();
   if( err != paNoError ) paerror(err);
   err = Pa_OpenDefaultStream( &stream, 0, 2, paFloat32, SAMPLE_RATE, FRAMES_PER_BUFFER,
      paCallback, &data); 
   if( err != paNoError ) paerror(err);
   err = Pa_StartStream( stream );
   if( err != paNoError ) paerror(err);

   startSong();
}

void takedown() {
   PaError err = Pa_StopStream( stream );
   if( err != paNoError ) paerror(err);
   err = Pa_CloseStream( stream );
   if( err != paNoError ) paerror(err);
   err = Pa_Terminate();
   if( err != paNoError ) paerror(err);
}

// void getSampleBuff(float * timeBlock) {
//    memcpy(timeBlock, timeBuff, FRAMES_PER_BUFFER);
// }

void togglePlaying() {
   playing = !playing;
}
