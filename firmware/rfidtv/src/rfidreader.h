#include <MFRC522.h>
#include <Arduino.h>
class RFIDReader
{

private:
  MFRC522 *mfrc522;
  String uid;
  String lastUid;
  bool cardPresent;
  unsigned long expirationTimestamp;

  bool cardRemoved;

  void setCardAbsent();

public:
  RFIDReader(MFRC522 *mfrc);

  bool getCardPresent();
  bool getCardRemoved();
  void clearCardRemoved();
  bool newCardPresent();
  String getUID();
  void read();
};
