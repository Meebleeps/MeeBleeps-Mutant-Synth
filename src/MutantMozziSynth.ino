/*----------------------------------------------------------------------------------------------------------
 * MutantMozziSynth.ino
 * 
 * Main project file for the Mutant Mozzi Synth. 
 * 
 * Instantiates all mutant class objects 
 * Calls the Mozzi audio hook functions
 * manages the user interface
 * 
 * was originally developed in Arduino IDE before moving to VS Code
 * 
 * (C) 2021 Meebleeps
*-----------------------------------------------------------------------------------------------------------
*/

#include <Arduino.h>
#include <MozziGuts.h>
#include <mozzi_rand.h>
#include <EventDelay.h>

#include "avSource.h"
#include "avSequencer.h"
#include "LedMatrix.h"

// MOZZI variables
#define CONTROL_RATE 256

// define the pins
#define PIN_SYNC_IN       8
#define PIN_SYNC_OUT      2
#define TIMER_SYNC_PULSE_OUTPUT_MILLIS 20

#define PIN_AUDIO_OUT     9
#define PIN_BUTTON0       4
#define PIN_BUTTON1       5
#define PIN_BUTTON2       6
#define PIN_BUTTON3       3
#define PIN_BUTTON4       10
#define PIN_BUTTON5       12
#define MAX_BUTTON_INPUTS 6


#define MAX_ANALOG_INPUTS               6
#define MAX_ANALOG_VALUE                1000

// these may change depending on how analog pots are wired up
#define ANALOG_INPUT_OSC2TUNE           0
#define ANALOG_INPUT_DECAY              1
#define ANALOG_INPUT_MUTATION           2
#define ANALOG_INPUT_FILTERSHAPE        3
#define ANALOG_INPUT_FILTER             4
#define ANALOG_INPUT_STEPCOUNT          5

// trial-and-error value to raise movement-detection above noise.  
//TODO: I'm sure there are libraries to do this!
#define ANALOG_INPUT_MOVEMENT_THRESHOLD 13

// listed in order of left to right.
#define BUTTON_INPUT_FUNC   0
#define BUTTON_INPUT_TONIC  3
#define BUTTON_INPUT_SCALE  2
#define BUTTON_INPUT_START  1
#define BUTTON_INPUT_REC    5
#define BUTTON_INPUT_RETRIG 4


//#define MAX_OSCILLATOR_MODES 1

#define INTERFACE_MODE_NORMAL 0
#define INTERFACE_MODE_SHIFT  1

// some settings persist on-screen after being changed - for this many millis
#define DISPLAY_SETTING_CHANGE_PERSIST_MILLIS 500

// motion recordind mode, depending on whether the REC button is held with/without FUNC
#define MOTION_RECORD_NONE  0
#define MOTION_RECORD_REC   1
#define MOTION_RECORD_CLEAR 2

// decrease INTERFACE_UPDATE_DIVIDER  if controls are externally modulated via CV voltage inputs
#define INTERFACE_UPDATE_DIVIDER 4

//TODO: store these in PROGMEM instead of RAM
const byte BITMAP_MEEBLEEPS[]  = {B00011000,B01111110,B11011011,B11011011,B11011011,B11011011,B11000011,B00000110};
const byte BITMAP_NUMERALS[8][8]  = {
                                       {B00000000,B00000000,B00000000,B11100000,B10100000,B10100000,B10100000,B11100111}
                                      ,{B00000000,B00000000,B00000000,B01000000,B01000000,B01000000,B01000111,B01000000}
                                      ,{B00000000,B00000000,B00000000,B11100000,B00100000,B11100111,B10000000,B11100000}
                                      ,{B00000000,B00000000,B00000000,B11100000,B00100111,B11100000,B00100000,B11100000}
                                      ,{B00000000,B00000000,B00000000,B10100111,B10100000,B11100000,B00100000,B00100000}
                                      ,{B00000000,B00000000,B00000111,B11100000,B10000000,B11100000,B00100000,B11100000}
                                      ,{B00000000,B00000111,B00000000,B11100000,B10000000,B11100000,B10100000,B11100000}
                                      ,{B00000000,B00000111,B00000000,B11100000,B00100000,B00100000,B00100000,B00100000}
                                    };
const byte BITMAP_ALPHA[7][8]  = {
                                     {B00000000,B00000000,B00000000,B11100000,B10100000,B11100000,B10100000,B10100111}
                                    ,{B00000000,B00000000,B00000000,B11100000,B10100000,B11000000,B10100111,B11100000}
                                    ,{B00000000,B00000000,B00000000,B11100000,B10000000,B10000111,B10000000,B11100000}
                                    ,{B00000000,B00000000,B00000000,B11000000,B10100111,B10100000,B10100000,B11000000}
                                    ,{B00000000,B00000000,B00000000,B11100111,B10000000,B11000000,B10000000,B11100000}
                                    ,{B00000000,B00000000,B00000111,B11100000,B10000000,B11000000,B10000000,B10000000}
                                    ,{B00000000,B00000111,B00000000,B11100000,B10000000,B10000000,B10100000,B11100000}
                                  };
const byte BITMAP_ALGORITHMS[2][8]  = {
                                        {B01000001,B10010000,B00000100,B01010000,B01000010,B01001000,B01000001,B01000100}
                                      , {B00100001,B01000010,B00000100,B11101000,B00100000,B11100001,B10000010,B11100100}
                                      };
