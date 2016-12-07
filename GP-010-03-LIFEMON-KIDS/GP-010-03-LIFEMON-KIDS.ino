// GarageProto sketch using Adafruit libs

#include <OneWire.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <Adafruit_MLX90614.h>
#include <SPI.h>
#include <SD.h>
#include <RTCZero.h>
#include "Sodaq_DS3231.h"

//BUZZER
const int buzzerPin = 9; //was 9
//const int ledPin = 13;    // was 7
// tone frequency C
const int tonefreq = 523;
const int dotlength = 100;
const int dashlength = dotlength * 3;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 5   // MIkey was 2 arduino
#define INT1 10         // RTCzero sleep interupt PIR WAS 0RX PIN DEBUG
#define VBATPIN A7     // USED to get battery voltage ONLY on Feather M0
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

int ledPin = 13;                 // choose the pin for the LED
int inputPin = 12;               // choose the input pin (for PIR sensor)
int pirState = LOW;              // we start, assuming no motion detected
int val = 0;                     // variable for reading the pin status
long i = 0;
volatile boolean pirActive = false;
float vbatAvg = 0.000;         // BAT VOLTAGE

float DS_temp = 0.00 ;
float MLX_temp = 0.00;

// Set the pins used
#define chipSelect 4



RTCZero rtcSleep;
File logfile;
///OneWire  ds(5);  // on pin 10

// ################################################################ SETUP START ############################################################################

void setup() {
 // DS3232 RTC test REMOVE

Serial.begin(9600);
rtc.begin();
//rtc.setDateTime(dt); //Adjust date-time as defined 'dt' above 

//while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
// }
 
// UNUSED PINS now 460uA (was 756uA without it!)
pinMode(8, OUTPUT);      // sets the digital pin as output This is the GREEN LED for the SD card
pinMode(10, OUTPUT);      // sets the digital pin as output
pinMode(11, OUTPUT);      // sets the digital pin as output
pinMode(12, OUTPUT);      // sets the digital pin as output
pinMode(14, OUTPUT);      // sets the digital pin as output
pinMode(15, OUTPUT);      // sets the digital pin as output
pinMode(16, OUTPUT);      // sets the digital pin as output
pinMode(17, OUTPUT);      // sets the digital pin as output
pinMode(18, OUTPUT);      // sets the digital pin as output
pinMode(19, OUTPUT);      // sets the digital pin as output
pinMode(1, OUTPUT);      // sets the digital pin as output
pinMode(0, OUTPUT);      // sets the digital pin as output
// PIR sensor PIN AS INPUT
pinMode(10, INPUT);      // sets the digital pin as INPUT fro PIR sensor

// MAXIM SWITCH ENABLE/DISABLE PIN
pinMode(6, OUTPUT);      // sets the digital pin as output

  digitalWrite(ledPin, LOW);   // sets the LED OFF
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
  }
  
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
  mlx.begin();  

  pinMode(ledPin, OUTPUT);      // declare LED as output
  pinMode(inputPin, INPUT);     // declare sensor as input
  pinMode(19, OUTPUT);      // sets the digital pin as output 
  //digitalWrite(19, HIGH);   // sets the MAXIM switch to ON

  // RTC Sleep mode stuff

  rtcSleep.begin();
  rtcSleep.setEpoch(1451606400); // Jan 1, 2016
  // Set the alarm at the 10 second mark
  rtcSleep.setAlarmSeconds(10);
  // Match only seconds (Periodic alarm every minute)
  rtcSleep.enableAlarm(RTCZero::MATCH_SS);

  // #### TEST CODE REMOVE
  //Attach the interrupt and set the wake flag 
  // GOLD WORKS attachInterrupt(INT1, ISR, CHANGE);
  attachInterrupt(INT1, ISR, RISING);
    
  // Set the XOSC32K to run in standby
  SYSCTRL->XOSC32K.bit.RUNSTDBY = 1;

  // Configure EIC to use GCLK1 which uses XOSC32K 
  // This has to be done after the first call to attachInterrupt()
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCM_EIC) | 
                      GCLK_CLKCTRL_GEN_GCLK1 | 
                      GCLK_CLKCTRL_CLKEN;
  // ### TEST CODE END
  // TEST REMOVE  LPC_SC->PCON = 0x0;
  // Set sleep mode
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  //__DSB();
  // Sleep sketch startup delay
  delay(1000); //Debug was 10seconds
}

// ################################################################## SETUP END ###########################################################################

// --------------------------------------------------------------------------------------------------------------------------------------------------------

