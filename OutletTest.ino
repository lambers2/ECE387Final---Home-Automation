//#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <elapsedMillis.h>
#include <BitVoicer11.h>
#include <LiquidCrystal.h>

//Instantiates the BitVoicerSerial class
BitVoicerSerial bvSerial = BitVoicerSerial();
int dht11pin = 7;
#define DHTTYPE DHT11   // DHT 11
DHT dht(dht11pin,DHTTYPE);

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//Stores true if the Audio Streaming Calibration tool
//is running
boolean sampleTest = false;
//Stores the data type retrieved by getData()
byte dataType = 0;
//Sets up the pins and default variables

int butPin = 13;
int pinOutL = 8;
int pinOutS = 9;
int pinOutT = 10;
byte pinValL = 0;
byte pinValS = 0;
byte pinValT = 0;

int interval = 5000;
int seconds = 0;
int temp = 0;



/* cli();             // disable global interrupts
 TCCR1A = 0;        // set entire TCCR1A register to 0
 TCCR1B = 0;
 
    // enable Timer1 overflow interrupt:
    TIMSK1 = (1 << TOIE1);
    // Set CS10 bit so timer runs at clock speed:
    TCCR1B |= (1 << CS10);
    // enable global interrupts:
    sei();*/

void setup()
{
  
 //Sets the analog reference to external (AREF pin)
 //WARNING!!! If anything is conected to the AREF pin,
 //this function MUST be called first. Otherwise, it will
 //damage the board.
 bvSerial.setAnalogReference(BV_EXTERNAL);
 //Sets up the microcontroller to perform faster analog reads
 //on the specified pin
 bvSerial.setAudioInput(0);
 //Starts serial communication at 115200 bps
 Serial.begin(115200);
// currentMillis = millis();
 pinMode(pinOutL,OUTPUT);
 pinMode(pinOutS,OUTPUT);
 pinMode(pinOutT,OUTPUT);
 pinMode(butPin,INPUT);
 digitalWrite(pinOutL,pinValL);
 digitalWrite(pinOutS,pinValS);
 digitalWrite(pinOutT,pinValT);
 lcd.begin(16, 2);
 dht.begin();
// sinceLastInput = 0;
 
    // initialize Timer1
    cli();          // disable global interrupts
    TCCR1A = 0;     // set entire TCCR1A register to 0
    TCCR1B = 0;     // same for TCCR1B
 
    // set compare match register to desired timer count:
    OCR1A = 15624;
    // turn on CTC mode:
    TCCR1B |= (1 << WGM12);
    // Set CS10 and CS12 bits for 1024 prescaler:
    TCCR1B |= (1 << CS10);
    TCCR1B |= (1 << CS12);
    // enable timer compare interrupt:
    TIMSK1 |= (1 << OCIE1A);
    // enable global interrupts:
    sei();
}

ISR(TIMER1_COMPA_vect){
  seconds++;
  if(seconds == 20){
    seconds = 0;
  pinValL = LOW;
  pinValS = LOW;
  pinValT = LOW;
  }
}

void loop()
{

  lcd.display();
  lcd.setCursor(0,0); 
  lcd.print("        ");
 // lcd.print(temp);
 // lcd.print((char)223);
  lcd.print("        ");
  lcd.setCursor(0,1);
  //lcd.display();
  
  //Serial.println("IN LOOP!!!");
 //Captures audio and sends it to BitVoicer if the Audio
 //Streaming Calibration Tool is running
 if (sampleTest == true)
 {
   //The value passed to the function is the time
   //(in microseconds) that the function has to wait before
   //performing the reading. It is used to achieve about
   //8000 readings per second.
   bvSerial.processAudio(46);
 }
 
 //Captures audio and sends it to BitVoicer if the Speech
 //Recognition Engine is running
 if (bvSerial.engineRunning)
 {
   //The value passed to the function is the time
   //(in microseconds) that the function has to wait before
   //performing the reading. It is used to achieve about
   //8000 readings per second.
   bvSerial.processAudio(46);
 }

  digitalWrite(pinOutL,pinValL);
  digitalWrite(pinOutS,pinValS);
  digitalWrite(pinOutT,pinValT);

 /* if(sinceLastInput > interval){
    pinValL = LOW;
    pinValS = LOW;
    pinValT = LOW;
    sinceLastInput -= interval;
  }*/
  
  if(pinValT==pinValS || pinValS == pinValT){
    pinValS = LOW;
  }

  
  int  val = digitalRead(butPin);
  if(val == LOW){
  lcd.setCursor(0,0); 
  lcd.print("Temp: ");
  temp = dht.readTemperature();
  temp = dht.convertCtoF(temp);
  lcd.print(temp);
  lcd.print((char)223);
  lcd.print("F");
  lcd.setCursor(0,1);
  delay(2000);
  lcd.noDisplay();
  temp = 0;
}
}

//This function runs every time serial data is available
//in the serial buffer after a loop
void serialEvent()
{
  
 //Reads the serial buffer and stores the received data type
 dataType = bvSerial.getData();
 
 //Changes the value of sampleTest if the received data was
 //the start/stop sampling command
 if (dataType == BV_COMMAND)
     sampleTest = bvSerial.cmdData;
 
 //Signals BitVoicer's Speech Recognition Engine to start
 //listening to audio streams after the engineRunning status
 //was received
 if (dataType == BV_STATUS && bvSerial.engineRunning == true)
   bvSerial.startStopListening();
 
 //Checks if the data type is the same as the one in the
 //Voice Schema
 if (dataType == BV_STR)
   setLEDs();
   //sinceLastInput = 0;
   seconds = 0;
}

//Performs the LED changes according to the value in
//bvSerial.strData
void setLEDs()
{
 //Turn light on
 if (bvSerial.strData == "LON")
 {
  pinValL = HIGH;
 }
 //Turn light off
 else if (bvSerial.strData == "LOF")
 {
  pinValL = LOW; 

 } //Turn TV on
 else if (bvSerial.strData == "TON")
 {
  pinValT = HIGH;
 }//Turn TV off
 else if (bvSerial.strData == "TOF")
 {
  pinValT = LOW; 
 
 }//Turn Soundbar on 
 else if (bvSerial.strData == "SON")
 {
  pinValS = HIGH;
 }//Turn Soundbar off
 else if (bvSerial.strData == "SOF")
 {
  pinValS = LOW; 
 

 }//If no commands are recognized, return an error
 else {
  
   bvSerial.startStopListening();
   bvSerial.sendToBV("ERROR:" + bvSerial.strData);
   bvSerial.startStopListening();
 }
}