/* old bitmaps
const byte BITMAP_NUMERALS[8][8]  = {
                                      {B00111000,B01000100,B01000100,B01000100,B01000100,B01000100,B00111000,B00000000}
                                      ,{B00010000,B00110000,B00010000,B00010000,B00010000,B00010000,B00111000,B00000000}
                                      ,{B00111000,B01000100,B00000100,B00001000,B00010000,B00100000,B01111100,B00000000}
                                      ,{B00111000,B01000100,B00000100,B00011000,B00000100,B01000100,B00111000,B00000000}
                                      ,{B00001000,B00011000,B00101000,B01001000,B01111100,B00001000,B00001000,B00000000}
                                      ,{B01111100,B01000000,B01000000,B00111000,B00000100,B00000100,B01111000,B00000000}
                                      ,{B00111100,B01000000,B01000000,B00111000,B01000100,B01000100,B00111000,B00000000}
                                      ,{B00111000,B01000100,B01000100,B00111000,B01000100,B01000100,B00111000,B00000000}
                                    };
*/

/*
const byte BITMAP_ALPHA[7][8]  = {
                                       {B00111000,B01000100,B01000100,B01111100,B01000100,B01000100,B01000100,B00000000}
                                      ,{B01111000,B01000100,B01000100,B01111000,B01000100,B01000100,B01111000,B00000000}
                                      ,{B00111000,B01000100,B01000000,B01000000,B01000000,B01000100,B00111000,B00000000}
                                      ,{B01111000,B01000100,B01000100,B01000100,B01000100,B01000100,B01111000,B00000000}
                                      ,{B01111100,B01000000,B01000000,B01111000,B01000000,B01000000,B01111100,B00000000}
                                      ,{B01111100,B01000000,B01000000,B01111000,B01000000,B01000000,B01000000,B00000000}
                                      ,{B00111000,B01000100,B01000000,B01001100,B01000100,B01000100,B00111000,B00000000}
                                    };
*/

// when user presses start for the first time, do things like seed rand generator and clear screen
byte firstTimeStart   = true;

// state of sync trigger input pin
byte iTrigger         = 1;      
byte iLastTrigger     = 0;

// state of button input pins
byte iCurrentButton[MAX_BUTTON_INPUTS];
byte iLastButton[MAX_BUTTON_INPUTS];

// state of analog input pins
int iLastAnalogValue[6]    = {0,0,0,0,0,0};
int iCurrentAnalogValue[6] = {0,0,0,0,0,0};

// used during updateControl to reduce frequency of input control updates for efficiency
byte updateCounter    = 0;

// state of interface and motion control UI modifiers
byte interfaceMode    = INTERFACE_MODE_NORMAL;
byte motionRecordMode = MOTION_RECORD_NONE;

// a bitmap setting is being displayed
byte settingDisplayIconOn = false;

//TODO: this should probably be in the sequencer
uint8_t lastStepParameterLock[6] = {false,false,false,false,false,false};

// synth voice instance
MutatingPhasor    synthVoice;

// generative sequencer instance
MutatingSequencer sequencer;

// LED matrix instance.  probably could have used a standard MAX7219 library but was easy and enjoyable to make my own :)
LedMatrix         ledDisplay;

// timer to pull sync output low at the end of the pulse 
EventDelay        syncOutputTimer;
EventDelay        settingDisplayTimer;


/*----------------------------------------------------------------------------------------------------------
 * setup()
 *----------------------------------------------------------------------------------------------------------
 */
void setup() 
{
  Serial.begin(115200);
  Serial.print(F("\n\n\n\n\n-----------------------------------------------------------------------\n"));
  Serial.print(F("Mozzi-based Mutating Techno Synth.   Build "));
  Serial.print  (__DATE__);
  Serial.print  (F(" "));
  Serial.println  (__TIME__);

  initialisePins();
  initialiseDisplay();
  initialiseSources();
  initialiseSequencer();

  startMozzi(CONTROL_RATE);
}



/*----------------------------------------------------------------------------------------------------------
 * initialiseDisplay
 *----------------------------------------------------------------------------------------------------------
 */
void initialiseDisplay()
{
  ledDisplay.initialise();
  ledDisplay.setOrientation(LEDMATRIX_ORIENTATION_0);
  
  settingDisplayTimer.start(0);
  settingDisplayIconOn = false;

  // display logo intro
  for (int i=10; i< 100; i+=10)
  {
    ledDisplay.clearScreen();
    delay(i);
    ledDisplay.displayIcon(BITMAP_MEEBLEEPS);
    delay(i);
  }
  
  // fade
  for(int i = 15; i>= 0; i--)
  {
    ledDisplay.setIntensity(i);
    delay(20);
  }

  // set default brightness to 4 for normal use
  ledDisplay.setIntensity(4);
  ledDisplay.displayIcon(BITMAP_MEEBLEEPS);
}




/*----------------------------------------------------------------------------------------------------------
 * initialisePins
 *----------------------------------------------------------------------------------------------------------
 */
void initialisePins()
{
  Serial.println(F("initialisePins()"));


  pinMode(PIN_SYNC_OUT, OUTPUT);
  pinMode(PIN_SYNC_IN, INPUT);          // sync is pulled down by external pulldown resistor
  pinMode(PIN_BUTTON0, INPUT_PULLUP);   // all other buttons are pulled up by internal pullup resistor
  pinMode(PIN_BUTTON1, INPUT_PULLUP);
  pinMode(PIN_BUTTON2, INPUT_PULLUP);
  pinMode(PIN_BUTTON3, INPUT_PULLUP);
  pinMode(PIN_BUTTON4, INPUT_PULLUP);
  pinMode(PIN_BUTTON5, INPUT_PULLUP);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT); 

  // initialise button state arrays
  for (int i = 0; i < MAX_BUTTON_INPUTS; i++)
  {
    iLastButton[i]    = 0;
    iCurrentButton[i] = 0;
  }

  for (int i = 0; i < MAX_ANALOG_INPUTS; i++)
  {
    iLastAnalogValue[i] = 0;
  }
}



