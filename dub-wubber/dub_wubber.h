#ifndef __BRISK_HELPER_H__
#define __BRISK_HELPER_H__

#include "portaudio.h"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 1024
#define GOOD_FRAME_COUNT 256
#define TRUE 1
#define FALSE 0
#define uint unsigned int
#define ushort unsigned short

typedef struct {
   float left_phase;
   float right_phase;
} Frame;

typedef struct {
   float channel[2]; // 0 for left, 1 for right
} Sample;

static Frame data;

void setup();
void takedown();
void togglePlaying();

void startSong();
Sample getNoteAmp();
void setRootFreq(float freq);
void nextMode();

void setBaseNote(int base);
void setTempo(int tempo);
void setAmpLFORate(float rate);
void setAmpLFOAmp(float amp);
void setFreqLFORate(float rate);
void setFreqLFOAmp(float amp);
void setFlangeDepth(int depth);
void setFlangeRate(float rate);
void setLPF(float angle);

#endif