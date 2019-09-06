#define pinLED 13
#include <SoftwareSerial.h>

bool start = false;
long startTime=0;
long elapsedTime=0;
long fractional;
int count = 0;


void setup() {
pinMode(A0,INPUT);
pinMode(pinLED,OUTPUT);
digitalWrite(pinLED,LOW);
digitalWrite(A0,LOW);
Serial.begin(115200);
}

void loop() {
  //Read the Serial
  if (Serial.available()>0)
  {
    // 'e' -> Stop Sweep
    // 's' -> Start Sweep
    String buff = Serial.readString();
    Serial.flush();
    // String1.indexOf(char) returns the index of where the char is found in String1 or -1 if char is not in String1
    if (buff.indexOf("e")!=-1) {start = false;}
    
    if (buff.indexOf("s")!=-1) {
      // Print 3 start codons so that the python code wont miss it
      Serial.print("s");
      Serial.print("s");
      Serial.print("s");
      
      digitalWrite(pinLED,HIGH);
      start = true;
      delay(100);
      startTime = micros();
      count = 0;
      Serial.println("");
    }
     
  }
  
  if (start){
    // Blink the LED
    if (count%50==0) {digitalWrite(pinLED,HIGH);}
    else { digitalWrite(pinLED,LOW);}
    
    printTime(startTime);
    Serial.print(",");
    int IR = analogRead(A0);
    Serial.println(IR);
    count++;
//    delay(5); // ******LOWER THIS*******
    if (count>=8000) { 
      start=false;
      // Send the Stop Codon to python
      Serial.println("***");
      digitalWrite(pinLED,LOW);
      count = 0;
    }
  }
}

void printTime(long startTime) {
  elapsedTime =   micros() - startTime;  // store elapsed time
  Serial.print( (int)(elapsedTime / 1000L));         // divide by 1000 to convert to milliseconds - then cast to an int to print
  Serial.print(".");                             // print decimal point
  fractional = (int)(elapsedTime % 1000L);
  if (fractional == 0)
    Serial.print("000");      // add three zero's
  else if (fractional < 10)    // if fractional < 10 the 0 is ignored giving a wrong time, so add the zeros
    Serial.print("00");       // add two zeros
  else if (fractional < 100)
    Serial.print("0");        // add one zero
  Serial.print(fractional);  // print fractional part of time 
}
