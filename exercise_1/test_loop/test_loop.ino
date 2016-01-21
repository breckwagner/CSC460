
char incomingByte = 0;   // for incoming serial data
char EOT = 0x04;
int count = 0;
int LINE_WIDTH = 30;


void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
}


void serialEvent() {
  while (Serial.available()) {
    Serial1.print((char)Serial.read());
  }
}

void serialEvent1(){
  while (Serial1.available()) {
    incomingByte = (char)Serial1.read();
    if(incomingByte == EOT) {
      Serial.print("\nNumber of Bytes sent/recieved: ");
      Serial.print(count);
      Serial.print("\n");
      count = 0;
    } else {
      if(count%LINE_WIDTH==0) Serial.println();
       count++;
      Serial.print(incomingByte);
    }
  }
}

void loop() {}


