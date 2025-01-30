#include <NTPClient.h>     // TID
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


// Tid och UDP
const long utcOffsetInSeconds = 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds); // TID


// Global variabler
const int esps = 1;
const int numSensors = 4; // Antal sensorer
const int maxMessages = 2; // Antal meddelanden att lagra för medelvärdesberäkning
int sensorData[numSensors][maxMessages] = {0}; // Buffert för sensorvärden
int messageCount = 0;
int sensorDataDay[7][39]; // Dag och tidsintervall


unsigned long lastFirebaseUpdate = 0;  // För att hålla koll på tiden för Firebase-uppdatering
WiFiUDP udp;
char packetBuffer[255]; // Buffert för att läsa UDP-paket


void setup() {
 Serial.begin(115200);


 // Anslut till Wi-Fi
 Serial.print("Connecting to Wi-Fi");
 WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 while (WiFi.status() != WL_CONNECTED) {
   Serial.print(".");
   delay(500);
 }
 Serial.println("\nWiFi connected");
 Serial.print("Receiver IP: ");
 Serial.println(WiFi.localIP());


 // Konfigurera Firebase
 firebaseConfig.host = "ultraljudsprojekt-default-rtdb.europe-west1.firebasedatabase.app";
 firebaseConfig.signer.tokens.legacy_token = "lAbQ4CiEUevvpthB0hr0mEC346UJ7x4KuDTe4hyi";
 Firebase.begin(&firebaseConfig, &firebaseAuth);


 // Starta UDP
 udp.begin(12345);
 Serial.println("UDP mottagning startad");


 // Starta NTP klienten
 timeClient.begin();
}

void extractData(String data) {
 // Extrahera ESP_ID från första siffran i datasträngen
 int ESP_ID = data.charAt(0) - '0'; // Första siffran är ESP_ID


 if (ESP_ID < 0 || ESP_ID > esps) {
   Serial.println("Ogiltigt ESP_ID: " + String(ESP_ID));
   return;  // Felaktigt ESP_ID, avbryt
 }




for (int j = 1; j < data.length(); j++){ //!OBS! DATA ÄR STRING, SE "DATA"

         int countZeros = 0;
         int countOnes = 0;

          // Ensure the index is within bounds            //TA BORT OM FUNGERAR!!!!
          if (j < 0 || j >= data.length()) {
            Serial.println("Index out of bounds!");
            return;
          }

          // Loop through the string starting from the position after the index
          for (int k = 1; k < data.length(); k++) {
            if (data[k] == '0') {
              countZeros++;
            } else if (data[k] == '1') {
              countOnes++;
            }

          if (countOnes >= 2){
            data[k] = 1;
          }
          else if (countZeros >= 2)
            data[k] = 0;
          }

          }
        
          String strData = String(data); // Convert integer to string
          sensorData[ESP_ID][messageCount % maxMessages] = strData.substring(1).toInt(); // Remove the first character (digit)
        

    messageCount++;
    Serial.println("Data från ESP " + String(ESP_ID) + " lagrad." + sensorData[ESP_ID][(messageCount -1) % maxMessages]);
}

void updateDataDay() {
 int timestamp = (((timeClient.getHours() - 8) * 60 + timeClient.getMinutes()) * 3) % 39; // Beräkna tidsstämpel
 int value = 0;


 // Beräkna medelvärde för varje sensor och tidsintervall
 for (int esp = 0; esp < esps; esp++) {
   for (int messageNr = 0; messageNr < maxMessages; messageNr++) {
     value += sensorData[esp][messageNr];
   }
 }


 value = floor(value / (esps * maxMessages) * 10); // Normalisera värdet
 sensorDataDay[timeClient.getDay()][timestamp] = value;


 // Uppdatera Firebase med det nya data
 //updateFirebase(timestamp);                                                     //Kanske behöver vara med
}


void updateFirebase(int timestamp) {
 String firebaseDataString = "";
  // Kontrollera om det är efter 14:00, skicka hela dagsprognosen
 if (timeClient.getHours() > 14) {
   firebaseDataString += String(timeClient.getDay());
   for (int i = 0; i < 39; i++) {
     firebaseDataString += String(sensorDataDay[timeClient.getDay()][i]);
   }
 } else {
   // Skicka uppdaterad data för specifikt tidsintervall
   if (timestamp >= 0 && timestamp < 39) {
     firebaseDataString = String(sensorDataDay[timeClient.getDay()][timestamp]);
   } else {
     Serial.println("Fel vid uppladdning av data");
     return; // Om timestamp är utanför intervallet, avbryt
   }
 }


 // Skicka data till Firebase
 if (Firebase.pushString(firebaseData, "/espData", firebaseDataString)) {
   Serial.println("Data skickad till Firebase: " + firebaseDataString);
 } else {
   Serial.println("Firebase error: " + firebaseData.errorReason());
 }
}

void loop() {
 // Hämta NTP-tid
 timeClient.update();


 // Hantera UDP-data
 int packetSize = udp.parsePacket();
 if (packetSize) {
   int len = udp.read(packetBuffer, sizeof(packetBuffer) - 1);
   if (len > 0) {
     packetBuffer[len] = 0;  // Lägg till null-terminering
   }


   String data = String(packetBuffer);
   Serial.println("Data mottagen: " + data);
   extractData(data);


   // Uppdatera dagdata var 20:e sekund
   if (timeClient.getSeconds() % 20 == 0) {
     updateDataDay();
   }
 }


 // Uppdatera Firebase var tredje sekund
 if (millis() - lastFirebaseUpdate >= 8000) {
   lastFirebaseUpdate = millis();
   updateFirebase(timeClient.getDay()); // Skicka uppdaterad dagdata till Firebase              /*FIX!!!!!!!!!!!*/
 }
}
