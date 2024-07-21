#include "rfapi.h"
#include <ArduinoLog.h>

RFApi::RFApi(String url)
{
  this->baseUrl = url;
  if (this->baseUrl.endsWith("/"))
  {
    // Remove the final character if it's a slash
    this->baseUrl = this->baseUrl.substring(0, this->baseUrl.length() - 1);
  }
  Log.infoln("Initialized RFApi with base URL %s", this->baseUrl.c_str());
}

APIResponse RFApi::call(RequestMethod method, String route, String payload)
{
  HTTPClient http;
  String url = this->baseUrl + route;
  Log.traceln("Calling url %s", url.c_str());
  http.begin(url);
  int statusCode = 0;

  const char *headerKeys[] = {"Content-Type"};
  http.collectHeaders(headerKeys, 1);

  if (method == POST)
  {
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "application/json");
    statusCode = http.POST(payload);
  }
  else if (method == GET)
  {
    statusCode = http.GET();
  }

  APIResponse response;

  response.error = "";
  response.statusCode = statusCode;
  response.body = http.getString();
  // response.doc = JsonDocument();

  Log.traceln("Headers: %s", String(http.headers()).c_str());
  Log.traceln("Content-Type: %s", http.header("Content-Type").c_str());

  if (http.hasHeader("Content-Type") && http.header("Content-Type") == "application/json")
  {
    Log.traceln("Deserializing JSON response");
    DeserializationError error = deserializeJson(response.doc, response.body);
    if (error)
    {
      response.error = String("failed to parse JSON response: ") + error.c_str();
    }
    else
    {
      Log.traceln("Successfully deserialized JSON response");
    }
  }
  else
  {
    Log.warningln("Response is not JSON %s", response.body.c_str());
  }

  if (response.error.length() > 0)
  {
    Log.errorln("Error calling %s: %s", url.c_str(), response.error.c_str());
  }

  http.end();
  return response;
}

APIResponse RFApi::get(String route)
{
  return this->call(GET, route, "");
}

APIResponse RFApi::post(String route, String payload)
{
  return this->call(POST, route, payload);
}

MovieInfo RFApi::setCard(String uid)
{

  APIResponse response = this->post("/cards/" + uid, "");
  MovieInfo m;
  m.isNull = true;
  if (response.statusCode == 404)
  {
    Log.warningln("Card not found - %s", uid.c_str());
    return m;
  }
  else if (response.statusCode > 0)
  {
    Log.traceln("Card set - %s", uid.c_str());
    m.isNull = false;
    m.uid = uid;
    m.title = response.doc["title"].as<const char *>();
    m.duration = response.doc["duration_mins"];
    m.thumbUrl = response.doc["thumb"].as<const char *>();
    return m;
  }
  return m;
}

String RFApi::getThumbnail(String uri)
{
  APIResponse response = this->get(uri);
  return response.body;
}

APIResponse RFApi::stopMovie()
{
  APIResponse response = this->post("/stop", "");
  return response;
}

APIResponse RFApi::startMovie(String movie)
{
  String payload = "{\"movie_name\": \"" + movie + "\"}";
  Log.traceln("Start movie with payload %s", payload.c_str());
  APIResponse response = this->post("/play", payload);
  return response;
}

APIResponse RFApi::pauseMovie()
{
  APIResponse response = this->post("/pause", "");
  return response;
}

APIResponse RFApi::resumeMovie()
{
  APIResponse response = this->post("/resume", "");
  return response;
}

PlayerState *RFApi::getState()
{
  APIResponse response = this->get("/status");

  if (response.error.length() > 0)
  {
    Log.errorln("Error getting player state: %s", response.error.c_str());
    return NULL;
  }

  PlayerState *state = new PlayerState();
  state->title = response.doc["title"].as<const char *>();
  state->isPlaying = response.doc["player_state"] == "PLAYING";
  state->currentTime = response.doc["current_time"];
  state->duration = response.doc["duration"];
  return state;
}