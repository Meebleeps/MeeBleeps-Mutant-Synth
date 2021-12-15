/*----------------------------------------------------------------------------------------------------------
 * avSourcePhasor.cpp
 * 
 * Implements a 2-oscillator synth voice with low pass filter, volume envelope and filter envelope
 * 
 * (C) 2021 Meebleeps
*-----------------------------------------------------------------------------------------------------------
*/

#include <MozziGuts.h>
#include <mozzi_midi.h>
#include <mozzi_rand.h>
#include <mozzi_analog.h>
#include <mozzi_fixmath.h>
#include <Oscil.h>  // a template for an oscillator
#include <IntMap.h>
#include "avSource.h"
#include <AutoRange.h>

static int32_t minAudioRange = 0;
static int32_t maxAudioRange = 0;


/*----------------------------------------------------------------------------------------------------------
 * MutatingSource::MutatingSource()
 *  empty constructor for base class
 *----------------------------------------------------------------------------------------------------------
 */
MutatingSource::MutatingSource()
{
  // null constructor
}



/*----------------------------------------------------------------------------------------------------------
 * MutatingPhasor::MutatingPhasor()
 *  constructor
 *----------------------------------------------------------------------------------------------------------
 */
MutatingPhasor::MutatingPhasor()
{
  // phasor constructor;
  masterGain    = 255;
  lastNote      = 30;
  lastPitch     = mtof(lastNote);
  aPhasor1.setFreq(lastPitch);
  aPhasor2.setFreq(lastPitch);
  lowPassFilter.setCutoffFreqAndResonance(255,0);


  envelopeAmplitude.setTimes(0,50,100,100);
  envelopeAmplitude.setADLevels(255,200);
  envelopeFilter.setTimes(5,60,100,20);
  envelopeFilter.setADLevels(255,50);
}





/*----------------------------------------------------------------------------------------------------------
 * MutatingPhasor::noteOn()
 *  set up envelopes and start note at given midi note pitch
 *----------------------------------------------------------------------------------------------------------
 */
int MutatingPhasor::noteOn(byte pitch, byte velocity, unsigned int length)
{
  if (velocity > 0 && pitch > 0)
  {
    envelopeAmplitude.setTimes(5,60,length/2,length/2);
    envelopeAmplitude.setADLevels(255,200);
    envelopeAmplitude.noteOn(true); 


    // setup filter envelope attack, decay time based on parameters
    envelopeFilter.setADLevels(param[SYNTH_PARAMETER_FILTER_CUTOFF],min(param[SYNTH_PARAMETER_FILTER_SUSTAIN],param[SYNTH_PARAMETER_FILTER_CUTOFF]));
    envelopeFilter.setTimes(param[SYNTH_PARAMETER_FILTER_ATTACK],param[SYNTH_PARAMETER_FILTER_DECAY],length,50);
    envelopeFilter.noteOn(true);

    //todo: obsolete?
    noteOffTimer.start(length);

    lastNote  = pitch;
    lastPitch = mtof(pitch);
    lastNoteLength = length;

    aPhasor1.setFreq(lastPitch);
    aPhasor2.setFreq(mtof(pitch+detuneSemis) * detunePct);
    
  }
  else
  {
    noteOff();
  }
  return 0;
}


/*----------------------------------------------------------------------------------------------------------
 * MutatingPhasor::noteOff()
 * end current note
 *----------------------------------------------------------------------------------------------------------
 */
int MutatingPhasor::noteOff()
{
  noteOffTimer.start(0);
  envelopeAmplitude.noteOff(); 
  return 0;
}


/*----------------------------------------------------------------------------------------------------------
 * MutatingPhasor::setGain()
 * sets the current master gain.  use this for controlling the volume via external sequencer (eg ducking)
 *----------------------------------------------------------------------------------------------------------
 */
void MutatingPhasor::setGain(byte gain)
{
  masterGain = gain;
}


/*----------------------------------------------------------------------------------------------------------
 * MutatingPhasor::updateAudio()
 * returns the next audio sample value
 * TODO: super inefficient due to failed attempt to  debug an overflow problem somewhere
 * TODO:  fix audio glitch related to filter - sometimes gets into a state where output is highly distorted
 *        can usually be fixed by setting cutoff to zero but not always 
 *----------------------------------------------------------------------------------------------------------
 */
AudioOutput_t MutatingPhasor::updateAudio()
{
    int8_t asig1      = (int8_t)(aPhasor1.next()>>24);
    int8_t asig2      = (int8_t)(aPhasor2.next()>>24);
    int16_t mixed1     = (int)asig1-asig2;
    int16_t mixed2     = MonoOutput::fromNBit(9, mixed1);
    int16_t filtered  = lowPassFilter.next(mixed2);
    int16_t gained    =  currentGain * filtered;
    
    minAudioRange = min(minAudioRange, mixed2);
    maxAudioRange = max(maxAudioRange, mixed2);

    return MonoOutput::fromNBit(16,gained);
}



