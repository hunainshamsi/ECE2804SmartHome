// Smart Home.cpp : This file contains the 'main' function. Program execution begins and ends there.

// Libraries

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <PCM.h>
#include <SPI.h>
#include <SD.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


//Initializations

int state = 0; //Bluetooth state
int flag = 0; //bluetooth flag3
int ThermistorPin = 1;
char recieve;

const unsigned char sample[] PROGMEM = {
  126, 127, 128, 128, 128, 128, 128, 127, 128, 128, 128, 129, 129, 128, 127, 128, 128, 127, 126, 127, 128, 129, 128, 127, 126, 127, 128, 128, 126, 126, 127, 127, 127, 127, 127, 127, 126, 127, 129, 130, 129, 128, 126, 126, 126, 126, 127, 129, 130, 129, 127, 127, 127, 127, 128, 128, 128, 128, 127, 127, 127, 127, 127, 127, 128, 130, 131, 129, 127, 126, 126, 126, 127, 127, 128, 128, 128, 128, 127, 128, 128, 127, 127, 128, 128, 130, 130, 129, 126, 125, 127, 129, 130, 129, 128, 126, 125, 126, 129, 131, 131, 127, 123, 125, 129, 131, 130, 128, 129, 130, 130, 129, 127, 127, 128, 130, 129, 128, 126, 125, 126, 129, 131, 130, 128, 128, 128, 126, 125, 126, 128, 129, 128, 125, 125, 127, 129, 129, 129, 129, 127, 124, 123, 125, 128, 128, 126, 125, 125, 127, 129, 127, 126, 127, 128, 129, 129, 127, 124, 121, 123, 127, 130, 130, 128, 124, 122, 123, 127, 130, 131, 129, 125, 122, 122, 126, 128, 128, 128, 125, 
};

//Function Stubs
void InitSensors();
int16_t readTempSensor();
int16_t readHumiditySensor();
bool IntruderAlert();
void NightLightSmart();
void SpeakerAlert


char  nightlight_status;
String cont;
int usertemp;


void setup() {

   Serial.begin(9600); //For communication with the app
   InitSensors(); //Function to initialize all sensors 
  
}



void loop()
{


int16_t temp = readTempSensor();
int16_t humidity = readHumiditySensor();


//Night Light Management

if (Serial.available()) {
    nightlight_status = Serial.read();
    cont = cont + nightlight_status;

    if (nightlight_status == '*') {
        cont = cont.substring(0, cont.length() - 1); // Delete last char *

        if (cont.indexOf("NL_ON") >= 0) {
            digitalWrite(8, HIGH);
            digitalWrite(6, HIGH);
        }
        if (cont.indexOf("NL_SMART_ON") >= 0) {
            NightLightSmart();
        }
        if (cont.indexOf("NL_OFF") >= 0) {
            digitalWrite(8, LOW);
            digitalWrite(6, LOW);
        }
        cont = "";
        delay(500);
    }
}



IntruderAlert();
//Intruder alert output to bluetooth. Returns true if alert detected

char intruder;
if (IntruderAlert) {
    intruder = "ALERT";
}
else {
    intruder = "NORMAL";
}


//TODO: Climate Control
if (Serial.available()) {
    usertemp = Serial.read();

    if (usertemp > temp) {
        digitalWrite(5, LOW);
    }
    if (usertemp < temp) {
        digitalWrite(5, HIGH);
    }
}

//Bluetooth Outputs
   Serial.print((float)temp);
   Serial.print("F");
   Serial.print("|");
   Serial.print(humidity); //debugging
   Serial.print("%");
   Serial.print("|");
   Serial.print("intruder");


//OLED Display Outputs
   //clear display
   display.clearDisplay();

   // display temperature
   display.setTextSize(1);
   display.setCursor(0, 0);
   display.print("Temperature: ");
   display.setTextSize(2);
   display.setCursor(0, 10);
   display.print(temp);
   display.print(" ");
   display.setTextSize(1);
   display.cp437(true);
   display.write(167);
   display.setTextSize(2);
   display.print("F");

   // display humidity
   display.setTextSize(1);
   display.setCursor(0, 35);
   display.print("Humidity: ");
   display.setTextSize(2);
   display.setCursor(0, 45);
   display.print(humidity);
   display.print("%");

   display.display();

}

