#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <MFRC522.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>

// // Include the jpeg decoder library
#include <TJpg_Decoder.h>

// // Include LittleFS
#include <FS.h>
#include "LittleFS.h"

// // Call up the SPIFFS FLASH filing system this is part of the ESP Core
// #include "LittleFS.h" // ESP32 only
// #include "SPIFFS.h"   // ESP32 only

// JPEG decoder library
// #include <JPEGDecoder.h>

// Replace with your network credentials
const char *ssid = "marvin";
const char *password = "gumboots";

// API endpoint URL
const String serverName = "http://192.168.0.2:8000";

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

struct MovieInfo
{
  String title;
  String thumbUrl;
  String thumbFilename;
  unsigned int duration;
  bool hasThumbnail;
  String uid;
  bool cardNotFound;
};

MovieInfo currentMovie;

struct PlayerState
{
  bool isPlaying;
  String title;
  int currentTime;
  int duration;
};

void getPlayerState(PlayerState *s)
{
  HTTPClient http;
  // Serial.println("Checking status of media");
  http.begin(serverName + "/status"); // Specify the URL
  http.setConnectTimeout(500);
  http.setTimeout(500);              // Set timeout to 500ms
  int httpResponseCode = http.GET(); // Make GET request

  if (httpResponseCode > 0)
  {
    String payload = http.getString(); // Get the response payload

    StaticJsonDocument<200> doc;

    // Serial.println(httpResponseCode);
    // Serial.println(payload);

    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      tft.drawString("JSON Parse Error", 10, 40);
      return;
    }

    // Serial.println("deserializeJson() succeeded!");

    s->title = doc["title"].as<const char *>();
    s->isPlaying = doc["is_playing"];
    s->currentTime = doc["current_time"];
    s->duration = doc["duration"];
  }
  else
  {
    Serial.print("Error fetching player status: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void playOrPause(PlayerState *s)
{
  String action;
  if (s->isPlaying)
  {
    action = "pause";
    Serial.println("Sending pause command");
  }
  else
  {
    action = "resume";
    Serial.println("Sending resume command");
  }
  HTTPClient http;
  http.begin(serverName + "/" + action); // Specify the URL
  http.addHeader("content-type", "application/json");
  int httpResponseCode = http.POST("{}"); // Make POST request
}

void startMovie(PlayerState *s, String movie)
{
  HTTPClient http;
  http.begin(serverName + "/play"); // Specify the URL
  http.addHeader("content-type", "application/json");
  String payload = "{\"movie_name\": \"" + movie + "\"}";
  Serial.println("Start movie with payload " + payload);
  int httpResponseCode = http.POST(payload); // Make POST request
  Serial.println("Response code: " + String(httpResponseCode));
}

void stopMovie()
{
  HTTPClient http;
  http.begin(serverName + "/stop"); // Specify the URL
  http.addHeader("content-type", "application/json");
  int httpResponseCode = http.POST("{}"); // Make POST request
  Serial.println("Response code: " + String(httpResponseCode));
}

void loadPhoto()
{
  // Get the width and height in pixels of the jpeg if you wish
  uint16_t w = 0, h = 0;
  TJpgDec.getFsJpgSize(&w, &h, "/self.jpg", LittleFS); // Note name preceded with "/"
  // Serial.print("Width = ");
  // Serial.print(w);
  // Serial.print(", height = ");
  // Serial.println(h);

  // Draw the image, top left at 0,0
  TJpgDec.drawFsJpg(0, 0, "/self.jpg", LittleFS);
}

TFT_eSprite sprite = TFT_eSprite(&tft);

void showPlayerState(PlayerState *s)
{
  // Create a sprite to reduce flickering

  sprite.createSprite(tft.width(), tft.height());
  sprite.fillSprite(TFT_BLACK);
  // tft.fillScreen(TFT_BLACK);

  if (currentMovie.hasThumbnail)
  {
    TJpgDec.drawFsJpg(0, 0, currentMovie.thumbFilename, LittleFS);
  }

  sprite.setTextColor(TFT_WHITE);
  sprite.setTextSize(2);

  // loadPhoto();

  String line1 = "";
  String line2 = "";

  if (currentMovie.cardNotFound)
  {
    line1 = "Register Card:";
    line2 = currentMovie.uid;
  }
  else if (s->title == NULL)
  {
    Serial.println("No media playing on device");
    line1 = "";
    line2 = "No Media";
  }
  else
  {
    char buffer[50];
    // format the currentTime in hours and minutes
    int hours = s->currentTime / 3600;
    int minutes = (s->currentTime % 3600) / 60;
    int seconds = s->currentTime % 60;
    snprintf(buffer, sizeof(buffer), "%s %01d:%02d:%02d", s->isPlaying ? "P" : "S", hours, minutes, seconds);

    // snprintf(buffer, sizeof(buffer), "%s: %d/%d", s->isPlaying ? "P" : "S", s->currentTime / 60, s->duration / 60);
    line1 = s->title;
    line2 = buffer;
  }

  // s
  if (line1 != "")
    sprite.drawString(line1, 3, sprite.height() - (2) * sprite.fontHeight() - 6);
  if (line2 != "")
    sprite.drawString(line2, 3, sprite.height() - (1) * sprite.fontHeight() - 3);
  // sprite.drawString(statusMessage, 10, 30);

  // Serial.print("is_playing: ");
  // Serial.println(s->isPlaying);

  // Serial.print("current_time: ");
  // Serial.println(s->currentTime);

  // Serial.print("duration: ");
  // Serial.println(s->duration);

  // Serial.print("title: ");
  // Serial.println(s->title);

  // tft.fillScreen(TFT_BLACK);

  // sprite.drawString(s->title, 10, 30);

  sprite.pushSprite(0, 0);
}

// bool button1Pressed = false;
// unsigned long button1DebounceUntil = 0;

// bool button2Pressed = false;
// unsigned long button2DebounceUntil = 0;

// void IRAM_ATTR toggleButton1()
// {
//   // Serial.println("Button 1 Pressed!");
//   if (millis() < button1DebounceUntil)
//     return;
//   button1Pressed = true;
// }

// void IRAM_ATTR toggleButton2()
// {
//   if (millis() < button2DebounceUntil)
//     return;
//   button2Pressed = true;
//   // Serial.println("Button 2 Pressed!");
// }

int nextUpdate = 0;

PlayerState s;

void onButton1()
{
  Serial.println("Button 1 Pressed");
  playOrPause(&s);
}

void onButton2()
{
  Serial.println("Button 2 Pressed");
  startMovie(&s, "Frozen");
  delay(1000);
}

class RFIDReader
{
private:
  MFRC522 *mfrc522;
  String uid;
  String lastUid;
  bool cardPresent;
  unsigned long expirationTimestamp;

  bool cardRemoved;

  void setCardAbsent()
  {
    this->cardPresent = false;
    this->uid = String("");
    if (this->lastUid != String("") && this->expirationTimestamp == 0)
    {
      // Serial.println("Setting expiration timestamp");
      this->expirationTimestamp = millis() + 400;
    }
    if (millis() > this->expirationTimestamp && this->lastUid != String(""))
    {
      // Serial.println("Expiring last UID");
      this->lastUid = String("");
      this->expirationTimestamp = 0;
      this->cardRemoved = true;
    }
  }

public:
  RFIDReader(MFRC522 *mfrc) : mfrc522(mfrc)
  {
    this->uid = String("");
    this->cardPresent = false;
    this->cardRemoved = false;
  }

  bool getCardPresent()
  {
    return this->cardPresent;
  }

  bool getCardRemoved()
  {
    return this->cardRemoved;
  }

  void clearCardRemoved()
  {
    this->cardRemoved = false;
  }

  bool newCardPresent()
  {
    if (!this->cardPresent)
      return false;
    // Serial.println("Comparing " + this->uid + " to " + this->lastUid);
    return this->uid != this->lastUid;
  }

  String getUID()
  {
    this->lastUid = this->uid;
    return this->uid;
  }

  void read()
  {

    if (!this->mfrc522->PICC_IsNewCardPresent())
    {
      this->setCardAbsent();
      return;
    }

    if (!this->mfrc522->PICC_ReadCardSerial())
    {
      this->setCardAbsent();
      return;
    }

    // Serial.println("New Card Present");
    this->expirationTimestamp = 0;
    char buf[50];

    for (byte i = 0; i < mfrc522->uid.size; i++)
    {
      sprintf(&buf[i * 2], "%02X", mfrc522->uid.uidByte[i]);
    }
    buf[this->mfrc522->uid.size * 2] = '\0'; // Add null terminating character

    this->uid = String(buf);
    this->cardPresent = true;
  }
};

RFIDReader rfid(&mfrc522);

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
  sprite.setSwapBytes(true);
  sprite.pushImage(x, y, w, h, bitmap);
  sprite.setSwapBytes(false);
  // This might work instead if you adapt the sketch to use the Adafruit_GFX library
  // tft.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

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

  // if (mfrc522.PCD_PerformSelfTest())
  // {
  //   Serial.println("Passed Self-Test");
  // }
  // else
  // {
  //   Serial.println("Failed self-test");
  // }
  // mfrc522.PCD_Init(); // Init MFRC522
  // mfrc522.PCD_Reset();
  // delay(100);

  // // Fill screen with a color (optional)
  tft.fillScreen(TFT_BLACK);

  // // Set text size and color
  // tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);

  // Initialise LittleFS
  if (!LittleFS.begin())
  {
    Serial.println("LittleFS initialisation failed!");
    while (1)
      yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.");

  tft.setSwapBytes(true); // We need to swap the colour bytes (endianess)

  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);

  // // Connect to WiFi
  WiFi.begin(ssid, password);
  tft.drawString("Connecting to WiFi...", 10, 10);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    tft.drawString(".", tft.width() - 20, 10); // Indicate waiting
  }
  tft.fillScreen(TFT_BLACK);
  Serial.println("Connected to WiFi");
  tft.drawString("Connected to WiFi", 10, 10);

  // // pinMode(BUTTON1PIN, INPUT_PULLUP);
  // // pinMode(BUTTON2PIN, INPUT_PULLUP);
  // // attachInterrupt(BUTTON1PIN, toggleButton1, RISING);
  // // attachInterrupt(BUTTON2PIN, toggleButton2, RISING);
}