/*----------------------------------------------------------------------------------------------------------
 * MutatingPhasor::updateControl()
 * updates the envelopes and filter control
 *----------------------------------------------------------------------------------------------------------
 */
void MutatingPhasor::updateControl()
{
  envelopeAmplitude.update();
  envelopeFilter.update();

  lowPassFilter.setCutoffFreqAndResonance(envelopeFilter.next(), param[SYNTH_PARAMETER_FILTER_RESONANCE]);

  if (masterGain < 255)
  {
    currentGain = (envelopeAmplitude.next() * masterGain) >> 8; 
  }
  else
  {
    currentGain = envelopeAmplitude.next();  
  }
}



/*----------------------------------------------------------------------------------------------------------
 * MutatingPhasor::setOscillatorDetune()
 * sets the oscillator 2 mode and detune
 * 0 - 199 = octave below osc1
 * 200 - 399 = perfect 4th below osc1
 * 400 - 484 = fractional detune below osc1
 * 485 - 514 = osc2 = osc1 (same as osc2 off)
 * 515 - 599 = fractional detune above osc1
 * 600 - 799 = perfect 5th above osc1
 * 800 - 949 = octave above osc1
 * 950+      = octave + fifth above osc1
 *----------------------------------------------------------------------------------------------------------
 */
void MutatingPhasor::setOscillatorDetune(int newValue)
{
  
  if(newValue < 0) 
  {
    param[SYNTH_PARAMETER_OSC2TUNE] = 0;
  }
  else if (newValue > 1000)
  {
    param[SYNTH_PARAMETER_OSC2TUNE] = 1000;
  }
  else
  {
    param[SYNTH_PARAMETER_OSC2TUNE] = newValue;
  }


  if (param[SYNTH_PARAMETER_OSC2TUNE] < 200)
  {
    detunePct = 0.5;                                //octave down
  }
  else if (param[SYNTH_PARAMETER_OSC2TUNE] < 400)
  {
    detunePct = 0.75;                               //perfect 4th below note
  }
  else if (param[SYNTH_PARAMETER_OSC2TUNE] < 485)
  {
    detunePct = (1+((float)(param[0]-500) / 3000));   // detune
  }
  else if (param[SYNTH_PARAMETER_OSC2TUNE] < 515)
  {
    detunePct = 0;                                  // allow for single oscillator in the deadzone
  }
  else if (param[SYNTH_PARAMETER_OSC2TUNE] < 600)
  {
    detunePct = (1+((float)(param[0]-500) / 3000));   // detune
  }
  else if (param[SYNTH_PARAMETER_OSC2TUNE] < 800)
  {
    detunePct = 1.498307;                           //perfect 5th above note
  }
  else if (param[SYNTH_PARAMETER_OSC2TUNE] < 950)
  {
    detunePct = 2;                                  //octave up
  }
  else
  {
    detunePct = 2.996614;                           //octave up + 5th
  }


  aPhasor2.setFreq(mtof(lastNote+detuneSemis) * detunePct);
}



/*----------------------------------------------------------------------------------------------------------
 * MutatingPhasor::setFilterCutoff()
 * 
 *----------------------------------------------------------------------------------------------------------
 */
void MutatingPhasor::setFilterCutoff(int newValue)
{
  if (newValue > 2 && newValue < MAX_FILTER_CUTOFF && newValue != param[SYNTH_PARAMETER_FILTER_CUTOFF])
  {
    param[SYNTH_PARAMETER_FILTER_CUTOFF] = newValue;
    lowPassFilter.setCutoffFreqAndResonance(param[SYNTH_PARAMETER_FILTER_CUTOFF], param[SYNTH_PARAMETER_FILTER_RESONANCE]);
    Serial.print(F("cutoff="));
    Serial.println(param[SYNTH_PARAMETER_FILTER_CUTOFF]);
  }
 
}



/*----------------------------------------------------------------------------------------------------------
 * MutatingPhasor::setFilterResonance()
 * 
 *----------------------------------------------------------------------------------------------------------
 */
void MutatingPhasor::setFilterResonance(int newValue)
{
  if (newValue >= 0 && newValue < MAX_FILTER_RESONANCE && newValue != param[SYNTH_PARAMETER_FILTER_RESONANCE])
  {
    param[SYNTH_PARAMETER_FILTER_RESONANCE] = newValue;
    lowPassFilter.setCutoffFreqAndResonance(param[SYNTH_PARAMETER_FILTER_CUTOFF], param[SYNTH_PARAMETER_FILTER_RESONANCE]);
    Serial.print(F("resonance="));
    Serial.println(param[SYNTH_PARAMETER_FILTER_RESONANCE]);
  }
 
}


