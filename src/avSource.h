/*----------------------------------------------------------------------------------------------------------
 * avSourcePhasor.h
 * 
 * Defines a base class MutantSource for all synth voices and  a derived class 
 * MutantPhasor - a 2-oscillator synth voice with low pass filter, volume envelope and filter envelope
 * 
 * (C) 2021 Meebleeps
*-----------------------------------------------------------------------------------------------------------
*/

#ifndef avSource_h
#define avSource_h
#include "Arduino.h"
#include "PDResonant.h"
#include <EventDelay.h>
#include <MozziGuts.h>
#include <mozzi_fixmath.h>
#include <ADSR.h>
#include <LowPassFilter.h>

// define this here to avoid clipping the filter function
#define MAX_FILTER_RESONANCE  100
#define MAX_FILTER_CUTOFF     240
#define MAX_FILTER_SHAPE      1023
#define MAX_FILTER_ENV_ATTACK 4096
#define MAX_FILTER_ENV_DECAY  4096
#define MIN_FILTER_ENV_DECAY  40
#define MAX_SOURCE_PARAMS     7

#define SYNTH_PARAMETER_OSC2TUNE          0
#define SYNTH_PARAMETER_FILTER_CUTOFF     1
#define SYNTH_PARAMETER_FILTER_RESONANCE  2
#define SYNTH_PARAMETER_FILTER_ATTACK     3
#define SYNTH_PARAMETER_FILTER_DECAY      4
#define SYNTH_PARAMETER_FILTER_SHAPE      5
#define SYNTH_PARAMETER_FILTER_SUSTAIN    6



class MutatingSource
{
  public:
    MutatingSource();

    virtual int noteOn(byte pitch, byte velocity, unsigned int length);
    virtual int noteOff();
    virtual int updateAudio();
    virtual void updateControl();
    virtual int mutate();
    virtual void setGain(byte gain);
    virtual void setParam(int paramIndex, int newValue);
    virtual int getParam(int paramIndex);
    
    
  protected:
    int param[MAX_SOURCE_PARAMS];

    bool debugMode;
    EventDelay noteOffTimer;
    
};

class MutatingPhasor : public MutatingSource
{
  public:
    MutatingPhasor();

    int noteOn(byte pitch, byte velocity, unsigned int length);
    int noteOff();
    int updateAudio();
    void updateControl();
    void setGain(byte gain);
    void setParam(int paramIndex, int newValue);
    int getParam(int paramIndex);
    int mutate();

    void setOscillatorDetune(int newValue);
    void setFilterCutoff(int newValue);
    void setFilterResonance(int newValue);
    void setFilterAttack(int newValue);
    void setFilterDecay(int newValue);
    void setFilterShape(int newValue);

    int getOscillatorDetune();
    int getFilterCutoff();
    int getFilterResonance();
    int getFilterAttack();
    int getFilterDecay();
    int getFilterShape();


  protected:
  
    Phasor <AUDIO_RATE> aPhasor1;
    Phasor <AUDIO_RATE> aPhasor2;
    //LowPassFilter16 lowPassFilter;
    LowPassFilter lowPassFilter;
    ADSR <CONTROL_RATE, CONTROL_RATE> envelopeAmplitude;
    ADSR <CONTROL_RATE, CONTROL_RATE> envelopeFilter;
    
  private:
    float detunePct;
    float lastPitch;
    byte  lastNote;
    uint16_t lastNoteLength; 
    char  detuneSemis;
    byte  currentGain;
    byte  masterGain;


};





/* TODO: deleted these as space grew to be a problem, re-enable to add different synth voices if porting for larger/faster MCU 

class MutatingAcid : public MutatingSource
{
  public:
    MutatingAcid();

    int noteOn(byte pitch, byte velocity, unsigned int length);
    int noteOff();
    int updateAudio();
    void updateControl();
    int mutate();
    void setParam(int paramIndex, int newValue);
    int getParam(int paramIndex);
    
    
  protected:

    // for acid resomnant oscillator
    PDResonant voice;

};


class MutatingFM : public MutatingSource
{
  public:
    MutatingFM();

    int noteOn(byte pitch, byte velocity, unsigned int length);
    int noteOff();
    int updateAudio();
    void updateControl();
    int mutate();
    void setParam(int paramIndex, int newValue);
    int getParam(int paramIndex);
    
  protected:
    
    // for FM oscillator
    void setFreqs(Q16n16 freq);
    Q16n16 carrier_freq; 
    Q16n16 mod_freq;     
    Q16n16 deviation ;   

    Oscil<SIN2048_NUM_CELLS, AUDIO_RATE>*    aCarrier;//(SIN2048_DATA);
    //Oscil<SIN2048_NUM_CELLS, AUDIO_RATE>    aModulator(SIN2048_DATA);
    //Oscil<SIN2048_NUM_CELLS, CONTROL_RATE>  kModIndex(SIN2048_DATA);

    
};

*/

#endif
