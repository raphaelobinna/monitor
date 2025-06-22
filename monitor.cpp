#include <Wire.h>
#include<LiquidCrystal_I2C_Hangul.h>

// LCD Setup
LiquidCrystal_I2C_Hangul lcd(0x27, 20, 4); 

const int analogPin = A0;       // Thermistor analog pin
const int ecgPin = A2;          // AD8232 analog output
const float BETA = 3435;
const float nominalTemp = 25.0; // °C
const float nominalResistance = 10000.0;
const float seriesResistor = 10000.0;

const int ecgThreshold = 550;
const int minBeatInterval = 300;
unsigned long lastBeatTime = 0;
int bpm = 0;

void setup() {
  Serial.begin(9600);
  pinMode(10, INPUT); // LO+
  pinMode(11, INPUT); // LO-
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp / BPM Ready");
}

void loop() {
  // ---- Temperature Reading ----
  int analogValue = analogRead(analogPin);
  float voltage = analogValue * 5.0 / 1023.0;
  float resistance = seriesResistor * (1023.0 / analogValue - 1.0);
  float temperatureK = 1.0 / (log(resistance / nominalResistance) / BETA + 1.0 / (nominalTemp + 273.15));
  float temperatureC = temperatureK - 273.15;
  float temperatureF = temperatureC * 9.0 / 5.0 + 32.0;

  // ---- ECG Reading ----
  int ecgValue = analogRead(ecgPin);
  unsigned long now = millis();

  if (ecgValue > ecgThreshold && (now - lastBeatTime) > minBeatInterval) {
    unsigned long interval = now - lastBeatTime;
    bpm = 60000 / interval;
    lastBeatTime = now;
  }

  // ---- Serial Output ----
  Serial.print("Temp: ");
  Serial.print(temperatureC);
  Serial.print(" °C | BPM: ");
  Serial.print(bpm);
  Serial.println();

  // ---- LCD Output ----
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.print(temperatureC, 1);
  lcd.print((char)223); // degree symbol
  lcd.print("C   ");

  lcd.setCursor(0, 1);
  if ((digitalRead(10) == 1) || (digitalRead(11) == 1)) {
    lcd.print("ECG: Leads Off   ");
  } else {
    lcd.print("BPM: ");
    lcd.print(bpm);
    lcd.print("         ");
  }

  delay(200); // Slower refresh rate for display
}
