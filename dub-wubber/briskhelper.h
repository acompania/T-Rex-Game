#ifndef __BRISK_HELPER_H__
#define __BRISK_HELPER_H__

#include "portaudio.h"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 1024
#define GOOD_FRAME_COUNT 256
#define TRUE 1
#define FALSE 0

typedef struct {
   float left_phase;
   float right_phase;
} Frame;

static Frame data;

void setup();
void takedown();
void getTimeBuff(float * timeBuff);
void getFreqBuff(float * freqBuff);
void getCancBuff(float * cancBuff);
void toggleRecording();
void togglePlaying();

#endif