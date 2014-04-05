
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fftw3.h>
#include <math.h>

#include "briskhelper.h"
#include "composer.h"

#define CANCEL_NUM_BUFFS 10000
#define CANCEL_BUFF_COUNT 50   // these last 20 of the buffer will be used to cancel
#define CANCEL_POLL_DELAY 100
#define POWER_DIFFERENCE 0
#define POWER_THRESHOLD 0.75

static float *timeBuff, *magFreqBuff, **cancelBuff, *cancSum, * cancAve, * cancFreqBuff;
static fftwf_complex *freqBuff;
static int recording = FALSE, playing = TRUE;
static fftwf_plan plan;
static PaStream *stream;
static int cancNdx = 0, callCount = 0;

static void paerror(PaError err) {
   printf("PortAudio error: %s\n", Pa_GetErrorText(err));
   exit(err);
}

// ==================== NOISE REMOVAL ===================== //

// // copies the freq buffer into the cancel buff array and updates the sum
// static void updateCancelArray() {
//    memcpy(cancelBuff[cancNdx], freqBuff, FRAMES_PER_BUFFER);
//    int lastNdx = cancNdx == 0 ? FRAMES_PER_BUFFER - 1 : cancNdx - 1;
//    for (int i = 0; i < FRAMES_PER_BUFFER; i++)
//       cancSum[i] += cancelBuff[cancNdx][i] - cancelBuff[lastNdx][i];
//    cancNdx = (cancNdx + 1) % CANCEL_NUM_BUFFS;
// }

// // removes background noise from the fft
// static void backCancel() {
//    for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
//       cancAve[i] = cancSum[i] / CANCEL_NUM_BUFFS;

//       if (freqBuff[i] - cancAve[i] < POWER_DIFFERENCE) {
//          cancFreqBuff[i] = 0;
//       } else {
//          cancFreqBuff[i] -= cancAve[i];
//       }
//    }
// }

static void processFFTOutput() {
   // we only care about the magnitude of the frequency
   for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
      float real = ((float*)freqBuff)[2*i];
      float imag = ((float*)freqBuff)[2*i+1];
      magFreqBuff[i] = real*real + imag*imag;
   }

   // callCount = (callCount + 1) % CANCEL_POLL_DELAY;
   // if (!callCount)
   //    updateCancelArray();
   // backCancel();

   // calculate the dominant frequency
   float domAmp = magFreqBuff[0];
   int domNdx = 0;
   for (int i = 1; i < FRAMES_PER_BUFFER; i++)
      if (magFreqBuff[i] > domAmp) {
         domAmp = magFreqBuff[i];
         domNdx = i;
      }

   float binwidth = SAMPLE_RATE / FRAMES_PER_BUFFER;
   float domFreq = binwidth * domNdx - 0.5 * binwidth;
   if (domAmp > POWER_THRESHOLD)
      setRootFreq(domFreq);
}

static int paCallback( const void *inputBuffer, void *outputBuffer,
   unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo,
   PaStreamCallbackFlags statusFlags, void *userData)
{
   /* Cast data passed through stream to our structure. */
   Frame *data = (Frame*)userData; 
   float *out = (float*)outputBuffer;

   if (recording) {
      memcpy(timeBuff, inputBuffer, framesPerBuffer);
      fftwf_execute(plan);
      processFFTOutput();
   }
   
   if (playing) {
      for (int i = 0; i < framesPerBuffer; i++) {
         Sample noteAmp = getNoteAmp();
         *out++ = noteAmp.channel[0];
         *out++ = noteAmp.channel[1];
      }
   }
   else {
      for (int i = 0; i < framesPerBuffer; i++) {
         getNoteAmp();  // continue the melody
         *out++ = 0.0f;
         *out++ = 0.0f;
      }
   }
   return 0;
}

void setup() {
   // cancFreqBuff = (float *) fftwf_malloc(sizeof(float) * FRAMES_PER_BUFFER);
   // cancAve = (float *) fftwf_malloc(sizeof(float) * FRAMES_PER_BUFFER);
   // cancSum = (float *) fftwf_malloc(sizeof(float) * FRAMES_PER_BUFFER);
   // memset(cancSum, 0, FRAMES_PER_BUFFER);
   // cancelBuff = (float **) fftwf_malloc(sizeof(float *) * CANCEL_NUM_BUFFS);
   // for (int i = 0; i < CANCEL_NUM_BUFFS; i++)
   //    cancelBuff[i] = (float *) fftwf_malloc(sizeof(float) * FRAMES_PER_BUFFER);

   timeBuff = (float *) fftwf_malloc(sizeof(float) * FRAMES_PER_BUFFER);
   magFreqBuff = (float *) fftwf_malloc(sizeof(float) * FRAMES_PER_BUFFER);
   freqBuff = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * FRAMES_PER_BUFFER);
   plan = fftwf_plan_dft_r2c_1d(FRAMES_PER_BUFFER, timeBuff, freqBuff, FFTW_ESTIMATE);
   if (!plan) {
      printf("plan not created\n");
      exit(0);
   }

   PaError err = Pa_Initialize();
   if( err != paNoError ) paerror(err);
   err = Pa_OpenDefaultStream( &stream, 1, 2, paFloat32, SAMPLE_RATE, FRAMES_PER_BUFFER,
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

   fftwf_destroy_plan(plan);
   fftwf_free(timeBuff);
   fftwf_free(magFreqBuff);
   fftwf_free(freqBuff);
   // for (int i = 0; i < CANCEL_NUM_BUFFS; i++)
   //    fftwf_free(cancelBuff[i]);
   // fftwf_free(cancelBuff);
   // fftwf_free(cancSum);
   // fftwf_free(cancAve);
   // fftwf_free(cancFreqBuff);
}

void getTimeBuff(float * timeBlock) {
   memcpy(timeBlock, timeBuff, FRAMES_PER_BUFFER);
}

void getFreqBuff(float * freqBlock) {
   memcpy(freqBlock, magFreqBuff, FRAMES_PER_BUFFER);
}

void getCancBuff(float * cancBlock) {
   memcpy(cancBlock, cancAve, FRAMES_PER_BUFFER);
}

void toggleRecording() {
   recording = !recording;
}

void togglePlaying() {
   playing = !playing;
}
