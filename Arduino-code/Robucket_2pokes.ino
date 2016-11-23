 #include <Wire.h>                                         // reference the Wire and LiquidCrystal libraries
 #include <LiquidCrystal.h>
 #include <SD.h>
 #include "RTClib.h"

const int activePin = 0;
const int drinkPin = 1; 
const int inactivePin = 2;
const int sugarPin = 3;
const int CS_pin = 10;

RTC_DS1307 RTC;

String year, month, day, hour, minute, second, date, time;

const int sugarAmt = 100;                                  // open the solenoid for 50 ms (controls amount of sucrose solution)
int timeOut = 1;                                    // set the timeout period between pokes to 1s

int activeState = 0;                                         // initialize counter variables to 0
int lastActiveState = 0;
int activeCount = 0;
int lastActiveCount = 0;
int inactiveState = 0;
int lastInactiveState = 0;
int inactiveCount = 0;
int drinkState = 0;
int lastDrinkstate = 0;
int drinkCount = 0;
int pelletNumber = 1;
              
int pokeTime = 0;                            
int ipokeTime = 0;
int startTime = 0;
long previousMillis = 0; 
int x = 0;

LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );
int ratio = 0;
int program = 0;

int logData() {
  DateTime datetime = RTC.now();
  year = String(datetime.year(), DEC);
  month = String(datetime.month(), DEC);
  day  = String(datetime.day(),  DEC);
  hour  = String(datetime.hour(),  DEC);
  minute = String(datetime.minute(), DEC);
  second = String(datetime.second(), DEC);
 
  //Concatenate the strings into date and time
  time = month + "/" + day + " " + hour + ":" + minute + ":" + second;
  
  File dataFile = SD.open("RoBucket.csv", FILE_WRITE);
  if (dataFile) {   
    dataFile.print(ratio);
    dataFile.print(",");
    dataFile.print(time);
    dataFile.print(",");
    dataFile.print(pelletNumber-1);
    dataFile.print(",");
    dataFile.print(activeCount);
    dataFile.print(",");
    dataFile.print(inactiveCount);
    dataFile.print(",");
    dataFile.println(drinkCount);
    dataFile.close();
  }
}

int deliverSugar() {
  lcd.setCursor(14, 0);            // move the LCD cursor to row 2, column 5
  lcd.print(pelletNumber);           // update the nosepoke count on the LCD
  digitalWrite(sugarPin, HIGH);   // send 5 volts through the sugar delivery pin
    lastActiveCount = activeCount;
  pelletNumber = pelletNumber + 1;
  delay(sugarAmt);
  if (program == 4) {
     ratio = round ((5 * exp (0.2 * pelletNumber)) - 5);
  }
  digitalWrite(sugarPin, LOW);
  delay(200);
}

void setup() {                
  // initialize the digital pins as output and input
  pinMode(activePin, INPUT_PULLUP);
  pinMode(drinkPin, INPUT);
  pinMode (inactivePin, INPUT);
  pinMode (sugarPin, OUTPUT);
  pinMode (CS_pin, OUTPUT);
  
  Wire.begin();
  RTC.begin();
  
  // set up the LCD
  lcd.begin(16, 2);
  lcd.clear();
  
  if (!SD.begin(CS_pin)) {
    lcd.print("Card Failed");
    while(1) {}
    }
  
  else {
  lcd.print("Up=FR1 Rght=Mag");
  lcd.setCursor(0,1);
  lcd.print("Down=FR5 Left=PR");
  
  File dataFile = SD.open("RoBucket.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Program,Time,Sugar Deliveries,Active Pokes,Inactive Pokes,Well Entries");
    dataFile.close();
    }
  }
}

