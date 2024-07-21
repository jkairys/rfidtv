#include <Arduino.h>
#ifndef MOVIEINFO_H
#define MOVIEINFO_H

struct MovieInfo
{
  bool isNull;
  String uid;
  String title;
  String thumbUrl;
  unsigned int duration;
};

#endif // MOVIEINFO_H