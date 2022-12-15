#include <secrets.h>
#include "ACS712.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#define ON_Board_LED 2

ACS712 sensor(ACS712_05B, A0);

const char* ssid = SECRET_SSID;
const char* password = SECRET_PSWD;

const char* host = "script.google.com";
const int httpsPort = 443;

WiFiClientSecure client;

String GAS_ID = SECRET_GUNOAPI;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("");
    
  pinMode(ON_Board_LED,OUTPUT);
  digitalWrite(ON_Board_LED, HIGH);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(ON_Board_LED, LOW);
    delay(100);
    digitalWrite(ON_Board_LED, HIGH);
    delay(100);
    digitalWrite(ON_Board_LED, LOW);
    delay(100);
    digitalWrite(ON_Board_LED, HIGH);
    delay(1000);
  }

  digitalWrite(ON_Board_LED, HIGH);
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  client.setInsecure();

  sensor.calibrate();
}

void loop() {

  float U = 7.5; //Gerilim V

  float I = sensor.getCurrentAC();
  if (I < 0.06 ){
    I = 0;
  }
  float P = U * I;


  float v1 = P*1000;
  float v2 = I*1000;
  float v3 = U*1000;

  Serial.println(String("I = ") + I + " A");
  Serial.println(String("P = ") + P + " Watts");

  sendData(v1,v2,v3);
}

// Subroutine for sending data to Google Sheets
void sendData(int v1, int v2, int v3) {
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  String string_val1 =  String(v1);
  String string_val2 =  String(v2); 
  String string_val3 =  String(v3);
  String url = "/macros/s/" + GAS_ID + "/exec?value1=" + string_val1 + "&value2=" + string_val2 + "&value3=" + string_val3;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
}