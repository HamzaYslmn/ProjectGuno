/*
 * Vrms & Irms values from analogRead A0 using ACS712 and NodeMCU ESP8266 ESP-12E module. 
 * Plus WiFi connection to an access point.  
 * by Alex Roman
 */
 
/*--------LIBRARIES--------*/
#include <ESP8266WiFi.h>                            // Need to add the ESP8266WiFi.h library 
#include "secrets.h"                                // Also add the secrets.h file (The one saved in the same folder location as this scrip). 
/*--------VARIABLES--------*/
/*--------NodeMCU--------*/
#define PIN A0
float resolution  = 3.3 / 1024;                     // Input Voltage Range is 1V to 3.3V
                                                    // ESP8266 ADC resolution is 10-bit. 2^10 = 1024
uint32_t period = 1000000 / 60;                     // One period of a 60Hz periodic waveform 
uint32_t t_start = 0;

// setup
float zero_ADC_Value = 0;   

// loop
float ADC = 0, Vrms = 0, Current = 0, Q = 0.000;
float sensitivity = 0.185;                          // 185 mV/A, 100 mV/A and 0.66 mV/A for ±5A, ±20A and ±30A current range respectively. 

/*--------WiFi--------*/
char ssid[] = SECRET_SSID;                          // your network SSID (name) 
char pass[] = SECRET_PASS;                          // your network password
int keyIndex = 0;                                   // your network key Index number (needed only for WEP)
WiFiClient  client;                                 // Object


void setup()
{
  Serial.begin(115200);                             // Initialize Serial communication
  pinMode(PIN, INPUT);                              // Set pin A0 as read. 
  
  /*--------WiFi--------*/
  Serial.println();
  WiFi.begin(ssid, pass);                 // Initializes the WiFi library's network settings and provides the current status.
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)   // Return the connection status.
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  
  //WiFi.mode(WIFI_OFF)
  wifi_set_sleep_type(NONE_SLEEP_T);
  /*--------NodeMCU--------*/
  t_start = micros();
  uint32_t ADC_SUM = 0, n = 0;
  while(micros() - t_start < period) {
    ADC_SUM += analogRead(PIN);
    n++;
  }
  zero_ADC_Value = ADC_SUM / n;                        // The avg analog value when no current pass throught the ACS712 sensor
    
}
void loop() {

   
  /*----Vrms & Irms Calculation----*/
  t_start = micros();                             
  uint32_t ADC_Dif = 0, ADC_SUM = 0, m = 0;        
  while(micros() - t_start < period) {            // Defining one period of the waveform. US frequency(f) is 60Hz. Period = 1/f = 0.016 seg = 16,666 microsec
    ADC_Dif = zero_ADC_Value - analogRead(PIN);   // To start from 0V we need to subtracting our initial value when no current passes through the current sensor, (i.e. 750 or 2.5V).
    ADC_SUM += ADC_Dif * ADC_Dif;                 // SUM of the square
    m++;                                          // counter to be used for avg.
  }
  ADC = sqrt(ADC_SUM / m);                        // The root-mean-square ADC value. 
  Vrms = ADC * resolution ;                       // The root-mean-square analog voltage value.   
  Current = (Vrms  / sensitivity) - Q;        // The root-mean-square analog current value. Note: Q
  //------------------------------//
  
  Serial.print("analogRead = ");
  Serial.println(analogRead(PIN));
  
  Serial.print("Vrms = ");                        
  Serial.print(Vrms, 6);
  Serial.println(" V");        

  Serial.print("Irms = ");                       
  Serial.print(Current, 6);
  Serial.println(" A");
  Serial.print("\n");
  
  delay(1000); 
}