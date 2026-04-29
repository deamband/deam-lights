// Art-Net to NeoPixel bridge over WiFi or W5500 SPI Ethernet
//
// Libraries (install via Arduino Library Manager):
//   - "ArtnetWifi" by rstephan   (used for both WiFi and Ethernet on ESP32;
//     on the ESP32 Arduino core, WiFiUDP is an alias for NetworkUDP and works
//     over any active interface, including ETH)
//   - "Adafruit NeoPixel" by Adafruit
//
// Target: Seeed XIAO ESP32-C6. Select the matching board in the Arduino IDE.
//
// Transport selection: comment the line below for WiFi, uncomment for Ethernet.
#define USE_ETHERNET

#include <WiFi.h> // required by ArtnetWifi for WiFiUDP/NetworkUDP
#if defined(USE_ETHERNET)
#include <SPI.h>
#include <ETH.h>
#endif
#include <ArtnetWifi.h>
#include <Adafruit_NeoPixel.h>

// ---------------------- Configuration ----------------------

// WiFi credentials (used only when USE_ETHERNET is NOT defined)
const char *WIFI_SSID = "Vodafone-5D7B";
const char *WIFI_PASSWORD = "RarcGpUJMBCgqtNF";

// W5500 Ethernet pin mapping (XIAO ESP32-C6).
// Numbers are raw GPIOs; the comment shows the silkscreen D-label.
//   D8 = GPIO19 (SCK), D9 = GPIO20 (MISO), D10 = GPIO18 (MOSI)
//   D7 = GPIO17 (CS),  D2 = GPIO2  (INT),  D1  = GPIO1  (RST)
#if defined(USE_ETHERNET)
const int ETH_SCLK_PIN = 19; // D8,  purple
const int ETH_MISO_PIN = 20; // D9,  blue
const int ETH_MOSI_PIN = 18; // D10, green
const int ETH_CS_PIN = 17;   // D7,  yellow
const int ETH_INT_PIN = 2;   // D2,  orange
const int ETH_RST_PIN = 1;   // D1,  red
const uint8_t ETH_PHY_ADDR = 1;
#endif

// Art-Net universe to listen on (0-based, matches most software)
const uint16_t ART_NET_UNIVERSE = 0;

// Number of NeoPixel LEDs on the strip
const uint16_t NUM_LEDS = 20;

// GPIO pin connected to the NeoPixel data line.
// GPIO21 = silkscreen D3, picked so it doesn't collide with W5500 SPI/INT/RST.
const uint8_t LED_PIN = 21;

// NeoPixel color order. Common values:
//   NEO_GRB (most WS2812/WS2812B)
//   NEO_RGB
//   NEO_BRG, NEO_RBG, NEO_GBR, NEO_BGR
// Combine with the strip frequency, e.g. NEO_KHZ800.
const neoPixelType LED_COLOR_ORDER = NEO_GRB + NEO_KHZ800;

// Channels per LED (RGB = 3). Keep at 3 unless you switch to RGBW pixels.
const uint8_t CHANNELS_PER_LED = 3;

// DMX start channel (1-based) within the universe
const uint16_t DMX_START_CHANNEL = 1;

// -----------------------------------------------------------

ArtnetWifi artnet;
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, LED_COLOR_ORDER);

#if defined(USE_ETHERNET)
static volatile bool eth_connected = false;

void onNetEvent(arduino_event_id_t event)
{
    switch (event)
    {
    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH started");
        ETH.setHostname("ledky-artnet");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("ETH link up");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.print("ETH IP: ");
        Serial.println(ETH.localIP());
        eth_connected = true;
        break;
    case ARDUINO_EVENT_ETH_LOST_IP:
    case ARDUINO_EVENT_ETH_DISCONNECTED:
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH disconnected");
        eth_connected = false;
        break;
    default:
        break;
    }
}

void connectEthernet()
{
    WiFi.onEvent(onNetEvent);

    SPI.begin(ETH_SCLK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN, ETH_CS_PIN);
    ETH.begin(ETH_PHY_W5500, ETH_PHY_ADDR, ETH_CS_PIN, ETH_INT_PIN, ETH_RST_PIN, SPI);

    Serial.print("Waiting for Ethernet (DHCP)");
    while (!eth_connected)
    {
        delay(250);
        Serial.print('.');
    }
    Serial.println();
}
#else
void connectWifi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        Serial.print('.');
    }
    Serial.println();
    Serial.print("Connected, IP: ");
    Serial.println(WiFi.localIP());
}
#endif

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data)
{
    if (universe != ART_NET_UNIVERSE)
    {
        return;
    }

    const uint16_t startIndex = DMX_START_CHANNEL - 1;
    if (length <= startIndex)
    {
        return;
    }

    const uint16_t available = length - startIndex;
    const uint16_t maxLeds = available / CHANNELS_PER_LED;
    const uint16_t ledsToSet = (maxLeds < NUM_LEDS) ? maxLeds : NUM_LEDS;

    // Adafruit_NeoPixel::setPixelColor(i, r, g, b) handles the configured
    // color order internally, so we always feed it as R, G, B.
    for (uint16_t i = 0; i < ledsToSet; i++)
    {
        const uint16_t base = startIndex + i * CHANNELS_PER_LED;
        strip.setPixelColor(i, data[base], data[base + 1], data[base + 2]);
    }

    strip.show();

    Serial.print("ArtNet u=");
    Serial.print(universe);
    Serial.print(" seq=");
    Serial.print(sequence);
    Serial.print(" len=");
    Serial.print(length);
    Serial.print(" leds=");
    Serial.print(ledsToSet);
    Serial.print(" :");
    for (uint16_t i = 0; i < ledsToSet; i++)
    {
        const uint16_t base = startIndex + i * CHANNELS_PER_LED;
        char buf[20];
        snprintf(buf, sizeof(buf), " %u(%02X,%02X,%02X)",
                 i, data[base], data[base + 1], data[base + 2]);
        Serial.print(buf);
    }
    Serial.println();
}

void setup()
{
    Serial.begin(115200);
    delay(100);

    strip.begin();
    strip.clear();
    strip.show();

#if defined(USE_ETHERNET)
    connectEthernet();
#else
    connectWifi();
#endif

    artnet.begin();
    artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
    artnet.read();
}
