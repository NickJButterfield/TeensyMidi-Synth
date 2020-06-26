// Thank you for taking an interest in my code! Feel free to modify it to your pleasing, I recognise this isn't the most efficient method.
// A big thank you to little-scale for personally helping me with this project.

//includes libraries
#include <Bounce2.h>
#include <mtof.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=95,74
AudioSynthWaveform       waveform3;      //xy=98,218
AudioSynthWaveform       waveform2;      //xy=100,146
AudioMixer4              mixer1;         //xy=301.00000762939453,150.00000381469727
AudioEffectEnvelope      envelope1;      //xy=475,156
AudioOutputAnalog        dac1;           //xy=695.0000076293945,160.00000190734863
AudioConnection          patchCord1(waveform1, 0, mixer1, 0);
AudioConnection          patchCord2(waveform3, 0, mixer1, 2);
AudioConnection          patchCord3(waveform2, 0, mixer1, 1);
AudioConnection          patchCord4(mixer1, envelope1);
AudioConnection          patchCord5(envelope1, dac1);
// GUItool: end automatically generated code

//establishes pins for buttons, set as a const as it never changes, saving RAM
const byte octDPin = 30;
const byte octUPin = 32;
const byte pins[8] = {1, 3, 5, 7, 9, 11, 24, 26 };

const byte debounce = 10;

//each value corresponds to each note on the lowest scale as a MIDI integer
const byte value[] = {0, 2, 4, 5, 7, 9, 11, 12};

//starts with an octave of 5, middle of the keyboard
byte octave = 5;

//sets up the channel and velocity for later
byte const channel = 1;
byte const velocity = 127;

// gives 10 values, one for the current value, and one for the value that has just passed 
byte current[10];
byte previous[10];



// Sets up 3 different waveforms
//SI = Sine, SQ = Square, SA = Saw
float freq;
float ampSi;
float ampSq;
float ampSa;

// CC values with previous vales
int value1;
int prev_val1;
int value2;
int prev_val2;
int value3;
int prev_val3;
int value4;
int prev_val4;

//variable for envelope
int eAttack;
int eRelease;

void setup() {

  //smoothens analog inputs
  analogReadResolution(10);
  analogReadAveraging(5);

  //establishes new functions for note on and note off messages
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleNoteOff(OnNoteOff);

  // sets up envelopes for different waves
  AudioMemory(32);
  envelope1.decay(500);
  envelope1.hold(0);
  envelope1.sustain(0.5);

  //labels each button pin as an input
  for (int i = 0; i < 8; i ++) {
   for (int i = 0; i < 8; i++) {
    pinMode(pins[i], INPUT_PULLUP); 
    
  }
  
  pinMode(octUPin, INPUT_PULLUP);
  pinMode(octDPin, INPUT_PULLUP);
  //LED for feedback
  pinMode(13, OUTPUT);
}
}

void loop() {
 usbMIDI.read();

  //if the octave up button's state is changed (pressed), send the octave up
  current[8] = digitalReadFast(octDPin);
  if (current[8] != previous[8]) {
    previous[8] = current[8];
    if (!current[8]) {
      octave = (octave - 1) % 10;
      delay(debounce);
    }
  }
  
 //if the octave down button's state is changed (pressed), send the octave down
  current[9] = digitalReadFast(octUPin);
  if (current[9] != previous[9]) {
    previous[9] = current[9];
    if (!current[9]) {
      octave = (octave + 1) % 10;
      delay(debounce);
    }
  }

    //if any of the midi buttons are pressed, send a note on, with the pitch being the value assigned to that repsective button, added to the current octave
    for (int i = 0; i < 8; i ++) {
    current[i] = digitalReadFast(pins[i]);
    if (current[i] != previous[i]) {
      usbMIDI.sendNoteOn((octave * 12) + value[i], (1 - current[i]) * velocity, channel);
      delay(debounce);
      previous[i] = current[i];
    }
    }

  //determines the attack and release of the synth envolope
  eAttack = analogRead(A0);
  eAttack = map(eAttack, 0, 1023, 0, 500);
  envelope1.attack(eAttack);

  eRelease  = analogRead(A1);
  eRelease = map(eRelease, 0, 1023, 0, 1000);
  envelope1.release(eRelease);

  
 //CC controllers, if the knob is touched and the value changes the value is sent  
 value1 = analogRead(A14);
  value1 = map(value1, 0, 1023, 0, 127);
  if (value1 != prev_val1) {
    usbMIDI.sendControlChange(1, value1, 1);
    value1 = prev_val1;
    delay(10);
  }
  
  value2 = analogRead(A15);
  value2 = map(value2, 0, 1023, 0, 127);
  if (value2 != prev_val2) {
    usbMIDI.sendControlChange(2, value2, 1);
    value2 = prev_val2;
    delay(10);
  }
  
  value3 = analogRead(A16);
  value3 = map(value3, 0, 1023, 0, 127);
  if (value3 != prev_val3) {
    usbMIDI.sendControlChange(3, value3, 1);
    value3 = prev_val3;
    delay(10);
  }

  value4 = analogRead(A17);
  value4 = map(value4, 0, 1023, 0, 127);
  if (value4 != prev_val4) {
    usbMIDI.sendControlChange(4, value4, 1);
    value4 = prev_val4;
    delay(10);
  }
}






void OnNoteOn (byte channel, byte pitch, byte velocity) {

  //If note on, play
  if (velocity > 0 && channel == 1) {
    // start envelope
    envelope1.noteOn();

    //Turn on LED
    digitalWrite(13, HIGH);
    
    // All waveoforms are on the same pitch
    freq = mtof.toFrequency(pitch);
    
    // Each amplitude is determined by their respective pins and velocity
    ampSi = (velocity / 127.0) * (analogRead(A18) / 1203.0);
    ampSq = (velocity / 127.0) * (analogRead(A19) / 1203.0);
    ampSa = (velocity / 127.0) * (analogRead(A20) / 1203.0);

    //pkay each waveform
    waveform1.begin(ampSi, freq, WAVEFORM_SINE);
    waveform2.begin(ampSq, freq, WAVEFORM_SQUARE);
    waveform3.begin(ampSa, freq, WAVEFORM_SAWTOOTH);
  }

  else {
    waveform1.amplitude(0.0);
    waveform2.amplitude(0.0);
    waveform3.amplitude(0.0);
  }

}


void OnNoteOff (byte channel, byte pitch, byte velocity) {
  //If note off, turn off LED, start the release phase
  digitalWrite(13, LOW);
  envelope1.noteOff();

}