void loop() { 
  if (analogRead(0) < 50) {
    lcd.clear();
    lcd.print("Mag");
    lcd.setCursor(12,0);
    lcd.print("S:0");
    lcd.setCursor(0,1);
    lcd.print("A:0  I:0  W:0");
    startTime = millis()/1000;
    program = 2;
    ratio = random(45,75);
   }
  else if (analogRead(0) < 400) {
    lcd.clear();
    lcd.print("FR1");
    lcd.setCursor(12,0);
    lcd.print("S:0");
    lcd.setCursor(0,1);
    lcd.print("A:0  I:0  W:0");
    startTime = millis()/1000;
    program = 1;
    ratio = 1;
  }
  else if (analogRead(0) < 600) {
    lcd.clear();
    lcd.print("FR5");
    lcd.setCursor(12,0);
    lcd.print("S:0");
    lcd.setCursor(0,1);
    lcd.print("A:0  I:0  W:0");
    startTime = millis()/1000;
    program = 3;
    ratio = 5;
  }
  else if (analogRead(0) < 650) {
    lcd.clear();
    lcd.print("PR");
    lcd.setCursor(12,0);
    lcd.print("S:0");
    lcd.setCursor(0,1);
    lcd.print("A:0  I:0  W:0");
    startTime = millis()/1000;
    program = 4;
    ratio = round ((5 * exp (0.2 * pelletNumber)) - 5);
   }
  
  long t = millis()/1000 - startTime;
  
  String timeStamp;
      int h = t / 3600;
      timeStamp += h;
      timeStamp += ":";
      t = t % 3600;
      int m = t / 60;
      timeStamp += m;
      timeStamp += ":";
      int s = t % 60;
      timeStamp += s;
   
  activeState = digitalRead(activePin);  // define activeState as the readout of the nosepoke pin
  inactiveState = digitalRead(inactivePin);
  drinkState = digitalRead(drinkPin);
   
    // this portion will act as a seconds clock
    unsigned long currentMillis = millis();                   // define currentMillis
    if(currentMillis - previousMillis > 1000)             // if the specified time interval has passed...
    {
      previousMillis = currentMillis;                         // save the last time you showed time
      lcd.setCursor(4,0);                                    // bring the cursor to the first row, column 11
    
      if (program > 0) {
      lcd.print(timeStamp);                                        // display time
      if (x < 1) {
        logData();
        x=x+1;
      }
      }
    }
    
    if (program == 1) {
      if (activeState != lastActiveState) {
        if (activeState == LOW) {
          activeCount = activeCount + 1;
          lcd.setCursor(2,1);
          lcd.print(activeCount);
          if (activeCount - lastActiveCount >= ratio && t - pokeTime >= timeOut) {
            pokeTime = t;
            deliverSugar();
          }
          logData();
        }
        lastActiveState = activeState;
      }

    }
    
    if (program == 3 || program == 4) {
      if (activeState != lastActiveState) {
        if (activeState == LOW && t - pokeTime >= timeOut) {
          activeCount = activeCount + 1;
          lcd.setCursor(2,1);
          lcd.print(activeCount);
          pokeTime = t;
          if (activeCount - lastActiveCount >= ratio) {
            deliverSugar();
          }
          logData();
        }
        lastActiveState = activeState;
    }
    }
    if (program == 4 && t > 3600 || program == 4 && t - pokeTime > 300) {
      File dataFile = SD.open("RoBucket.csv", FILE_WRITE);
        if (dataFile) {   
          dataFile.print("FINAL");
          dataFile.print(",");
          dataFile.print(timeStamp);
          dataFile.print(",");
          dataFile.print(pelletNumber-1);
          dataFile.print(",");
          dataFile.print(activeCount);
          dataFile.print(",");
          dataFile.println(drinkCount);
          dataFile.close();  
      while(1) { }
      }
  }  
  if (program == 2) {
    if (t - pokeTime >= ratio) {
      deliverSugar();
      logData();
      pokeTime = t;
      ratio = random(45,75);
    }  
    
    if (activeState != lastActiveState) {
        if (activeState == LOW) {
          activeCount = activeCount + 1;
          lcd.setCursor(2,1);
          lcd.print(activeCount);
          logData();
        }
        lastActiveState = activeState;
    }
    }
   
    if (drinkState != lastDrinkstate) {
     if (drinkState == LOW) {
       drinkCount = drinkCount + 1;
       lcd.setCursor (12, 1);
       lcd.print(drinkCount);
       logData();
     }
     lastDrinkstate = drinkState;
   }
   
   if (inactiveState != lastInactiveState) {
     if (inactiveState == LOW) {
       inactiveCount = inactiveCount + 1;
       lcd.setCursor (7, 1);
       lcd.print(inactiveCount);
       logData();
     }
     lastInactiveState = inactiveState;
   }
    
   if (analogRead(0) < 900 && analogRead(0) > 750) {
      File dataFile = SD.open("RoBucket.csv", FILE_WRITE);
      if (dataFile) {   
        dataFile.print("FINAL");
        dataFile.print(",");
        dataFile.print(timeStamp);
        dataFile.print(",");
        dataFile.print(pelletNumber-1);
        dataFile.print(",");
        dataFile.print(activeCount);
        dataFile.print(",");
        dataFile.print(inactiveCount);
        dataFile.print(",");
        dataFile.println(drinkCount);
        dataFile.close();
      }
   while(1) {};
   }
}
