void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
int receive;
int light = analogRead(A0);
int motion = digitalRead(2);
  if(Serial.available()>0){
    if(receive == 2){
      digitalWrite(8, HIGH);
      digitalWrite(6, HIGH);
    }
    else if(receive == 3){
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
    else if(receive == 4){
      digitalWrite(8, LOW);
       digitalWrite(6, LOW);
    }
    delay(500);
    
  }

 
}