void loadThumbnail(String uri)
{
  String url = serverName + uri;
  Serial.println("- Fetching thumbnail from " + url);
  HTTPClient http;
  // Serial.println("Setting up HTTP client");
  http.begin(url);
  int httpResponseCode = http.GET(); // Make GET request
  if (httpResponseCode > 0)
  {
    String payload = http.getString(); // Get the response payload
    // Serial.println("Thumbnail response code: " + String(httpResponseCode));

    // Write the payload to SPIFFS as thumbnail.jpg
    currentMovie.hasThumbnail = true;
    currentMovie.thumbFilename = "/thumbnail.jpg";
    File file = LittleFS.open(currentMovie.thumbFilename, "w");
    if (file)
    {
      file.write((uint8_t *)payload.c_str(), payload.length());
      file.close();
      file.flush();
      // Serial.println("Thumbnail saved to SPIFFS");
    }
    else
    {
      Serial.println("- Failed to save thumbnail to SPIFFS");
    }

    // TJpgDec.drawJpg(0, 0, (uint8_t *)payload.c_str(), payload.length());
  }
  else
  {
    Serial.print("- thumbnail API call failed: ");
    Serial.println(httpResponseCode);
    currentMovie.hasThumbnail = false;
  }
  http.end();
}

// void getMovieInfo(String name)
// {
//   name.replace(" ", "%20");
//   HTTPClient http;
//   String url = serverName + "/movies/" + name;
//   Serial.println("Fetching movie info from " + url);
//   http.begin(url);
//   http.addHeader("content-type", "application/json");
//   int httpResponseCode = http.GET(); // Make POST request
//   Serial.println("Movie info response code: " + String(httpResponseCode));