/*----------------------------------------------------------------------------------------------------------
 * initialiseSources
 * Do anything necessary to initialise the synth voice/s that wasn't done in the constructor
 *----------------------------------------------------------------------------------------------------------
 */
void initialiseSources()
{
  
  //TODO: anything necessary to initialise the synth voice/s that wasn't done in the constructor
  
}



/*----------------------------------------------------------------------------------------------------------
 * initialiseSequencer
 * set the scalemode to minor and creates a new sequence
 *----------------------------------------------------------------------------------------------------------
 */
void initialiseSequencer()
{
  sequencer.setScale(SCALEMODE_MINOR);
  sequencer.newSequence(16);
}




/*----------------------------------------------------------------------------------------------------------
 * updateControl
 * called by Mozzi Library on CONTROL_RATE frequency
 * every INTERFACE_UPDATE_DIVIDER steps, check controls
 * every step - check for sync trigger and update sequencer
 * if this gets delayed, note on/off commands will drift
 *----------------------------------------------------------------------------------------------------------
 */
void updateControl()
{
  updateCounter++;

  // do this every time to ensure sync
  updateSyncTrigger();

  // check controls once every INTERFACE_UPDATE_DIVIDER steps for efficiency
  // todo:  decrease INTERFACE_UPDATE_DIVIDER  if these are externally modulated via CV voltage inputs
  if(updateCounter % INTERFACE_UPDATE_DIVIDER == 0)
  {

    // check button controls first to ensure interface mode is correctly set
    updateButtonControls();

    // now check analog controls
    updateAnalogControls();
  }

  // update sequencer every time to minimise latency
  // returns true if the seqeuncer has moved to next step
  if (updateSequencer())
  {
     //only update the display once per sequencer step    
     //TODO: this could be increased for more responsive display
    updateDisplay();
  }
  
  // check to see if the output sync pulse needs to be pulled low
  if (syncOutputTimer.ready())
  {
    digitalWrite(PIN_SYNC_OUT, LOW);
  }

  
  // now that all controls are updated, update the source.
  
  
  // sequencer sets the ducking amount based on internal envelope
  long duckingAmount = sequencer.getDuckingAmount();
  
  
  if (duckingAmount > 0)
  {
    long duckingGain;

    duckingGain = sequencer.getDuckingEnvelope();

    if (duckingGain < 250)
    {
      duckingGain = ((duckingGain*duckingAmount) >> 8) + (256-duckingAmount);
      
      // display the ducking gain reduction as a bar on the LED matrix
      ledDisplay.drawLineV(6, 0, 6,true);
      ledDisplay.drawLineV(6, 0, scaleAnalogInput( duckingGain*4,6),false);
    }
    else
    {
      duckingGain = 255;
    }
    
    synthVoice.setGain(duckingGain);
  }
  else
  {
    synthVoice.setGain(255);
  }

  synthVoice.updateControl();
}


/*----------------------------------------------------------------------------------------------------------
 * updateSequencer
 * returns true if sequencer advances to the next step
 *----------------------------------------------------------------------------------------------------------
 */
int updateSequencer()
{
  byte      nextNote;
  uint16_t  nextNoteLength;
  
  // if the sequencer is due to make a new step, 
  if (sequencer.update(false))
  {
    // if the sequencer is on an even step - output a sync pulse
    if (sequencer.outputSyncPulse())
    {
      outputSyncPulse();
    }
    
    nextNote        = sequencer.getCurrentNote();
    nextNoteLength  = sequencer.getNextNoteLength();
    
    if (nextNote > 0)
    {
      updateSequencerStepModulation();

      synthVoice.noteOn(nextNote, 255, nextNoteLength);
    }
    
    return true;
  }
  else
  {
    return false;
  }

  
}


/*----------------------------------------------------------------------------------------------------------
 * outputSyncPulse
 * sends a digital signal to the output pin and sets the timer to pull it low after TIMER_SYNC_PULSE_OUTPUT_MILLIS
 *----------------------------------------------------------------------------------------------------------
 */
void outputSyncPulse()
{
  syncOutputTimer.start(TIMER_SYNC_PULSE_OUTPUT_MILLIS);
  digitalWrite(PIN_SYNC_OUT,  HIGH);
}


/*----------------------------------------------------------------------------------------------------------
 * updateSequencerModulation
 * updates source parameters based on current control values or recorded motion (if any) 
 *----------------------------------------------------------------------------------------------------------
 */
