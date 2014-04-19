#include <complex.h>
#include <iostream>

#define NUMBER_OF_POLES 4

AudioSampleBuffer lowPassBuffer, inputBuffer;
int lowPassPosition;
std::complex<float> chebyshevPoles[NUMBER_OF_POLES];

const std::complex<float> I = std::complex<float>(0.0, 1.0);
const float defaultLowPassFrequency = 440.0f;


void setupFilter()
    : lowPassBuffer(2, 4),
      inputBuffer(2, 4)
{
    // Set up some default values..
    lowPassFrequency = defaultLowPassFrequency;
    
    for (int iter = 0; iter < NUMBER_OF_POLES; iter++) {
        chebyshevPoles[iter] = I * cos(1.0f / NUMBER_OF_POLES * acos(I / 0.5f) + iter * float_Pi / NUMBER_OF_POLES);
    }
    
    lowPassPosition = 0;
}

// chebyshevFilter(angleToFilter, buffer, channel);

void chebyshevFilter(float angleToFilter, AudioSampleBuffer &buffer, int channel) {
    float pastOutputTemp;
    float pastInputTemp;
    std::complex<float> zPoles[NUMBER_OF_POLES];
    std::complex<float> zZeros[NUMBER_OF_POLES];
    float bottomCoefficients[NUMBER_OF_POLES + 1];
    float topCoefficients[NUMBER_OF_POLES + 1];
    int lPP = lowPassPosition, numSamples = buffer.getNumSamples();

    calculateZZeros(zZeros);
    calculateZPoles(angleToFilter, zPoles);
    calculateTopCoefficients(zZeros, topCoefficients);
    calculateBottomCoefficients(zPoles, bottomCoefficients);
    
    // float gain = calculateDCGain(zPoles, zZeros);
    
    // buffer.applyGain (channel, 0, buffer.getNumSamples(), 1.0 / gain);

    float* channelData = buffer.getSampleData (channel);
    float* lowPassData = lowPassBuffer.getSampleData(jmin(channel, lowPassBuffer.getNumChannels()));
    float* pastInputData = inputBuffer.getSampleData(jmin(channel, inputBuffer.getNumChannels()));
    
    
    for (int i = 0; i < numSamples; ++i) {
        pastInputData[lPP] = channelData[i];
        
        if (lPP >= 4) {
            pastInputTemp = topCoefficients[0] * pastInputData[lPP] + 
             topCoefficients[1] * pastInputData[lPP - 1] + 
             topCoefficients[2] * pastInputData[lPP - 2] +
             topCoefficients[3] * pastInputData[lPP - 3] + 
             topCoefficients[4] * pastInputData[lPP - 4];
            
            pastOutputTemp = bottomCoefficients[1] * lowPassData[lPP - 1] +
             bottomCoefficients[2] * lowPassData[lPP - 2] + 
             bottomCoefficients[3] * lowPassData[lPP - 3] +
             bottomCoefficients[4] * lowPassData[lPP - 4];
        }
        // Simple edge case
        else if (lPP == 3) {
            pastInputTemp = topCoefficients[0] * pastInputData[lPP] + 
             topCoefficients[1] * pastInputData[lPP - 1] + 
             topCoefficients[2] * pastInputData[lPP - 2] + 
             topCoefficients[3] * pastInputData[lPP - 3] + 
             topCoefficients[4] * pastInputData[lowPassBuffer.getNumSamples() - 1];
            
            pastOutputTemp = bottomCoefficients[1] * lowPassData[lPP - 1] +
             bottomCoefficients[2] * lowPassData[lPP - 2] + 
             bottomCoefficients[3] * lowPassData[lPP - 3] + 
             bottomCoefficients[4] * lowPassData[lowPassBuffer.getNumSamples() - 1];
        }
        else if (lPP == 2) {
            pastInputTemp = topCoefficients[0] * pastInputData[lPP] + 
             topCoefficients[1] * pastInputData[lPP - 1] + 
             topCoefficients[2] * pastInputData[lPP - 2] + 
             topCoefficients[3] * pastInputData[lowPassBuffer.getNumSamples() - 1] +
             topCoefficients[4] * pastInputData[lowPassBuffer.getNumSamples() - 2];
            
            pastOutputTemp = bottomCoefficients[1] * lowPassData[lPP - 1] + 
             bottomCoefficients[2] * lowPassData[lPP - 2] + 
             bottomCoefficients[3] * lowPassData[lowPassBuffer.getNumSamples() - 1] +
             bottomCoefficients[4] * lowPassData[lowPassBuffer.getNumSamples() - 2];
        }
        else if (lPP == 1) {
            pastInputTemp = topCoefficients[0] * pastInputData[lPP] + 
             topCoefficients[1] * pastInputData[lPP - 1] + 
             topCoefficients[2] * pastInputData[lowPassBuffer.getNumSamples() - 1] + 
             topCoefficients[3] * pastInputData[lowPassBuffer.getNumSamples() - 2] + 
             topCoefficients[4] * pastInputData[lowPassBuffer.getNumSamples() - 3];
            
            pastOutputTemp = bottomCoefficients[1] * lowPassData[lPP - 1] + 
             bottomCoefficients[2] * lowPassData[lowPassBuffer.getNumSamples() - 1] + 
             bottomCoefficients[3] * lowPassData[lowPassBuffer.getNumSamples() - 2] + 
             bottomCoefficients[4] * lowPassData[lowPassBuffer.getNumSamples() - 3];
        }
        else if (lPP == 0) {
            pastInputTemp = topCoefficients[0] * pastInputData[lPP] + 
             topCoefficients[1] * pastInputData[lowPassBuffer.getNumSamples() - 1] +
             topCoefficients[2] * pastInputData[lowPassBuffer.getNumSamples() - 2] + 
             topCoefficients[3] * pastInputData[lowPassBuffer.getNumSamples() - 3] + 
             topCoefficients[4] * pastInputData[lowPassBuffer.getNumSamples() - 4];
            
            pastOutputTemp = bottomCoefficients[1] * lowPassData[lowPassBuffer.getNumSamples() - 1] + 
             bottomCoefficients[2] * lowPassData[lowPassBuffer.getNumSamples() - 2] + 
             bottomCoefficients[3] * lowPassData[lowPassBuffer.getNumSamples() - 3] + 
             bottomCoefficients[4] * lowPassData[lowPassBuffer.getNumSamples() - 4];
        }
        else {
            assert(false);
        }

        channelData[i] = lowPassData[lPP] = pastInputTemp - pastOutputTemp;
        
        if (++lPP >= lowPassBuffer.getNumSamples()) {
            lPP = 0;
        }
    }
    
    lowPassPosition = lPP;
}

