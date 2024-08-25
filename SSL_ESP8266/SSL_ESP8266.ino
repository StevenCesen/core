/*
==========================================================================
|                                                                        |
|                 LOXA FIDELIS RFID CONTROLLER RW-RFID                   |
|                               V1.0.0                                   |
|                                                                        |
==========================================================================
*/
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <MFRC522.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "config.h"
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

ESP8266WiFiMulti WiFiMulti;

String ID="";
String TIMESTAMP="";
String PAGO="";

// Replace with your network credentials
// const char* ssid = "ac_Q";
// const char* password = "12345678";

int TIMESTAMP_ACTUAL=0;

#define RST_PIN D3
#define SS_PIN D4

//--------------ESCRIBIR EN RC522
unsigned char data[16]; 
unsigned char *writeData = data; 
unsigned char *str;

//--------------Super globales
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);  // Adafruit ESP8266/32u4/ARM Boards + FeatherWing OLED

int contconexion = 0;
bool status_connect=0;

char ssid[50];      
char pass[50];
char ip[50];

const char *ssidConf = "LoxaFidelisConf";
const char *passConf = "";

String mensaje = "";

String pagina = MAIN_page;

String paginafin = "</body>"
"</html>";

void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,pass);
  while (WiFi.status() != WL_CONNECTED and contconexion<50) {
    ++contconexion;
    delay(250);
    Serial.print(".");
  }

  Serial.print(contconexion);

  if(contconexion<50){
    status_connect=0;
    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.println(WiFi.localIP()); 
  }else{
    status_connect=1;
  }
}

WiFiClient espClient;
ESP8266WebServer server(80);

void paginaconf() {
  server.send(200, "text/html", pagina); 
}

void modoconf() {
  WiFi.softAP(ssidConf, passConf);
  IPAddress myIP = WiFi.softAPIP(); 
 
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 10, "WIFI: Desconectado"); // write something to the internal memory
  u8g2.drawStr(0, 20, "IPconf: 192.168.4.1"); // write something to the internal memory
  u8g2.sendBuffer();

  Serial.print("IP del acces point: ");
  Serial.println(myIP);
  Serial.println("WebServer iniciado...");

  server.on("/", paginaconf); //esta es la pagina de configuracion

  server.on("/saveEEPROM", guardar_conf); //Graba en la eeprom la configuracion
  
  server.begin();

  while (true) {
    server.handleClient();
  }
}

void guardar_conf() {
  Serial.println(server.arg("ssid"));
  grabar(0,server.arg("ssid"));
  Serial.println(server.arg("pass"));
  grabar(50,server.arg("pass"));
  Serial.println(server.arg("ip"));
  grabar(100,server.arg("ip"));
  mensaje = "Configuracion Guardada...";
}

//----------------Función para grabar en la EEPROM-------------------
void grabar(int addr, String a) {
  int tamano = a.length(); 
  char inchar[50]; 
  a.toCharArray(inchar, tamano+1);
  for (int i = 0; i < tamano; i++) {
    EEPROM.write(addr+i, inchar[i]);
  }
  for (int i = tamano; i < 50; i++) {
    EEPROM.write(addr+i, 255);
  }
  EEPROM.commit();
}

//-----------------Función para leer la EEPROM------------------------
String leer(int addr) {
   byte lectura;
   String strlectura;
   for (int i = addr; i < addr+50; i++) {
      lectura = EEPROM.read(i);
      if (lectura != 255) {
        strlectura += (char)lectura;
      }
   }
   return strlectura;
}

void setup() {
  Serial.begin(115200);
  //Serial.setDebugOutput(true);

  //Connect to Wi-Fi
  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, password);
  // Serial.print("Connecting to WiFi ..");
  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.print('.');
  //   delay(1000);
  // }

  EEPROM.begin(512);
  SPI.begin();
  u8g2.begin();

  leer(0).toCharArray(ssid, 50);
  leer(50).toCharArray(pass, 50);

  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 10, "WIFI: Conectando"); // write something to the internal memory
  u8g2.sendBuffer();

  setup_wifi();

  if (status_connect == 1) {
    modoconf();
  }else{
    leer(0).toCharArray(ssid, 50);
    leer(50).toCharArray(pass, 50);
    leer(100).toCharArray(ip,50);
  }

  // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));

  mfrc522.PCD_Init();

  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.print("Lector RFID conectado");

  u8g2.clear();

  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 10, "                 "); // write something to the internal memory
  u8g2.drawStr(0, 10, "WIFI: Conectado"); // write something to the internal memory
  u8g2.sendBuffer();

}

void loop() {

  // Serial.println("-- Acercar Pulsera --");
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  
  if (!mfrc522.PICC_IsNewCardPresent())
    return;
  if (!mfrc522.PICC_ReadCardSerial())
    return;
  
  // Obtengo el ID
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    ID+=String(mfrc522.uid.uidByte[i], HEX);             
  }

  String view_ID = "ID:" + String(ID);
  char array_id[view_ID.length() + 1];
  view_ID.toCharArray(array_id, view_ID.length() + 1);

  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 10,"RFID DETECTADO"); // write something to the internal memory
  u8g2.drawStr(0, 20,array_id); // write something to the internal memory
  u8g2.drawStr(0, 30,"- No la retires"); // write something to the internal memory
  u8g2.sendBuffer();

  Serial.println("ID TAG:");
  Serial.println(ID);
  Serial.println("\n");

  WiFiClient client;

  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    HTTPClient http;

    Serial.print("[HTTPS] begin...\n");
    String url="http://"+String(ip)+"/loxa/public/api/tags/verify?id=";
    url+=ID;

    if (http.begin(client, url)) {  // HTTPS

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }

      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

  String key_loxa="LoxaFidelis";

  for (int i = 0; i < key_loxa.length(); i++) {
    data[i] = key_loxa[i];
  }

  MFRC522::StatusCode status;
  byte trailerBlock = 7;
  byte sector = 1;
  byte blockAddr = 4;
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  // Write data to the block
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, (byte*)data, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  } else {
    u8g2.clearBuffer();
    ID = "";
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 10,"ID enviado");
    u8g2.sendBuffer();
  }

  Serial.println();
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1(); 
  delay(2000);

  ID="";
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 10,"Wifi:Conectado");
  u8g2.sendBuffer();

}