void updateSequencerStepModulation()
{
 
  if (sequencer.getParameterLock(0) != 0)
  {
    sequencer.setNextNoteLength(sequencer.getParameterLock(0));
    lastStepParameterLock[0] = true;
  }
  else if (lastStepParameterLock[0])  // if the last step was parameter locked, then reset to the current knob position
  {
    updateNoteDecay(true);
    lastStepParameterLock[0] = false;
  }

  if (sequencer.getParameterLock(1) != 0)
  {
    synthVoice.setOscillatorDetune(sequencer.getParameterLock(1));
    lastStepParameterLock[1] = true;
  }
  else if (lastStepParameterLock[1])  // if the last step was parameter locked, then reset to the current knob position
  {
    updateOscillatorDetune(true);
    lastStepParameterLock[1] = false;
  }

  if (sequencer.getParameterLock(2) != 0)
  {
    synthVoice.setFilterCutoff(sequencer.getParameterLock(2));
    lastStepParameterLock[2] = true;
  }
  else if (lastStepParameterLock[2])  // if the last step was parameter locked, then reset to the current knob position
  {
    updateFilterCutoff(true);
    lastStepParameterLock[1] = false;
  }

  if (sequencer.getParameterLock(3) != 0)
  {
    synthVoice.setFilterResonance(sequencer.getParameterLock(3));
    lastStepParameterLock[3] = true;
  }
  else if (lastStepParameterLock[3])  // if the last step was parameter locked, then reset to the current knob position
  {
    updateFilterResonance(true);
    lastStepParameterLock[3] = false;
  }

  if (sequencer.getParameterLock(4) != 0)
  {
    synthVoice.setFilterShape(sequencer.getParameterLock(4));
    lastStepParameterLock[4] = true;    
  }
  else if (lastStepParameterLock[4])  // if the last step was parameter locked, then reset to the current knob position
  {
    updateFilterShape(true);
    lastStepParameterLock[4] = false;
  }
}




/*----------------------------------------------------------------------------------------------------------
 * updateSyncTrigger
 * check for sync trigger and update sequencer
 *----------------------------------------------------------------------------------------------------------
 */
void updateSyncTrigger()
{
  iTrigger = digitalRead(PIN_SYNC_IN);   // read the sync pin

  if (iTrigger != iLastTrigger)
  {
    digitalWrite(PIN_SYNC_OUT, iTrigger);
    
    //trigger note on or off
    if (iTrigger == HIGH)
    {
      //Serial.println("sync!");
      sequencer.syncPulse(SYNC_STEPS_PER_PULSE);
    }

    iLastTrigger = iTrigger;
  }
}


/*----------------------------------------------------------------------------------------------------------
 * updateButtonControls
 * check for button presses
 *----------------------------------------------------------------------------------------------------------
 */
void updateButtonControls()
{
  //  IMPORTANT:
  //  These calls assume you are using NORMALLY CLOSED switches, which is what I used in the build
  //  if your switches are NORMALLY OPEN delete the definition of SWITCH_TYPE_NORMALLY_CLOSED
  #define SWITCH_TYPE_NORMALLY_CLOSED

  #ifdef SWITCH_TYPE_NORMALLY_CLOSED
  iCurrentButton[0] = digitalRead(PIN_BUTTON0);   // read the button 0 pin
  iCurrentButton[1] = digitalRead(PIN_BUTTON1);   // read the button 1 pin
  iCurrentButton[2] = digitalRead(PIN_BUTTON2);   // read the button 2 pin
  iCurrentButton[3] = digitalRead(PIN_BUTTON3);   // read the button 3 pin
  iCurrentButton[4] = digitalRead(PIN_BUTTON4);   // read the button 3 pin
  iCurrentButton[5] = digitalRead(PIN_BUTTON5);   // read the button 3 pin
  #else
  iCurrentButton[0] = !digitalRead(PIN_BUTTON0);   // read the button 0 pin
  iCurrentButton[1] = !digitalRead(PIN_BUTTON1);   // read the button 1 pin
  iCurrentButton[2] = !digitalRead(PIN_BUTTON2);   // read the button 2 pin
  iCurrentButton[3] = !digitalRead(PIN_BUTTON3);   // read the button 3 pin
  iCurrentButton[4] = !digitalRead(PIN_BUTTON4);   // read the button 3 pin
  iCurrentButton[5] = !digitalRead(PIN_BUTTON5);   // read the button 3 pin
  #endif


  // if func button is not pressed, all UI controls are normal
  if (iCurrentButton[BUTTON_INPUT_FUNC] != iLastButton[BUTTON_INPUT_FUNC])
  {
    if(iCurrentButton[BUTTON_INPUT_FUNC] == LOW)
    {
      interfaceMode = INTERFACE_MODE_NORMAL;
    }
    else
    {
      interfaceMode = INTERFACE_MODE_SHIFT;
    }
    Serial.print(interfaceMode);
  }


  if (iCurrentButton[BUTTON_INPUT_REC] != iLastButton[BUTTON_INPUT_REC])
  {
    if (iCurrentButton[BUTTON_INPUT_REC] == LOW)
    {
        motionRecordMode = MOTION_RECORD_NONE;
    }
    else
    {
      switch(interfaceMode)
      {
        case INTERFACE_MODE_NORMAL:   
          motionRecordMode = MOTION_RECORD_REC;
          break;

        case INTERFACE_MODE_SHIFT:    
          motionRecordMode = MOTION_RECORD_CLEAR;
          break;
      }
    }
    Serial.print(F("motionRecordMode="));
    Serial.println(motionRecordMode);
  }


  if (iCurrentButton[BUTTON_INPUT_RETRIG] != iLastButton[BUTTON_INPUT_RETRIG])
  {
    switch(interfaceMode)
    {
      case INTERFACE_MODE_NORMAL:   
        sequencer.setRetrigger(iCurrentButton[BUTTON_INPUT_RETRIG] == HIGH);
        break;
      case INTERFACE_MODE_SHIFT:    
        sequencer.setRetrigger(iCurrentButton[BUTTON_INPUT_RETRIG] == HIGH); 
        break;
    }
  }
  
  if (iCurrentButton[BUTTON_INPUT_START] != iLastButton[BUTTON_INPUT_START] && iCurrentButton[BUTTON_INPUT_START] == HIGH)
  {
    switch(interfaceMode)
    {
      case INTERFACE_MODE_NORMAL:   
        startStopSequencer();     
        break;
      case INTERFACE_MODE_SHIFT:    
        // send tap tempo message to the sequencer
        // todo: average the last 
        sequencer.syncPulse(SYNC_STEPS_PER_TAP);  
        break;
    }
  }

  if (iCurrentButton[BUTTON_INPUT_TONIC] != iLastButton[BUTTON_INPUT_TONIC] && iCurrentButton[BUTTON_INPUT_TONIC] == HIGH)
  {
    switch(interfaceMode)
    {
      case INTERFACE_MODE_NORMAL:   
        updateTonic(1);   
        break;
      case INTERFACE_MODE_SHIFT:    
        updateTonic(-1);
        break;
    }
  }

  if (iCurrentButton[BUTTON_INPUT_SCALE] != iLastButton[BUTTON_INPUT_SCALE] && iCurrentButton[BUTTON_INPUT_SCALE] == HIGH)
  {
    switch(interfaceMode)
    {
      case INTERFACE_MODE_NORMAL:   
        // todo: update scale 
        updateScale();
        break;
      case INTERFACE_MODE_SHIFT:    
        updateAlgorithm();
        break;
    }
  }


  for (int i=0; i<MAX_BUTTON_INPUTS; i++)
  {
    //for debug only
    /*
    if (iLastButton[i] != iCurrentButton[i])
    {
      Serial.print(F("Button state changed - button["));
      Serial.print(i);
      Serial.print(F("] state = "));
      Serial.println(iCurrentButton[i]);

      ledDisplay.setPixel(7, i+1, iCurrentButton[i] == HIGH);
    }
    */
    iLastButton[i] = iCurrentButton[i];
  }

  ledDisplay.refresh();

}