void InitSensors() {
    
   //Pin Declarations
    pinMode(7, INPUT); //humidity sensor
    pinMode(8, OUTPUT); //output for the first LED (right)
    pinMode(6, OUTPUT); //output for the second LED (left)
    pinMode(2, INPUT); //input for the PIR sensor
    pinMode(3, INPUT); //input for intruder alert PIR
    pinMode(5, INPUT); //output for climate control LED
    digitalWrite(2, LOW); //PIR 
    digitalWrite(3, LOW); //PIR
    digitalWrite(5, LOW); //set Climate Control LED to low

    //OLED Display confugiration 
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
      //for debugging purposes
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    delay(2000);
    display.clearDisplay();
    //  display.setTextSize(1);
    display.setTextColor(WHITE);
    // display.setCursor(0, 0);


}




int16_t readTempSensor()
{

    float R0 = 10000;
    float R1 = 10000;
    float B = 3936000;
    float T0 = 297.15;
    float R, ln, T, F;
    int Vo;
    Vo = analogRead(ThermistorPin);
    R = R1 * (float)Vo / (1023 - (float)Vo); // resistance value of the thermistor
    ln = log(R / R0);
    T = 1 / ((ln / B) + (1 / T0)); // temperature value in Kelvin
    F = (1.8 * (T - 273.15)) + 32; // temperature value converted to Fahrenheit

    return F;
}

int16_t readHumiditySensor()
{
    // Humidity sensor
    int ontime, offtime;
    float period, interp;
    char freq;
    char output[5];
    //float output;

    ontime = pulseIn(7, HIGH);
    offtime = pulseIn(7, LOW);
    period = ontime + offtime;
    freq = 1000000.0 / period;

    switch (freq)
    {
    case 6210 ... 6304:
        interp = 85 + (freq - 6305) * ((95 - 85) / (6210 - 6305));
        break;
    case 6305 ... 6399:
        interp = 75 + (freq - 6400) * ((85 - 75) / (6305 - 6400));
        break;
    case 6400 ... 6499:
        interp = 65 + (freq - 6500) * ((75 - 65) / (6400 - 6500));
        break;
    case 6500 ... 6599:
        interp = 55 + (freq - 6600) * ((65 - 55) / (6500 - 6600));
        break;
    case 6600 ... 6704:
        interp = 45 + (freq - 6705) * ((55 - 45) / (6705 - 6600));
        break;
    case 6705 ... 6819:
        interp = 35 + (freq - 6820) * ((45 - 35) / (6820 - 6705));
        break;
    case 6820 ... 6944:
        interp = 25 + (freq - 6945) * ((35 - 25) / (6945 - 6820));
        break;
    case 6945 ... 7080:
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

    delay(1000);

    return output;
}

bool IntruderAlert() {

    //intruder alert
    
    bool alert = false;

    if (digitalRead(3) == HIGH) {
        Serial.println("INTRUDER ALERT");  //for debugging purposes
        alert = true;
        //startPlayback(sample, sizeof(sample));
        SpeakerAlert();
    }
    else {
        Serial.println("ALL SYSTEMS NORMAL");
        alert = false;
        digitalWrite(11,LOW);
    }
    delay(1000);

    return alert;
}

void NightLightSmart() {

    //night light
    int light = analogRead(A0);
    int motion = digitalRead(2);

    // the LEDs only turn on if both of the conditions met (darkness and motion)
  // if it is dark
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
    delay(1000);
}

void SpeakerAlert(){

    for(int i=1; i<2000; i=i+10){
        digitalWrite(11,HIGH);
        delayMicroSeconds(i);
        digitalWrite(11,LOW);
        delayMicroSeconds(i);
    }
}