static void calculateZPoles(float angleToFilter, std::complex<float> zPoles[]) {
   for (int i = 0; i < NUMBER_OF_POLES; i++) {
      std::complex<float> cTemp = angleToFilter * chebyshevPoles[i];
      zPoles[i] = (1.0f + cTemp / 2.0f) / (1.0f - cTemp / 2.0f);
   }
}

static void calculateZZeros(std::complex<float> zZeros[]) {
   for (int i = 1; i < NUMBER_OF_POLES; i++) {
      zZeros[i] = std::complex<float>(-1.0, 0.0);
   }
}

static void calculateTopCoefficients(std::complex<float> zZeros[], float coefficients[]) {
   coefficients[0] = 1;
   coefficients[1] = ((-zZeros[0]) +
                     (-zZeros[1]) +
                     (-zZeros[2]) +
                     (-zZeros[3])).real();
   coefficients[2] = ((-zZeros[0]) * (-zZeros[1]) +
                     (-zZeros[0]) * (-zZeros[2]) +
                     (-zZeros[0]) * (-zZeros[3]) +
                     (-zZeros[1]) * (-zZeros[2]) +
                     (-zZeros[1]) * (-zZeros[3]) +
                     (-zZeros[2]) * (-zZeros[3])).real();
   coefficients[3] = ((-zZeros[0]) * (-zZeros[1]) * (-zZeros[2]) +
                     (-zZeros[0]) * (-zZeros[1]) * (-zZeros[3]) +
                     (-zZeros[0]) * (-zZeros[2]) * (-zZeros[3]) +
                     (-zZeros[1]) * (-zZeros[2]) * (-zZeros[3])).real();
   coefficients[4] = ((-zZeros[0]) * (-zZeros[1]) * (-zZeros[2]) * (-zZeros[3])).real();
}

static void calculateBottomCoefficients(std::complex<float> zPoles[], float coefficients[]) {
   coefficients[0] = 0;
   coefficients[1] = ((-zPoles[0]) +
                     (-zPoles[1]) +
                     (-zPoles[2]) +
                     (-zPoles[3])).real();
   coefficients[2] = ((-zPoles[0]) * (-zPoles[1]) +
                     (-zPoles[0]) * (-zPoles[2]) +
                     (-zPoles[0]) * (-zPoles[3]) +
                     (-zPoles[1]) * (-zPoles[2]) +
                     (-zPoles[1]) * (-zPoles[3]) +
                     (-zPoles[2]) * (-zPoles[3])).real();
   coefficients[3] = ((-zPoles[0]) * (-zPoles[1]) * (-zPoles[2]) +
                     (-zPoles[0]) * (-zPoles[1]) * (-zPoles[3]) +
                     (-zPoles[0]) * (-zPoles[2]) * (-zPoles[3]) +
                     (-zPoles[1]) * (-zPoles[2]) * (-zPoles[3])).real();
   coefficients[4] = ((-zPoles[0]) * (-zPoles[1]) * (-zPoles[2]) * (-zPoles[3])).real();
}
