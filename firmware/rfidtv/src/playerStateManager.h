#include "Arduino.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
// #include <TFT_eSprite.h>
#include <FS.h>
#include "LittleFS.h"
#include "movieinfo.h"
#include "playerstate.h"
#include "rfapi.h"
#include "cardinfo.h"

struct MovieThumbnail
{
  String uri;
  bool available;
  String spiffsFilename;
};

class PlayerStateManager
{
private:
  PlayerState *state;
  TFT_eSPI *tft;
  String uri(String route);
  RFApi *rfApi;
  void loadThumbnail();
  MovieThumbnail thumbnail;
  CardInfo *card;
  bool _cardPresent;
  void setCard(CardInfo *card);
  void setState(PlayerState *newState);

public:
  TFT_eSprite *sprite;
  PlayerStateManager(TFT_eSPI *tft, TFT_eSprite *sprite, RFApi *rfApi);
  void refresh();
  void show();
  void cardPlaced(String uid);
  void cardRemoved();
  bool cardPresent();
  String cardUid();
  MovieInfo *currentMovie;
};