// ################################################################## LOOP START ###########################################################################
void loop() {
  digitalWrite(6, HIGH);         // ENABLE POWER TO SENSORS FROm MAXIm SWITCH
  digitalWrite(ledPin, HIGH);   // sets the LED ON
  delay(100);                  // Give time to Sensors to power up
  digitalWrite(ledPin, LOW);   // sets the LED OFF
sensors.requestTemperatures(); // Send the command to get temperatures
// OK TES Writing Data to the file
//String dataString = "";

float vbatReturn = 0.00;       // RESET BAT VOLTAGE VAR

//File dataFile = SD.open("LIFEMONB.txt", FILE_WRITE); // BREADBAORD
//File dataFile = SD.open("LIFEMONC.txt", FILE_WRITE); // PCB BOARD
// File dataFile = SD.open("LIFEMOND.txt", FILE_WRITE); // PCB BOARD GP-010-02
File dataFile = SD.open("LIFEMOND.txt", FILE_WRITE); // PCB BOARD GP-010-03 KIDS KIT
// if the file is available, write to it:
  if (dataFile) {
    DateTime now = rtc.now(); //get the current date-time from DS3232!!
    digitalWrite(8, HIGH);                       // GREEN LED FOR SD Tells US CARD HAS bEEN ACCESSED
    dataFile.print(sensors.getTempFByIndex(0));  // DS18B20
    dataFile.print(",");
    dataFile.print(mlx.readAmbientTempF());      // MLX90614 Ambient TI
    dataFile.print(",");
    dataFile.print(mlx.readObjectTempF());       // MLX90614 Object T0
    dataFile.print(",");
    //dataFile.print(checkSensor());             // PIR Sensor
    dataFile.print(pirActive);                   // PIR Sensor
    dataFile.print(",");
    //dataFile.println(rtc.getEpoch());          // GET EPOCH TIME
    dataFile.print((getVbat()/ 50),3);         // GET VOLATAGE ON VBAT SAMPLED 50 times so div by 50
    dataFile.print(",");
    dataFile.print(now.year(), DEC);
    dataFile.print('-');
    dataFile.print(now.month(), DEC);
    dataFile.print('-');
    dataFile.print(now.date(), DEC);
    dataFile.print(' ');
    
    if ((now.hour(), DEC) < 10)
       dataFile.print('0'); // Pad the 0
    dataFile.print(now.hour());
       
    dataFile.print(':');

    if ((now.minute(), DEC) < 10)
      dataFile.print('0'); // Pad the 0
    dataFile.print(now.minute(), DEC);
    
    dataFile.print(':');

    if ((now.second(), DEC) < 10)
      dataFile.print('0'); // Pad the 0
    dataFile.print(now.second(), DEC);
    dataFile.print(" "); // Pad the 0

    switch(now.dayOfWeek()){
  case 0:
    dataFile.print("Sunday");
    break;
  case 1:
    dataFile.print("Monday");
    break;
  case 2:
    dataFile.print("Tuesday");
    break;
  case 3:
    dataFile.print("Wednesday");
    break;
  case 4:
    dataFile.print("Thursday");
    break;
  case 5:
    dataFile.print("Friday");
    break;
  case 6:
    dataFile.print("Saturday");
    break;
  }
    
    
    //dataFile.print(now.dayOfWeek());
    dataFile.println();
    dataFile.close();                             // CLOSE FILE ON SD
    digitalWrite(8, LOW);                         // GREEN LED FOR SD WE MAGE IT THROUGH THE CODE!!
    pirActive = false;                            // toggle pirActive variable
    digitalWrite(6, LOW); // DISABLE POWER TO SENSORS FROM MAXIM SWITCH
  }
  else {
    //Serial.println("error opening datalog.txt");
    
  }

   // BUZZ
  //tone(buzzerPin, tonefreq);
//digitalWrite(ledPin, HIGH);
//delay(dotlength);
//delay(dashlength);
//noTone(buzzerPin);
  digitalWrite(6, LOW); // DISABLE POWER TO SENSORS FROM MAXIM SWITCH
  __WFI();
}

// ################################################################## LOOP END ###########################################################################

int checkSensor(bool){ 
    if (pirActive == true)
    {
      //pirActive = !pirActive;                // toggle pirActive variable
      //pirActive = false;
      return pirActive ;
    }
}

void ISR()
{
  //detachInterrupt(INT1);
  pirActive = true;
  //delayMicroseconds(3500000); // 5 seconds

 
}

float getVbat(){
    vbatAvg = 0.00;
    for (float i=0; i < 50; i++){
      float measuredvbat = analogRead(VBATPIN);
    measuredvbat *= 2;    // we divided by 2, so multiply back
    measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
    measuredvbat /= 1024; // convert to voltage
    vbatAvg = vbatAvg + measuredvbat;
   } 
    Serial.print("VBat: " ); Serial.println((vbatAvg / 50),3);
    return vbatAvg;
}




