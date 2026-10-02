#include "Controller.h"

// === Mux ===
Mux::Mux(byte outpin_, byte numPins_, bool analog_) {
  outpin = outpin_; numPins = numPins_; analog = analog_;
  if (!analog) pinMode(outpin, INPUT_PULLUP);
  pinMode(2, OUTPUT); pinMode(3, OUTPUT); pinMode(4, OUTPUT);
  if (numPins > 8) pinMode(5, OUTPUT);
}

void Mux::muxUpdate() {
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  if (numPins > 8) digitalWrite(5, LOW);
}

// === Button ===
Button::Button(byte pin, byte command, byte value, byte channel, byte debounce) {
  _pin = pin; _value = value; _command = command; _debounce = debounce;
  pinMode(_pin, INPUT_PULLUP);
  _busy = false; _status = 0b00000010; _last = 1;
  Bcommand = command; Bvalue = value; Bchannel = channel; Btoggle = 0;
}

Button::Button(Mux mux, byte muxpin, byte command, byte value, byte channel, byte debounce) {
  _pin = mux.outpin; _numMuxPins = mux.numPins; _muxpin = muxpin;
  _value = value; _command = command; _debounce = debounce;
  pinMode(_pin, INPUT_PULLUP);
  _busy = false; _status = 0b00000010; _last = 1;
  Bcommand = command; Bvalue = value; Bchannel = channel; Btoggle = 0;
}

void Button::muxUpdate() {
  byte temp = _muxpin;
  digitalWrite(2, bitRead(temp, 0));
  digitalWrite(3, bitRead(temp, 1));
  digitalWrite(4, bitRead(temp, 2));
  if (_numMuxPins > 8) digitalWrite(5, bitRead(temp, 3));
}

byte Button::getValue() {
  if (!bitRead(_status, 0) && digitalRead(_pin) == _last) return 2;
  if (bitRead(_status, 1)) {
    bitSet(_status, 0); bitClear(_status, 1);
    _time = millis(); return 255;
  }
  if (millis() - _time < _debounce) return 255;
  if (digitalRead(_pin) == _last) {
    bitClear(_status, 0); bitSet(_status, 1); return 255;
  }
  bitClear(_status, 0); bitSet(_status, 1);
  _last = (~_last) & 0b00000001;
  return _last;
}

// === Pot ===
Pot::Pot(byte pin, byte command, byte control, byte channel) {
  _pin = pin; _control = control;
  analogReadResolution(10);
  _value = analogRead(_pin);
  _value = _value >> 3;
  _oldValue = _value << 3;
  _value = _value << 3;
  Pcommand = command; Pcontrol = control; Pchannel = channel;
  _minVal = 0; _maxVal = 1023; _calibrated = false; _baseline = 0;
}

Pot::Pot(Mux mux, byte muxpin, byte command, byte control, byte channel) {
  _pin = mux.outpin; _numMuxPins = mux.numPins; _muxpin = muxpin; _control = control;
  muxUpdate();
  _value = analogRead(_pin);
  _value = _value >> 3;
  _oldValue = _value << 3;
  _value = _value << 3;
  Pcommand = command; Pcontrol = control; Pchannel = channel;
  _minVal = 0; _maxVal = 1023; _calibrated = false; _baseline = 0;
}

void Pot::muxUpdate() {
  byte temp = _muxpin;
  digitalWrite(2, bitRead(temp, 0));
  digitalWrite(3, bitRead(temp, 1));
  digitalWrite(4, bitRead(temp, 2));
  if (_numMuxPins > 8) digitalWrite(5, bitRead(temp, 3));
}

byte Pot::getValue() {
  muxUpdate();
  delayMicroseconds(20);
  const int NUM_SAMPLES = 16;
  const int DEADZONE = 8;
  long sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) sum += analogRead(_pin);
  _value = sum / NUM_SAMPLES;
  int diff = abs(_value - _oldValue);
  if (diff >= DEADZONE) {
    _oldValue = _value;
    return _value >> 3;
  }
  return 255;
}

int Pot::getRawValue() {
  muxUpdate();
  delayMicroseconds(20);
  const int NUM_SAMPLES = 4;
  long sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) sum += analogRead(_pin);
  return (int)(sum / NUM_SAMPLES);
}

void Pot::calibrate() {
  int samples[200];
  long sum = 0;
  
  for (int i = 0; i < 200; i++) {
    muxUpdate();
    delayMicroseconds(100);  // longer settle between each sample
    samples[i] = analogRead(_pin);
    sum += samples[i];
    delay(5);  // longer gap between sample batches
  }
  
  _minVal = samples[0];
  _maxVal = samples[0];
  for (int i = 1; i < 200; i++) {
    if (samples[i] < _minVal) _minVal = samples[i];
    if (samples[i] > _maxVal) _maxVal = samples[i];
  }
  
  // Baseline = midpoint of the resting range
  _baseline = (_minVal + _maxVal) / 2;
  
  _minVal = max(0, _minVal - 30);
  _maxVal = min(1023, _maxVal + 30);
  _calibrated = true;
}