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




void setup() {
 
Serial.begin(9600);
pinMode(8, OUTPUT); // digital pin 13 is configured as an output for the first LED (right)
  pinMode(6, OUTPUT); // digital pin 12 is configured as an output for the second LED (left)
  pinMode(2, INPUT); // digital pin 2 is configured as an input for the PIR sensor

   pinMode(pin, INPUT); //Humidity

   pinMode(3, INPUT); //input for intruder alert PIR
 digitalWrite(3, LOW); //PIR


}

void loop() {
//Night Light
    int light = analogRead(A0);
  int motion = digitalRead(2);
  
//   Serial.println(light); // print analogRead value for photocell (trouble shooting purposes)
//   Serial.println(motion); // print digitalRead value for PIR
  
  // the LEDs only turn on if both of the conditions met (darkness and motion)
  // if it is dark
  if (light <= 100 && motion == HIGH) {
    //Serial.println("It is dark.");
    digitalWrite(8, HIGH); // turn right LED on
    digitalWrite(6, HIGH); // turn left LED on
  }
  // if there is low light
  else if (light > 100 && light < 250 && motion == HIGH) {
  //  Serial.println("There is low light.");
    digitalWrite(8, HIGH); // turn right LED on
    digitalWrite(6, LOW); // turn left LED off
  }
  // if it is bright
  else {
  //  Serial.println("It is bright");
    digitalWrite(8, LOW); // turn right LED off
    digitalWrite(6, LOW); // turn left LED off
  }
  delay(1000);

  //Humidity

  unsigned long ontime, offtime;
  double freq, period, interp;
  char output[3];

  //Serial.println(RH);
   
   ontime = pulseInLong(pin, HIGH);
   offtime = pulseInLong(pin, LOW);
   period = (double)ontime + (double)offtime;
   freq = 1000000.0/period;

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

   Serial.println("Humidity: ")
    Serial.print(output);
   delay(1000);

   //Thermistor
    Vo = analogRead(ThermistorPin);
  R = R1 * (float)Vo / (1023 - (float)Vo); // resistance value of the thermistor
  ln = log(R/R0); 
  
  T = 1 / ((ln / B) + (1 / T0)); // temperature value in Kelvin
  F = (1.8 * (T - 273.15)) + 32; // temperature value converted to Fahrenheit

//   Serial.print("Vo: "); 
//   Serial.print(Vo);
//   Serial.print(" V"); 

  Serial.print("  Temperature: "); 
  Serial.print(F);
  Serial.println(" F"); 

  delay(500);

  //Intruder

  if (digitalRead(3) == HIGH) {
        //Serial.println("INTRUDER ALERT");  //for debugging purposes
        //startPlayback(sample, sizeof(sample));
        SpeakerAlert();
    }
    else {
        //Serial.println("ALL SYSTEMS NORMAL");
        digitalWrite(11,LOW);
    }
    delay(1000);


    //displays

    //clear display
  display.clearDisplay();

  // display temperature
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Temperature: ");
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print(F);
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
  display.print(output);
  display.print(" %"); 
  
  display.display(); 

}
void SpeakerAlert(){

    for(int i=1; i<2000; i=i+10){
        digitalWrite(11,HIGH);
        delayMicroseconds(i);
        digitalWrite(11,LOW);
        delayMicroseconds(i);
    }
}
