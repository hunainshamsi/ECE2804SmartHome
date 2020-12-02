#define OFF 0
#define ON 1
#define TWO 2
#define THREE 3
#define FOUR 4
#define FIVE 5



//READING
int SpeakerRead(){

if (Serial.available()) {
    int second = Serial.read();

if (second == 1){
  return ON;
}
if(second == 0){
  return OFF;
}

return OFF;
}
}

void SpeakerAlert() {

  for (int i = 1; i < 2000; i = i + 10) {
    digitalWrite(11, HIGH);
    delayMicroseconds(i);
    digitalWrite(11, LOW);
    delayMicroseconds(i);
  }
}

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

int Read = SpeakerRead();

switch(Read){
    case OFF: {
        digitalWrite(11,LOW);
        break;
    }
    case ON:{
        SpeakerAlert();
        break;
    }
    
}
}
