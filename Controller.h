#ifndef Controller_h
#define Controller_h

#include <Arduino.h>

class Mux {
public:
  Mux(byte outpin_, byte numPins_, bool analog_);
  byte outpin, numPins;
  bool analog;
  void muxUpdate();
};

class Button {
public:
  Button(byte pin, byte command, byte value, byte channel, byte debounce);
  Button(Mux mux, byte muxpin, byte command, byte value, byte channel, byte debounce);
  byte getValue();
  void muxUpdate();
  byte Bcommand, Bvalue, Bchannel, Btoggle;
private:
  byte _pin, _muxpin, _numMuxPins;
  byte _value, _command, _debounce;
  int _time, _last;
  bool _busy;
  byte _status;
};

class Pot {
public:
  Pot(byte pin, byte command, byte control, byte channel);
  Pot(Mux mux, byte muxpin, byte command, byte control, byte channel);
  void muxUpdate();
  byte getValue();
  int getRawValue();
  void calibrate();
  byte Pcommand, Pcontrol, Pchannel;
  bool _calibrated;
  int _minVal, _maxVal;
  int _baseline;          // ← NEW: resting baseline per channel
private:
  byte _pin, _muxpin, _numMuxPins;
  byte _value, _command, _debounce;
  int _oldValue;
  int _control;
};

#endif