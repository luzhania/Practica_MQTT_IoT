#include "Utilities.h"
#include "WiFiConnection.h"
#include "MQTTSensorPublisher.h"
#include "MQTTActuatorController.h"
#include "RelayObserver.h"
#include "SoundSensor.h"

#define RELAY_PIN 32
#define SOUND_SENSOR_PIN 35

using namespace std;

WiFiConnection wifi("HUAWEI-2.4G-M6xZ", "HT7KU2Xv");
// "Galaxy S9+7c14", "betitox007.,"
MQTTHandler mqttHandler("broker.hivemq.com", 1883);

MQTTActuatorController mqttActuatorController(mqttHandler, "titos/place/actuator");
MQTTSensorPublisher mqttSensorPublisher(mqttHandler, "titos/place/sound");

RelayObserver relayObserver(RELAY_PIN);
SoundSensor soundSensor(SOUND_SENSOR_PIN);

void setup() {
  Serial.begin(115200);
  wifi.connect();
  
  mqttHandler.connect();
  
  mqttActuatorController.subscribe();
  mqttActuatorController.attach(&relayObserver);
  mqttActuatorController.setCallback();
  
  soundSensor.attach(&mqttSensorPublisher);
}

void loop() {
  mqttActuatorController.loop();
  mqttSensorPublisher.loop();

  Utilities::nonBlockingDelay(500, []()
                              { soundSensor.checkSoundLevel(); });
}