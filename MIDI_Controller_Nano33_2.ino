#include <MIDIUSB.h>
#include "Controller.h"

void midiSendNoteOn(byte channel, byte note, byte velocity);
void midiSendNoteOff(byte channel, byte note, byte velocity);
void midiSendControlChange(byte channel, byte control, byte value);
void updateButtons();
void updateMuxButtons();
void updatePots();
void updateMuxPots();
void updateLeds();
void midiPoll();

/*  Physical layout (left to right):
 *  LED pins:  D10 (red), D11 (yellow), D12 (blue), D9 (green)
 *  → ledPin array must follow that order: {10, 11, 12, 9}
 *  LED → mux‑pot mapping (left to right):
 *      D10 → MPO3 (ribbon Y2)
 *      D11 → MPO4 (ribbon Y3)
 *      D12 → MPO5 (FSR Y4)
 *      D9  → MPO6 (FSR Y5)
 */
const int NLeds = 4;
int ledPin[NLeds] = {10, 11, 12, 9};
int ledMuxPotMap[NLeds] = {0, 1, 2, 3};

byte NUMBER_BUTTONS = 3;
byte NUMBER_MUX_BUTTONS = 14;
byte NUMBER_POTS = 5;
byte NUMBER_MUX_POTS = 4;

Mux M1(A4, 16, true);
Mux M2(A5, 8, true);

Pot PO1(A0, 0, 4, 1);
Pot PO2(A1, 0, 5, 1);
Pot PO3(A2, 0, 6, 1);
Pot PO4(A3, 0, 7, 1);
Pot PO5(A6, 0, 8, 1);

Button BU1(6, 0, 36, 1, 5);
Button BU2(7, 0, 38, 1, 5);
Button BU3(8, 0, 40, 1, 5);

Button MBU1(M1, 0, 0, 36, 10, 5);
Button MBU2(M1, 1, 0, 37, 10, 5);
Button MBU3(M1, 2, 0, 38, 10, 5);
Button MBU4(M1, 3, 0, 39, 10, 5);
Button MBU5(M1, 4, 0, 40, 10, 5);
Button MBU6(M1, 5, 0, 41, 10, 5);
Button MBU7(M1, 6, 0, 76, 1, 5);
Button MBU8(M1, 7, 0, 77, 1, 5);
Button MBU9(M1, 8, 0, 78, 1, 5);
Button MBU10(M1, 9, 0, 79, 1, 5);
Button MBU11(M1, 10, 0, 80, 1, 5);
Button MBU12(M1, 11, 0, 81, 1, 5);
Button MBU13(M1, 12, 0, 82, 1, 5);
Button MBU14(M1, 13, 0, 83, 1, 5);

Pot MPO3(M2, 2, 0, 11, 1);
Pot MPO4(M2, 3, 0, 12, 1);
Pot MPO5(M2, 4, 0, 13, 1);
Pot MPO6(M2, 5, 0, 10, 1);

Pot *POTS[] = {&PO1, &PO2, &PO3, &PO4, &PO5};
Button *BUTTONS[] = {&BU1, &BU2, &BU3};
Button *MUXBUTTONS[] = {&MBU1, &MBU2, &MBU3, &MBU4, &MBU5, &MBU6, &MBU7, &MBU8,
                           &MBU9, &MBU10, &MBU11, &MBU12, &MBU13, &MBU14};
Pot *MUXPOTS[] = {&MPO3, &MPO4, &MPO5, &MPO6};

