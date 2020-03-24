#include <EEPROM.h>

// Set all pins for project
const int LED_ARMED = 10;
const int READY_LED = 12;
const int Pepper_LED = 8;
const int Siren_LED = 6;
const int Arm_Button = 4;
const int PIR_Button = 2;

// Set Variables
int ArmState = 0;
int AlarmedState = 0;
int LastArmState = 0;
int Armed = 0;
int Alarmed = 0;
int Ready = 0;
unsigned long previousMillis1 = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 1000;
unsigned long alarmedDebounceDelay = 5000;
int alarmedtimes = 0;
unsigned long lastAlarmTime = 0;
int addr = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);                                    // Init serial
  // Set pinmodes
  pinMode(LED_ARMED, OUTPUT);
  pinMode(READY_LED, OUTPUT);
  pinMode(Pepper_LED, OUTPUT);
  pinMode(Siren_LED, OUTPUT);
  pinMode(PIR_Button, INPUT);
  pinMode(Arm_Button, INPUT);
  Armed = EEPROM.read(addr);                             // Set Armed Var from EEPROM
  delay(5000);                                           // Delay so PIR can warmup
}

void loop() {
  // put your main code here, to run repeatedly:
  // Set var that reads state of 
  ArmState = digitalRead(Arm_Button);
  AlarmedState = digitalRead(PIR_Button);
  ArmStateChange();
  AlarmedStateChanged();
  // If Alarm is Disarmed and PIR is not detecting movement 
  // Switch Green, Ready LED on
  if (!Armed && !AlarmedState){
    digitalWrite(READY_LED, HIGH);
    Ready = 1;
  }else{ // If Armed or PIR detects movement Switch Green, Ready LED off
    digitalWrite(READY_LED, LOW);
    Ready = 0;
  }
  if (Armed){                                            // If Alarm has been Armed Switch Red Armed LED on
    digitalWrite(LED_ARMED, HIGH);
  }else{
    digitalWrite(LED_ARMED, LOW);                        // If not Armed Switch Red Armed LED off
  }
   if (Alarmed && Armed){                                // If System is Armed and PIR detects movement Run the siren and pepper sequence
     Siren_Pepper();
   }
}

int ArmStateChange(){
  if (ArmState){                                         // If Arm button is pressed
    if ((millis() - lastDebounceTime) > debounceDelay) { // Makes sure debounce doesn't happen by not allowing 
                                                         // arming to happen for 1 sec after last arm
      if (!Armed && Ready){                              // If PIR not activated and not yet Armed Arm
        Armed = !Armed;                                  // Set Armed to true
        EEPROM.update(addr, 1);                          // Set armed state to EEPROM
        alarmedtimes = 0;                                // reset swinger shutdown
        lastDebounceTime = millis();                     // reset debounce for arming
      }else if (Armed){                                  // If already Armed
        Armed = !Armed;                                  // set Armed to False
        EEPROM.update(addr, 0);                          // Set Armed state to EEPROM
        lastDebounceTime = millis();                     // reset debounce for arming
      }
    }
    if (Alarmed){                                        // If Alarmed at the time of Disarming
      Alarmed = !Alarmed;                                // Set Alarmed to False
    }
    return Armed, Alarmed;
  }
}

int AlarmedStateChanged(){
  if (Armed && AlarmedState){                            // If Armed and PIR is activated
    if (!Alarmed){                                       // If not already Alarmed
      Alarmed = !Alarmed;                                // Set Alarmed to true
    }
  }
}

void Siren_Pepper(){
  unsigned long CounterStart = millis();                 // Set a time to compair
  unsigned long CounterEnd = CounterStart;               // Set a time to compair
  int delay123 = 10000;                                  // Set a time period of 10 seconds
  int siren_delay = 5000;                                // Set a time period of 5 seconds
  unsigned long siren_last = CounterStart;               // Set a time to compair
  digitalWrite(Siren_LED, HIGH);                         // Activate siren
  while ((millis() - siren_last) < siren_delay){         // While delay is not over
    if (digitalRead(Arm_Button)){                        // Check if Arm button is pushed
      ArmStateChange();
      Alarmed = !Alarmed;                                // Set Alarmed to false
      digitalWrite(Siren_LED, LOW);                      // Deactivate siren
      return;
    }
  }
  Pepper();
  while (Alarmed){                                       // While Alarmed equil to true
    ArmStateChange();
    if ((millis() - siren_last) >= siren_delay){         // If delay is over
      if (digitalRead(PIR_Button)){                      // If PIR gets Activated again
        if ((millis() - lastAlarmTime) > alarmedDebounceDelay) {  // If PIR has not been activated in a period of time
          if (alarmedtimes <= 3){                        // If swinger shutdown is not true
            CounterEnd = millis();                       // Reset time to compair to
            lastAlarmTime = millis();                    // Reset last time system has alarmedDebounceDelay
            Pepper();
          }
        }
      }
    }
    ArmStateChange();
    if (digitalRead(Arm_Button)){                        // Check if Arm button is pushed
      ArmStateChange();
      Alarmed = !Alarmed;                                // Set Alarmed to False
      digitalWrite(Siren_LED, LOW);                      // Deactivate siren
    }else if ((millis() - CounterEnd) >= delay123){      // If siren is on for longer than x amount of time
      Alarmed = !Alarmed;                                // Set Alarmed to False
      digitalWrite(Siren_LED, LOW);                      // Deactivate siren
    }
  }
}

void Pepper(){
  for (int i = 0; i < 5; i++){                           // Run for loop 5 times
    if (digitalRead(Arm_Button)){                        // Check if Arm button is pushed
      ArmStateChange();
      Alarmed = !Alarmed;                                // Set Alarmed to False
      digitalWrite(Siren_LED, LOW);                      // Deactivate siren
      digitalWrite(Pepper_LED, LOW);                     // Deactivate pepper
    }
    digitalWrite(Pepper_LED, HIGH);                      // Press pepper spray for 1 second
    delay(1000);
    digitalWrite(Pepper_LED, LOW);                       // Stop pressing pepper
    delay(5000);                                         // Wait 5 seconds then Press pepper again
  }
  lastAlarmTime = millis();                              // Reset time of last alarm
  alarmedtimes++;                                        // Add 1 to times Alarmed for swinger shutdown
}
