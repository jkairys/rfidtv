#include <Arduino.h>
#ifndef CARDINFO_H
#define CARDINFO_H

struct CardInfo
{
  String uid;
  bool isUnregistered;
};

#endif // CARDINFO_H