/*----------------------------------------------------------------------------------------------------------
 * updateAnalogControls
 * check for knob movements
 *----------------------------------------------------------------------------------------------------------
 */
void updateAnalogControls()
{
  int rawValue[MAX_ANALOG_INPUTS];

  for (int i = 0; i < MAX_ANALOG_INPUTS; i++)
  {
    rawValue[i] = mozziAnalogRead(A0 + i);
    //rudimentary smoothing - average of this reading and the last 2.
    iCurrentAnalogValue[i] = (iCurrentAnalogValue[i] + rawValue[i] + iLastAnalogValue[i])/3;
  }

  
  for (int i=0; i < MAX_ANALOG_INPUTS; i++)
  {
    if (analogInputHasChanged(i))
    {
      switch(i)
      {
        case ANALOG_INPUT_DECAY: 
          updateNoteDecay(false);
          break;

        case ANALOG_INPUT_OSC2TUNE:
          switch (interfaceMode)
          {
            case INTERFACE_MODE_NORMAL: 
              updateOscillatorDetune(false);
              break;

            case INTERFACE_MODE_SHIFT:  
              if (motionRecordMode == MOTION_RECORD_CLEAR)
              {
                // if user is clearing motion record, clear the detune because the ducking isn't recordable
                updateOscillatorDetune(false);
                Serial.print("clearing updateOscillatorDetune");
              }
              else
              {
                sequencer.setDuckingAmount(scaleAnalogInput(iCurrentAnalogValue[ANALOG_INPUT_OSC2TUNE],255));
              }
              break;
          }
          break;
        
          
        case ANALOG_INPUT_FILTER:
          switch (interfaceMode)
          {
            case INTERFACE_MODE_NORMAL: 
              updateFilterCutoff(false);
              break;
            case INTERFACE_MODE_SHIFT:  
              updateFilterResonance(false);
              break;
          }
          break;
        
        case ANALOG_INPUT_MUTATION:
          switch (interfaceMode)
          {
            case INTERFACE_MODE_NORMAL: 
              sequencer.setMutationProbability(scaleAnalogInputNonLinear(iCurrentAnalogValue[ANALOG_INPUT_MUTATION],512,20,100));
              break;
            case INTERFACE_MODE_SHIFT:  
              sequencer.setNoteProbability(scaleAnalogInput(iCurrentAnalogValue[ANALOG_INPUT_MUTATION],100));
              break;
          }
          break;

        case ANALOG_INPUT_STEPCOUNT:
          updateStepcount();
          break;

        case ANALOG_INPUT_FILTERSHAPE:
          updateFilterShape(false);
          break;
      }
      
      iLastAnalogValue[i] = iCurrentAnalogValue[i];
    }
  }

}



/*----------------------------------------------------------------------------------------------------------
 * startStopSequencer
 * starts the sequencer if stopped, or stops it if started
 *----------------------------------------------------------------------------------------------------------
 */
void startStopSequencer()
{

  byte nextNote;
  uint16_t nextNoteLength;

  if (firstTimeStart)
  {
    ledDisplay.clearScreen();
    sequencer.newSequence(16);
    firstTimeStart = false;
  }

  sequencer.toggleStart();

  // if the sequencer is due to make a new step, 
  if (sequencer.update(true))
  {
    nextNote        = sequencer.getCurrentNote();
    nextNoteLength  = sequencer.getNextNoteLength();
    
    if (nextNote > 0)
    {
      updateSequencerStepModulation();

      synthVoice.noteOn(nextNote, 255, nextNoteLength);
    }
    updateDisplay();
  }

}


