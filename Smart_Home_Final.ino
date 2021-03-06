//Libraries

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif12pt7b.h>

//Definations
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Pin definations
int pin = 7; //Humidity
int ThermistorPin = 1; //Thermistor Pin
//Tempreture global variables
int Vo;
float R0 = 10000;
float R1 = 10000;
float B = 3936000;
float T0 = 297.15;
float R, ln, T, F;
int output;

//Function Stubs
void NightLightSmart();
void SpeakerAlert();
int IntruderAlert();

void setup() {
  Serial.begin(9600); //Serial communication initialize
  pinMode(8, OUTPUT); // digital pin 13 is configured as an output for the first LED (right)
  pinMode(6, OUTPUT); // digital pin 12 is configured as an output for the second LED (left)
  pinMode(5, OUTPUT); //Climate Control LED
  pinMode(2, INPUT); // digital pin 2 is configured as an input for the PIR sensor
  pinMode(pin, INPUT); //Humidity
  pinMode(3, INPUT); //input for intruder alert PIR
  digitalWrite(3, LOW); //PIR

  //OLED Display confugiration
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    //for debugging purposes
    //Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

}

//Global variables for bluetooth logic
int receive;
int nl;
int intd;
int ac;

void loop() {

  NightLightSmart(); //Function Running in the background
  
  if (Serial.available() > 0) {
    receive = Serial.read();

    switch (intd) {
      case 1: {
          digitalWrite(11, LOW);
          break;
        }
      case 0: {
          IntruderAlert();
          break;
        }
    }

    //CLIMATE CONTROL
    if (receive == 5 || receive == 6) {
      ac = receive;
    }
    //case off
    if (ac == 6) {
      digitalWrite(5, LOW);
      //Serial.println("ac CASE Off");
    }
    //  case on
    if (ac == 5) {
      digitalWrite(5, HIGH);
      //Serial.println("ac CASE ON");
    }
  }
  //End of Serial Availaible Loop

  int IntruderReturn = IntruderAlert();

  //Thermistor
  Vo = analogRead(ThermistorPin);
  R = R1 * (float)Vo / (1023 - (float)Vo); // resistance value of the thermistor
  ln = log(R / R0);

  T = 1 / ((ln / B) + (1 / T0)); // temperature value in Kelvin
  F = (1.8 * (T - 273.15)) + 32; // temperature value converted to Fahrenheit

  //Humidity

  unsigned long ontime, offtime;
  double freq, period, interp;
  char output[3];

  ontime = pulseInLong(pin, HIGH);
  offtime = pulseInLong(pin, LOW);
  period = (double)ontime + (double)offtime; // calculate period
  freq = 1000000.0 / period; // calculate frequency

  // determine which range the frequency is in
  switch ((int)freq)
  {
    case 6210 ... 6304 :
    // perform linear interpolation to find the relative humidity
      interp = 95 + (freq - 6305) * ((95 - 85) / (6210 - 6305));
      break;
    case 6305 ... 6399 :
      interp = 85 + (freq - 6400) * ((85 - 75) / (6305 - 6400));
      break;
    case 6400 ... 6499 :
      interp = 75 + (freq - 6500) * ((75 - 65) / (6400 - 6500));
      break;
    case 6500 ... 6599 :
      interp = 65 + (freq - 6600) * ((65 - 55) / (6500 - 6600));
      break;
    case 6600 ... 6704 :
      interp = 55 + (freq - 6705) * ((55 - 45) / (6705 - 6600));
      break;
    case 6705 ... 6819 :
      interp = 45 + (freq - 6820) * ((45 - 35) / (6820 - 6705));
      break;
    case 6820 ... 6944 :
      interp = 35 + (freq - 6945) * ((35 - 25) / (6945 - 6820));
      break;
    case 6945 ... 7080 :
      interp = 25 + (freq - 7080) * ((25 - 15) / (7080 - 6945));
      break;
    default:
      interp = 0;
      break;
  }

  //displays
  //clear display
  display.clearDisplay();
  // display temperature
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.setTextSize(1);
  display.setCursor(0, 8);
  display.print(F);
  display.print(" ");
  display.setTextSize(1);
  display.setTextSize(1);
  display.print("F");
  // display humidity
  display.setTextSize(1);
  display.setCursor(0, 16);
  display.print("Humidity: ");
  display.setTextSize(1);
  display.setCursor(0, 24);
  if (period == 0) {
    display.print("ERROR");
  }
  else {
    display.print(interp);
    display.print(" %");
  }
  display.display();

  //Bluetooth Outputs
  Serial.print(F);
  Serial.print("F");
  Serial.print("|");
  if (period == 0) {
    Serial.print("ERROR");
    Serial.print("|");
  }
  else {
    Serial.print(interp);
    Serial.print("%");
    Serial.print("|");
  }
  if (IntruderReturn == 1) {
    Serial.println("ALERT");
  }
  if (IntruderReturn == 0) {
    Serial.println("NORMAL");
  }

  //delay(1000);

}

void NightLightSmart() {

  int light = analogRead(A0);
  int motion = digitalRead(2);

      //NIGHTLIGHT
    if (receive == 2 || receive == 3 || receive == 4) {
      nl = receive;
    }
    //Case ON
    if (nl == 2) {
      digitalWrite(8, HIGH);
      digitalWrite(6, HIGH);
    }
    //Case Smart
    else if (nl == 3) {
      if (light <= 100 && motion == HIGH) {
    digitalWrite(8, HIGH); // turn right LED on
    digitalWrite(6, HIGH); // turn left LED on
  }
  // if there is low light
  else if (light > 100 && light < 250 && motion == HIGH) {
    digitalWrite(8, HIGH); // turn right LED on
    digitalWrite(6, LOW); // turn left LED off
  }
  else {
    digitalWrite(8, LOW); // turn right LED off
    digitalWrite(6, LOW); // turn left LED off
  }
    }
    //Case Off
    else if (nl == 4) {
      digitalWrite(8, LOW);
      digitalWrite(6, LOW);
    }
}

//Function for speaker alert
void SpeakerAlert() {

  for (int i = 1; i < 5000; i = i + 10) {
    digitalWrite(11, HIGH);
    delayMicroseconds(i);
    digitalWrite(11, LOW);
    delayMicroseconds(i);
  }

}

//Function for intruder alert
int IntruderAlert() {

  int intruder;

  if (intd == 0) {
    if (digitalRead(3) == HIGH) {
      SpeakerAlert();
      intruder = 1;
    }
    else {
      digitalWrite(11, LOW);
      intruder = 0;
    }
    return intruder;
  }

  if (intd == 1) {
    digitalWrite(11, LOW);
    intruder = 0;
    return intruder;
  }
}
