// Alarm.cpp

#include "Alarm.h"
#include <Arduino.h>

Alarm::Alarm(int pinLedA, int pinLedB, int pinBuzzer, float* pDistance)
  : ledAPin(pinLedA), ledBPin(pinLedB), buzzerPin(pinBuzzer), distance(pDistance)
{
  pinMode(ledAPin, OUTPUT);
  pinMode(ledBPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  state = OFF;
  lastStateChange = millis();
}

void Alarm::setDistance(float dist) {
  *distance = dist;
}

void Alarm::setTimeout(unsigned long time) {
  timeoutDelay = time;
}

void Alarm::setVariationTiming(unsigned long t) {
  variationTiming = t;
}

void Alarm::setColourA(byte r, byte g, byte b) {
  colorA[0] = r;
  colorA[1] = g;
  colorA[2] = b;
}

void Alarm::setColourB(byte r, byte g, byte b) {
  colorB[0] = r;
  colorB[1] = g;
  colorB[2] = b;
}

void Alarm::turnOn() {
  state = WATCHING;
  lastStateChange = millis();
}

void Alarm::turnOff() {
  state = OFF;
  stopAlarm();
  setRGBColor(ledAPin, 0, 0, 0);
  setRGBColor(ledBPin, 0, 0, 0);
  digitalWrite(buzzerPin, LOW);
}

void Alarm::activateAlarm() {
  alarmEnabled = true;
   if (state == OFF) {
    state = WATCHING;
    lastStateChange = millis();
  }
}

void Alarm::deactivateAlarm() {
  alarmEnabled = false;
  stopAlarm();
}

void Alarm::update() {
  unsigned long currentTime = millis();
  float dist = *distance;

  switch (state) {
    case OFF:
      stopAlarm();
      setRGBColor(ledAPin, 0, 0, 0);
      setRGBColor(ledBPin, 0, 0, 0);
      digitalWrite(buzzerPin, LOW);
      break;

    case WATCHING:
      setRGBColor(ledAPin, colorA[0], colorA[1], colorA[2]);
      if (dist < distanceTrigger) {
        state = ON;
        lastStateChange = currentTime;
        if (alarmEnabled) soundAlarm();
      }
      break;

    case ON:
      setRGBColor(ledBPin, colorB[0], colorB[1], colorB[2]);
      if (alarmEnabled) {
        if (currentTime - lastStateChange < timeoutDelay) {
          soundAlarm();
        } else {
          stopAlarm();
        }
      } else {
        stopAlarm();
      }

      if (dist >= distanceTrigger && currentTime - lastStateChange >= timeoutDelay) {
        state = WATCHING;
        lastStateChange = currentTime;
      }
      break;

    case TESTING:
     
      setRGBColor(ledAPin, 255, 255, 0); 
      if (currentTime - lastStateChange > timeoutDelay) {
        state = OFF;
        lastStateChange = currentTime;
      }
      break;
  }
}

void Alarm::setRGBColor(int pin, byte r, byte g, byte b) {
  analogWrite(pin, r); 
}

void Alarm::soundAlarm() {
  digitalWrite(buzzerPin, HIGH);
}

void Alarm::stopAlarm() {
  digitalWrite(buzzerPin, LOW);
}
