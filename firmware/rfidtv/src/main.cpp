#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <MFRC522.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>

// Include the jpeg decoder library
#include <TJpg_Decoder.h>

// Include LittleFS
#include <FS.h>
#include "LittleFS.h"
#include "rfidreader.h"
#include "playerStateManager.h"
#include "rfapi.h"

#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <ArduinoLog.h>
#include <WebServer.h>

#define LOG_LEVEL LOG_LEVEL_INFO

// API endpoint URL
#define SDA_PIN 33
#define SCK_PIN 25
#define MISO_PIN 27
#define MOSI_PIN 26
#define RST_PIN 17

#define BUTTON1PIN 35
#define BUTTON2PIN 0

MFRC522 mfrc522(SDA_PIN, RST_PIN);

// Create an instance of the TFT_eSPI class
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

String serverUri = "http://192.168.0.2:8000";

WebServer server(80);

RFIDReader rfid(&mfrc522);
RFApi rfApi(serverUri);

PlayerStateManager playerStateManager(&tft, &sprite, &rfApi);

int nextUpdate = 0;

// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
  // Stop further decoding as image is running off bottom of screen
  if (y >= tft.height())
    return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  // tft.pushImage(x, y, w, h, bitmap);
  playerStateManager.sprite->setSwapBytes(true);
  playerStateManager.sprite->pushImage(x, y, w, h, bitmap);
  playerStateManager.sprite->setSwapBytes(false);
  // This might work instead if you adapt the sketch to use the Adafruit_GFX library
  // tft.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}

void setupWifi()
{
  // Load Wifi credentials from file
  File file = LittleFS.open("/settings.json", "r");
  String ssid = "";
  String password = "";
  String hostname = "";
  if (file)
  {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    if (!error)
    {
      ssid = doc["ssid"].as<String>();
      password = doc["password"].as<String>();
      hostname = doc["hostname"].as<String>();
      Log.infoln("Loaded settings ssid=%s, password=%s, hostname=%s", ssid.c_str(), password.c_str(), hostname.c_str());
    }
    file.close();
  }
  else
  {
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setRotation(3);
    tft.drawString("Failed to open", 10, 10);
    tft.drawString("settings.json", 10, 30);
    Log.errorln("Failed to open settings.json");
    Log.errorln("Unable to continue boot process");
    while (1)
    {
      sleep(1000);
    }
  }

  // Connect to WiFi
  WiFi.setHostname(hostname.c_str());
  WiFi.begin(ssid, password);
  tft.drawString("Connecting to WiFi...", 10, 10);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Log.infoln("Connecting to WiFi...");
    tft.drawString(".", tft.width() - 20, 10); // Indicate waiting
  }
  tft.fillScreen(TFT_BLACK);
  Log.infoln("Connected to WiFi");
  tft.drawString("WiFi Connected", 10, 10);
}

void setupOTA()
{
  ArduinoOTA
      .onStart([]()
               {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Log.infoln("Start updating %s", type.c_str()); })
      .onEnd([]()
             { Log.infoln("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Log.traceln("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Log.errorln("Error[%u]: ", String(error).c_str());
      if (error == OTA_AUTH_ERROR) {
        Log.errorln("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Log.errorln("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Log.errorln("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Log.errorln("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Log.errorln("End Failed");
      } });

  ArduinoOTA.begin();

  Log.infoln("OTA Ready");
  Log.infoln("IP address: %s", WiFi.localIP().toString().c_str());
}

void handleGetPlayerStatus()
{
  JsonDocument doc;
  doc["cardPresent"] = playerStateManager.cardPresent();
  doc["cardUid"] = playerStateManager.cardUid();
  String response = "";
  serializeJson(doc, response);
  server.send(200, "application/json", response.c_str());
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  Log.begin(LOG_LEVEL, &Serial);
  Log.setShowLevel(true);

  // Serial.println(SS);
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, HIGH);

  // Initialize the TFT screen MUST happen before SPI.begin()
  tft.init();
  // tft.setRotation(1); // Adjust rotation as needed (0-3)

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN); // Init SPI bus
  mfrc522.PCD_Init();                     // Init MFRC522
  delay(5);
  mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details

  // // Fill screen with a color (optional)
  tft.fillScreen(TFT_BLACK);

  // // Set text size and color
  // tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);

  // Initialise LittleFS
  if (!LittleFS.begin())
  {
    Log.errorln("LittleFS initialisation failed!");
    while (1)
      yield(); // Stay here twiddling thumbs waiting
  }
  Log.infoln("Initialisation done.");

  tft.setSwapBytes(true); // We need to swap the colour bytes (endianess)

  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);

  setupWifi();

  setupOTA();

  server.on("/status", handleGetPlayerStatus);
  server.begin();
}

void loop()
{

  if (WiFi.status() == WL_CONNECTED)
  {
    ArduinoOTA.handle();
    server.handleClient();
    if (millis() > nextUpdate)
    {
      playerStateManager.refresh();
      playerStateManager.show();
      nextUpdate = millis() + 2000;
    }
    rfid.read();

    // There's a card!
    if (rfid.newCardPresent())
    {
      String uid = rfid.getUID();
      Log.infoln("Card placed: %s", uid.c_str());
      playerStateManager.cardPlaced(uid);
    }

    if (rfid.getCardRemoved())
    {
      Log.infoln("Card removed");
      rfid.clearCardRemoved();
      playerStateManager.cardRemoved();
    }
    delay(10);
  }
  else
  {
    Log.warningln("WiFi not connected");
    tft.drawString("WiFi not connected", 10, 10);
    delay(100);
  }
}
