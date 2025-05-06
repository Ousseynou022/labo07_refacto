// Alarm.h

#include <Arduino.h>

class Alarm {
public:
  Alarm(int pinLedA, int pinLedB, int pinBuzzer, float* pDistance);

  void update();
  void setDistance(float dist);
  void setTimeout(unsigned long time);
  void setVariationTiming(unsigned long t);
  void setColourA(byte r, byte g, byte b);
  void setColourB(byte r, byte g, byte b);
  void turnOn();   // active le mode WATCHING (surveillance)
  void turnOff();  // désactive le système d'alarme
  void activateAlarm();   // alm;ON pour autoriser l’alarme à sonner
  void deactivateAlarm(); // alm;OFF pour désactiver les sons même si on détecte un objet

private:
  enum State { OFF, WATCHING, ON, TESTING };
  State state;

  int ledAPin, ledBPin, buzzerPin;
  float* distance;
  float distanceTrigger = 20.0; // valeur par défaut, peut être ajustée ailleurs
  unsigned long timeoutDelay = 2000;
  unsigned long lastStateChange = 0;
  unsigned long variationTiming = 100;
  bool alarmEnabled = true;

  byte colorA[3] = {255, 0, 0}; // Couleur LED A par défaut (rouge)
  byte colorB[3] = {0, 0, 255}; // Couleur LED B par défaut (bleu)

  void setRGBColor(int pin, byte r, byte g, byte b);
  void soundAlarm();
  void stopAlarm();
};
