#include <Arduino.h>
#include <PubSubClient.h>
#pragma once

class MQTTHandler {
private:
    WiFiClient wifiClient;
    PubSubClient client;
    const char* SERVER;
    const unsigned int PORT;
    const char* SUBSCRIBE_TOPIC;

public:
    MQTTHandler(const char* SERVER, unsigned int PORT)
        : SERVER(SERVER), PORT(PORT), client(wifiClient) {}

    void connect() {
        client.setServer(SERVER, PORT);
        reconnect();
    }

    void reconnect() {
        while (!client.connected()) {
            String clientId = "ESP32Client-" + String(random(0xffff), HEX);
            if (client.connect(clientId.c_str())) {
                Serial.println("Connected to MQTT broker");
                if(SUBSCRIBE_TOPIC) {
                    subscribe(SUBSCRIBE_TOPIC);
                }
            } else {
                Serial.print("Error MQTT connection, rc=");
                Serial.println(client.state());
                delay(5000);
            }
        }
    }

    void publish(const String& message, const char* topic) {
        if (client.publish(topic, message.c_str())) {
            Serial.println("Data sent successfully");
        } else {
            Serial.println("Error sending data");
        }
    }

    void setCallback(std::function<void(char*, byte*, unsigned int)> callback) {
        client.setCallback(callback);
    }

    void subscribe(const char* topic) {
        client.subscribe(topic);
        Serial.println("Subscribed to topic: " + String(topic));
        SUBSCRIBE_TOPIC = topic;
    }

    void loop() {
        if (!client.connected()) {
            reconnect();
        }
        client.loop();
    }
};