#define BLYNK_TEMPLATE_ID "your_id"
#define BLYNK_TEMPLATE_NAME "Water Level Monitoring using NodeMCU"
#define BLYNK_AUTH_TOKEN "your_token_here"
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiClient.h>

// Wi-Fi credentials
char ssid[] = "wifi_name";
char pass[] = "wifi_password";
char auth[] = BLYNK_AUTH_TOKEN;
BlynkTimer timer;

// Define the component pins
#define trig D5
#define echo D7
#define LED1 D0
#define LED2 D1
#define LED3 D2
#define LED4 D3
#define LED5 D8
#define relay D4
#define tdsSensor A0
#define pushButton D6

//Enter your tank max value(CM)
int MaxLevel = 20;

int Level1 = (MaxLevel * 75) / 100;
int Level2 = (MaxLevel * 65) / 100;
int Level3 = (MaxLevel * 55) / 100;
int Level4 = (MaxLevel * 45) / 100;
int Level5 = (MaxLevel * 35) / 100;

int pushButtonState = HIGH;

void setup() {
  Serial.begin(9600);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(tdsSensor, INPUT);
  pinMode(pushButton, INPUT_PULLUP); //not pushed : HIGH
  digitalWrite(relay, LOW);
  Blynk.begin(auth, ssid, pass);

  //Call the functions
  timer.setInterval(100L, ultrasonic);
  timer.setInterval(100L, checkPushButton);
}

//Get the ultrasonic sensor values
void ultrasonic() {
  digitalWrite(trig, LOW);
  delayMicroseconds(4);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long t = pulseIn(echo, HIGH);
  int distance = t / 29 / 2;

  int blynkDistance = (distance - MaxLevel) * -1;
  if (distance <= MaxLevel) {
    Blynk.virtualWrite(V0, blynkDistance);
  } else {
    Blynk.virtualWrite(V0, 0);
  }

  if (Level1 <= distance) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);
    digitalWrite(LED5, LOW);
  } else if (Level2 <= distance && Level1 > distance) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);
    digitalWrite(LED5, LOW);
  } else if (Level3 <= distance && Level2 > distance) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
    digitalWrite(LED4, LOW);
    digitalWrite(LED5, LOW);
  } else if (Level4 <= distance && Level3 > distance) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
    digitalWrite(LED4, HIGH);
    digitalWrite(LED5, LOW);
  } else if (Level5 >= distance) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
    digitalWrite(LED4, HIGH);
    digitalWrite(LED5, HIGH);
  }

  // Read TDS sensor value
  int tdsValue = analogRead(tdsSensor);

  // Set a threshold below which the sensor reading is considered zero
  int threshold = 15; // Adjust this threshold as needed

  // Adjust the sensor reading to be zero when below the threshold
  if (tdsValue < threshold) 
  {
    tdsValue = 0;
  }

  // Scale the tdsValue to fit within 0-100 range
  int scaledTDS = map(tdsValue, 0, 840, 0, 100);

  // Serial.print("Raw TDS Value: ");
  // Serial.println(tdsValue);
  // Serial.print("Scaled TDS Value: ");
  // Serial.println(scaledTDS);
  Blynk.virtualWrite(V1, scaledTDS);  // Assuming V1 is your TDS value widget in Blynk

  // Automatic mode to stop pump if water level crosses a particular value
  // if (distance >= 15) {
  //   digitalWrite(relay, HIGH); // Stop the pump
  //   Blynk.virtualWrite(V2, 0); // Update app button status
  // }
}

void checkPushButton() {
  int newPushButtonState = digitalRead(pushButton);
  if (newPushButtonState != pushButtonState) {
    pushButtonState = newPushButtonState;
    if (pushButtonState == LOW) {
      // Button is pressed, toggle the relay
      digitalWrite(relay, !digitalRead(relay));
      Blynk.virtualWrite(V2, digitalRead(relay)); // Update app button status
    }
  }
}

//Get the button value
BLYNK_WRITE(V2) {
  bool Relay = param.asInt();
  Serial.print(Relay);
  if (Relay == 1) {
    digitalWrite(relay,LOW); //ON
  } else {
    digitalWrite(relay,HIGH); //OFF
  }
}

void loop() {
  Blynk.run();
  timer.run();
}
