#include <Thread.h>
#include <ESP8266.h>
#include <SoftwareSerial.h>
#include "DHT.h"

#define DHTTYPE DHT11

String server = "avitec-api.herokuapp.com";
String port = "80";
String uri = "/api/indicadores/raw";
//String server = "192.168.2.25";
//String port = "3333";
//String uri = "/tcc";

String temperatura1 = "";
String umidade1 = "";
String temperatura2 = "";
String umidade2 = "";
String temperatura3 = "";
String umidade3 = "";
String temperatura4 = "";
String umidade4 = "";
String temperatura5 = "";
String umidade5 = "";

boolean servidorConectado  = false;

SoftwareSerial mySerial(4, 2);
Thread threadEnviar = Thread();
DHT dht1(A0, DHTTYPE);
DHT dht2(A1, DHTTYPE);
DHT dht3(A2, DHTTYPE);
DHT dht4(A3, DHTTYPE);
DHT dht5(A4, DHTTYPE);

void setup()
{
  Serial.begin(9600);
  mySerial.begin(115200);
  dht1.begin();
  dht2.begin();
  dht3.begin();
  dht4.begin();
  dht5.begin();

  mySerial.print("AT+UART_CUR=9600,8,1,0,0\r\n");

  connectWifi();

  threadEnviar.onRun(enviaServidor);
  threadEnviar.setInterval(15000);
}

void loop(void) {
  if (threadEnviar.shouldRun())
    threadEnviar.run();
  //  enviaServidor();
  //  delay(10000);
}

bool containsNan(String temp, String umid) {
  return temp.equals(" NAN") || umid.equals(" NAN");
}

void connectWifi() {
  ESP8266 wifi(mySerial);
  wifi.setOprToStation();

  if (wifi.joinAP("Wifi-Indaia", "#indaia210#")) {
    Serial.println("Conectado no wifi");
    //      Serial.print("IP: ");
    //      Serial.println(wifi.getLocalIP().c_str());
    //      Serial.println("\n");
    conectaServidor();
  } else {
    Serial.println("Falha na conexão wifi");
    Serial.println("Reconectando...");
    delay(5000);
    connectWifi();
  }

  wifi.disableMUX();
}

void conectaServidor() {
  mySerial.println("AT+CIPSTART=\"TCP\",\"" + server + "\"," + port + "");

  if (mySerial.find("OK")) {
    servidorConectado = true;
    Serial.println("Conectado no servidor");
  }
}

void capturaDados() {
  Serial.println("Iniciando captura de dados");
  umidade1 = dht1.readHumidity();
  temperatura1 = dht1.readTemperature();
  umidade2 = dht2.readHumidity();
  temperatura2 = dht2.readTemperature();
  umidade3 = dht3.readHumidity();
  temperatura3 = dht3.readTemperature();
  umidade4 = dht4.readHumidity();
  temperatura4 = dht4.readTemperature();
  umidade5 = dht5.readHumidity();
  temperatura5 = dht5.readTemperature();
  Serial.println("Captura de dados finalizada");
}

void enviaServidor() {
  Serial.println("Iniciando envio de dados ao servidor");
  if (servidorConectado) {
    mySerial.println("AT+CIPSTART=\"TCP\",\"" + server + "\"," + port + "");

    if (mySerial.find("OK")) {
      Serial.println("Conectado servidor");
    }

    capturaDados();

    String text = "";
    if (containsNan(temperatura1, umidade1) == false) {
      text = text + "1," + temperatura1 + "," + umidade1 + ";";
    }

    text = text + "2," + temperatura2 + "," + umidade2 + ";3," + temperatura3 + "," + umidade3 + ";4," + temperatura4 + "," + umidade4 + ";5," + temperatura5 + "," + umidade5 + ";";

    Serial.println(text);

    String postRequest = "POST " + uri + " HTTP/1.1\r\n" +
                         "Host:" + server + "\r\n" +
                         "Accept: *" + "/" + "*\r\n" +
                         "Content-Length:" + text.length() + "\r\n" +
                         "Content-Type:text/plain\r\n" +
                         "\r\n" + text + "\r\n";

    //Serial.println(postRequest);
    Serial.println(postRequest.length());

    mySerial.print("AT+CIPSEND=");
    mySerial.println(postRequest.length());

    if (mySerial.find(">")) {
      Serial.println("Enviando..");
      mySerial.print(postRequest);

      if (mySerial.find("SEND OK")) {
        Serial.println("Enviado!");
        while (mySerial.available()) {
          String tmpResp = mySerial.readString();
          Serial.println(tmpResp);
        }

      }
    }

    mySerial.println("AT+CIPCLOSE");
  }
}
