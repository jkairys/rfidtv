#include "playerStateManager.h"
#include <TJpg_Decoder.h>
#include <ArduinoLog.h>
#include <FS.h>

PlayerStateManager::PlayerStateManager(TFT_eSPI *tft, TFT_eSprite *sprite, RFApi *rfApi)
{
  this->tft = tft;
  this->rfApi = rfApi;
  this->sprite = sprite;
  this->card = NULL;
  this->_cardPresent = false;

  this->thumbnail = MovieThumbnail();
  this->thumbnail.available = false;
  this->thumbnail.uri = "";
  this->thumbnail.spiffsFilename = "/thumbnail.jpg";
}

void PlayerStateManager::setState(PlayerState *newState)
{
  if (this->state != NULL)
  {
    Log.verboseln("Deleting old state");
    delete (this->state);
  }
  this->state = newState;
}

void PlayerStateManager::refresh()
{
  // Log.verboseln("PlayerStateManager::refresh()");
  PlayerState *newState = this->rfApi->getState();
  this->setState(newState);
}

bool PlayerStateManager::cardPresent()
{
  return this->_cardPresent;
}

String PlayerStateManager::cardUid()
{
  if (this->cardPresent())
  {
    return this->card->uid;
  }
  return "";
}

void PlayerStateManager::setCard(CardInfo *card)
{
  if (this->card != NULL)
  {
    Log.verboseln("Deleting old card");
    delete this->card;
  }
  this->card = card;
}

void PlayerStateManager::cardPlaced(String uid)
{

  CardInfo *card = new CardInfo();

  card->uid = uid;
  this->_cardPresent = true;

  if (this->card != NULL && this->card->uid == uid)
  {
    Log.infoln("Card has been put back, resuming playback");
    this->rfApi->resumeMovie();
    this->setCard(card);
    return;
  }

  MovieInfo movie = this->rfApi->setCard(uid);
  if (!movie.isNull)
  {
    card->isUnregistered = false;
    this->currentMovie = &movie;
    if (movie.thumbUrl.length() > 0)
    {
      Log.infoln("Loading thumbnail");
      this->thumbnail.uri = movie.thumbUrl;
      this->loadThumbnail();
    }
    else
    {
      this->thumbnail.uri = "";
      this->thumbnail.available = false;
      Log.warningln("No thumbnail found");
    }
  }
  else
  {
    card->isUnregistered = true;
    this->currentMovie = NULL;
    this->thumbnail.available = false;
  }

  this->setCard(card);
  return;
}

void PlayerStateManager::cardRemoved()
{
  this->_cardPresent = false;
  this->rfApi->pauseMovie();
  this->state->isPlaying = false;
  // this->currentMovie = NULL;
  // this->card = NULL;
}

void PlayerStateManager::loadThumbnail()
{
  this->thumbnail.available = false;
  Log.infoln("Loading thumbnail from uri=%s", this->thumbnail.uri.c_str());
  String payload = this->rfApi->getThumbnail(this->thumbnail.uri);
  if (payload.length() == 0)
  {
    Log.warningln("Failed to load thumbnail");
    return;
  }

  File file = LittleFS.open(this->thumbnail.spiffsFilename, "w");
  if (file)
  {
    file.write((uint8_t *)payload.c_str(), payload.length());
    file.close();
    file.flush();
    this->thumbnail.available = true;
  }
  else
  {
    Log.errorln("failed to save thumbnail to SPIFFS %s", this->thumbnail.spiffsFilename.c_str());
  }
}

void PlayerStateManager::show()
{
  Log.verboseln("PlayerStateManager::show()");

  // Create a sprite to reduce flickering

  this->sprite->createSprite(this->tft->width(), this->tft->height());
  this->sprite->fillSprite(TFT_BLACK);

  if (this->thumbnail.available)
  {
    TJpgDec.drawFsJpg(0, 0, this->thumbnail.spiffsFilename, LittleFS);
  }

  this->sprite->setTextColor(TFT_WHITE);

  String line1 = "";
  String line2 = WiFi.localIP().toString();
  if (this->card && this->card->isUnregistered)
  {
    line1 = this->card->uid;
    // line2 = "Register Card:";
  }
  else if (this->state == NULL)
  {
    line1 = "No Media";
    // line2 = "";
  }
  else
  {
    char buffer[50];
    // format the currentTime in hours and minutes
    int hours = this->state->currentTime / 3600;
    int minutes = (this->state->currentTime % 3600) / 60;
    int seconds = this->state->currentTime % 60;
    snprintf(buffer, sizeof(buffer), "%s %01d:%02d:%02d", this->state->isPlaying ? "P" : "S", hours, minutes, seconds);
    line1 = buffer;

    // snprintf(buffer, sizeof(buffer), "%s: %d/%d", s->isPlaying ? "P" : "S", s->currentTime / 60, s->duration / 60);
    // line1 = this->state->title;
  }

  Log.traceln("Drawing %s, %s", line1.c_str(), line2.c_str());
  if (line1 != "")
  {
    this->sprite->setTextSize(2);
    this->sprite->drawString(line1, 3, this->sprite->height() - (2) * this->sprite->fontHeight() - 4);
  }
  if (line2 != "")
  {
    this->sprite->setTextSize(1);
    this->sprite->drawString(line2, 3, this->sprite->height() - (1) * this->sprite->fontHeight() - 3);
  }

  this->sprite->setTextSize(2);
  this->sprite->pushSprite(0, 0);
  u32_t free = esp_get_free_heap_size();
  Log.traceln("Free memory: %s", String(free).c_str());
}