/*----------------------------------------------------------------------------------------------------------
 * MutatingPhasor::setFilterAttack()
 * 
 * doesn't do anything currently
 *----------------------------------------------------------------------------------------------------------
 */
void MutatingPhasor::setFilterAttack(int newValue)
{
  if (newValue >= 0 && newValue < MAX_FILTER_ENV_ATTACK && newValue != param[SYNTH_PARAMETER_FILTER_ATTACK])
  {
    param[SYNTH_PARAMETER_FILTER_ATTACK] = newValue;
    //TODO: adjust filter envelope attack
//    envelopeFilter.setADLevels(param[1],50);
//    envelopeFilter.setTimes(param[3]*16,param[4]*16,0,50);
    
  }
}



/*----------------------------------------------------------------------------------------------------------
 * MutatingPhasor::setFilterDecay()
 * 
 * doesn't do anything currently
 *----------------------------------------------------------------------------------------------------------
 */
void MutatingPhasor::setFilterDecay(int newValue)
{
  if (newValue >= 0 && newValue < MAX_FILTER_ENV_DECAY && newValue != param[SYNTH_PARAMETER_FILTER_DECAY])
  {
    param[SYNTH_PARAMETER_FILTER_DECAY] = newValue;
//    envelopeFilter.setADLevels(param[1],50);
//    envelopeFilter.setTimes(param[3]*16,param[4]*16,0,50);
  }
}



/*----------------------------------------------------------------------------------------------------------
 * MutatingPhasor::setFilterShape()
 * 
 * adjusts attack, decay and sustain level for the filter to provide 1-knob control of filter envelope
 *----------------------------------------------------------------------------------------------------------
 */
void MutatingPhasor::setFilterShape(int newValue)
{
  if (newValue >= 0 && newValue < MAX_FILTER_SHAPE && newValue != param[SYNTH_PARAMETER_FILTER_SHAPE])
  {
    // for first 60% of range, attack = 0
    // for second part of range, attack is linear
    if (newValue < 600)
    {
      param[SYNTH_PARAMETER_FILTER_ATTACK] = 0;
    }
    else
    {
      param[SYNTH_PARAMETER_FILTER_ATTACK] = (newValue - 600)*2;
    }

    // set decay
    param[SYNTH_PARAMETER_FILTER_DECAY] = newValue + MIN_FILTER_ENV_DECAY;

    // set sustain volume
    // for first 1/2 of range, sustain is cubic
    // for second 3/4 of range, sustain is maxxed
    // for last part sustain decreases from 255 to 128
    if (newValue < 512)
    {
      uint32_t p = pow(newValue,3) / 262144;
      param[SYNTH_PARAMETER_FILTER_SUSTAIN] = min(p,255);// (255.0 * (float)newValue) / 512;
    }
    else if (newValue < 768)
    {
      param[SYNTH_PARAMETER_FILTER_SUSTAIN] = 255;
    }
    else
    {
      param[SYNTH_PARAMETER_FILTER_SUSTAIN] = 255 + ((768 - newValue)/2);
    }

    param[SYNTH_PARAMETER_FILTER_SHAPE] = newValue;
  }
}



/*----------------------------------------------------------------------------------------------------------
 * MutatingPhasor::setParam()
 * 
 * generic interface for controlling parameters for polymorphic source design
 *----------------------------------------------------------------------------------------------------------
 */
void MutatingPhasor::setParam(int paramIndex, int newValue)
{
  switch(paramIndex)
  {

    case SYNTH_PARAMETER_OSC2TUNE:          setOscillatorDetune(newValue);
    case SYNTH_PARAMETER_FILTER_CUTOFF:     setFilterCutoff(newValue);
    case SYNTH_PARAMETER_FILTER_RESONANCE:  setFilterResonance(newValue);
    case SYNTH_PARAMETER_FILTER_ATTACK:     setFilterDecay(newValue);
    case SYNTH_PARAMETER_FILTER_DECAY:      setFilterAttack(newValue);
    case SYNTH_PARAMETER_FILTER_SHAPE:      setFilterShape(newValue);
  }
  
}


/*----------------------------------------------------------------------------------------------------------
 * MutatingPhasor::getParam()
 * 
 * generic interface for getting parameters for polymorphic source design
 *----------------------------------------------------------------------------------------------------------
 */
int MutatingPhasor::getParam(int paramIndex)
{
  if (paramIndex > 0 && paramIndex < MAX_SOURCE_PARAMS)
  {
    return param[paramIndex];
  }
  else
  {
    return 0;
  }
}


/*----------------------------------------------------------------------------------------------------------
 * MutatingPhasor::mutate()
 * 
 * generic interface for randomising parameters for polymorphic source design
 *----------------------------------------------------------------------------------------------------------
 */
int MutatingPhasor::mutate()
{
  for (int i = 0; i < MAX_SOURCE_PARAMS; i++)
  {
    setParam(i, rand(100) );
  }
  
  return 0;
}
