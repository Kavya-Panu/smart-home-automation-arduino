#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define PIR_PIN 6
#define LDR_PIN A0
#define RELAY_FAN 8
#define RELAY_LIGHT 9

#define TEMP_THRESHOLD 30
#define LIGHT_THRESHOLD 150

// ----- STATE DEFINITIONS -----
enum FanState { FAN_OFF, FAN_ON };
enum LightState { LIGHT_OFF, LIGHT_ON };

FanState fanState = FAN_OFF;
LightState lightState = LIGHT_OFF;

unsigned long previousMillis = 0;
const long interval = 1000;

float temp = 0;
int ldrValue = 0;
bool motion = false;

void setup() {
  Serial.begin(9600);

  dht.begin();
  pinMode(PIR_PIN, INPUT);

  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_LIGHT, OUTPUT);

  digitalWrite(RELAY_FAN, HIGH);
  digitalWrite(RELAY_LIGHT, HIGH);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
}

void loop() {

  unsigned long currentMillis = millis();

  // ---- Sensor Update ----
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    temp = dht.readTemperature();
    ldrValue = analogRead(LDR_PIN);
    motion = digitalRead(PIR_PIN);
  }

// ---- FAN FSM (Logic) ----
switch(fanState) {

  case FAN_OFF:
    if(temp > TEMP_THRESHOLD && motion) {
      fanState = FAN_ON;
    }
    break;

  case FAN_ON:
    if(temp <= TEMP_THRESHOLD || !motion) {
      fanState = FAN_OFF;
    }
    break;
}
  // ---- LIGHT FSM ----
  switch(lightState) {

    case LIGHT_OFF:
      if(motion && ldrValue < LIGHT_THRESHOLD) {
        lightState = LIGHT_ON;
      }
      break;

    case LIGHT_ON:
      if(!motion || ldrValue >= LIGHT_THRESHOLD) {
        lightState = LIGHT_OFF;
      }
      break;
  }

  // ---- Apply Outputs ----
  digitalWrite(RELAY_FAN, fanState == FAN_ON ? LOW : HIGH);
  digitalWrite(RELAY_LIGHT, lightState == LIGHT_ON ? LOW : HIGH);

  // ---- Display ----
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0,0);
  display.print("Temp: ");
  display.print(temp);

  display.setCursor(0,12);
  display.print("Light: ");
  display.print(ldrValue);

  display.setCursor(0,24);
  display.print("Motion: ");
  display.print(motion ? "YES" : "NO");

  display.setCursor(0,36);
  display.print("Fan: ");
  display.print(fanState == FAN_ON ? "ON" : "OFF");

  display.setCursor(0,48);
  display.print("Light: ");
  display.print(lightState == LIGHT_ON ? "ON" : "OFF");

  display.display();
}