/*----------------------------------------------------------------------------------------------------------
 * updateAlgorithm
 * updates the sequencer mutation algorithm
 *----------------------------------------------------------------------------------------------------------
 */
void updateAlgorithm()
{
  sequencer.nextAlgorithm();
  displaySettingIcon(BITMAP_ALGORITHMS[sequencer.getAlgorithm()]);
}


/*----------------------------------------------------------------------------------------------------------
 * updateScale
 * updates the sequencer's musical scale
 *----------------------------------------------------------------------------------------------------------
 */
void updateScale()
{
  sequencer.setScale((sequencer.getScale() + 1) % MAX_SCALE_COUNT);
  displaySettingIcon(BITMAP_NUMERALS[sequencer.getScale()]);
}


/*----------------------------------------------------------------------------------------------------------
 * updateDuckingAmount
 * sets the sequencer ducking intensity based on current input control value
 *----------------------------------------------------------------------------------------------------------
 */
void updateDuckingAmount()
{
  sequencer.setDuckingAmount(scaleAnalogInput(iCurrentAnalogValue[ANALOG_INPUT_OSC2TUNE],255));
}


/*----------------------------------------------------------------------------------------------------------
 * updateNoteDecay
 * sets the synth voice note duration 
 * if the current step has a parameter lock, use that value
 * otherwise use the current control input value and if the motionRecordMode == MOTION_RECORD_REC, record the 
 * current value as a parameter lock on this step 
 *----------------------------------------------------------------------------------------------------------
 */
void updateNoteDecay(bool ignoreRecordMode)
{
  uint16_t nextNoteLength;
  
  nextNoteLength = scaleAnalogInputNonLinear(iCurrentAnalogValue[ANALOG_INPUT_DECAY], 768, 2048, 10000);
  
  sequencer.setNextNoteLength(nextNoteLength);
  
  if (!ignoreRecordMode)
  {

    switch (motionRecordMode)
    {
      case MOTION_RECORD_REC:
          sequencer.setParameterLock(0, nextNoteLength);
          Serial.println(F("setParameterLock(0)"));
          break;

      case MOTION_RECORD_CLEAR:
          sequencer.clearAllParameterLocks(0);
          //sequencer.setParameterLock(0, 0);
          Serial.println(F("Clear ParameterLock(0)"));
          break;
      
      default:
        // no action if motion recording mode isn't record or clear
        break;
    }
  }
}




/*----------------------------------------------------------------------------------------------------------
 * updateOscillatorDetune
 * if the current step has a parameter lock, use that value
 * otherwise use the current control input value and if the motionRecordMode == MOTION_RECORD_REC, record the 
 * current value as a parameter lock on this step 
 *----------------------------------------------------------------------------------------------------------
 */
void updateOscillatorDetune(bool ignoreRecordMode)
{
  int newValue;

  newValue = scaleAnalogInput(iCurrentAnalogValue[ANALOG_INPUT_OSC2TUNE],1000);

  if (newValue != synthVoice.getParam(0))
  
  synthVoice.setOscillatorDetune(newValue);  

  if (!ignoreRecordMode)
  {
    switch (motionRecordMode)
    {
      case MOTION_RECORD_REC:
          sequencer.setParameterLock(1, newValue);
          Serial.println(F("setParameterLock(1)"));
          break;

      case MOTION_RECORD_CLEAR:
          sequencer.clearAllParameterLocks(1);

          //sequencer.setParameterLock(1, 0);
          Serial.println(F("setParameterLock(1,0)"));
          break;
      
      default:
        // no action if motion recording mode isn't record or clear
        break;
    }
  }
}



/*----------------------------------------------------------------------------------------------------------
 * updateFilterCutoff
 * if the current step has a parameter lock, use that value
 * otherwise use the current control input value and if the motionRecordMode == MOTION_RECORD_REC, record the 
 * current value as a parameter lock on this step 
 *----------------------------------------------------------------------------------------------------------
 */
void updateFilterCutoff(bool ignoreRecordMode)
{
  int newValue;

  newValue = scaleAnalogInput(iCurrentAnalogValue[ANALOG_INPUT_FILTER],MAX_FILTER_CUTOFF);
  synthVoice.setFilterCutoff(newValue);  

  if (!ignoreRecordMode)
  {

    switch (motionRecordMode)
    {
      case MOTION_RECORD_REC:
          sequencer.setParameterLock(2, newValue);
          Serial.println(F("updateFilterCutoff setParameterLock(2)"));
          break;

      case MOTION_RECORD_CLEAR:
          sequencer.clearAllParameterLocks(2);
          //sequencer.setParameterLock(2, 0);
          Serial.println(F("updateFilterCutoff clearParameterLock(2)"));
          break;
      
      default:
        // no action if motion recording mode isn't record or clear
        break;
    }  
  }
}



/*----------------------------------------------------------------------------------------------------------
 * updateFilterResonance
  * if the current step has a parameter lock, use that value
 * otherwise use the current control input value and if the motionRecordMode == MOTION_RECORD_REC, record the 
 * current value as a parameter lock on this step 
*----------------------------------------------------------------------------------------------------------
 */
