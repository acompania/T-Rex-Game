#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "dub_wubber.h"

#define SAMPLE_RATE 44100
#define PI 3.14159265359f
#define BASE_TEMPO 60.0f
#define NUM_FREQS 128
#define NUM_NOTES 7
#define OCTAVE_HALFSTEPS 12
#define MODES 7
#define START_BASE 70
#define START_LFO_RATE 0
#define START_LFO_AMP 0
#define FLANGE_BUFF_SIZE 2000
#define START_FLANGE_DEPTH 200
#define START_FLANGE_RATE 0

typedef struct {
   int pitch;        // index to the frequency in the frequencies array
   int length;       // number of frames to play note (includes silence after note)
   int cutoff;       // frame when the sound ends
   int scaleNdx;
   int offset;     // current frame of note being played
} Note;

typedef struct {
   int attX;
   float attY;
   int susX;
   float susY;
   int tapX;
   float attM;
   float fallM, fallYo;
} Envelope;

static float frequencies[] = {
   8.175, 8.662, 9.177, 10.301, 10.600, 10.913,                   // midi octave -5
   11.562, 12.230, 12.988, 13.750, 14.568, 15.434,                // midi octave -5
   16.352, 17.324, 18.354, 19.445, 20.602, 21.827,                // midi octave -4
   23.125, 24.500, 25.957, 27.500, 29.135, 30.868,                // midi octave -4
   32.703, 34.648, 36.708, 38.891, 41.203, 43.654,                // midi octave -3
   46.249, 48.999, 51.913, 55.000, 58.270, 61.735,                // midi octave -3
   65.406, 69.296, 73.416, 77.782, 82.407, 87.307,                // midi octave -2
   92.499, 97.999, 103.826, 110.000, 116.541, 123.471,            // midi octave -2
   130.813, 138.591, 146.832, 155.563, 164.814, 174.614,          // midi octave -1
   184.997, 195.998, 207.652, 220.000, 233.082, 246.942,          // midi octave -1
   261.626, 277.183, 293.665, 311.127, 329.628, 349.228,          // midi octave  0
   369.994, 391.995, 415.305, 440.000, 466.164, 493.883,          // midi octave  0
   523.251, 554.365, 587.330, 622.254, 659.255, 698.456,          // midi octave  1
   739.989, 783.991, 830.609, 880.000, 932.328, 987.767,          // midi octave  1
   1046.500, 1108.730, 1174.660, 1244.510, 1318.510, 1396.910,    // midi octave  2
   1479.980, 1567.980, 1661.220, 1760.000, 1864.660, 1975.530,    // midi octave  2
   2093.000, 2217.460, 2349.320, 2489.020, 2637.020, 2793.830,    // midi octave  3
   2959.960, 3135.960, 3322.440, 3520.000, 3729.310, 3951.070,    // midi octave  3
   4186.010, 4434.922, 4698.626, 4978.032, 5274.041, 5587.652,    // midi octave  4
   5919.911, 5919.911, 6644.875, 7040.000, 7458.620, 7902.133,    // midi octave  4
   8372.018, 8869.844, 9397.273, 9956.063, 10548.082, 11175.303,  // midi octave  5
   11839.822, 12543.854                                           // midi octave  5
};

static char ionian[]       = { 0, 2, 4, 5, 7, 9, 11 };
static char dorian[]       = { 0, 2, 3, 5, 7, 9, 10 };
static char phrygian[]     = { 0, 1, 3, 5, 7, 8, 10 };
static char lydian[]       = { 0, 2, 4, 6, 7, 9, 11 };
static char mixolydian[]   = { 0, 2, 4, 5, 7, 9, 10 };
static char aeolian[]      = { 0, 2, 3, 5, 7, 8, 10 };
static char locrian[]      = { 0, 1, 3, 5, 6, 8, 10 };

static int modeNum = 0;
static char * mode = ionian;
static int tempo = BASE_TEMPO;
static Note base, third, fifth, seventh, firstMel, secondMel;
static Envelope basicEnv;
static int rootPitch = START_BASE;
static float lfoRate = START_LFO_RATE;
static float lfoAmp = START_LFO_AMP;
static float flangeBuff[FLANGE_BUFF_SIZE];
static int flangeDepth = START_FLANGE_DEPTH;
static float flangeRate = START_FLANGE_RATE;
static int flangeNdx = 0;
static int timePos = 0;
static int chordCount = 0;

static int randInt(int low, int high) {
   return rand() % (high - low + 1) + low;
}

static float randFloat(float low, float high) {
   return (high - low) * static_cast<float>(rand()) / (static_cast<float>(RAND_MAX)) + low;
}

// returns the chosen index
static int randDistribution(float *weights, int count) {
   int total = 0;
   for (int i = 0; i < count; i++)
      total += weights[i];

   float num = randFloat(0, total);

   for (int i = 0; i < count; i++) {
      if (num <= weights[i])
         return i;
      else
         num -= weights[i];
   }
   return -1;
}


