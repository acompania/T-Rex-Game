#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dub_wubber.h"

int main() {
   char command = 0;
   float amp, rate;
   int depth, tempo;
   int retVal;
   int base;
   char *timestamp;

   printf("Testing the dub_wubber library\n\n");
   setup();

   while (command != 'Q') {
      fscanf(stdin, "%c", &command);
      command = toupper(command);

      switch (command) {
         case 'H':
            printf(
               "SPACE ----- Toggle Playing\n"
               "B [base] -- Set base note (from 0 - 127)"
               "N --------- Next mode\n"
               "A [amp] --- Set amp of freqLFO\n"
               "R [rate] -- Set rate of freqLFO\n"
               "F [rate] -- Set flange rate\n"
               "D [depth] - Set flange depth\n"
               "T [tempo] - Set tempo\n"
            );
            break;
         case 32: // space
            togglePlaying();
            break;
         case 'N':
            nextMode();
            break;
         case 'B':
            fscanf(stdin, "%d", &base);
            setBaseNote(base);
         case 'A':
            fscanf(stdin, "%f", &amp);
            setFreqLFOAmp(amp);
            break;
         case 'R':
            fscanf(stdin, "%f", &rate);
            setFreqLFORate(rate);
            break;
         case 'F':
            fscanf(stdin, "%f", &rate);
            setFlangeRate(rate);
            break;
         case 'D':
            fscanf(stdin, "%d", &depth);
            setFlangeDepth(depth);
            break;
         case 'T':
            fscanf(stdin, "%d", &tempo);
            setTempo(tempo);
            break;
         case 'Q':
            printf("Bye!\n");
            break;
      }
   }

   return 0;
}
