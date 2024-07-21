#include <MFRC522.h>
#include <Arduino.h>
#include "rfidreader.h"

void RFIDReader::setCardAbsent()
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

RFIDReader::RFIDReader(MFRC522 *mfrc) : mfrc522(mfrc)
{
  this->uid = String("");
  this->cardPresent = false;
  this->cardRemoved = false;
}

bool RFIDReader::getCardPresent()
{
  return this->cardPresent;
}

bool RFIDReader::getCardRemoved()
{
  return this->cardRemoved;
}

void RFIDReader::clearCardRemoved()
{
  this->cardRemoved = false;
}

bool RFIDReader::newCardPresent()
{
  if (!this->cardPresent)
    return false;
  return this->uid != this->lastUid;
}

String RFIDReader::getUID()
{
  this->lastUid = this->uid;
  return this->uid;
}

void RFIDReader::read()
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