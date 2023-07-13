
#include <WiFi.h>
#include "DHT.h"

#define DHTPIN1 33
#define DHTPIN2 5

#define DHTTYPE DHT11


const char* ssid     = "Termometro - ESP32";

WiFiServer server(80);
IPAddress local_IP(192,168,4,22);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

String header;
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
float tempMinima;
float ultimaTemp;

void setup() {
  tempMinima = NULL;
  ultimaTemp = NULL;
  dht1.begin();
  dht2.begin();
  pinMode(2, OUTPUT);
  Serial.begin(9600);
  
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();

  xTaskCreate(
    lerTemperatura,
    "Ler Temperatura",
    1000,
    NULL,
    1,
    NULL
    );
    
}

void loop(){
    WiFiClient client = server.available();   // Listen for incoming clients
    
    if (client) {                             
      String currentLine = "";                
      while (client.connected()) {            
        if (client.available()) {             
          char c = client.read();             
          header += c;
          if (c == '\n') {               
            if (currentLine.length() == 0) {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();  
              client.println("<!DOCTYPE html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<meta charset=\"utf-8\"><title>Termometro</title>");
              client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
              client.println("button { background-color: #4CAF50; border: none; color: white; padding: 32px 80px;}</style>");
              String linhaTemp = "<body><div><h1>Temperatura: ";
              linhaTemp = linhaTemp + ultimaTemp;
              linhaTemp = linhaTemp + "</h1>"; 
              client.println(linhaTemp);
              String linhaTempMinima = "<h1>Temperatura Minima: ";
              linhaTempMinima = linhaTempMinima + tempMinima;
              linhaTempMinima = linhaTempMinima + "</h1>"; 
              client.println(linhaTempMinima);
              client.println("</div></body></html>");
              client.println();
              // Break out of the while loop
              break;
            } else { // if you got a newline, then clear currentLine
              currentLine = "";
            }
          } else if (c != '\r') {  // if you got anything else but a carriage return character,
            currentLine += c;      // add it to the end of the currentLine
          }
        }
      }
      // Clear the header variable
      header = "";
      // Close the connection
      client.stop();
    }
}

void lerTemperatura(void * parameter){
  for(;;){
    digitalWrite(2, HIGH);
    float tempMedia;
    float temp1 = dht1.readTemperature();
    float temp2 = dht2.readTemperature();
    Serial.print("Temp 1: ");
    Serial.print(temp1);
    Serial.print(" Temp 2: ");
    Serial.print(temp2);
    if(isnan(temp1) and isnan(temp2)){
      tempMedia = ultimaTemp;
    }else if(isnan(temp1)){
      tempMedia = temp2;
    }else if(isnan(temp2)){
      tempMedia = temp1;
    }else{
      tempMedia = (temp1 + temp2)/2;
    }
    Serial.print(" TempMedia: ");
    Serial.print(tempMedia);
    Serial.print(" TempMinima: ");
    Serial.println(tempMinima);
    if(tempMinima == NULL){
      tempMinima = tempMedia;
    }
    if(tempMedia < tempMinima){
      tempMinima = tempMedia;
    }
    ultimaTemp = tempMedia;
    digitalWrite(2, LOW);
    vTaskDelay(60000 / portTICK_PERIOD_MS);
  };
}