void updateFilterResonance(bool ignoreRecordMode)
{
  int newValue;

  newValue = scaleAnalogInput(iCurrentAnalogValue[ANALOG_INPUT_FILTER],MAX_FILTER_RESONANCE);
  synthVoice.setFilterResonance(newValue);  
  
  if (!ignoreRecordMode)
  {
    switch (motionRecordMode)
    {
      case MOTION_RECORD_REC:
          sequencer.setParameterLock(3, newValue);
          Serial.println(F("updateFilterResonance setParameterLock(3)"));
          break;

      case MOTION_RECORD_CLEAR:
          sequencer.clearAllParameterLocks(3);
          // also clear filter cutoff as this uses the same input 
          sequencer.clearAllParameterLocks(2);
          //sequencer.setParameterLock(3, 0);
          Serial.println(F("updateFilterResonance clearParameterLock(3)"));
          break;
      
      default:
        // no action if motion recording mode isn't record or clear
        break;
    }  
  }
}


/*----------------------------------------------------------------------------------------------------------
 * updateFilterShape
 * if the current step has a parameter lock, use that value
 * otherwise use the current control input value and if the motionRecordMode == MOTION_RECORD_REC, record the 
 * current value as a parameter lock on this step 
 *----------------------------------------------------------------------------------------------------------
 */
void updateFilterShape(bool ignoreRecordMode)
{
  int newValue;

  newValue = scaleAnalogInput(iCurrentAnalogValue[ANALOG_INPUT_FILTERSHAPE],1023);
  synthVoice.setFilterShape(newValue);  

  if (!ignoreRecordMode)
  {

    switch (motionRecordMode)
    {
      case MOTION_RECORD_REC:
          sequencer.setParameterLock(4, newValue);
          Serial.println(F("setParameterLock(4)"));
          break;

      case MOTION_RECORD_CLEAR:
          sequencer.clearAllParameterLocks(4);
          //sequencer.setParameterLock(4, 0);
          Serial.println(F("clearParameterLock(4)"));
          break;
      
      default:
        // no action if motion recording mode isn't record or clear
        break;
    }  
  }
}





/*----------------------------------------------------------------------------------------------------------
 * updateNoteOrStepcount
 * use the current control input value to set the sequence length
 *----------------------------------------------------------------------------------------------------------
 */
void updateStepcount()
{
  sequencer.setSequenceLength(scaleAnalogInput(iCurrentAnalogValue[ANALOG_INPUT_STEPCOUNT],15) + 1);
}



/*----------------------------------------------------------------------------------------------------------
 * updateTonic
 * increments the tonic by 1 note on natural scale
 *----------------------------------------------------------------------------------------------------------
 */
void updateTonic(int incr)
{

                          // C   D   E   F   G   A   B   C   D   E   F   G   A   B   C   D   E   F   G   A   B   C   D   E   F   G
  uint8_t tonicNotes[26] = { 25, 26, 28, 29, 31, 33, 35, 36, 38, 40, 41, 43, 45, 47, 48, 50, 52, 53, 55, 57, 59, 60, 62, 64, 65, 67};
  uint8_t tonicIndex;

  for (int i = 0; i < 26; i++)
  {
    if(tonicNotes[i] == sequencer.getTonic())
    {
      tonicIndex = i;    
    }
  }
  tonicIndex = (tonicIndex + incr) % 26;
  
  sequencer.setTonic(tonicNotes[tonicIndex]);
  
  displaySettingIcon(BITMAP_ALPHA[getMidiNoteIconIndex(sequencer.getTonic())]);
}



uint8_t getMidiNoteIconIndex(uint8_t midinote)
{
  uint8_t basenote;
  basenote = (midinote - 9) % 12;

  switch(basenote)
  {
    case 0: return 0;
    case 2: return 1;
    case 3: return 2;
    case 5: return 3;
    case 7: return 4;
    case 8: return 5;
    case 10: return 6;
    default: return 0;
  }
}



/*----------------------------------------------------------------------------------------------------------
 * updateAudio
 * returns the current source audio to be output on pin 9
 *----------------------------------------------------------------------------------------------------------
 */
int updateAudio()
{
  return synthVoice.updateAudio();
}



/*----------------------------------------------------------------------------------------------------------
 * toggleOscillatorMode
 * changes the oscillator source
 *----------------------------------------------------------------------------------------------------------
 */
void toggleOscillatorMode()
{
  synthVoice.noteOff();
  // todo: if we add more synth voices here  
  //oscMode = ++oscMode % MAX_OSCILLATOR_MODES;

}




/*----------------------------------------------------------------------------------------------------------
 * updateDisplay
 * updates the matrix display
 *----------------------------------------------------------------------------------------------------------
 */
