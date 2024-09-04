/*
==========================================================================
|                                                                        |
|                 LOXA FIDELIS RFID CONTROLLER CORE                      |
|                               V1.0.0                                   |
|                                                                        |
==========================================================================
*/

#include <WiFiClient.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <MFRC522.h>
#include <Wire.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <WebServer.h>
#include "config.h"


String ID = "";
String TIMESTAMP = "";
String PAGO = "";

char ssid[50];
char pass[50];
char host[50];

const char *ssidConf = "LoxaFidelisConf";
const char *passConf = "";

uint8_t peerMacAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

String mensaje = "";

String pagina = MAIN_page;

String paginafin = "</body>"
                   "</html>";

// const char* ssid = "ac_Q";
// const char* password = "12345678";
// const char*  server = "loxa.softsen.space";

int TIMESTAMP_ACTUAL = 0;

#define RST_PIN 22
#define SS_PIN 21


// ********************** ESPNOW ********************* //
esp_now_peer_info_t peerInfo;
// callback when data is sent

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Mensaje entregado" : "Mensaje fallido");
}

//--------------  ESCRIBIR EN RC522  --------------
unsigned char data[16];
unsigned char *writeData = data;
unsigned char *str;

//-----------------  RFID PINOUT  -----------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

//--------------  VARIABLES GLOBALES  -------------
int contconexion = 0;
bool status_connect = 0;
int prev_value = 0;

const char *test_root_ca =
  "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\n"
  "k6eNf/rrABNm3NYhnfAJgg==\n"
  "-----END CERTIFICATE-----\n";

int id_disp = 8;
int id_product = 65;
int PIN_RELE = 26;
int PIN_DISPLAY = 25;

byte sensorInterrupt = 4;
byte sensorPin = 4;

float calibrationFactor = 4.40;

volatile byte pulseCount;
//const int motor= 50;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;

int count = 0;


/*
==================================================================================

                                FUNCIONES DE WIFI

==================================================================================
*/

WebServer svr(80);
WiFiClient client;

//---------------------GUARDAR CONFIGURACION-------------------------

void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin("LOXA FIDELIS", "@LoxaFidelis2024");
  while (WiFi.status() != WL_CONNECTED and contconexion < 50) {
    ++contconexion;
    delay(250);
    Serial.print(".");
  }

  Serial.print(contconexion);

  if (contconexion < 10000) {
    status_connect = 0;
    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.println(WiFi.localIP());
  } else {
    status_connect = 1;
  }
}

void paginaconf() {
  svr.send(200, "text/html", pagina);
}

void modoconf() {
  WiFi.softAP(ssidConf, passConf);
  IPAddress myIP = WiFi.softAPIP();

  Serial.print("IP del acces point: ");
  Serial.println(myIP);
  Serial.println("WebServer iniciado...");

  svr.on("/", paginaconf);  //esta es la pagina de configuracion

  svr.on("/saveEEPROM", guardar_conf);  //Graba en la eeprom la configuracion

  svr.begin();

  while (true) {
    svr.handleClient();
  }
}


void guardar_conf() {
  Serial.println(svr.arg("ssid"));
  grabar(0, svr.arg("ssid"));
  Serial.println(svr.arg("pass"));
  grabar(50, svr.arg("pass"));
  Serial.println(svr.arg("ip"));
  grabar(100, svr.arg("ip"));
  mensaje = "Configuracion Guardada...";
  ESP.restart();
}

//----------------Función para grabar en la EEPROM-------------------
void grabar(int addr, String a) {
  int tamano = a.length();
  char inchar[50];
  a.toCharArray(inchar, tamano + 1);
  for (int i = 0; i < tamano; i++) {
    EEPROM.write(addr + i, inchar[i]);
  }
  for (int i = tamano; i < 50; i++) {
    EEPROM.write(addr + i, 255);
  }
  EEPROM.commit();
}

String getCode() {
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  MFRC522::StatusCode status;

  String stamp = "";
  String copy = "";

  byte trailerBlock = 7;
  byte sector = 1;
  byte blockAddr = 4;

  status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return "";
  }

  byte buffer[18];
  byte size = sizeof(buffer);

  // Read data from the block (again, should now be what we have written)
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(blockAddr, buffer, &size);

  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }

  stamp = String((char *)buffer);
  stamp = stamp.substring(0, 11);
  for (int i = 0; i < stamp.length(); i++) {
    if (stamp[i] != '\n') {
      copy += stamp[i];
    }
  }

  return copy;
}

//-----------------Función para leer la EEPROM------------------------
String leer(int addr) {
  byte lectura;
  String strlectura;
  for (int i = addr; i < addr + 50; i++) {
    lectura = EEPROM.read(i);
    if (lectura != 255) {
      strlectura += (char)lectura;
    }
  }
  return strlectura;
}

//________________________________________________________________________________

