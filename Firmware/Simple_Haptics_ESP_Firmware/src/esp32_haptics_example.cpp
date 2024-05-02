#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "SSID";
const char* password = "PASSWORD";
WiFiUDP udp;
bool blink = false;
const uint8_t motors_pins[] = { 0, 1, 2, 3, 4, 5 };

void setup() {
    // setup pins
    pinMode(LED_BUILTIN, OUTPUT);
    for (int i = 0; i < sizeof(motors_pins); ++i) {
        ledcAttachChannel(motors_pins[i], 8000, 8, i);
    }

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); // should fix high latency
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        digitalWrite(LED_BUILTIN, (blink = !blink));
        delay(500);
    }
    digitalWrite(LED_BUILTIN, (blink = 0));

    // server
    udp.begin(1234);

    Serial.println("started, ready");
}

void loop() {
    int size = udp.parsePacket();
    if (size == 0) return;

    byte data[size];
    size = udp.read(data, size);
    size -= 1;

    switch (data[0]) {

    case '/': { // blink test
        digitalWrite(LED_BUILTIN, (blink = !blink));
        break;
    }

    case 'm': { // send motor value
        struct { uint8_t motor_id; uint8_t strength; }* data;
        if (size != sizeof(*data)) goto signal_error;
        data = (typeof(data))raw;
        if (data->motor_id >= sizeof(motors_pins)) goto signal_error;
        ledcWrite(data->motor_id, data->strength);
        break;
    }


    default: {
        goto signal_error;
    }
    }

    return;

signal_error:
    printf("error command: %c\n", data[0]);
}