//   if (httpResponseCode > 0)
//   {
//     String payload = http.getString(); // Get the response payload

//     StaticJsonDocument<200> doc;

//     Serial.println(httpResponseCode);
//     Serial.println(payload);

//     DeserializationError error = deserializeJson(doc, payload);
//     if (error)
//     {
//       Serial.print(F("deserializeJson() failed: "));
//       Serial.println(error.f_str());
//       tft.drawString("JSON Parse Error", 10, 40);
//       return;
//     }

//     Serial.println("deserializeJson() succeeded!");

//     currentMovie.title = doc["title"].as<const char *>();
//     currentMovie.duration = doc["duration_mins"];
//     currentMovie.thumbUrl = doc["thumb"].as<const char *>();

//     loadThumbnail(currentMovie.thumbUrl);
//   }
//   else
//   {
//     Serial.print("Fetching movie info: ");
//     Serial.println(httpResponseCode);
//   }
//   http.end();
// }

void handleCard(String uid)
{

  currentMovie.uid = uid;

  HTTPClient http;
  String url = serverName + "/cards/" + uid;
  Serial.println("Handling card " + uid + " with URL " + url);
  http.begin(url);
  http.addHeader("content-type", "application/json");
  int httpResponseCode = http.POST("{}");
  if (httpResponseCode == 404)
  {
    Serial.println("Card not found");
    currentMovie.cardNotFound = true;
    currentMovie.title = "";
    return;
  }

  currentMovie.cardNotFound = false;

  if (httpResponseCode > 0)
  {
    String payload = http.getString(); // Get the response payload
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      Serial.print(F("- Handle card returned invalid JSON: "));
      Serial.println(payload);
      Serial.println(error.f_str());
      return;
    }
    Serial.println("- Parsing response from server");
    currentMovie.title = doc["title"].as<const char *>();
    currentMovie.duration = doc["duration_mins"];
    currentMovie.thumbUrl = doc["thumb"].as<const char *>();
    http.end();
    Serial.println("- Loading thumbnail");
    loadThumbnail(currentMovie.thumbUrl);
  }
  else
  {
    Serial.print("Request to " + url + " failed, response=" + String(httpResponseCode));
  }
}

