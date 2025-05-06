#include <Wire.h>
#include <LCD_I2C.h>
#include <LedControl.h>
#include "Alarm.h"
#include "PorteAutomatique.h"

#define TRIGGER_PIN 9
#define ECHO_PIN 10
#define RED_PIN 3
#define BLUE_PIN 2
#define BUZZER 4
#define DIN_PIN 34
#define CLK_PIN 30
#define CS_PIN 32

LCD_I2C lcd(0x27, 16, 2);
LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, 1);

float distance = 0;
Alarm alarm(RED_PIN, BLUE_PIN, BUZZER, &distance);
PorteAutomatique porte(31, 33, 35, 37, distance);

int limInf = 10, limSup = 100;

void showCheck() {
  byte check[8] = {
    B00000000,
    B00000001,
    B00000010,
    B10000100,
    B01001000,
    B00110000,
    B00000000,
    B00000000
  };
  lc.clearDisplay(0);
  for (int i = 0; i < 8; i++) lc.setRow(0, i, check[i]);
  delay(2000);
  lc.clearDisplay(0);
}

void showError() {
  byte error[8] = {
    B10000001,
    B01000010,
    B00100100,
    B00011000,
    B00011000,
    B00100100,
    B01000010,
    B10000001
  };
  lc.clearDisplay(0);
  for (int i = 0; i < 8; i++) lc.setRow(0, i, error[i]);
  delay(3000);
  lc.clearDisplay(0);
}

void showPannError() {
  byte interdit[8] = {
    B00111100,
    B11000100,
    B10001001,
    B10010001,
    B10100001,
    B11000001,
    B11000010,
    B00111100
  };
  lc.clearDisplay(0);
  for (int i = 0; i < 8; i++) lc.setRow(0, i, interdit[i]);
  delay(2000);
  lc.clearDisplay(0);
}

void updateDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("Dist : ");
  lcd.print(distance);
  lcd.print(" cm   ");
  lcd.setCursor(0, 1);
  lcd.print("Porte : ");
  lcd.print(porte.getEtatTexte());
}

unsigned long measureDistance() {
  digitalWrite(TRIGGER_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

void analyserCommande(const String& tampon, String& commande, String& arg1, String& arg2) {
  int firstSep = tampon.indexOf(';');
  int secondSep = tampon.indexOf(';', firstSep + 1);
  if (firstSep == -1) {
    commande = tampon;
    return;
  }
  commande = tampon.substring(0, firstSep);
  if (secondSep != -1) {
    arg1 = tampon.substring(firstSep + 1, secondSep);
    arg2 = tampon.substring(secondSep + 1);
  } else {
    arg1 = tampon.substring(firstSep + 1);
  }
}

void serialEvent() {
  String tampon = Serial.readStringUntil('\n');
  tampon.trim();
  String commande, arg1, arg2;
  analyserCommande(tampon, commande, arg1, arg2);

  if (commande == "g_dist") {
    Serial.println(distance);
    showCheck();
  } else if (commande == "cfg") {
    if (arg1 == "alm") {
      alarm.setDistance(arg2.toFloat());
      Serial.println("Configure la distance de l’alarme à " + String(arg2) + " cm");
      showCheck();
    } else if (arg1 == "lim_inf") {
      int val = arg2.toInt();
      if (val >= limSup) {
        Serial.println("Erreur – Limite inférieure >= limite supérieure");
        showPannError();
      } else {
        limInf = val;
        Serial.println("Limite inférieure configurée à " + String(limInf));
        showCheck();
      }
    } else if (arg1 == "lim_sup") {
      int val = arg2.toInt();
      if (val <= limInf) {
        Serial.println("Erreur – Limite supérieure <= limite inférieure");
        showPannError();
      } else {
        limSup = val;
        Serial.println("Limite supérieure configurée à " + String(limSup));
        showCheck();
      }
    } else {
      Serial.println("Argument cfg inconnu");
      showError();
    }
  } else if (commande == "alm") {
    if (arg1 == "OFF") {
      alarm.deactivateAlarm();
      Serial.println("Alarme désactivée");
      showCheck();
    } else if (arg1 == "ON") {
      alarm.activateAlarm();
      Serial.println("Alarme activée");
      showCheck();
    } else {
      Serial.println("Commande alm inconnue");
      showError();
    }
  } else {
    Serial.println("Commande inconnue");
    showError();
  }
}

void setup() {
  Serial.begin(9600);
  lcd.begin(); lcd.backlight();
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("2419796");
  lcd.setCursor(0, 1); lcd.print("Labo 7");
  delay(2000); lcd.clear();

  alarm.setColourA(255, 0, 0);
  alarm.setColourB(0, 0, 255);
  alarm.setVariationTiming(500);
  alarm.setDistance(12);
  alarm.setTimeout(2000);
  alarm.activateAlarm();

  porte.setPasParTour(2048);
  porte.setAngleFerme(10);
  porte.setAngleOuvert(170);
  porte.setDistanceOuverture(60);
  porte.setDistanceFermeture(60);
}

void loop() {
  static unsigned long lastMeasure = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastMeasure >= 100) {
    lastMeasure = currentTime;
    distance = measureDistance();
    updateDisplay();
  }
  alarm.update();
  porte.update();
}