static int randPitch() {
   float choices[] = {5, 2, 9, 2, 9, 3, 6};
   return mode[randDistribution(choices, 7)];
}

static int randLength() {
   float choices[] = {10, 5, 0, 2};
   return 11025 * (1 + randDistribution(choices, 4));
}

static Note randNote() {
   Note n;
   n.pitch = rootPitch + randPitch();
   n.length = randLength() * 60.0f / tempo;
   n.cutoff = 1.0 * n.length;
   n.offset = 0;
   n.scaleNdx = 0;
   return n;
}

static Note baseNote() {
   Note n;
   n.pitch = rootPitch;
   n.length = 168400 * 60.0f / tempo;;
   n.cutoff = n.length * 15 / 16;
   n.offset = 0;
   n.scaleNdx = 0;
   return n;
}

static Note randCloseNote(Note lastNote) {
   Note n;
   n.scaleNdx = lastNote.scaleNdx;
   int dir;

   if (lastNote.pitch > NUM_FREQS)
      dir = 0;
   else if (lastNote.pitch < 0)
      dir = 1;
   else
      dir = randInt(0, 1);

   if (dir) {
      n.scaleNdx++;
      if (n.scaleNdx >= NUM_NOTES)
         n.scaleNdx -= 2; //n.scaleNdx -= NUM_NOTES;
   } else {
      n.scaleNdx--;
      if (n.scaleNdx < 0)
         n.scaleNdx += 2; //n.scaleNdx += NUM_NOTES;
   }

   n.pitch = rootPitch + mode[n.scaleNdx];
   n.length = randLength() * 60.0f / tempo;
   n.cutoff = 1.0 * n.length;
   n.offset = 0;
   return n;
}

void setBaseNote(int base) {
   if (base < 0)
      rootPitch = frequencies[0];
   else if (base > 127)
      rootPitch = frequencies[127];
   else
      rootPitch = frequencies[base];
}

static void setEnvelope(Envelope * env, float attX, float attY, float susX, float susY, float tapX) {
   env->attX = attX;
   env->attY = attY;
   env->susX = susX;
   env->susY = susY;
   env->tapX = tapX;
   env->attM = attY / attX;
   env->fallM = (susY - attY)/(susX - attX);
   env->fallYo = - env->fallM * susX + susY;
}

static void createChord() {
   base = baseNote();
   third = base;
   third.scaleNdx = 2;
   third.pitch = rootPitch + mode[third.scaleNdx] - 12;
   fifth = base;
   fifth.scaleNdx = 4;
   fifth.pitch = rootPitch + mode[fifth.scaleNdx] - 12;
   seventh = base;
   seventh.scaleNdx = 6;
   seventh.pitch = rootPitch + mode[seventh.scaleNdx] - 12;
}

void startSong() {
   createChord();

   firstMel = randNote();
   secondMel = randNote();

   float attX = SAMPLE_RATE / 64.0f;
   float attY = 1.0f;
   float susX = SAMPLE_RATE / 16.0f;
   float susY = 0.7f;
   float tapX = SAMPLE_RATE / 32.0f;
   setEnvelope(&basicEnv, attX, attY, susX, susY, tapX);
}

static float getEnvAmp(Envelope env, Note note) {
   int tapStart = note.cutoff - env.tapX;
   // printf("tapStart %i cutoff %i\n", tapStart, note.cutoff);
   if (tapStart <= env.attX) {
      float quartX = note.cutoff / 3;
      if (note.offset < quartX)
         return note.offset * env.attY / quartX;
      else if (note.offset < 2 * quartX)
         return - note.offset * env.attY / quartX + 2 * env.attY;
      else
         return 0.0f;
   } else if (tapStart <= env.susX) {
      if (note.offset < env.attX)
         return note.offset * env.attM;
      else if (note.offset < tapStart)
         return note.offset * env.fallM + env.fallYo;
      else if (note.offset < note.cutoff) {
         float fallYo = env.fallM * tapStart + env.fallYo;
         float cutM = - fallYo / (note.cutoff - tapStart);
         float cutYo = - cutM * note.cutoff;
         return note.offset * cutM + cutYo;
      } else
         return 0.0f;
   } else {
      if (note.offset < env.attX)
         return note.offset * env.attM;
      else if (note.offset < env.susX)
         return note.offset * env.fallM + env.fallYo;
      else if (note.offset < tapStart) // offset from end of note
         return env.susY;
      else if (note.offset < note.cutoff) {
         float cutM = - env.susY / (note.cutoff - tapStart);
         float cutYo = - cutM * note.cutoff;
         return note.offset * cutM + cutYo;
      } else
         return 0.0f;
   }
}

static float computeAngle(Note note) {
   return 2.0 * PI * note.offset * frequencies[note.pitch] / SAMPLE_RATE;
}

