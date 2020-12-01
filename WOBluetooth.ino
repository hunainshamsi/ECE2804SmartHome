// Libraries

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif12pt7b.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int pin = 7;
int ThermistorPin = 1;
int Vo;
float R0 = 10000;
float R1 = 10000;
float B = 3936000;
float T0 = 297.15;
float R, ln, T, F;

int output;

void setup() {

  Serial.begin(9600);
  pinMode(8, OUTPUT); // digital pin 13 is configured as an output for the first LED (right)
  pinMode(6, OUTPUT); // digital pin 12 is configured as an output for the second LED (left)
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
  delay(2000);
  display.clearDisplay();
  //  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
}

int light;
int motion;
int receive;
int nl;
int intd;
int ac;

void loop() {

  if (Serial.available()) {
    receive = Serial.read();

    //Serial.print("Receive: ");
    //Serial.println(receive);

    //Night Light
    light = analogRead(A0);
    motion = digitalRead(2);

    if (receive == 2 || receive == 3 || receive == 4) {
      nl = receive;
    }
    if (nl == 2) {
      digitalWrite(8, HIGH);
      digitalWrite(6, HIGH);
    }
    else if (nl == 3) {
      if (light <= 100 && motion == HIGH) {
        //   Serial.println("It is dark.");
        digitalWrite(8, HIGH); // turn right LED on
        digitalWrite(6, HIGH); // turn left LED on
      }
      // if there is low light
      else if (light > 100 && light < 250 && motion == HIGH) {
        //   Serial.println("There is low light.");
        digitalWrite(8, HIGH); // turn right LED on
        digitalWrite(6, LOW); // turn left LED off
      }
      // if it is bright
      else {
        // Serial.println("It is bright");
        digitalWrite(6, LOW); // turn right LED off
        digitalWrite(8, LOW); // turn left LED off
      }
      delay(500);
    }
    else if (nl == 4) {
      digitalWrite(8, LOW);
      digitalWrite(6, LOW);
    }
    delay(500);

    //Humidity

    unsigned long ontime, offtime;
    double freq, period, interp;
    char output[3];

    //Serial.println(RH);

    ontime = pulseInLong(pin, HIGH);
    offtime = pulseInLong(pin, LOW);
    period = (double)ontime + (double)offtime;
    freq = 1000000.0 / period;

    //    Serial.println(ontime);
    //    Serial.println(offtime);

    //    Serial.print("Frequency: ");
    //    Serial.println(freq);

    switch ((int)freq)
    {
      case 6210 ... 6304 :
        interp = 85 + (freq - 6305) * ((95 - 85) / (6210 - 6305));
        break;
      case 6305 ... 6399 :
        interp = 75 + (freq - 6400) * ((85 - 75) / (6305 - 6400));
        break;
      case 6400 ... 6499 :
        interp = 65 + (freq - 6500) * ((75 - 65) / (6400 - 6500));
        break;
      case 6500 ... 6599 :
        interp = 55 + (freq - 6600) * ((65 - 55) / (6500 - 6600));
        break;
      case 6600 ... 6704 :
        interp = 45 + (freq - 6705) * ((55 - 45) / (6705 - 6600));
        break;
      case 6705 ... 6819 :
        interp = 35 + (freq - 6820) * ((45 - 35) / (6820 - 6705));
        break;
      case 6820 ... 6944 :
        interp = 25 + (freq - 6945) * ((35 - 25) / (6945 - 6820));
        break;
      case 6945 ... 7080 :
        interp = 15 + (freq - 7080) * ((25 - 15) / (7080 - 6945));
        break;
      default:
        interp = 0;
        break;
    }

    if (interp == 0) {
      //    output = "UNK";
    }
    else {
      itoa(interp, output, 10);
    }

    //   Serial.print("Humidity: ");
    //    Serial.println(output);
    //delay(1000);

    //Thermistor
    Vo = analogRead(ThermistorPin);
    R = R1 * (float)Vo / (1023 - (float)Vo); // resistance value of the thermistor
    ln = log(R / R0);

    T = 1 / ((ln / B) + (1 / T0)); // temperature value in Kelvin
    F = (1.8 * (T - 273.15)) + 32; // temperature value converted to Fahrenheit

    //   Serial.print("Vo: ");
    //   Serial.print(Vo);
    //   Serial.print(" V");

    //  Serial.print("  Temperature: ");
    //  Serial.print(F);
    //  Serial.println(" F");

    delay(500);

    //Intruder

    if (receive == 1 || receive == 0) {
      intd = receive;
    }

    //char intruder[6] = "  ";
    if (receive == 0) {
      if (digitalRead(3) == HIGH) {
        //        Serial.println("INTRUDER ALERT");  //for debugging purposes
        //startPlayback(sample, sizeof(sample));
        //        //intruder = "ALERT";
        SpeakerAlert();
      }
      else if (receive == 1) {
        //        Serial.println("ALL SYSTEMS NORMAL");
        //intruder = "NORMAL";
        digitalWrite(11, LOW);
      }
    }
    //delay(5000);
  }

  if (receive == 5 || receive == 6) {
    ac = receive;
  }

  //Climate Control
  int climate;
  int usertemp;
  int temp;
  if (Serial.available()) {
    climate = Serial.read();
    if (climate == 5) {
      if (Serial.available()) {
        usertemp = Serial.read();

        if (usertemp > temp) {
          digitalWrite(5, LOW);
        }
        if (usertemp < temp) {
          digitalWrite(5, HIGH);
        }
      }
    }
    else if (climate == 6)
      digitalWrite(5, LOW);
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
  //  display.cp437(true);
  //  display.write(167);
  display.setTextSize(1);
  display.print("F");

  // display humidity
  display.setTextSize(1);
  display.setCursor(0, 16);
  display.print("Humidity: ");
  display.setTextSize(1);
  display.setCursor(0, 24);
  display.print(output);
  display.print(" %");

  display.display();

  //Bluetooth Outputs
  Serial.print(F);
  Serial.print("F");
  Serial.print("|");
  Serial.print(output); //debugging
  Serial.print("%");
  Serial.print("|");
  Serial.println("NORMAL");

  delay(1000);

}
void SpeakerAlert() {

  for (int i = 1; i < 2000; i = i + 10) {
    digitalWrite(11, HIGH);
    delayMicroseconds(i);
    digitalWrite(11, LOW);
    delayMicroseconds(i);
  }
}
