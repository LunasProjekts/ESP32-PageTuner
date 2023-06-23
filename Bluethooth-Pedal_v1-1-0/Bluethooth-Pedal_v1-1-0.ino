/*
  liberay:
    BleKeyboard [https://github.com/T-vK/ESP32-BLE-Keyboard]
    ezButton [https://arduinogetstarted.com/tutorials/arduino-button-library, https://github.com/ArduinoGetStarted/button]
  specs:
    esp32 [https://joy-it.net/en/products/SBC-NodeMCU-ESP32]
      "Esp32 Devmodule"
      "CPU Frequency: "80 MHz" // Prevent switching 2 Sites at a time
  components:
    Esp32,
    2 Buttons,
    Battery compartment (~3.3V),
    Cables
  helpful/thanks:
    https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/ // for sleep mode
    https://achnina.info/logbuch/f/t-rex-spiel-mit-gy521 // for BleKeyboard
  
*/
// Import liberaies
#include <ezButton.h> // in my case object.isPressed() and object.isReleased() // used for the debounce buttons
#include <BleKeyboard.h> // used for Bluethooth-Connection

// for ezButton.h
int DEBOUNCE_TIME = 50;

// for
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define S_TO_MIN_FACTOR 60        /* Conversion factor for seconds to minute    */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */

// for Sleep mode Check // looks when to sleep
unsigned long lastPress = millis(); // millis() of the last Button Press // init var
unsigned long lastConnection = millis(); // millis() of the last Bluethooth connection // init var
unsigned long TimeToSleep_Connection = 5*S_TO_MIN_FACTOR*1000; // 5 Min. // in ms // time when the Esp32 sleeps after the last Connection
unsigned long TimeToSleep_Press = 15*S_TO_MIN_FACTOR*1000; // 15 Min. // in ms // time when the Esp32 sleeps after the last Button Press

// init ezButton
ezButton b_left(35);  // create ezButton object that attach to pin 35, 34
ezButton b_right(34);

// init BleKeyboard
BleKeyboard bleKeyboard("DerLunaihrBP", "luna"); // to change name / metadata, see https://github.com/T-vK/ESP32-BLE-Keyboard/blob/master/README.md#api-docs

void setup() {
  Serial.begin(115200); // init Serial
  delay(500); // 0.5s // wait for Serial to realy start functioning
  
  /* print_wakeup_reason(); */ //good for debuging -> Esp32-Example/DeepSleep/TimerWakeUp

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

  Serial.println("Starting BLE work!");
  bleKeyboard.begin();
  delay(500); //0.5s // wait for bleKeyboard to realy start functioning
}

void loop() {
  SleepCheck();

  b_left.loop();  // the .loop() func is the brain of ezButton, that does the debounce
  b_right.loop(); // source-code of func https://github.com/ArduinoGetStarted/button/blob/master/src/ezButton.cpp

  if (bleKeyboard.isConnected()) {
    // for Sleep
    lastConnection = millis();

    // depaning on wireing diagramm, it´s possible that isRelesaed invertied
    if (b_left.isReleased()){
      /* Serial.println(String(b_left.isReleased())); */
      Serial.println("The LEFT is Pressed / Sending 'Page left'");
      bleKeyboard.press(KEY_LEFT_ARROW);
      delay(120);  //0.12 sec
      bleKeyboard.releaseAll();
      lastPress = millis();
    } 
    if (b_right.isReleased()){
      Serial.println(String(b_right.isReleased()));
      Serial.println("The Right is Pressed / Sending 'Page Right'");
      bleKeyboard.press(KEY_RIGHT_ARROW);
      //delay(120);  //0.12 sec
      bleKeyboard.releaseAll();
      lastPress = millis();
    }
  }
}

void sleep(){
  Serial.println("Going to sleep now");

  bleKeyboard.end();
  delay(1000);
  Serial.println("bleKeyboard off");

  Serial.flush(); 
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void SleepCheck(){
  if(millis() >= lastConnection + TimeToSleep_Connection || millis() >= lastPress + TimeToSleep_Press){ // Wenn die Letzte Verbindung oder der letzte Button-Press 15min her sind, dann
    sleep();
  }
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