void nextMode() {
   if (modeNum == 0) {
      mode = dorian;
      printf("Switched to mode Dorian\n");
   } else if (modeNum == 1) {
      mode = phrygian;
      printf("Switched to mode Phrygian\n");
   } else if (modeNum == 2) {
      mode = lydian;
      printf("Switched to mode Lydian\n");
   } else if (modeNum == 3) {
      mode = mixolydian;
      printf("Switched to mode Mixolydian\n");
   } else if (modeNum == 4) {
      mode = aeolian;
      printf("Switched to mode Aeolian\n");
   } else if (modeNum == 5) {
      mode = locrian;
      printf("Switched to mode Locrian\n");
   } else if (modeNum == 6) {
      mode = ionian;
      printf("Switched to mode Ionian\n");
   }

   modeNum = (modeNum + 1) % MODES;
}

// gets the sample that was played "backCount" samples ago
static float getSample(float backCount) {
   if (backCount > FLANGE_BUFF_SIZE) {
      printf("can't go back that far\n");
      exit(0);
   }

   float midNdx;
   if (backCount > flangeNdx)
      midNdx = flangeNdx + FLANGE_BUFF_SIZE - backCount;
   else
      midNdx = flangeNdx - backCount;
   // printf("lowNdx %d\n", lowNdx);
   float lowHeight = flangeBuff[(int)midNdx];
   float highHeight;
   if ((int)midNdx + 1 >= FLANGE_BUFF_SIZE)
      highHeight = flangeBuff[0];
   else
      highHeight = flangeBuff[(int)midNdx + 1];
   float highWeight = midNdx - (int)midNdx;
   return lowHeight * (1.0f - highWeight) + highHeight * highWeight;
}

void setFreqLFORate(float rate) {
   lfoRate = rate;
   if (lfoRate < 0)
      lfoRate = 0;
   printf("Freq LFO Rate = %f\n", lfoRate);

}

void setFreqLFOAmp(float amp) {
   lfoAmp = amp;
   if (lfoAmp < 0)
      lfoAmp = 0;
   printf("Freq LFO Amp = %f\n", lfoAmp);
}

void setTempo(int tem) {
   tempo = tem;
   printf("Tempo = %d\n", tempo);
}

void setFlangeDepth(int depth) {
   flangeDepth = depth;
   if (flangeDepth < 0)
      flangeDepth = 0;
   printf("Flange Depth = %d\n", flangeDepth);
}
void setFlangeRate(float rate) {
   flangeRate = rate;
   if (flangeRate < 0)
      flangeRate = 0;
   printf("Flange Rate = %f\n", flangeRate);
}

Sample getNoteAmp() {
   float chordAmp, firstAmp, secondAmp, envAmpChord, envAmpFirst, envAmpSecond, lfoAngle, lfoHeight, totAmp, flangeAmp, samplesBack;
   
   if (base.offset >= base.length) {
      if (chordCount)
         rootPitch += 4;
      else
         rootPitch -= 4;

      chordCount = (chordCount + 1) % 2;
      createChord();
   }
   if (firstMel.offset >= firstMel.length)
      firstMel = randNote(); //randCloseNote(firstMel);
   if (secondMel.offset >= secondMel.length)
      secondMel = randNote(); //randCloseNote(secondMel);

   lfoAngle = 2.0 * PI * timePos * lfoRate / SAMPLE_RATE;
   lfoHeight = lfoAmp * sin(lfoAngle);

   chordAmp = 0;
   chordAmp += sin(computeAngle(base) + lfoHeight);
   chordAmp += sin(computeAngle(third) + lfoHeight);
   chordAmp += sin(computeAngle(fifth) + lfoHeight);
   chordAmp += sin(computeAngle(seventh) + lfoHeight);
   chordAmp /= 2;
   firstAmp = sin(computeAngle(firstMel) + lfoHeight);
   secondAmp = sin(computeAngle(secondMel) + lfoHeight);

   envAmpChord = getEnvAmp(basicEnv, base);
   envAmpFirst = getEnvAmp(basicEnv, firstMel);
   envAmpSecond = getEnvAmp(basicEnv, secondMel);

   totAmp = (firstAmp * envAmpFirst + chordAmp * envAmpChord) / 2;

   Sample sample;
   flangeNdx = (flangeNdx + 1) % FLANGE_BUFF_SIZE;
   flangeBuff[flangeNdx] = totAmp;
   samplesBack = flangeDepth * sin(2.0 * PI * timePos * flangeRate / SAMPLE_RATE) + flangeDepth;
   sample.channel[0] = getSample(samplesBack);
   samplesBack = - flangeDepth * sin(2.0 * PI * timePos * flangeRate / SAMPLE_RATE) + flangeDepth;
   sample.channel[1] = getSample(samplesBack);

   base.offset += 1;
   third.offset += 1;
   fifth.offset += 1;
   seventh.offset += 1;
   firstMel.offset += 1;
   secondMel.offset += 1;
   timePos++;
   return sample;
}
