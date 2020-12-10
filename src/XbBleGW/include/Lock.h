#ifndef _Lock
#define _Lock

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Mqtt.h"
#include "Debug.h"

class Lock {
 public:
  Lock();
  bool handle(int did, int eid, const char *edata);
 protected:
  DynamicJsonDocument json_;
  bool eventHandle_(int did, const char *eidName, const char *edata);
  bool attributesHandle_(int did, const char *eidName, const char *edata);
  bool getTimestamp_(const char *edata);
  bool keyIDHandle_(const char *edata);
  bool operateHandle_(const char *edata);
  void mqttPublish_(int deviceID, const char *eidName, const char *payload);
};

#endif