void setup() {
  analogReadResolution(10);

  for (int i = 0; i < NLeds; i++) pinMode(ledPin[i], OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  M1.muxUpdate();
  M2.muxUpdate();

  for (int i = 0; i < NUMBER_MUX_POTS; i++) {
    MUXPOTS[i]->calibrate();
  }
}

void loop() {
  if (NUMBER_BUTTONS != 0) updateButtons();
  if (NUMBER_MUX_BUTTONS != 0) updateMuxButtons();
  if (NUMBER_POTS != 0) updatePots();

  updateMuxPots();
  updateLeds();
  midiPoll();
}

void midiSendNoteOn(byte channel, byte note, byte velocity) {
  midiEventPacket_t event = {0x09, (byte)(0x90 | (channel - 1)), note, velocity};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}
void midiSendNoteOff(byte channel, byte note, byte velocity) {
  midiEventPacket_t event = {0x08, (byte)(0x80 | (channel - 1)), note, velocity};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}
void midiSendControlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, (byte)(0xB0 | (channel - 1)), control, value};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void updateButtons() {
  for (int i = 0; i < NUMBER_BUTTONS; i++) {
    byte message = BUTTONS[i]->getValue();
    if (message == 0) {
      switch (BUTTONS[i]->Bcommand) {
        case 0: midiSendNoteOn(BUTTONS[i]->Bchannel, BUTTONS[i]->Bvalue, 127); break;
      }
    }
    if (message == 1) {
      switch (BUTTONS[i]->Bcommand) {
        case 0: midiSendNoteOff(BUTTONS[i]->Bchannel, BUTTONS[i]->Bvalue, 0); break;
      }
    }
  }
}
void updateMuxButtons() {
  for (int i = 0; i < NUMBER_MUX_BUTTONS; i++) {
    MUXBUTTONS[i]->muxUpdate();
    byte message = MUXBUTTONS[i]->getValue();
    if (message == 0) {
      switch (MUXBUTTONS[i]->Bcommand) {
        case 0: midiSendNoteOn(MUXBUTTONS[i]->Bchannel, MUXBUTTONS[i]->Bvalue, 127); break;
      }
    }
    if (message == 1) {
      switch (MUXBUTTONS[i]->Bcommand) {
        case 0: midiSendNoteOff(MUXBUTTONS[i]->Bchannel, MUXBUTTONS[i]->Bvalue, 0); break;
      }
    }
  }
}

void updatePots() {
  for (int i = 0; i < NUMBER_POTS; i++) {
    byte potmessage = POTS[i]->getValue();
    if (potmessage != 255) midiSendControlChange(POTS[i]->Pchannel, POTS[i]->Pcontrol, potmessage);
  }
}

void updateMuxPots() {
  for (int i = 0; i < NUMBER_MUX_POTS; i++) {
    MUXPOTS[i]->muxUpdate();
    byte potmessage = MUXPOTS[i]->getValue();
    if (potmessage != 255) midiSendControlChange(MUXPOTS[i]->Pchannel, MUXPOTS[i]->Pcontrol, potmessage);
  }
}

/*  LED logic per sensor:
 *  - Only mux pots drive LEDs (PO5 on A6 is a direct pot, never touches an LED)
 *  - Each LED only responds to its own mux channel
 *  - LED is OFF unless raw value exceeds baseline + touchThreshold
 *  - Brightness maps from (baseline + touchThreshold) up to maxVal
 */
void updateLeds() {
  const int touchThreshold = 25;  // must exceed baseline by this much to light

  for (int i = 0; i < NLeds; i++) {
    int potIndex = ledMuxPotMap[i];
    Pot *pot = MUXPOTS[potIndex];
    
    pot->muxUpdate();
    int raw = pot->getRawValue();
    
    int pwmValue = 0;
    if (pot->_calibrated) {
      int delta = raw - pot->_baseline;
      int effectiveRange = pot->_maxVal - pot->_baseline;
      
      if (delta > touchThreshold && effectiveRange > 0) {
        pwmValue = map(delta, touchThreshold, effectiveRange, 0, 255);
      }
      // else: delta <= threshold → pwmValue stays 0 (LED off)
    } else {
      pwmValue = raw >> 2;
    }
    
    pwmValue = constrain(pwmValue, 0, 255);
    analogWrite(ledPin[i], pwmValue);
  }
}

void midiPoll() {
  midiEventPacket_t event = MidiUSB.read();
  if (event.header != 0) {
    byte type = event.header & 0xF0;
    byte channel = event.byte1 & 0x0F;
    if (type == 0x90) handleNoteOn(channel, event.byte2, event.byte3);
    else if (type == 0x80) handleNoteOff(channel, event.byte2, event.byte3);
    else if (type == 0xB0) handleControlChange(channel, event.byte2, event.byte3);
  }
}

void handleControlChange(byte channel, byte number, byte value) {}
void handleNoteOn(byte channel, byte number, byte value) {}
void handleNoteOff(byte channel, byte number, byte value) {}