void setup() {
  Serial.begin(38400);
  EEPROM.begin(512);

  pinMode(sensorPin, INPUT_PULLUP);
  pinMode(PIN_RELE, OUTPUT);
  pinMode(PIN_DISPLAY, OUTPUT);

  //digitalWrite(sensorPin, HIGH);
  digitalWrite(PIN_DISPLAY, LOW);


  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  oldTime = 0;

  // Inicializamos el lector RFID
  SPI.begin();
  mfrc522.PCD_Init();

  while (!Serial)
    ;
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.print("Lector RFID conectado");

  // Serial.print("Attempting to connect to SSID: ");
  // Serial.println(ssid);
  // WiFi.begin(ssid, password);

  // // attempt to connect to Wifi network:
  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.print(".");
  //   // wait 1 second for re-trying
  //   delay(1000);
  // }

  // Serial.print("Connected to ");
  // Serial.println(ssid);

  leer(0).toCharArray(ssid, 50);
  leer(50).toCharArray(pass, 50);

  setup_wifi();

  if (status_connect == 1) {
    modoconf();
  } else {
    leer(0).toCharArray(ssid, 50);
    leer(50).toCharArray(pass, 50);
    leer(100).toCharArray(host, 50);
  }

  digitalWrite(PIN_RELE, HIGH);

  Serial.println("\nStarting connection to server...");

  attachInterrupt(digitalPinToInterrupt(sensorPin), pulseCounter, FALLING);

    // Init ESP-NOW
  //if (esp_now_init() != ESP_OK) {
    //Serial.println("Error initializing ESP-NOW");
    //return;
  //}

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  //esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  //memcpy(peerInfo.peer_addr, peerMacAddress, 6);
  //peerInfo.channel = 0;  
  //peerInfo.encrypt = false;
  //peerInfo.ifidx = WIFI_IF_STA;
  
  // Add peer        
  //if (esp_now_add_peer(&peerInfo) != ESP_OK){
    //Serial.println("Failed to add peer");
    //return;
  //}
}

void loop() {

  Serial.print(pulseCount);
  // Serial.println("-- Acercar Pulsera --");
  if (ID == "") {
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

    if (!mfrc522.PICC_IsNewCardPresent())
      return;
    if (!mfrc522.PICC_ReadCardSerial())
      return;

    // Obtengo el ID
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      ID += String(mfrc522.uid.uidByte[i], HEX);
    }

    Serial.println("ID TAG PREVIO:");
    Serial.println(ID);
    Serial.println("\n");

    Serial.print(getCode());

    // BLOQUEO POR CÓDIGO INTERNO
    if (getCode() != "LoxaFidelis") {
      ID = "";
    } else {
      Serial.println("ID TAG AUTORIZADO:");
      Serial.println(ID);
      Serial.println("\n");

      digitalWrite(PIN_RELE, LOW);
      digitalWrite(PIN_DISPLAY, HIGH);
      //delay(300);
      //digitalWrite(PIN_DISPLAY, LOW);
    }
  } else {
    if ((millis() - oldTime) > 1000) {
      detachInterrupt(sensorInterrupt);

      //flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
      flowRate = pulseCount * 2.25;
      oldTime = millis();
      //flowMilliLitres = (flowRate / 60) * 1000;
      flowMilliLitres = flowRate;

      prev_value = totalMilliLitres;
      totalMilliLitres += flowMilliLitres;

      if (prev_value == totalMilliLitres) {

        prev_value = totalMilliLitres;

        count++;
        Serial.print(count);

        if (count == 5) {
          totalMilliLitres += 10;
          digitalWrite(PIN_RELE, HIGH);
          //digitalWrite(PIN_DISPLAY, HIGH);
          //delay(300);
          digitalWrite(PIN_DISPLAY, LOW);

          count = 0;
          /*
           _______________________________________________
          |                                               |
          |        ENVIAMOS LOS DATOS AL SERVIDOR         |
          |           {ID_TAG,VALUE,ID_DISP}              |
          |_______________________________________________|
          */

          String url = "/loxa/public/api/dispensers/create/";
          url += String(id_disp);
          url += "?value=";
          url += String(totalMilliLitres);
          url += "&id_product=";
          url += id_product;
          url += "&tag=";
          url += ID;

          ID = "";
          // Reseteamos a 0 el contador de mililitros
          totalMilliLitres = 0;

          if (!client.connect("192.168.0.33", 80)) {
            Serial.println("Connection failed!");
          } else {
            Serial.print(url);
            // Make a HTTP request:
            client.println("GET " + url + " HTTP/1.0");
            // client.println("Host: "+String(host));
            client.println("Host: 192.168.0.33");
            client.println("Connection: close");
            client.println();

            while (client.connected()) {
              String line = client.readStringUntil('\n');
              if (line == "\r") {
                Serial.println("headers received");
                break;
              }
            }
            // if there are incoming bytes available
            // from the server, read them and print them:
            while (client.available()) {
              char c = client.read();
              Serial.write(c);
              ID = "";
              // Reseteamos a 0 el contador de mililitros
              totalMilliLitres = 0;
            }
            // ID="";
            // // Reseteamos a 0 el contador de mililitros
            // totalMilliLitres=0;
            client.stop();
          }
        }

      } else {
        count = 0;
      }

      pulseCount = 0;
      Serial.print("  Output Liquid Quantity: ");
      Serial.print(totalMilliLitres);
      Serial.println("mL");
      attachInterrupt(digitalPinToInterrupt(sensorPin), pulseCounter, FALLING);
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void pulseCounter() {
  pulseCount++;
}