void updateDisplay()
{
  byte currentNote;
  byte currentStep;
  byte sequenceLength;
  
  byte scaledInputValue[MAX_ANALOG_INPUTS];
  
  // don't  update the display if we are waiting for the settings display timer to elapse
  if (settingDisplayTimer.ready())
  {
    
    // if the settings is currently displayed, clear it
    if (settingDisplayIconOn)
    {
      ledDisplay.clearScreen();
      settingDisplayIconOn = false;
    }

    currentNote = sequencer.getCurrentNote();
    currentStep = sequencer.getCurrentStep();
    sequenceLength = sequencer.getSequenceLength();
    
    // clear sequencer step rows
    ledDisplay.setRowPixels(6, 0);
    ledDisplay.setRowPixels(7, 0);

    if(currentStep >= 8)
    {
      ledDisplay.setPixel(currentStep%8, 7, (currentNote > 0));
    }
    else
    {
      ledDisplay.setPixel(currentStep%8, 6, (currentNote > 0));
    }  

    // invert last step in sequence so user can see how long the sequence is
    if(sequenceLength > 8)
    {
      ledDisplay.togglePixel((sequenceLength-1)%8, 7);
    }
    else if (sequenceLength > 1)
    {
      ledDisplay.togglePixel((sequenceLength-1)%8, 6);
    }  
    else
    {
      // if sequence length = 1 then just light step 1
      ledDisplay.setPixel(0, 6, true);
    }

    // display current decay length in col 0
    if (sequencer.getParameterLock(0) > 0)
    {
      scaledInputValue[0] = scaleAnalogInput(sequencer.getParameterLock(0)*4,5);
    }
    else
    {
      scaledInputValue[0] = scaleAnalogInput(iCurrentAnalogValue[ANALOG_INPUT_DECAY],5);
    }

    // display current detune in col 1
    if (sequencer.getParameterLock(1)>0)
    {
      scaledInputValue[1] = scaleAnalogInput(sequencer.getParameterLock(1)*4,5);
    }
    else
    {
      scaledInputValue[1] = scaleAnalogInput(iCurrentAnalogValue[ANALOG_INPUT_OSC2TUNE],5);
    }

    // display current filter cutoff in col 2
    if (sequencer.getParameterLock(2)>0)
    {
      scaledInputValue[2] = scaleAnalogInput(sequencer.getParameterLock(2)*4,5);
    }
    else
    {
      scaledInputValue[2] = scaleAnalogInput(iCurrentAnalogValue[ANALOG_INPUT_FILTER],5);
    }

    // display current filter shape
    if (sequencer.getParameterLock(4)>0)
    {
      scaledInputValue[3] = scaleAnalogInput(sequencer.getParameterLock(4),5);
    }
    else
    {
      scaledInputValue[3] = scaleAnalogInput(iCurrentAnalogValue[ANALOG_INPUT_FILTERSHAPE],5);
    }



    // display mutation probability in col 4
    scaledInputValue[4] = scaleAnalogInput( (int)sequencer.getMutationProbability() * 10,5);

    // display note probability in col 5
    scaledInputValue[5] = scaleAnalogInput( (int)sequencer.getNoteProbability() * 10,5);

    
    //display Analog Values in the range 0-6
    for (byte i = 0; i < 6; i++)
    {
      ledDisplay.drawLineV(i, 0, 6,false);
      //ledDisplay.drawLineV(i, 6-scaledInputValue[i], 6, true);
    
      ledDisplay.setPixel(i, 5-scaledInputValue[i], true);
    }

    //display trigger
    ledDisplay.setPixel(7, 0, iTrigger == HIGH);
    ledDisplay.refresh();
  }
}


/*----------------------------------------------------------------------------------------------------------
 * displaySettingIcon
 * displays the given icon on the screen and sets a timer and a flag for it to be cleared after a set time
 *----------------------------------------------------------------------------------------------------------
 */
void displaySettingIcon(const byte* bitmap)
{
  ledDisplay.displayIcon(bitmap);
  settingDisplayTimer.start(DISPLAY_SETTING_CHANGE_PERSIST_MILLIS);
  settingDisplayIconOn = true;
}



/*----------------------------------------------------------------------------------------------------------
 * debugWriteValue
 *----------------------------------------------------------------------------------------------------------
 */
void debugWriteValue(char* valueName, int value)
{
  Serial.print(valueName);
  Serial.print("=");
  Serial.println(value);
}






/*----------------------------------------------------------------------------------------------------------
 * scaleAnalogInput
 * scales an analog input from 0-1023 to 0-maxScale
 * also trims off bottom and top end of range to saturate signal at high and low values
 * TODO: add special case for maxScale = MAX_ANALOG_VALUE
 *----------------------------------------------------------------------------------------------------------
 */
int scaleAnalogInput(int rawValue, int maxScale)
{
  if (rawValue < 5)
  {
    return 0;    
  }
  else if (rawValue > MAX_ANALOG_VALUE)
  {
    return maxScale;
  }
  else
  {
    return ((float)rawValue / MAX_ANALOG_VALUE) * maxScale;
  }
}




/*----------------------------------------------------------------------------------------------------------
 * scaleAnalogInput
 * scales an analog input from 0-1023 to 0-maxScale with a non-linear kink at kneeX,kneeY
 * assumes 1000 = max
 *----------------------------------------------------------------------------------------------------------
 */
long scaleAnalogInputNonLinear(long rawValue, long kneeX, long kneeY, long maxScale)
{
  if (rawValue<=kneeX)
  {
    return rawValue * kneeY / kneeX;
  }
  else
  {

    return rawValue * (maxScale-kneeY) / (MAX_ANALOG_VALUE-kneeX) 
           - (maxScale-kneeY)*kneeX/(MAX_ANALOG_VALUE-kneeX) + kneeY;
  }
           
}



/*----------------------------------------------------------------------------------------------------------
 * analogInputHasChanged
 * returns true if the given analog input channel has moved by more than the noise threshold
 *----------------------------------------------------------------------------------------------------------
 */
bool analogInputHasChanged(byte inputChannel)
{
  return abs(iCurrentAnalogValue[inputChannel] - iLastAnalogValue[inputChannel]) > ANALOG_INPUT_MOVEMENT_THRESHOLD;
}


/*----------------------------------------------------------------------------------------------------------
 * loop
 * calls Mozzi's audioHook() function
 * all other control code moved to updateControls()
 *----------------------------------------------------------------------------------------------------------
 */

void loop() 
{
  //required for Mozzi
  audioHook();
}
