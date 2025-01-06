/*
This example will receive multiple universes via Artnet and control a strip of ws2811 leds via
Adafruit's NeoPixel library: https://github.com/adafruit/Adafruit_NeoPixel
This example may be copied under the terms of the MIT license, see the LICENSE file for details
*/
#include <Adafruit_NeoPixel.h>
#include <ArtnetWifi.h>
#include <Arduino.h>

// Wifi settings
const char *ssid = "DzusNoci IoT";
const char *password = "FranzFerdinand62";

// led settings
#define NUM_LEDS 30
#define PIN_LEDS 28

// Artnet settings
#define UNIVERSE 0
#define START_CHANNEL 0
#define NUM_CHANNELS NUM_LEDS * 3

// led data
volatile uint8_t led_data[NUM_CHANNELS] = {0};

// Artnet settings
ArtnetWifi artnet;

// leds
Adafruit_NeoPixel leds = Adafruit_NeoPixel(NUM_LEDS, PIN_LEDS, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

// connect to wifi – returns true if successful or false if not
bool ConnectWifi(void)
{
  bool state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");

    // toggle led
    if (i % 2 == 0)
    {
      digitalWrite(LED_BUILTIN, LOW);
    }
    else
    {
      digitalWrite(LED_BUILTIN, HIGH);
    }

    if (i > 20)
    {
      state = false;
      break;
    }

    i++;
  }

  if (state)
  {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    digitalWrite(LED_BUILTIN, HIGH);
  }
  else
  {
    Serial.println("");
    Serial.println("Connection failed.");

    digitalWrite(LED_BUILTIN, LOW);
  }

  return state;
}

// update led data
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data)
{
  if (UNIVERSE != universe)
    return;

  Serial.print(data[0]);
  Serial.print(" - ");
  Serial.print(data[1]);
  Serial.print(" - ");
  Serial.println(data[2]);

  size_t copy_size = NUM_CHANNELS < (length - START_CHANNEL) ? NUM_CHANNELS : (length - START_CHANNEL);

  memcpy((void *)led_data, data + START_CHANNEL, copy_size);
}

// CORE 0 - ArtNet
void setup()
{
  // internal led
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // turn on serial log
  Serial.begin(115200);

  // connect to wifi
  ConnectWifi();

  // start the Artnet listener
  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
  artnet.read();
}

// CORE 1 - LED
void setup1()
{
  leds.begin();
}

void loop1()
{

  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds.setPixelColor(i, led_data[i * 3], led_data[i * 3 + 1], led_data[i * 3 + 2]);
  }

  leds.show();
}