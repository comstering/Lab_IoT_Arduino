/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "LEA_core.h"
#include "LEA_core.c"

#ifndef STASSID
#define STASSID "SecurityLab(2.4)"    //  와이파이 이름
#define STAPSK  "security915"    //  와이파이 비밀번호
#endif

/*초음파 센서*/
int trigPin = D5;
int echoPin = D6;

/*와이파이*/
const char* ssid     = STASSID;
const char* password = STAPSK;

const char* host = "210.125.212.191";    //  공용PC IP
const uint16_t port = 8888;    //  웹서버 포트

/*LEA 암호*/
BYTE pbUserKey[16] = { "security915!@#" };

int doorstatic = 0;

void setup() {
  Serial.begin(115200);

  /*초음파 센서 setup*/
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  /*WiFi setup*/
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);    //  와이파이 설정

  while (WiFi.status() != WL_CONNECTED) {    //  와이파이 연결 될때까지 반복
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  /*초음파 센서*/
  float duration, cm;
  long inches;

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);

  //convert the time into a distance
  cm = microsecondsToCentimeters(duration);
  Serial.print(cm);
  Serial.println(" cm");
  
  /*와이파이*/
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;    // 와이파이 통신
  if (!client.connect(host, port)) {    //  ip와 포트로 연결이 안될경우
    Serial.println("connection failed");
    delay(5000);
    return;
  }
  
  /*Lea 암호*/
  BYTE pbData[16] = { "security" };
  String check = LEA_Encrypto(pbData);

  String url = "/IoT/DoorStatusUpdate.jsp?check=";    //  DB 통신할 문장
  url += check;    //  check 값
  url += "&door=";
  String dv = doorValue(cm);
  if(dv.equals("same")) {
    delay(1000);
    return;
  }
  url += dv;    //  open/close 값

  /*URL 연결*/
  // This will send a string to the server
  Serial.println("sending data to server");
  client.print(String("POST ") + url + " HTTP/1.1\r\n" + "HOST: " + host + "\r\n" + "Connection: close\r\n\r\n");    //    웹서버에 있는 DB 통신 url과 연결

  // wait for data to be available
  unsigned long timeout = millis();
  while (client.available() == 0) {    //  웹서버와 통신 응답대기
    if (millis() - timeout > 5000) {    //  응답 시간이 너무 길어질 경우 timeout
      Serial.println(">>> Client Timeout !");
      client.stop();
      delay(60000);
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("receiving from remote server");
  // not testing 'client.connected()' since we do not need to send data here
  int count = 1;
  String result;
  while (client.available()) {    //  응답후 결과 값 받아오기
    String line = client.readStringUntil('\r');

    count++;
    if(count==11)
      Serial.println("result: " + line);
  }

  // Close the connection
  Serial.println();
  Serial.println("closing connection");
  client.stop();

  delay(1000); // execute once every 5 minutes, don't flood remote service
}

/*LEA 암호화*/
String LEA_Encrypto(BYTE data[16]) {
  BYTE pdwRoundKey[384] = { 0x0, };

  LEA_Key(pbUserKey, pdwRoundKey);
  LEA_Enc(pdwRoundKey, data);

  String encData = "";
  char str[3];
  for(int i = 0; i < 16; i++) {
    /*16진수 문자열로 변환*/
    sprintf(str, "%02x", data[i]);
    encData += str;
  }
  Serial.println(encData);
  return encData;
}

/*cm로 변경*/
float microsecondsToCentimeters(float microseconds) {
  return microseconds / 29 / 2;
}

/*door open/close 암호화*/
String doorValue(float cm) {
  if(cm < 5.5) {    //  문이 닫혀 있을 경우
    int dv = 0;
    if(doorstatic == dv) {
      return "same";
    }
    doorstatic = 0;
    BYTE value[16] = { "0" };
    int dc = 0;
    return LEA_Encrypto(value);
  }
  else {    //  문이 열려 있을 경우
    int dv = 1;
    if(doorstatic == dv) {
      return "same";
    }
    doorstatic = 1;
    BYTE value[16] = { "1" };
    return LEA_Encrypto(value);
  }
}
