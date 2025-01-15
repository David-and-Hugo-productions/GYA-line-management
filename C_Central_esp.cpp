#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <FirebaseESP8266.h>

// WiFi-konfiguration
#define WIFI_SSID "David - 15 PRO"
#define WIFI_PASSWORD "Hej pa dig!!"

// Firebase-konfiguration
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;


void extractData(string data){

  // Extrahera ESP_ID (första siffran i den mottagna datasträngen)
  int ESP_ID = data.charAt(0) - '0'; // Första siffran är ESP_ID

  // Kontrollera att ESP_ID är giltigt (t.ex. inom ett förväntat intervall)
  if (ESP_ID < 0 || ESP_ID >= numSensors) {
    Serial.println("Ogiltigt ESP_ID: " + String(ESP_ID));
    return;
  }

  // Lagra sensorvärden i bufferten för motsvarande ESP_ID
  for (int i = 4 * ESP_ID; i < numSensors / 2; i++) {     /* 1 * ESP_ID, esp 0 startar med i=0 esp 1 startar med i=4  ÄNDRA OM ANTAL SENSORER INTE ÄR 8*/    
    sensorData[messageCount % maxMessages][i] = data.charAt(i + 1) - '0'; // Läser sensorvärden från index 1
  }
  messageCount++;

  Serial.println("Data från ESP " + String(ESP_ID) + " lagrad.");

}



WiFiUDP udp;
const int localPort = 12345; // Port för att lyssna på inkommande data

const int numSensors = 8; // Antal sensorer
const int maxMessages = 6; // Antal meddelanden att lagra för medelvärdesberäkning
int sensorData[maxMessages][numSensors] = {0}; // Buffert för sensorvärden
int messageCount = 0;

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

 // Skriv ut IP-adressen
 Serial.print("Receiver IP: ");
 Serial.println(WiFi.localIP());

 // Konfigurera Firebase
 firebaseConfig.host = "ultraljudsprojekt-default-rtdb.europe-west1.firebasedatabase.app";
 firebaseConfig.signer.tokens.legacy_token = "lAbQ4CiEUevvpthB0hr0mEC346UJ7x4KuDTe4hyi";

 Firebase.begin(&firebaseConfig, &firebaseAuth);

 // Starta UDP
 udp.begin(localPort);
 Serial.println("UDP mottagning startad");
}

void loop() {

// Datan som tas emot är "10100" Första siffran är nummret på ESP ( ESP_ID) 
// Femsiffriga numret är "data"

 static unsigned long lastFirebaseUpdate = 0;
 char packetBuffer[255];
 int packetSize = udp.parsePacket();
 if (packetSize) {
   int len = udp.read(packetBuffer, 255);
   if (len > 0) {
     packetBuffer[len] = 0;
   }
   String data = String(packetBuffer);

   Serial.println("Data mottagen: " + data);
   
  extractData(data);

   // Uppdatera Firebase var tredje sekund
   if (millis() - lastFirebaseUpdate >= 3000) {
     lastFirebaseUpdate = millis();

     String averageData = String(ESP_ID); // Starta med ESP_ID (den första siffran)

     // Beräkna medelvärde för varje sensor och lägg till i den femsiffriga koden
     for (int i = 0; i < numSensors; i++) {
       int sum = 0;
       for (int j = 0; j < maxMessages; j++) {
         sum += sensorData[j][i];
       }
       int average = sum / maxMessages; // Beräkna medelvärde
       averageData += String(average); // Lägg till medelvärde för den aktuella sensorn
     }

     // Skicka den femsiffriga koden till Firebase
     if (Firebase.pushString(firebaseData, "/esp", averageData)) {
       Serial.println("Data skickad till Firebase: " + averageData);
     } else {
       Serial.println("Firebase error: " + firebaseData.errorReason());
     }
   }
 }
}
