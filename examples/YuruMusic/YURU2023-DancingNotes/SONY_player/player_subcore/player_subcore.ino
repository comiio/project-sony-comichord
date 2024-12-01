#if (SUBCORE != 1)
#error "Core selection is wrong!!"
#endif


#include <CytronWiFiShield.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <MP.h>

const char ssid[] = "espWiFi";    //  your network SSID (name)
const char pass[] = "128215781";  // your network password

enum direct {
    nothing = 0,
    left,
    right,
    down,
    up
};

void setup() {
    Serial.begin(115200);  // initialize serial communication
    pinMode(13, OUTPUT);   // set the LED pin mode

  if ( MP.begin() < 0) {
    Serial.printf("MP.begin error");
  }
    // check for the presence of the shield:
    if (!wifi.begin(2, 3)) {
        Serial.println("wifi shield not present");
        while (true)
            ;  // don't continue
    }

    String fv = wifi.firmwareVersion();
    Serial.println(fv);

    // attempt to enable esp hotspot
    wifi.setMode(WIFI_AP);
    Serial.print("MODE:");
    Serial.println(wifi.getMode());
    if (!wifi.softAP(ssid, pass, 11, 4)) Serial.println("Setting softAP failed");
    Serial.println(wifi.softAPIP());

    wifi.setMux(0);
    bool ack = false;
    do
    {
        ack = wifi.udpConnect("192.168.4.2", 1234, 1234);
        Serial.println("UDP Connect failed");
        delay(1000);
    } while (!ack);

    // if (!wifi.configureTCPServer(1, 1234)){
    //     Serial.println("TCP Server set up failed");
    //     while (true);
    // }

    Serial.println("Conect UDP Server successfully");
    // Serial.println("Conect TCP Server successfully");
}

void loop() {
    if (wifi.available()){
        char buf[64];
        for (int i = 0; wifi.available() && i < 64; i++) {
            buf[i] = wifi.read();
            delayMicroseconds(60);
        }
        
  if (MP.Send(100, (uint8_t)(buf[28]-'0')) < 0) {
    Serial.printf("MP.Send error");
  }
        Serial.print(buf[28]);
    }
}
