#include <Wire.h>
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

//Estas son las credenciales del wifi al cuál el NodeMCU se va a aconectar
#define WLAN_SSID       "SkynetMobile"
#define WLAN_PASS       "KurisuAMD2"

//Este es el IP y el puerto del Broker (Raspberry Pi) al cuál el NodeMCU debe suscribirse
#define HOST        "172.20.10.2"
#define PORT        1883

//Este es el usuario y contraseña que tiene el NodeMCU como cliente
#define USERNAME    "username"
#define PASSWORD    "12345678" 

const int intervall = 10000;


WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, HOST, PORT, USERNAME, PASSWORD);


Adafruit_MQTT_Publish estado1 = Adafruit_MQTT_Publish(&mqtt, "estado1");

void MQTT_connect();

//Definición de Variables
const uint16_t kIrLed = 4;  
const int PIR = 14;
const int LED = 16;
int estado;
int pirState = LOW;
IRsend irsend(kIrLed);

void setup() {
  irsend.begin();
  //Definicion de las variables de salida y entrada
  pinMode(PIR, INPUT);
  pinMode(LED, OUTPUT);

  WiFi.forceSleepWake();
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  MQTT_connect();
}

void loop() {
  estado = digitalRead(PIR);
  //Si el estado del PIR es activo, se enciende el AIRE acondicionado
  if (estado == HIGH)
  {
    digitalWrite(LED, HIGH);
    if (pirState == LOW)
    {
      irsend.sendSony(0xa90, 12);
      Serial.println("Encendido");
      pirState = HIGH;
      estado1.publish("Encendido");
      delay(60000);
    }
  }
  //Si el estado del PIR es inactivo, se apaga el AIRE acondicionado
  else
  {
    digitalWrite(LED, LOW);
    if (pirState == HIGH)
    {
      irsend.sendSony(0xa90, 12);
      Serial.println("Apagado");
      pirState = LOW;
      estado1.publish("Apagado");
      delay(10000);
    }
  }
  
}

//Esta es la función de conexión para que el NodeMCU se suscriba al Broker a través del protocolo MQTT
void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
