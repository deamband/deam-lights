/*
artnet_button_input

This code is designed to send Art-Net data to a lighting console based on the state of buttons connected to the ESP32.
*/
#include <ArtnetWifi.h>
#include <Arduino.h>

TaskHandle_t ArtnetCommunicationTask;
TaskHandle_t GpioDetectionTask;

const uint8_t buttonCount = 8;
const uint8_t buttonGpio[buttonCount] = {32, 33, 25, 26, 27, 14, 12, 13};
uint8_t buttonState[buttonCount] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

//Wifi settings
const char* ssid = "xxx";
const char* password = "yyy";

// Artnet settings
ArtnetWifi artnet;
const char* host = "192.168.31.255";
const int startUniverse = 1; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

// connect to wifi – returns true if successful or false if not
bool ConnectWifi(void)
{
  bool state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");
  
  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20){
      state = false;
      break;
    }
    i++;
  }
  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  
  return state;
}



void ArtnetCommunication( void * pvParameters ) {
  for(;;) {
    for (int i = 0; i < buttonCount; i++) {
      if (buttonState[i] == LOW) {
        artnet.setByte(i, 255);
      } else {
        artnet.setByte(i, 0);
      }
    }
    artnet.write();
    delay(50);
  }
}

void GpioDetection( void * pvParameters ) {
  for(;;) {
    for (int i = 0; i < buttonCount; i++) {
      buttonState[i] = digitalRead(buttonGpio[i]);
      Serial.print(buttonState[i]);
    }
    Serial.println("");

    delay(20);
  }
}

void setup()
{
  Serial.begin(115200);
  ConnectWifi();

  // broadcast artnet on all interfaces
  artnet.begin(host);
  artnet.setLength(buttonCount);
  artnet.setUniverse(startUniverse);

  for (int i = 0; i < buttonCount; i++) {
    pinMode(buttonGpio[i], INPUT_PULLUP);
  }

  xTaskCreatePinnedToCore(
    ArtnetCommunication,   /* Task function. */
    "ArtnetCommunication",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &ArtnetCommunicationTask,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */

  xTaskCreatePinnedToCore(
    GpioDetection,   /* Task function. */
    "GpioDetection",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &GpioDetectionTask,      /* Task handle to keep track of created task */
    1);          /* pin task to core 1 */
}

void loop() {

}
