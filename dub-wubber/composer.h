#ifndef __COMPOSER_H__
#define __COMPOSER_H__

#define uint unsigned int
#define ushort unsigned short

typedef struct {
   float channel[2]; // 0 for left, 1 for right
} Sample;

void startSong();
Sample getNoteAmp();
void setRootFreq(float freq);
void nextMode();

void decreaseLFORate();
void increaseLFORate();
void decreaseLFOAmp();
void increaseLFOAmp();
void decreaseTempo();
void increaseTempo();
void decreaseFlangeDepth();
void increaseFlangeDepth();
void decreaseFlangeRate();
void increaseFlangeRate();

#endif