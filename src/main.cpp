#include "Utilities.h"
#include "WiFiConnection.h"
#include "MQTTSensorPublisher.h"
#include "MQTTActuatorController.h"
#include "RelayObserver.h"
#include "SoundSensor.h"

#define RELAY_PIN 32
#define SOUND_SENSOR_PIN 35

using namespace std;

WiFiClient espClient;
WiFiConnection wifi("Galaxy S9+7c14", "betitox007.,");
// "Galaxy S9+7c14", "betitox007.,"
MQTTHandler mqttActuatorHandler(espClient, "broker.hivemq.com", 1883, "titos/place/actuator");
MQTTSensorPublisher mqttSensorPublisher(mqttActuatorHandler);
MQTTActuatorController mqttActuatorController(mqttActuatorHandler);
RelayObserver relayObserver(RELAY_PIN);
SoundSensor soundSensor(SOUND_SENSOR_PIN);

void setup() {
  Serial.begin(115200);
  wifi.connect();
  
  mqttActuatorController.connect();

  mqttActuatorController.attach(&relayObserver);
  mqttActuatorController.setCallback();
  
  soundSensor.attach(&mqttSensorPublisher);
}

void loop() {
  mqttSensorPublisher.loop();
  mqttActuatorController.loop();

  Utilities::nonBlockingDelay(500, []()
                              { soundSensor.checkSoundLevel(); });
}