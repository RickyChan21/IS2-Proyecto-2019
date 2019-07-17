    
#include <Wire.h>// Libreria Para usar puertos SDA SSL
#include <ESP8266WiFi.h>//Libreria Para utilizar El wifi en Node MCU
#include <Adafruit_MQTT.h>//Librería para conexion del protocolo MQTT
#include <Adafruit_MQTT_Client.h>//Librería para conexion del protocolo MQTT Cliente
#include <Arduino.h>//Librería que importa los protocolos de arduino al NodeMCU
#include <IRremoteESP8266.h>//Libreria para el módulo infrarrojo del NodeMCU
#include <IRsend.h>//Libre para enviar señal infrarroja a través del módulo infrarrojo

//Estas son las credenciales del wifi al cuál el NodeMCU se va a aconectar
#define WLAN_SSID       "SkynetMobile"
#define WLAN_PASS       "KurisuAMD2"

//Este es el IP y el puerto del Broker (Raspberry Pi) al cuál el NodeMCU debe suscribirse
#define HOST        "172.20.10.2"
#define PORT        1883

//Este es el usuario y contraseña que tiene el NodeMCU como cliente
#define USERNAME    "username"
#define PASSWORD    "12345678" 

const int intervall = 10000; //Intervalo de tiempo para la conexión al core


WiFiClient client; //Suscripción del NodeMCU como cliente
Adafruit_MQTT_Client mqtt(&client, HOST, PORT, USERNAME, PASSWORD); //Función que realiza la conexión entre el NodeMCU y el core


Adafruit_MQTT_Publish estado1 = Adafruit_MQTT_Publish(&mqtt, "estado1"); //Función que publica los valores obtenidos con el NodeMCU

void MQTT_connect(); //Método que hace la conexión entre el NodeMCU y el core

//Definición de Variables
const uint16_t kIrLed = 4;  //Pin dónde está conectado el módulo infrarrojo
const int PIR = 14; //Pin dónde está conectado el PIR
const int LED = 16; //Pin dónde está conectado el LED
int estado; //Variable que obtendrá el estado del pir
int pirState = LOW; //Variable auxiliar del estado del pir
IRsend irsend(kIrLed); //Declaramos la variable kIrLed como emisor de señal infrarroja

void setup() {
  irsend.begin(); //iniciamos el módulo infrarrojo
  //Definicion de las variables de salida y entrada
  pinMode(PIR, INPUT); //Declaramos el PIR como entrada
  pinMode(LED, OUTPUT); //Declaramos el LED como salida

  WiFi.forceSleepWake(); //Despierta el módulo Wifi
  WiFi.mode(WIFI_STA); //Inicializa el módulo wifi
  Serial.begin(115200); //Esta es la frecuencia en la que trabaja el NodeMCU
  Serial.print("Connecting to "); //Mensaje de conexión
  Serial.println(WLAN_SSID); //Impresión del SSID del wifi al que nos estamos conectando

  WiFi.begin(WLAN_SSID, WLAN_PASS); //Inicia la conección del NodeMCU al wifi con sus credenciales
  while (WiFi.status() != WL_CONNECTED)  //Mientras el Wifi no esté conectado imprime un punto a cada 5 milisegundos
  {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected"); //Imprime que el wifi está conectado
  Serial.println("IP address: "); Serial.println(WiFi.localIP()); //Imprime el IP
  MQTT_connect(); //Conexión con el protocolo MQTT
}

void loop() {
  estado = digitalRead(PIR); //Se lee el estado del PIR
  //Si el estado del PIR es activo, se enciende el AIRE acondicionado
  if (estado == HIGH)
  {
    digitalWrite(LED, HIGH); //Se enciende el LED
    if (pirState == LOW) //Si el PIR está apagado
    {
      irsend.sendSony(0xa90, 12); //Se envía la señal infrarroja de encendido
      Serial.println("Encendido"); //Se imprime "encendido
      pirState = HIGH; //La variable auxiliar cambia a HIGH
      estado1.publish("Encendido"); //Se publica "Encendido" en el core
      delay(60000); //Retraso de 1 minuto
    }
  }
  //Si el estado del PIR es inactivo, se apaga el AIRE acondicionado
  else
  {
    digitalWrite(LED, LOW); //Se apaga el LED
    if (pirState == HIGH)
    {
      irsend.sendSony(0xa90, 12); //Se envía la señal de apagado
      Serial.println("Apagado"); //Se imprime que se apagó
      pirState = LOW; //La variable auxiliar cambia a LOW
      estado1.publish("Apagado"); //Se publica "Apagado" en el core
      delay(10000); //Retraso de 10 segundos
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
