#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// WiFi-konfiguration
#define WIFI_SSID "David - 15 PRO"
#define WIFI_PASSWORD "Hej pa dig!!"

// Mottagarens IP och port
const char* receiverIP = "172.20.10.2"; // Sätt den centrala ESPs IP
const int receiverPort = 12345;

WiFiUDP udp;

// Sensorpins
const int trigPins[4] = {D1, D2, D3, D4};
const int echoPins[4] = {D5, D6, D7, D8};

// ESP ID
const int ESP_ID = 1;

void setup() {
 Serial.begin(115200);

 // Anslut till WiFi
 WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 Serial.print("Connecting to Wi-Fi");
 while (WiFi.status() != WL_CONNECTED) {
   Serial.print(".");
   delay(500);
 }
 Serial.println("\nWiFi connected");

 // Ställ in sensorpins
 for (int i = 0; i < 4; i++) {
   pinMode(trigPins[i], OUTPUT);
   pinMode(echoPins[i], INPUT);
 }
}

void loop() {
 String data = String(ESP_ID); // Startar med ESP-ID

 // Läs avstånd från sensorer och bygg datasträng
 for (int i = 0; i < 4; i++) {
   long duration, distance;
   digitalWrite(trigPins[i], LOW);
   delayMicroseconds(2);
   digitalWrite(trigPins[i], HIGH);
   delayMicroseconds(10);
   digitalWrite(trigPins[i], LOW);

   duration = pulseIn(echoPins[i], HIGH);
   distance = duration * 0.034 / 2; // Omvandla till cm

   // Lägg till "1" om mindre än 80 cm, annars "0"
   data += (distance < 80) ? "1" : "0";
 }

 // Skicka data till mottagar-ESP
 udp.beginPacket(receiverIP, receiverPort);
 udp.print(data);
 udp.endPacket();

 Serial.println("Data skickad: " + data);

 delay(500); // Vänta 500 ms innan nästa mätning (2 ggr/sekund)
}


