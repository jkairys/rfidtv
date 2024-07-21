#include <Arduino.h>

#ifndef PLAYER_STATE_H
#define PLAYER_STATE_H

struct PlayerState
{
  bool isPlaying;
  String title;
  int currentTime;
  int duration;
};

#endif // PLAYER_STATE_H