void loop()
{

  if (WiFi.status() == WL_CONNECTED)
  {
    // if (button1Pressed)
    // {
    //   onButton1();
    //   button1DebounceUntil = millis() + 500;
    //   button1Pressed = false;
    //   nextUpdate = 0;
    // }

    // if (button2Pressed)
    // {
    //   onButton2();
    //   button2DebounceUntil = millis() + 500;
    //   button2Pressed = false;
    //   nextUpdate = 0;
    // }

    // // tft.drawString("HTTP Response:", 10, 40);
    // // tft.drawString(payload.substring(0, tft.width() / tft.textWidth("HTTP Response:") - 1), 10, 60); // Display response
    if (millis() > nextUpdate)
    {
      getPlayerState(&s);
      showPlayerState(&s);
      nextUpdate = millis() + 2000;
    }
    rfid.read();

    // There's a card!
    if (rfid.newCardPresent())
    {
      String uid = rfid.getUID();
      Serial.println("New Card: " + uid);
      handleCard(uid);
    }

    if (rfid.getCardRemoved())
    {
      Serial.println("Card removed");
      rfid.clearCardRemoved();
      stopMovie();
      currentMovie.hasThumbnail = false;
      currentMovie.title = "";
    }
    delay(10);
  }
  else
  {
    // tft.fillScreen(TFT_BLACK);
    Serial.println("WiFi not connected");
    tft.drawString("WiFi not connected", 10, 10);
    delay(100);
  }
}
