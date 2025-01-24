#include <NTPClient.h>    //TID
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

const long utcOffsetInSeconds = 3600;

WiFiUDP udp;

WiFiUDP ntpUDP; //TID
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds); //TID



int ESP_ID = -1; // Declare ESP_ID globally
const int localPort = 12345; // Port för att lyssna på inkommande data
const int esps = 1;
const int numSensors = 4; // Antal sensorer
const int maxMessages = 8; // Antal meddelanden att lagra för medelvärdesberäkning
int sensorData[numSensors][maxMessages] = {0}; // Buffert för sensorvärden
int messageCount = 0;
int sensorDataDay[7][39]; //Day and in which time interval

unsigned long lastFirebaseUpdate = 0;  // För att hålla koll på tiden för Firebase-uppdatering

void extractData(String data){

  // Extrahera ESP_ID (första siffran i den mottagna datasträngen)
  int ESP_ID = data.charAt(0) - '0'; // Första siffran är ESP_ID

  // Kontrollera att ESP_ID är giltigt (t.ex. inom ett förväntat intervall)
  if (ESP_ID < 0 || ESP_ID >= esps) {
    Serial.println("Ogiltigt ESP_ID: " + String(ESP_ID));
  }
  else{
       // Lagra sensorvärden i bufferten för motsvarande ESP_ID
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
        }
        if (messageCount % maxMessages == 0){
          updateDataDay();
        }
      messageCount++;

      Serial.println("Data från ESP " + String(ESP_ID) + " lagrad.");

      }

void updateDataDay(){
    int timestamp = floor((timeClient.getHours() - 11) * 60 + timeClient.getMinutes() / 4); //!ÄNDRA 5MIN TILL ANPASSAT
    int value = 0;

  // Bygg upp en sträng med alla sensorvärden från båda ESP-enheterna
  for (int esp = 0; esp < (esps); esp++) {
    for (int messageNr = 0; messageNr < maxMessages; messageNr++) {
          value += sensorData[esp][messageNr];
        }
  }

    value = floor(value / (esps * maxMessages) * 10);
    sensorDataDay[timeClient.getDay()][timestamp] = value;
    updateFirebase(timestamp);
}

void updateFirebase(int timestamp2) {
  String firebaseDataString = "";
 // int timestamp = floor((timeClient.getHours() - 11) * 60 + timeClient.getMinutes() / 4); //!ÄNDRA 5MIN TILL ANPASSAT

  if (timeClient.getHours()>14){
  // Bygg upp en sträng med alla sensorvärden från båda ESP-enheterna
  firebaseDataString += timeClient.getDay();

    for (int cordinate = 0; cordinate < 39; cordinate++) {
    firebaseDataString += sensorDataDay[timeClient.getDay()][cordinate];
    }
  
  }
  else{
    if (timestamp2 >= 0 && timestamp2 <=39){
    firebaseDataString = sensorDataDay[timeClient.getDay()][timestamp2];    //MAY NEED TO FIX (ie t.ex timestamp-1)
    }
    else{
    Serial.println("Error with upploading");
    }
  }

  // Skicka data till Firebase
  if (Firebase.pushString(firebaseData, "/espData", firebaseDataString)) {
    Serial.println("Data skickad till Firebase: " + firebaseDataString);
  } else {
    Serial.println("Firebase error: " + firebaseData.errorReason());
  }
}


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
 //  if (millis() - lastFirebaseUpdate >= 8000) { 
  //    lastFirebaseUpdate = millis();

  //    updateFirebase();
/*
     String averageData = String(ESP_ID); // Starta med ESP_ID (den första siffran)

     // Beräkna medelvärde för varje sensor och lägg till i den femsiffriga koden
     for (int i = 0; i < numSensors / 2; i++) {   //ÄNDRA OM INTE 8 SENSORER
       int sum = 0;
       for (int j = 0; j < maxMessages; j++) {
          if (sensorData[j][i] === 2){

          }
          else{
            sum += sensorData[j][i];
          }
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
     */
   }
 } /*only one sensor */
//}
