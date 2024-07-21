#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "movieinfo.h"
#include "playerstate.h"

#ifndef RFAPI_H
#define RFAPI_H

struct APIResponse
{
  int statusCode;
  String body;
  JsonDocument doc;
  String error;
};

enum RequestMethod
{
  POST,
  GET
};

class RFApi
{

private:
  String baseUrl;

  APIResponse call(RequestMethod method, String route, String payload);
  APIResponse get(String route);
  APIResponse post(String route, String payload);

public:
  RFApi(String url);
  MovieInfo setCard(String uid);
  String getThumbnail(String uri);
  APIResponse stopMovie();
  APIResponse startMovie(String movie);
  APIResponse resumeMovie();
  APIResponse pauseMovie();
  PlayerState *getState();
};

#endif // RFAPI_H
